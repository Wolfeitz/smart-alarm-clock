/* Link the production media owner without HA, HTTP, or JSON components. */
#include "media_service.h"
#include "media_backend.h"
#include "nvs.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static char queue[2048];static size_t queue_size;static bool queued,playing;
static unsigned actions,reads;static int64_t now;
int64_t esp_timer_get_time(void){return now;}
QueueHandle_t xQueueCreate(unsigned count,size_t size){assert(count==1&&size<sizeof(queue));queue_size=size;return queue;}
int xQueueSend(QueueHandle_t q,const void *v,unsigned t){(void)q;(void)t;if(queued)return 0;memcpy(queue,v,queue_size);queued=true;return 1;}
int xQueueReceive(QueueHandle_t q,void *v,unsigned t){(void)q;(void)t;if(!queued)return 0;memcpy(v,queue,queue_size);queued=false;return 1;}
SemaphoreHandle_t xSemaphoreCreateMutex(void){return (void*)1;}
int xSemaphoreTake(SemaphoreHandle_t s,unsigned t){(void)s;(void)t;return 1;}
int xSemaphoreGive(SemaphoreHandle_t s){(void)s;return 1;}
esp_err_t nvs_open_from_partition(const char *p,const char *ns,int mode,nvs_handle_t *h)
{assert(!strcmp(p,"clockcfg")&&!strcmp(ns,"media"));(void)mode;*h=1;return 0;}
esp_err_t nvs_get_blob(nvs_handle_t h,const char *k,void *out,size_t *size){(void)h;(void)k;(void)out;(void)size;return 1;}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *k,const void *v,size_t size){(void)h;(void)v;assert(!strcmp(k,"player")&&size>0);return 0;}
esp_err_t nvs_commit(nvs_handle_t h){(void)h;return 0;}
void diagnostics_printf(const char *format,...){(void)format;}
void media_backend_config_for(const char *target,media_backend_config_t *out){(void)target;*out=(media_backend_config_t){.configured=true,.identity="test:local"};}
bool media_backend_identity_valid(const char *s){return s&&!strcmp(s,"test:local");}
bool media_backend_target_valid(const char *s){return s&&!strcmp(s,"speaker/bedroom");}
int media_backend_read(const char *target,media_player_t *out)
{assert(media_backend_target_valid(target));reads++;*out=(media_player_t){.state=playing?MEDIA_PLAYING:MEDIA_PAUSED,.capabilities=(1u<<MEDIA_PLAY)|(1u<<MEDIA_PAUSE)};return MEDIA_BACKEND_OK;}
int media_backend_action(const char *target,media_action_t action,const media_player_t *p,const char *id,const char *type)
{assert(media_backend_target_valid(target)&&media_action_supported(p,action));assert(!*id&&!*type);actions++;playing=action==MEDIA_PLAY;return MEDIA_BACKEND_OK;}
int main(void)
{
    media_service_init();assert(media_service_configure("speaker/bedroom"));media_service_poll(true);
    media_snapshot_t s;media_service_snapshot(&s);assert(s.configured&&s.fresh&&reads==1);
    assert(media_service_action(MEDIA_PLAY));media_service_poll(true);media_service_snapshot(&s);
    assert(actions==1&&s.player.state==MEDIA_PLAYING&&strstr(s.status,"confirmed"));
    assert(media_service_action(MEDIA_PAUSE));media_service_poll(true);assert(actions==2&&!playing);
    unsigned before=reads;media_service_poll(false);media_service_snapshot(&s);assert(!s.fresh&&reads==before);
    assert(!media_service_action(MEDIA_PLAY));
    puts("PASS generic media owner with alternate identity/target and no HA/HTTP/JSON linkage");
}
