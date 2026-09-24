#include "media_service.h"
#include "diagnostics.h"
#include "media_backend.h"
#include "esp_timer.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {uint32_t version;char endpoint[192],entity[96],content[384],content_type[48];} preferences_t;
typedef struct {unsigned kind;uint32_t tag;media_action_t action;preferences_t prefs;} command_t;
static preferences_t prefs;
static media_snapshot_t state;
static SemaphoreHandle_t lock;
static QueueHandle_t queue;
static nvs_handle_t storage;
static bool opened;
static atomic_bool accepting;
static int64_t observed,next_poll,deadline;
static media_state_t expected;
static bool selection_pending;
static void take(void){if(lock)xSemaphoreTake(lock,portMAX_DELAY);}
static void give(void){if(lock)xSemaphoreGive(lock);}
static void message(const char *s){take();snprintf(state.status,sizeof(state.status),"%s",s);give();}
static bool valid(const preferences_t *p)
{return p->version==2&&memchr(p->content,0,sizeof(p->content))&&memchr(p->content_type,0,sizeof(p->content_type))&&media_selection_valid(p->content,p->content_type)&&memchr(p->entity,0,sizeof(p->entity))&&memchr(p->endpoint,0,sizeof(p->endpoint))&&media_backend_target_valid(p->entity)&&media_backend_identity_valid(p->endpoint);}
void media_service_init(void)
{
    lock=xSemaphoreCreateMutex();queue=xQueueCreate(1,sizeof(command_t));accepting=lock&&queue;
    if(nvs_open_from_partition("clockcfg","media",NVS_READWRITE,&storage)==ESP_OK){
        opened=true;size_t n=sizeof(prefs);
        esp_err_t err=nvs_get_blob(storage,"player",&prefs,&n);
        if(err==ESP_OK&&n==offsetof(preferences_t,content)&&prefs.version==1){prefs.version=2;n=sizeof(prefs);}
        if(err!=ESP_OK||n!=sizeof(prefs)||!valid(&prefs))memset(&prefs,0,sizeof(prefs));
    }
    state.configured=valid(&prefs);if(state.configured){strcpy(state.entity,prefs.entity);strcpy(state.content,prefs.content);strcpy(state.content_type,prefs.content_type);}
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
{return media_service_configure_tagged(entity,0);}
static bool configure(const char *entity,const char *content,const char *type,uint32_t tag)
{
    if(!media_backend_target_valid(entity)||!media_selection_valid(content,type))return false;
    media_backend_config_t h;media_backend_config(&h);if(!h.configured)return false;
    command_t c={.kind=1,.tag=tag,.prefs={.version=2}};strcpy(c.prefs.endpoint,h.identity);strcpy(c.prefs.entity,entity);
    strcpy(c.prefs.content,content);strcpy(c.prefs.content_type,type);return submit(&c);
}
bool media_service_select(const char *entity,const char *content,const char *type)
{return configure(entity,content,type,0);}
bool media_service_configure_tagged(const char *entity,uint32_t tag)
{
    media_snapshot_t current;media_service_snapshot(&current);
    bool same=entity&&!strcmp(entity,current.entity);
    return configure(entity,same?current.content:"",same?current.content_type:"",tag);
}
bool media_service_refresh(void){command_t c={.kind=3};return submit(&c);}
bool media_service_action(media_action_t action)
{
    media_snapshot_t s;media_service_snapshot(&s);if(!s.fresh||!media_action_supported(&s.player,action)||(action==MEDIA_START_SAVED&&!s.content[0]))return false;
    command_t c={.kind=2,.action=action};strcpy(c.prefs.entity,s.entity);
    media_backend_config_t h;media_backend_config(&h);strcpy(c.prefs.endpoint,h.identity);return submit(&c);
}
static void unavailable(int code)
{take();state.fresh=false;give();message(code==MEDIA_BACKEND_DENIED?"Player access denied":code==MEDIA_BACKEND_NOT_FOUND?"Player entity not found":"Player unavailable; local alarms still work");}
void media_service_poll(bool online)
{
    if(!accepting)return;
    command_t c={0};bool received=xQueueReceive(queue,&c,0)==pdTRUE;
    media_backend_config_t h;media_backend_config(&h);
    if(received){
        take();state.busy=false;give();next_poll=0;
        if(c.kind==1){
            esp_err_t err=ESP_ERR_INVALID_ARG;
            if(opened&&h.configured&&!strcmp(h.identity,c.prefs.endpoint)&&valid(&c.prefs)){
                err=nvs_set_blob(storage,"player",&c.prefs,sizeof(c.prefs));if(err==ESP_OK)err=nvs_commit(storage);
            }
            if(c.tag)diagnostics_printf("SETUP_MEDIA tag=%lu saved=%u\n",(unsigned long)c.tag,err==ESP_OK);
            if(err!=ESP_OK){message("Player not saved; check connection and storage");next_poll=esp_timer_get_time()+10000000;return;}
            prefs=c.prefs;expected=MEDIA_UNKNOWN;deadline=0;selection_pending=false;
            take();state.configured=true;state.fresh=false;memset(&state.player,0,sizeof(state.player));strcpy(state.entity,prefs.entity);strcpy(state.content,prefs.content);strcpy(state.content_type,prefs.content_type);give();
        }
    }
    if(!valid(&prefs))return;
    if(!h.configured||strcmp(prefs.endpoint,h.identity)){unavailable(-1);message("Set up this player for the current connection");expected=MEDIA_UNKNOWN;return;}
    if(!online){unavailable(-1);expected=MEDIA_UNKNOWN;return;}
    int64_t now=esp_timer_get_time();if(now<next_poll)return;
    take();state.busy=true;give();
    if(received&&c.kind==2){
        media_snapshot_t current;media_service_snapshot(&current);
        if(strcmp(c.prefs.endpoint,prefs.endpoint)||strcmp(c.prefs.entity,prefs.entity)||!current.fresh||!media_action_supported(&current.player,c.action)||(c.action==MEDIA_START_SAVED&&!prefs.content[0])){
            message("Player changed or state expired; refresh first");goto done;
        }
        int code=media_backend_action(prefs.entity,c.action,&current.player,prefs.content,prefs.content_type);
        if(code!=MEDIA_BACKEND_OK){unavailable(code);expected=MEDIA_UNKNOWN;goto done;}
        selection_pending=c.action==MEDIA_START_SAVED;
        expected=(c.action==MEDIA_PLAY||selection_pending)?MEDIA_PLAYING:c.action==MEDIA_PAUSE?MEDIA_PAUSED:MEDIA_UNKNOWN;
        deadline=esp_timer_get_time()+10000000;message("Request accepted; reading actual player state");
    }
    media_player_t player;int code=media_backend_read(prefs.entity,&player);
    if(code!=MEDIA_BACKEND_OK){unavailable(code);goto done;}
    take();state.player=player;state.fresh=true;observed=esp_timer_get_time();give();
    if(expected!=MEDIA_UNKNOWN){
        if(player.state==expected){expected=MEDIA_UNKNOWN;message(selection_pending?"Player playing; selection not verified":"Playback state confirmed");selection_pending=false;}
        else message("Playback request not yet confirmed");
    }else message("Showing reported player state");
 done:
    if(expected!=MEDIA_UNKNOWN&&esp_timer_get_time()>=deadline){expected=MEDIA_UNKNOWN;message("Playback change not confirmed");}
    next_poll=esp_timer_get_time()+(expected!=MEDIA_UNKNOWN?1000000:10000000);
    take();state.busy=false;give();
}
