/* Exercise the production owner with deterministic network/NVS boundaries. */
#include "ha_service.h"
#include "media_service.h"
#include "network_http.h"
#include "nvs.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int64_t now;
static char diagnostic[128],last_body[1152],last_path[164];
static unsigned requests,posts;
static int code=200,storage_error;
static const char *remote="paused";
static bool apply=true,queued,fail_mutex,fail_queue;
static size_t queue_size,stored_size;
static char queue_data[2048],stored[2048];
int64_t esp_timer_get_time(void){return now;}
QueueHandle_t xQueueCreate(unsigned count,size_t size){assert(count==1&&size<sizeof(queue_data));queue_size=size;return fail_queue?NULL:queue_data;}
int xQueueSend(QueueHandle_t q,const void *value,unsigned timeout){(void)q;(void)timeout;if(queued)return 0;memcpy(queue_data,value,queue_size);queued=true;return 1;}
int xQueueReceive(QueueHandle_t q,void *value,unsigned timeout){(void)q;(void)timeout;if(!queued)return 0;memcpy(value,queue_data,queue_size);queued=false;return 1;}
SemaphoreHandle_t xSemaphoreCreateMutex(void){return fail_mutex?NULL:(void *)1;}
int xSemaphoreTake(SemaphoreHandle_t s,unsigned timeout){(void)s;(void)timeout;return 1;}
int xSemaphoreGive(SemaphoreHandle_t s){(void)s;return 1;}
esp_err_t nvs_open_from_partition(const char *p,const char *ns,int mode,nvs_handle_t *h){assert(!strcmp(p,"clockcfg")&&!strcmp(ns,"media"));(void)mode;*h=1;return 0;}
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *size){(void)h;(void)key;if(!stored_size)return 1;assert(*size>=stored_size);memcpy(out,stored,stored_size);*size=stored_size;return 0;}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *value,size_t size){(void)h;(void)key;if(storage_error)return storage_error;assert(size<sizeof(stored));memcpy(stored,value,size);stored_size=size;return 0;}
esp_err_t nvs_commit(nvs_handle_t h){(void)h;return storage_error;}
static ha_snapshot_t ha={.configured=true,.endpoint="http://192.168.1.232:8123"};
void ha_service_snapshot(ha_snapshot_t *s){*s=ha;}
int ha_service_request(const char *path,const char *body,char *buffer,size_t capacity,size_t *size)
{
    requests++;now+=1000;
    if(body){
        posts++;assert(strstr(body,"media_player.bedroom"));strcpy(last_body,body);strcpy(last_path,path);
        if(strstr(path,"volume_set"))assert(strstr(body,"0.400"));
        if(apply&&code==200){if(strstr(path,"media_pause"))remote="paused";else if(strstr(path,"media_play")||strstr(path,"/play_media"))remote="playing";}
    }
    *size=snprintf(buffer,capacity,"{\"entity_id\":\"media_player.bedroom\",\"state\":\"%s\",\"attributes\":{\"supported_features\":16949,\"volume_level\":0.35}}",remote);return code;
}
static media_snapshot_t snapshot(void){media_snapshot_t s;media_service_snapshot(&s);return s;}
int main(int argc,char **argv)
{
    if(argc>1&&!strcmp(argv[1],"selection-reload")){
        struct {uint32_t version;char endpoint[192],entity[96],content[384],type[48];} saved={.version=2,
            .endpoint="http://192.168.1.232:8123",.entity="media_player.bedroom",
            .content="https://example.test/radio",.type="music"};
        memcpy(stored,&saved,sizeof(saved));stored_size=sizeof(saved);media_service_init();
        assert(snapshot().configured&&!strcmp(snapshot().content,saved.content)&&!strcmp(snapshot().content_type,saved.type));
        media_service_poll(true);assert(media_service_action(MEDIA_START_SAVED));media_service_poll(true);
        assert(posts==1&&!strcmp(last_path,"/api/services/media_player/play_media"));
        puts("PASS stored media selection reload and explicit start");return 0;
    }
    if(argc>1&&!strcmp(argv[1],"legacy")){
        struct {uint32_t version;char endpoint[192],entity[96];} old={.version=1,.endpoint="http://192.168.1.232:8123",.entity="media_player.bedroom"};
        memcpy(stored,&old,sizeof(old));stored_size=sizeof(old);media_service_init();
        assert(snapshot().configured&&!strcmp(snapshot().entity,old.entity)&&!snapshot().content[0]);
        media_service_poll(true);assert(snapshot().fresh&&!media_service_action(MEDIA_START_SAVED));
        puts("PASS version1 player migration preserves target with empty selection");return 0;
    }
    if(argc>1){
        fail_mutex=!strcmp(argv[1],"mutex-failure");fail_queue=!strcmp(argv[1],"queue-failure");
        media_service_init();media_service_poll(true);assert(!requests);
        assert(strstr(snapshot().status,"resources unavailable"));
        assert(!media_service_refresh()&&!media_service_configure("media_player.bedroom"));
        puts("PASS media allocation failure snapshots and rejected commands");return 0;
    }
    media_service_init();media_service_poll(true);assert(!requests);
    assert(media_service_configure_tagged("media_player.bedroom",42));assert(!media_service_refresh());
    media_service_poll(true);assert(strstr(diagnostic,"tag=42 saved=1"));assert(snapshot().fresh&&snapshot().player.state==MEDIA_PAUSED);
    assert(media_service_action(MEDIA_PLAY));media_service_poll(true);assert(posts==1&&snapshot().player.state==MEDIA_PLAYING);
    assert(strstr(snapshot().status,"confirmed"));
    assert(media_service_action(MEDIA_LOUDER));media_service_poll(true);assert(posts==2);
    apply=false;assert(media_service_action(MEDIA_PAUSE));media_service_poll(true);
    assert(snapshot().player.state==MEDIA_PLAYING&&strstr(snapshot().status,"not yet confirmed"));
    now+=11000000;code=-1;media_service_poll(true);assert(strstr(snapshot().status,"not confirmed"));
    assert(!snapshot().fresh&&!media_service_action(MEDIA_PLAY));
    unsigned count=requests;now+=2000000;media_service_poll(true);assert(requests==count);
    code=401;now+=11000000;media_service_poll(true);assert(strstr(snapshot().status,"access denied"));
    code=200;assert(media_service_refresh());media_service_poll(true);assert(snapshot().fresh);
    assert(media_service_action(MEDIA_PAUSE));strcpy(ha.endpoint,"http://another-server");
    count=requests;media_service_poll(true);assert(requests==count&&!snapshot().fresh);
    strcpy(ha.endpoint,"http://192.168.1.232:8123");media_service_poll(false);assert(!snapshot().fresh);
    assert(media_service_refresh());media_service_poll(true);
    assert(!media_service_action(MEDIA_START_SAVED));
    assert(!media_service_select("media_player.bedroom","id",""));
    assert(media_service_select("media_player.bedroom","https://example.test/radio","music"));
    media_service_poll(true);assert(!strcmp(snapshot().content,"https://example.test/radio"));
    assert(media_service_configure("media_player.bedroom"));media_service_poll(true);
    assert(!strcmp(snapshot().content,"https://example.test/radio"));
    unsigned prior_posts=posts;assert(media_service_action(MEDIA_START_SAVED));media_service_poll(true);
    assert(posts==prior_posts+1&&!strcmp(last_path,"/api/services/media_player/play_media"));
    assert(strstr(last_body,"media_content_id")&&strstr(last_body,"https://example.test/radio"));
    assert(strstr(snapshot().status,"selection not verified"));
    storage_error=1;assert(media_service_configure("media_player.other"));media_service_poll(true);
    assert(!strcmp(snapshot().entity,"media_player.bedroom")&&strstr(snapshot().status,"not saved"));
    assert(!strcmp(snapshot().content,"https://example.test/radio"));
    media_service_disable();assert(!media_service_refresh()&&!media_service_action(MEDIA_PLAY));
    puts("PASS media owner: explicit actions, observed confirmation, volume, failure expiry, auth, server binding, offline and failed saves");
}

void diagnostics_printf(const char *format,...){va_list args;va_start(args,format);vsnprintf(diagnostic,sizeof(diagnostic),format,args);va_end(args);}
