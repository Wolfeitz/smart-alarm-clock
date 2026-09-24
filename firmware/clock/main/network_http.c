#include "network_http.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_timer.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
typedef struct {char *data;size_t size,capacity;bool overflow;int64_t deadline;} response_t;
static esp_err_t http_event(esp_http_client_event_t *event)
{
    response_t *r=event->user_data;
    if(esp_timer_get_time()>r->deadline)return ESP_ERR_TIMEOUT;
    if(event->event_id==HTTP_EVENT_ON_DATA){
        if(event->data_len<0||r->size+(size_t)event->data_len>=r->capacity){r->overflow=true;return ESP_FAIL;}
        memcpy(r->data+r->size,event->data,event->data_len);r->size+=event->data_len;r->data[r->size]=0;
    }
    return ESP_OK;
}
int network_http_request(const char *url,const char *token,const char *post_body,char *buffer,size_t capacity,size_t *size)
{
    *size=0;if(!buffer||capacity<2)return -1;
    response_t response={.data=buffer,.capacity=capacity,.deadline=esp_timer_get_time()+20000000};
    esp_http_client_config_t config={.url=url,.crt_bundle_attach=esp_crt_bundle_attach,.timeout_ms=8000,
        .event_handler=http_event,.user_data=&response,.disable_auto_redirect=true,.buffer_size=1024};
    esp_http_client_handle_t client=esp_http_client_init(&config);if(!client)return -1;
    char authorization[520]={0};esp_err_t err=ESP_OK;
    if(token){snprintf(authorization,sizeof(authorization),"Bearer %s",token);err=esp_http_client_set_header(client,"Authorization",authorization);}
    if(post_body&&err==ESP_OK){
        err=esp_http_client_set_method(client,HTTP_METHOD_POST);
        if(err==ESP_OK)err=esp_http_client_set_header(client,"Content-Type","application/json");
        if(err==ESP_OK)err=esp_http_client_set_post_field(client,post_body,strlen(post_body));
    }
    if(err==ESP_OK)err=esp_http_client_perform(client);
    int result=err==ESP_OK&&!response.overflow?esp_http_client_get_status_code(client):-1;
    *size=response.size;esp_http_client_cleanup(client);memset(authorization,0,sizeof(authorization));return result;
}
