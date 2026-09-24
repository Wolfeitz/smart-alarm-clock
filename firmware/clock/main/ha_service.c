#include "ha_service.h"
#include "network_http.h"
#include "esp_timer.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct {uint32_t version;char endpoint[192],entity[96],token[512];} config_t;
typedef struct {unsigned kind;config_t config;ha_light_state_t desired;} command_t;
static config_t config;
static ha_snapshot_t state;
static SemaphoreHandle_t lock;
static QueueHandle_t queue;
static nvs_handle_t storage;
static bool opened;
static int64_t observed,next_poll,confirm_until;
static ha_light_state_t desired;
static void message(const char *text)
{xSemaphoreTake(lock,portMAX_DELAY);snprintf(state.status,sizeof(state.status),"%s",text);xSemaphoreGive(lock);}
static bool config_valid(const config_t *c)
{
    return c->version==1&&memchr(c->endpoint,0,sizeof(c->endpoint))&&memchr(c->entity,0,sizeof(c->entity))&&memchr(c->token,0,sizeof(c->token))&&
        ha_endpoint_valid(c->endpoint)&&ha_entity_valid(c->entity)&&ha_token_valid(c->token);
}
void ha_service_init(void)
{
    lock=xSemaphoreCreateMutex();queue=xQueueCreate(1,sizeof(command_t));if(!lock||!queue)abort();
    if(nvs_open_from_partition("clockcfg","ha_private",NVS_READWRITE,&storage)==ESP_OK){
        opened=true;size_t n=sizeof(config);if(nvs_get_blob(storage,"config",&config,&n)!=ESP_OK||n!=sizeof(config)||!config_valid(&config))memset(&config,0,sizeof(config));
    }
    state.configured=config_valid(&config);
    if(state.configured){strcpy(state.endpoint,config.endpoint);strcpy(state.entity,config.entity);}
    else strcpy(state.endpoint,"http://192.168.1.232:8123");
    message(state.configured?"Waiting for connection":"Set up Home Assistant");
}
void ha_service_snapshot(ha_snapshot_t *out)
{
    xSemaphoreTake(lock,portMAX_DELAY);*out=state;
    out->fresh=state.fresh&&esp_timer_get_time()-observed<30000000;xSemaphoreGive(lock);
}
static bool submit(command_t *c)
{
    xSemaphoreTake(lock,portMAX_DELAY);if(state.busy){xSemaphoreGive(lock);return false;}state.busy=true;xSemaphoreGive(lock);
    bool ok=xQueueSend(queue,c,0)==pdTRUE;
    if(!ok){xSemaphoreTake(lock,portMAX_DELAY);state.busy=false;xSemaphoreGive(lock);}return ok;
}
bool ha_service_configure(const char *endpoint,const char *token,const char *entity)
{
    if(!endpoint||!token||!entity||strlen(endpoint)>=192||strlen(entity)>=96||strlen(token)>=512)return false;
    command_t c={.kind=1,.config={.version=1}};strcpy(c.config.endpoint,endpoint);
    size_t n=strlen(c.config.endpoint);if(n&&c.config.endpoint[n-1]=='/')c.config.endpoint[n-1]=0;
    strcpy(c.config.entity,entity);strcpy(c.config.token,token);
    if(!ha_endpoint_valid(c.config.endpoint)||!ha_entity_valid(entity)||(*token&&!ha_token_valid(token))){memset(&c,0,sizeof(c));return false;}
    /* Empty token is resolved by owner only when endpoint is unchanged. */
    bool ok=submit(&c);memset(&c,0,sizeof(c));return ok;
}
bool ha_service_refresh(void){command_t c={.kind=3};return submit(&c);}
bool ha_service_toggle(void)
{
    ha_snapshot_t s;ha_service_snapshot(&s);
    if(!s.configured||!s.fresh||(s.light.state!=HA_ON&&s.light.state!=HA_OFF))return false;
    command_t c={.kind=2,.desired=s.light.state==HA_ON?HA_OFF:HA_ON};return submit(&c);
}
static void set_busy(bool busy)
{xSemaphoreTake(lock,portMAX_DELAY);state.busy=busy;xSemaphoreGive(lock);}
static void failure(int code)
{
    xSemaphoreTake(lock,portMAX_DELAY);state.fresh=false;xSemaphoreGive(lock);
    message(code==401||code==403?"Access denied - check token":code==404?"Light entity not found":"Home Assistant unavailable; clock works offline");
}
void ha_service_poll(bool online)
{
    command_t c={0};int64_t now=esp_timer_get_time();bool toggle=false,config_failed=false;
    if(xQueueReceive(queue,&c,0)==pdTRUE){
        if(c.kind==1){
            if(!c.config.token[0]&&!strcmp(c.config.endpoint,config.endpoint))strcpy(c.config.token,config.token);
            esp_err_t err=ESP_ERR_INVALID_ARG;
            if(opened&&config_valid(&c.config)){err=nvs_set_blob(storage,"config",&c.config,sizeof(c.config));if(err==ESP_OK)err=nvs_commit(storage);}
            if(err==ESP_OK){config=c.config;desired=HA_UNKNOWN;confirm_until=0;
                xSemaphoreTake(lock,portMAX_DELAY);state.configured=true;state.fresh=false;state.light=(ha_light_t){0};
                strcpy(state.endpoint,config.endpoint);strcpy(state.entity,config.entity);xSemaphoreGive(lock);message("Saved; reading light state...");
            }else{config_failed=true;message("Setup not saved - check URL, token and light");}
        }else if(c.kind==2){toggle=true;desired=c.desired;}
        next_poll=0;memset(&c,0,sizeof(c));
        xSemaphoreTake(lock,portMAX_DELAY);state.busy=false;xSemaphoreGive(lock);
    }
    if(config_failed||!config_valid(&config))return;
    if(!online){failure(-1);desired=HA_UNKNOWN;confirm_until=0;return;}
    if(!toggle&&now<next_poll)return;
    set_busy(true);
    char *response=malloc(12289);if(!response){failure(-1);desired=HA_UNKNOWN;confirm_until=0;set_busy(false);next_poll=now+10000000;return;}
    char url[336],body[144];size_t size;
    if(toggle){
        snprintf(url,sizeof(url),"%s/api/services/light/%s",config.endpoint,desired==HA_ON?"turn_on":"turn_off");
        snprintf(body,sizeof(body),"{\"entity_id\":\"%s\"}",config.entity);
        int code=network_http_request(url,config.token,body,response,12289,&size);
        if(code!=200){failure(code);desired=HA_UNKNOWN;goto done;}
        confirm_until=esp_timer_get_time()+10000000;message("Command sent; checking actual light state...");
    }
    snprintf(url,sizeof(url),"%s/api/states/%s",config.endpoint,config.entity);
    int code=network_http_request(url,config.token,NULL,response,12289,&size);ha_light_t light;
    if(code!=200){failure(code);goto done;}
    if(!ha_parse_light(response,size,config.entity,&light)){failure(-1);goto done;}
    xSemaphoreTake(lock,portMAX_DELAY);state.light=light;observed=esp_timer_get_time();state.fresh=true;xSemaphoreGive(lock);
    if(desired!=HA_UNKNOWN){
        if(light.state==desired){desired=HA_UNKNOWN;confirm_until=0;message("Light state confirmed");}
        else if(esp_timer_get_time()>=confirm_until){desired=HA_UNKNOWN;confirm_until=0;message("Change not confirmed by Home Assistant");}
    }else message(light.state==HA_UNAVAILABLE?"Light is unavailable":light.state==HA_UNKNOWN?"Light state unknown":"Light state updated");
 done:
    if(desired!=HA_UNKNOWN&&esp_timer_get_time()>=confirm_until){
        desired=HA_UNKNOWN;confirm_until=0;message("Change not confirmed by Home Assistant");
    }
    set_busy(false);free(response);next_poll=esp_timer_get_time()+(desired!=HA_UNKNOWN?1000000:10000000);
}
