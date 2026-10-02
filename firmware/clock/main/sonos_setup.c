#include "sonos_setup.h"
#include "sonos_network.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#endif
typedef struct {char endpoint[48],uuid[48];unsigned start;} request_t;
static SemaphoreHandle_t lock;
static QueueHandle_t queue;
static atomic_bool enabled;
static sonos_setup_snapshot_t state;
static void take(void){if(lock)xSemaphoreTake(lock,portMAX_DELAY);}
static void give(void){if(lock)xSemaphoreGive(lock);}
void sonos_setup_init(void)
{
    lock=xSemaphoreCreateMutex();queue=xQueueCreate(1,sizeof(request_t));enabled=lock&&queue;
    snprintf(state.status,sizeof(state.status),"%s",enabled?"Enter the speaker IP from the Sonos app":"Network resources unavailable");
}
void sonos_setup_disable(void)
{enabled=false;take();state.busy=false;state.ready=false;snprintf(state.status,sizeof(state.status),"Network worker unavailable");give();}
void sonos_setup_snapshot(sonos_setup_snapshot_t *out){take();*out=state;give();}
static bool submit(request_t *r)
{
    if(!enabled)return false;
    take();bool ok=!state.busy&&xQueueSend(queue,r,0)==pdTRUE;
    if(ok){state.busy=true;state.ready=false;state.revision++;snprintf(state.status,sizeof(state.status),"Finding speaker and favorites...");}
    give();return ok;
}
bool sonos_setup_lookup(const char *address)
{
    if(!address||strlen(address)>47)return false;
    request_t r={0};int n=snprintf(r.endpoint,sizeof(r.endpoint),"%s%s",address,strchr(address,':')?"":":1400");
    return n>0&&(size_t)n<sizeof(r.endpoint)&&sonos_endpoint_valid(r.endpoint)&&submit(&r);
}
bool sonos_setup_page(unsigned start)
{
    sonos_device_t d;
    take();bool valid=state.ready&&start<state.favorites.total&&sonos_target_parse(state.target,&d);give();
    if(!valid)return false;
    request_t r={.start=start};strcpy(r.endpoint,d.endpoint);strcpy(r.uuid,d.uuid);return submit(&r);
}
void sonos_setup_poll(bool online)
{
    if(!enabled)return;
    request_t r;if(xQueueReceive(queue,&r,0)!=pdTRUE)return;
    int result=SONOS_ERROR;sonos_device_t d={0};sonos_favorites_t page={0};
    sonos_client_t *c=NULL;
    if(online){
#ifdef ESP_PLATFORM
        c=heap_caps_calloc(1,sizeof(*c),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
#else
        c=calloc(1,sizeof(*c));
#endif
        if(c){
            sonos_network_t network;sonos_network_begin(c,&network,NULL,NULL);
            result=sonos_probe(c,r.endpoint,&d);
            if(result==SONOS_OK&&r.uuid[0]&&strcmp(r.uuid,d.uuid))result=SONOS_IDENTITY_CHANGED;
            if(result==SONOS_OK)result=sonos_favorites(c,&d,r.start,&page);
            free(c);
        }
    }
    take();state.busy=false;state.ready=result==SONOS_OK;state.revision++;
    if(state.ready){
        sonos_target_format(&d,state.target);strcpy(state.name,d.name);state.favorites=page;
        snprintf(state.status,sizeof(state.status),"%s",page.total?"Choose a Sonos favorite or controls only":"No favorites; add them in the Sonos app");
    }else snprintf(state.status,sizeof(state.status),"%s",!online?"Wi-Fi offline; saved player unchanged":result==SONOS_GROUP_UNSUPPORTED?"Choose an ungrouped speaker":result==SONOS_IDENTITY_CHANGED?"Speaker identity changed; look up again":!c?"Speaker lookup needs more memory":"Speaker unavailable or favorites invalid");
    give();
}
