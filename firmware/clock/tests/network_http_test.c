#include "network_http.h"
#include "sonos_network.h"
#include "esp_http_client.h"
#include "freertos/semphr.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static int64_t now,lock_delay,perform_delay;
static bool lock_ok=true,init_ok=true,fail_header;
static int acquired,released,initialized,cleaned,closed,status=200,method,wait_ms;
static esp_http_client_config_t cfg;
static char url[256],action[256],type[80],auth[80],key[80],body[256];
static const char *reply="<ok/>";
int64_t esp_timer_get_time(void){return now;}
SemaphoreHandle_t xSemaphoreCreateMutex(void){return (void *)1;}
int xSemaphoreTake(SemaphoreHandle_t s,unsigned timeout){assert(s);wait_ms=timeout;now+=lock_delay;if(lock_ok)acquired++;return lock_ok;}
int xSemaphoreGive(SemaphoreHandle_t s){assert(s);released++;return 1;}
esp_http_client_handle_t esp_http_client_init(const esp_http_client_config_t *c){initialized++;cfg=*c;snprintf(url,sizeof(url),"%s",c->url);return init_ok?(void *)2:NULL;}
int esp_http_client_set_header(void *c,const char *name,const char *value){assert(c);if(fail_header)return ESP_FAIL;
    if(!strcmp(name,"SOAPACTION"))snprintf(action,sizeof(action),"%s",value);
    else if(!strcmp(name,"Content-Type"))snprintf(type,sizeof(type),"%s",value);
    else if(!strcmp(name,"Authorization"))snprintf(auth,sizeof(auth),"%s",value);
    else if(!strcmp(name,"X-API-Key"))snprintf(key,sizeof(key),"%s",value);
    else assert(0);
    return ESP_OK;}
int esp_http_client_set_method(void *c,int m){assert(c);method=m;return ESP_OK;}
int esp_http_client_set_post_field(void *c,const char *b,int n){assert(c);assert(n==(int)strlen(b));snprintf(body,sizeof(body),"%s",b);return ESP_OK;}
int esp_http_client_perform(void *c){assert(c);now+=perform_delay;esp_http_client_event_t e={.user_data=cfg.user_data,.client=c,.event_id=HTTP_EVENT_ON_DATA,.data=(void *)reply,.data_len=strlen(reply)};cfg.event_handler(&e);return closed?ESP_FAIL:ESP_OK;}
int esp_http_client_get_status_code(void *c){assert(c);return status;}
int esp_http_client_close(void *c){assert(c);closed++;assert(closed==1);esp_http_client_event_t e={.user_data=cfg.user_data,.client=c,.event_id=2};cfg.event_handler(&e);return ESP_OK;}
int esp_http_client_cleanup(void *c){assert(c);cleaned++;return ESP_OK;}
static const char *endpoint="192.168.1.50:1400",*path="/MediaRenderer/AVTransport/Control",*soap="urn:schemas-upnp-org:service:AVTransport:1#Play";
static void reset(void){now=lock_delay=perform_delay=0;lock_ok=init_ok=true;fail_header=false;acquired=released=initialized=cleaned=closed=method=0;status=200;reply="<ok/>";action[0]=type[0]=auth[0]=key[0]=body[0]=0;}
static bool permit(void *context){return *(bool *)context;}
int main(void){network_http_init();char out[32];size_t n;
    reset();assert(network_http_sonos(endpoint,path,soap,"<Play/>",10000000,out,sizeof(out),&n)==200);
    assert(!strcmp(url,"http://192.168.1.50:1400/MediaRenderer/AVTransport/Control"));
    assert(!strcmp(action,"\"urn:schemas-upnp-org:service:AVTransport:1#Play\""));
    assert(!strcmp(type,"text/xml; charset=utf-8")&&!auth[0]&&!key[0]);
    assert(method==HTTP_METHOD_POST&&!strcmp(body,"<Play/>")&&cfg.disable_auto_redirect);
    assert(cfg.timeout_ms==2000&&wait_ms==2000&&acquired==released&&cleaned==1&&n==5);
    reset();assert(network_http_sonos(endpoint,"/xml/device_description.xml",NULL,NULL,10000000,out,sizeof(out),&n)==200);assert(!method&&!action[0]);
    reset();assert(network_http_sonos("8.8.8.8:1400",path,soap,"x",10000000,out,sizeof(out),&n)==-1&&!initialized);
    assert(network_http_sonos(endpoint,"/evil",soap,"x",10000000,out,sizeof(out),&n)==-1&&!initialized);
    assert(network_http_sonos(endpoint,path,"urn:schemas-upnp-org:service:X\r\nEvil:1","x",10000000,out,sizeof(out),&n)==-1&&!initialized);
    reset();now=10000000;assert(network_http_sonos(endpoint,path,soap,"x",10000000,out,sizeof(out),&n)==-1&&!acquired);
    reset();lock_delay=1000000;assert(network_http_sonos(endpoint,path,soap,"x",1000000,out,sizeof(out),&n)==-1&&!initialized&&acquired==released);
    reset();lock_delay=500000;assert(network_http_sonos(endpoint,path,soap,"x",1000000,out,sizeof(out),&n)==200&&cfg.timeout_ms==500);
    reset();lock_ok=false;assert(network_http_sonos(endpoint,path,soap,"x",10000000,out,sizeof(out),&n)==-1&&!initialized);
    reset();init_ok=false;assert(network_http_sonos(endpoint,path,soap,"x",10000000,out,sizeof(out),&n)==-1&&acquired==released&&!cleaned);
    reset();fail_header=true;assert(network_http_sonos(endpoint,path,soap,"x",10000000,out,sizeof(out),&n)==-1&&cleaned==1&&acquired==released);
    reset();reply="this payload exceeds the fixed response buffer size";assert(network_http_sonos(endpoint,path,soap,"x",10000000,out,sizeof(out),&n)==-1&&closed==1&&cleaned==1&&acquired==released);
    reset();perform_delay=10000001;assert(network_http_sonos(endpoint,path,soap,"x",10000000,out,sizeof(out),&n)==-1&&cleaned==1&&acquired==released);
    reset();status=302;assert(network_http_sonos(endpoint,path,soap,"x",10000000,out,sizeof(out),&n)==302&&cfg.disable_auto_redirect);
    reset();assert(network_http_request("https://ha.invalid/api","test-token","{}",out,sizeof(out),&n)==200);assert(!strcmp(type,"application/json")&&!strcmp(auth,"Bearer test-token")&&!action[0]&&cfg.timeout_ms==8000);
    reset();assert(network_http_wallhaven("https://wallhaven.cc/api/v1/search","test-key",out,sizeof(out),&n)==200);assert(!strcmp(key,"test-key")&&!auth[0]&&!action[0]);
    reset();static sonos_client_t client;sonos_network_t network;bool permitted=true;
    sonos_network_begin(&client,&network,permit,&permitted);
    assert(client.allowed(client.allowed_context));
    assert(client.http(client.context,endpoint,path,soap,"x",out,sizeof(out),&n)==200);
    permitted=false;assert(!client.allowed(client.allowed_context));
    assert(client.http(client.context,endpoint,path,soap,"x",out,sizeof(out),&n)==-1&&initialized==1);
    permitted=true;now=10000000;assert(!client.allowed(client.allowed_context));
    assert(client.http(client.context,endpoint,path,soap,"x",out,sizeof(out),&n)==-1&&initialized==1);
    puts("network HTTP Sonos boundaries PASS");return 0;
}
