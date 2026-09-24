#include "media_service.h"
#include "ha_service.h"
#include "esp_timer.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {uint32_t version;char endpoint[192],entity[96];} preferences_t;
typedef struct {unsigned kind;media_action_t action;preferences_t prefs;} command_t;
static preferences_t prefs;
static media_snapshot_t state;
static SemaphoreHandle_t lock;
static QueueHandle_t queue;
static nvs_handle_t storage;
static bool opened;
static atomic_bool accepting;
static int64_t observed,next_poll,deadline;
static media_state_t expected;
static void take(void){if(lock)xSemaphoreTake(lock,portMAX_DELAY);}
static void give(void){if(lock)xSemaphoreGive(lock);}
static void message(const char *s){take();snprintf(state.status,sizeof(state.status),"%s",s);give();}
static bool valid(const preferences_t *p)
{return p->version==1&&memchr(p->entity,0,sizeof(p->entity))&&memchr(p->endpoint,0,sizeof(p->endpoint))&&media_entity_valid(p->entity)&&ha_endpoint_valid(p->endpoint);}
void media_service_init(void)
{
    lock=xSemaphoreCreateMutex();queue=xQueueCreate(1,sizeof(command_t));accepting=lock&&queue;
    if(nvs_open_from_partition("clockcfg","media",NVS_READWRITE,&storage)==ESP_OK){
        opened=true;size_t n=sizeof(prefs);
        if(nvs_get_blob(storage,"player",&prefs,&n)!=ESP_OK||n!=sizeof(prefs)||!valid(&prefs))memset(&prefs,0,sizeof(prefs));
    }
    state.configured=valid(&prefs);if(state.configured)strcpy(state.entity,prefs.entity);
    message(!accepting?"Media resources unavailable":state.configured?"Waiting for player":"Choose a player in Setup");
}
void media_service_disable(void)
{accepting=false;take();state.busy=false;state.fresh=false;give();message("Network unavailable; local alarms still work");}
void media_service_snapshot(media_snapshot_t *out)
{take();*out=state;out->fresh=state.fresh&&esp_timer_get_time()-observed<30000000;give();}
static bool submit(command_t *c)
{
    if(!accepting)return false;
    take();
    if(!accepting||state.busy){give();return false;}
    bool ok=xQueueSend(queue,c,0)==pdTRUE;state.busy=ok;give();return ok;
}
bool media_service_configure(const char *entity)
{
    if(!media_entity_valid(entity))return false;
    ha_snapshot_t h;ha_service_snapshot(&h);if(!h.configured)return false;
    command_t c={.kind=1,.prefs={.version=1}};strcpy(c.prefs.endpoint,h.endpoint);strcpy(c.prefs.entity,entity);return submit(&c);
}
bool media_service_refresh(void){command_t c={.kind=3};return submit(&c);}
bool media_service_action(media_action_t action)
{
    media_snapshot_t s;media_service_snapshot(&s);if(!s.fresh||!media_action_supported(&s.player,action))return false;
    command_t c={.kind=2,.action=action};strcpy(c.prefs.entity,s.entity);
    ha_snapshot_t h;ha_service_snapshot(&h);strcpy(c.prefs.endpoint,h.endpoint);return submit(&c);
}
static void unavailable(int code)
{take();state.fresh=false;give();message(code==401||code==403?"Home Assistant access denied":code==404?"Player entity not found":"Player unavailable; local alarms still work");}
void media_service_poll(bool online)
{
    if(!accepting)return;
    command_t c={0};bool received=xQueueReceive(queue,&c,0)==pdTRUE;
    ha_snapshot_t h;ha_service_snapshot(&h);
    if(received){
        take();state.busy=false;give();next_poll=0;
        if(c.kind==1){
            esp_err_t err=ESP_ERR_INVALID_ARG;
            if(opened&&h.configured&&!strcmp(h.endpoint,c.prefs.endpoint)&&valid(&c.prefs)){
                err=nvs_set_blob(storage,"player",&c.prefs,sizeof(c.prefs));if(err==ESP_OK)err=nvs_commit(storage);
            }
            if(err!=ESP_OK){message("Player not saved; check HA setup and storage");next_poll=esp_timer_get_time()+10000000;return;}
            prefs=c.prefs;expected=MEDIA_UNKNOWN;deadline=0;
            take();state.configured=true;state.fresh=false;memset(&state.player,0,sizeof(state.player));strcpy(state.entity,prefs.entity);give();
        }
    }
    if(!valid(&prefs))return;
    if(!h.configured||strcmp(prefs.endpoint,h.endpoint)){unavailable(-1);message("Set up this player for the current HA server");expected=MEDIA_UNKNOWN;return;}
    if(!online){unavailable(-1);expected=MEDIA_UNKNOWN;return;}
    int64_t now=esp_timer_get_time();if(now<next_poll)return;
    take();state.busy=true;give();
    char *response=malloc(12289);if(!response){unavailable(-1);goto done;}
    char path[164],body[192];size_t size=0;
    if(received&&c.kind==2){
        media_snapshot_t current;media_service_snapshot(&current);
        if(strcmp(c.prefs.endpoint,prefs.endpoint)||strcmp(c.prefs.entity,prefs.entity)||!current.fresh||!media_action_supported(&current.player,c.action)){
            message("Player changed or state expired; refresh first");goto done;
        }
        const char *actions[]={"media_previous_track","media_play","media_pause","media_next_track","volume_down","volume_up"};
        const char *action=actions[c.action];
        snprintf(body,sizeof(body),"{\"entity_id\":\"%s\"}",prefs.entity);
        if(c.action>=MEDIA_QUIETER&&!(current.player.features&1024)){
            action="volume_set";double v=current.player.volume+(c.action==MEDIA_LOUDER?.05:-.05);if(v<0)v=0;if(v>1)v=1;
            snprintf(body,sizeof(body),"{\"entity_id\":\"%s\",\"volume_level\":%.3f}",prefs.entity,v);
        }
        snprintf(path,sizeof(path),"/api/services/media_player/%s",action);
        int code=ha_service_request(path,body,response,12289,&size);
        if(code!=200){unavailable(code);expected=MEDIA_UNKNOWN;goto done;}
        expected=c.action==MEDIA_PLAY?MEDIA_PLAYING:c.action==MEDIA_PAUSE?MEDIA_PAUSED:MEDIA_UNKNOWN;
        deadline=esp_timer_get_time()+10000000;message("Request accepted; reading actual player state");
    }
    snprintf(path,sizeof(path),"/api/states/%s",prefs.entity);
    int code=ha_service_request(path,NULL,response,12289,&size);media_player_t player;
    if(code!=200){unavailable(code);goto done;}
    if(!media_parse(response,size,prefs.entity,&player)){unavailable(-1);goto done;}
    take();state.player=player;state.fresh=true;observed=esp_timer_get_time();give();
    if(expected!=MEDIA_UNKNOWN){
        if(player.state==expected){expected=MEDIA_UNKNOWN;message("Playback state confirmed");}
        else message("Playback request not yet confirmed");
    }else message("Showing reported player state");
 done:
    if(expected!=MEDIA_UNKNOWN&&esp_timer_get_time()>=deadline){expected=MEDIA_UNKNOWN;message("Playback change not confirmed");}
    next_poll=esp_timer_get_time()+(expected!=MEDIA_UNKNOWN?1000000:10000000);
    free(response);take();state.busy=false;give();
}
