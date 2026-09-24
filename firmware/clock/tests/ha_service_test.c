/* Exercise the production owner with deterministic network/NVS boundaries. */
#include "ha_service.h"
#include "network_http.h"
#include "nvs.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int64_t now;
static unsigned requests,posts;
static int code=200,storage_error;
static const char *remote="off";
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
esp_err_t nvs_open_from_partition(const char *p,const char *ns,int mode,nvs_handle_t *h){assert(!strcmp(p,"clockcfg")&&!strcmp(ns,"ha_private"));(void)mode;*h=1;return 0;}
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *size){(void)h;(void)key;if(!stored_size)return 1;assert(*size>=stored_size);memcpy(out,stored,stored_size);*size=stored_size;return 0;}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *value,size_t size){(void)h;(void)key;if(storage_error)return storage_error;assert(size<sizeof(stored));memcpy(stored,value,size);stored_size=size;return 0;}
esp_err_t nvs_commit(nvs_handle_t h){(void)h;return storage_error;}
int network_http_request(const char *url,const char *token,const char *body,char *buffer,size_t capacity,size_t *size)
{
    assert(!strcmp(token,"synthetic-test-token"));requests++;now+=1000;
    if(body){posts++;assert(strstr(body,"light.bedside"));if(apply&&code==200)remote=strstr(url,"turn_on")?"on":"off";}
    *size=snprintf(buffer,capacity,"{\"entity_id\":\"light.bedside\",\"state\":\"%s\"}",remote);return code;
}
static ha_snapshot_t snapshot(void){ha_snapshot_t s;ha_service_snapshot(&s);return s;}
int main(int argc,char **argv)
{
    if(argc>1){
        fail_mutex=!strcmp(argv[1],"mutex-failure");fail_queue=!strcmp(argv[1],"queue-failure");
        ha_service_init();ha_service_poll(true);assert(!requests);
        assert(strstr(snapshot().status,"memory unavailable"));
        assert(!ha_service_refresh());
        assert(!ha_service_configure("http://host","synthetic-test-token","light.bedside"));
        puts("PASS unavailable HA resources preserve safe snapshots and reject commands");return 0;
    }
    ha_service_init();ha_service_poll(true);assert(!requests&&!snapshot().configured);
    assert(ha_service_configure("http://192.168.1.232:8123","synthetic-test-token","light.bedside"));
    assert(!ha_service_refresh()); /* bounded queue / operation */
    ha_service_poll(true);assert(snapshot().configured&&snapshot().fresh&&snapshot().light.state==HA_OFF);
    assert(ha_service_toggle());ha_service_poll(true);assert(posts==1&&snapshot().light.state==HA_ON);
    assert(strstr(snapshot().status,"confirmed"));
    apply=false;assert(ha_service_toggle());ha_service_poll(true);
    assert(snapshot().light.state==HA_ON); /* HTTP200 cannot invent an OFF state */
    now+=11000000;code=-1;ha_service_poll(true);assert(strstr(snapshot().status,"not confirmed"));
    assert(!snapshot().fresh&&!ha_service_toggle());
    unsigned count=requests;now+=2000000;ha_service_poll(true);assert(requests==count); /* no endless 1s retry */
    now+=11000000;code=401;ha_service_poll(true);assert(strstr(snapshot().status,"Access denied"));
    code=200;assert(ha_service_refresh());ha_service_poll(true);assert(snapshot().fresh);
    now+=31000000;assert(!snapshot().fresh&&!ha_service_toggle());
    ha_service_poll(false);assert(!snapshot().fresh);
    assert(ha_service_configure("http://192.168.1.232:8123","","light.bedside"));ha_service_poll(true);assert(snapshot().fresh);
    count=requests;
    assert(ha_service_configure("http://different.invalid","","light.bedside"));ha_service_poll(true);
    assert(requests==count&&strstr(snapshot().status,"not saved"));
    assert(!strcmp(snapshot().endpoint,"http://192.168.1.232:8123"));
    storage_error=1;
    assert(ha_service_configure("http://192.168.1.232:8123","synthetic-test-token","light.other"));ha_service_poll(true);
    assert(requests==count&&!strcmp(snapshot().entity,"light.bedside"));
    assert(strstr(snapshot().status,"not saved"));
    storage_error=0;
    assert(ha_service_configure("http://192.168.1.232:8123","",""));ha_service_poll(true);
    assert(snapshot().configured&&!snapshot().entity[0]);
    char response[256];size_t size=0;
    assert(ha_service_request("/api/states/media_player.bedroom",NULL,response,sizeof(response),&size)==200);
    count=requests;
    assert(ha_service_request("https://other.invalid",NULL,response,sizeof(response),&size)==-1&&requests==count);
    ha_service_disable();assert(!snapshot().busy&&!snapshot().fresh);
    assert(!ha_service_refresh()&&!ha_service_toggle());ha_service_poll(true);assert(requests==count);
    puts("PASS HA owner: persistence, queue, confirmation, HTTP200 without change, failure expiry, auth, stale and offline state");
}
