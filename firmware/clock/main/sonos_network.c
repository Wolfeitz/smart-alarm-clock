#include "sonos_network.h"
#include "network_http.h"
#include "esp_timer.h"
static bool allowed(void *context)
{
    sonos_network_t *n=context;
    return esp_timer_get_time()<n->deadline&&(!n->allowed||n->allowed(n->context));
}
static int http(void *context,const char *endpoint,const char *path,const char *action,
    const char *body,char *response,size_t capacity,size_t *size)
{
    sonos_network_t *n=context;
    if(!allowed(n)){*size=0;return -1;}
    return network_http_sonos(endpoint,path,action,body,n->deadline,response,capacity,size);
}
void sonos_network_begin(sonos_client_t *c,sonos_network_t *n,bool (*check)(void *),void *context)
{
    *n=(sonos_network_t){.deadline=esp_timer_get_time()+10000000,.allowed=check,.context=context};
    c->http=http;c->context=n;c->allowed=allowed;c->allowed_context=n;
    c->response_size=0;c->response[0]=0;c->body[0]=0;
}
