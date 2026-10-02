#include "network_http.h"
#include "sonos_client.h"
#include <ctype.h>
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
typedef struct {char *data;size_t size,capacity;bool overflow,aborted;int64_t deadline;} response_t;
/* Concurrent TLS handshakes exhaust the C5 internal heap. Only network workers
 * take this mutex; UI, alarm scheduling and local fallback never wait on it. */
static SemaphoreHandle_t request_lock;
void network_http_init(void){request_lock=xSemaphoreCreateMutex();}
static esp_err_t http_event(esp_http_client_event_t *event)
{
    response_t *r=event->user_data;
    if(r->aborted)return ESP_FAIL;
    bool expired=esp_timer_get_time()>=r->deadline;
    bool overflow=event->event_id==HTTP_EVENT_ON_DATA&&
        (event->data_len<0||(size_t)event->data_len>=r->capacity-r->size);
    if(expired||overflow){
        /* IDF ignores callback return values for received data. Close the socket
         * explicitly; guard the resulting disconnect callback against recursion. */
        r->aborted=true;r->overflow=overflow;
        esp_http_client_close(event->client);
        return expired?ESP_ERR_TIMEOUT:ESP_FAIL;
    }
    if(event->event_id==HTTP_EVENT_ON_DATA){
        memcpy(r->data+r->size,event->data,event->data_len);r->size+=event->data_len;r->data[r->size]=0;
    }
    return ESP_OK;
}
static int request(const char *url,const char *token,const char *post_body,const char *api_key,const char *soap_action,int64_t deadline,char *buffer,size_t capacity,size_t *size)
{
    if(!size)return -1;
    *size=0;if(!buffer||capacity<2)return -1;
    buffer[0]=0;
    bool local=deadline!=0;
    int timeout=local?2000:8000;
    int64_t remaining=deadline-esp_timer_get_time();
    if(local){if(remaining<1000)return -1;if(remaining/1000<timeout)timeout=remaining/1000;}
    if(!request_lock||xSemaphoreTake(request_lock,pdMS_TO_TICKS(timeout))!=pdTRUE)return -1;
    remaining=deadline-esp_timer_get_time();
    if(local&&remaining<1000){xSemaphoreGive(request_lock);return -1;}
    if(local&&remaining/1000<timeout)timeout=remaining/1000;
    response_t response={.data=buffer,.capacity=capacity,.deadline=local?deadline:esp_timer_get_time()+20000000};
    esp_http_client_config_t config={.url=url,.crt_bundle_attach=esp_crt_bundle_attach,.timeout_ms=timeout,
        .event_handler=http_event,.user_data=&response,.disable_auto_redirect=true,.buffer_size=1024};
    esp_http_client_handle_t client=esp_http_client_init(&config);
    if(!client){xSemaphoreGive(request_lock);return -1;}
    char authorization[520]={0};esp_err_t err=ESP_OK;
    if(token){snprintf(authorization,sizeof(authorization),"Bearer %s",token);err=esp_http_client_set_header(client,"Authorization",authorization);}
    if(api_key&&*api_key&&err==ESP_OK)err=esp_http_client_set_header(client,"X-API-Key",api_key);
    char quoted_action[192];
    if(soap_action&&err==ESP_OK){
        snprintf(quoted_action,sizeof(quoted_action),"\"%s\"",soap_action);
        err=esp_http_client_set_header(client,"SOAPACTION",quoted_action);
    }
    if(post_body&&err==ESP_OK){
        err=esp_http_client_set_method(client,HTTP_METHOD_POST);
        if(err==ESP_OK)err=esp_http_client_set_header(client,"Content-Type",soap_action?"text/xml; charset=utf-8":"application/json");
        if(err==ESP_OK)err=esp_http_client_set_post_field(client,post_body,strlen(post_body));
    }
    if(err==ESP_OK)err=esp_http_client_perform(client);
    int result=err==ESP_OK&&!response.overflow&&!response.aborted&&esp_timer_get_time()<=response.deadline?esp_http_client_get_status_code(client):-1;
    *size=response.size;esp_http_client_cleanup(client);memset(authorization,0,sizeof(authorization));
    xSemaphoreGive(request_lock);return result;
}

int network_http_request(const char *url,const char *token,const char *body,char *buffer,size_t capacity,size_t *size)
{return request(url,token,body,NULL,NULL,0,buffer,capacity,size);}
int network_http_wallhaven(const char *url,const char *key,char *buffer,size_t capacity,size_t *size)
{
    const char *prefix="https://wallhaven.cc/api/v1/";
    if(strncmp(url,prefix,strlen(prefix))){*size=0;return -1;}
    return request(url,NULL,NULL,key,NULL,0,buffer,capacity,size);
}

int network_http_sonos(const char *endpoint,const char *path,const char *action,const char *body,
    int64_t deadline,char *buffer,size_t capacity,size_t *size)
{
    if(!size)return -1;
    *size=0;
    if(!sonos_endpoint_valid(endpoint)||!path||deadline<=0)return -1;
    if(!action){if(body||strcmp(path,"/xml/device_description.xml"))return -1;}
    else {
        if(!body||strlen(action)>180||strncmp(action,"urn:schemas-upnp-org:service:",28))return -1;
        for(const char *p=action;*p;p++)if(!isalnum((unsigned char)*p)&&*p!=':'&&*p!='-'&&*p!='#')return -1;
        if(strcmp(path,"/MediaRenderer/AVTransport/Control")&&strcmp(path,"/MediaRenderer/RenderingControl/Control")&&
           strcmp(path,"/ZoneGroupTopology/Control")&&strcmp(path,"/MediaServer/ContentDirectory/Control"))return -1;
    }
    char url[192];int n=snprintf(url,sizeof(url),"http://%s%s",endpoint,path);
    if(n<0||(size_t)n>=sizeof(url))return -1;
    return request(url,NULL,body,NULL,action,deadline,buffer,capacity,size);
}
