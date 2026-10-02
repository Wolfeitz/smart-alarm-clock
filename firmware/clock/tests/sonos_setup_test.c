#include "sonos_setup.h"
#include "sonos_network.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static char command[256];static unsigned command_size;static bool queued;static int code;static unsigned probes;static bool changed;
SemaphoreHandle_t xSemaphoreCreateMutex(void){return (void *)1;}
int xSemaphoreTake(SemaphoreHandle_t s,unsigned t){(void)s;(void)t;return 1;}
int xSemaphoreGive(SemaphoreHandle_t s){(void)s;return 1;}
QueueHandle_t xQueueCreate(unsigned n,size_t size){assert(n==1&&size<=sizeof(command));command_size=size;return command;}
int xQueueSend(QueueHandle_t q,const void *data,unsigned t){(void)q;(void)t;if(queued)return 0;memcpy(command,data,command_size);queued=true;return 1;}
int xQueueReceive(QueueHandle_t q,void *data,unsigned t){(void)q;(void)t;if(!queued)return 0;memcpy(data,command,command_size);queued=false;return 1;}
void sonos_network_begin(sonos_client_t *c,sonos_network_t *n,bool (*allowed)(void *),void *ctx){(void)c;(void)n;(void)allowed;(void)ctx;}
bool sonos_endpoint_valid(const char *s){return !strcmp(s,"192.168.1.50:1400");}
bool sonos_target_parse(const char *s,sonos_device_t *d){if(strcmp(s,"sonos:192.168.1.50:1400/RINCON_TEST"))return false;strcpy(d->endpoint,"192.168.1.50:1400");strcpy(d->uuid,"RINCON_TEST");return true;}
bool sonos_target_format(const sonos_device_t *d,char out[96]){assert(!strcmp(d->uuid,"RINCON_TEST"));strcpy(out,"sonos:192.168.1.50:1400/RINCON_TEST");return true;}
int sonos_probe(sonos_client_t *c,const char *endpoint,sonos_device_t *d){(void)c;probes++;assert(sonos_endpoint_valid(endpoint));strcpy(d->endpoint,endpoint);strcpy(d->uuid,changed?"RINCON_CHANGED":"RINCON_TEST");strcpy(d->name,"Test room");return SONOS_OK;}
int sonos_favorites(sonos_client_t *c,const sonos_device_t *d,unsigned start,sonos_favorites_t *p){(void)c;(void)d;*p=(sonos_favorites_t){.count=1,.total=7,.start=start,.items={{.id="FV:2/1",.title="Morning"}}};return code;}
int main(void){sonos_setup_snapshot_t s;sonos_setup_init();
 assert(!sonos_setup_lookup("bad")&&!sonos_setup_page(0));
 assert(sonos_setup_lookup("192.168.1.50"));assert(!sonos_setup_lookup("192.168.1.50"));sonos_setup_snapshot(&s);assert(s.busy&&!s.ready&&!probes);
 sonos_setup_poll(false);sonos_setup_snapshot(&s);assert(!s.busy&&!s.ready&&!probes&&strstr(s.status,"offline"));
 assert(sonos_setup_lookup("192.168.1.50:1400"));sonos_setup_poll(true);sonos_setup_snapshot(&s);assert(s.ready&&!s.busy&&s.favorites.total==7&&!strcmp(s.name,"Test room"));
 assert(!sonos_setup_page(7)&&sonos_setup_page(6));sonos_setup_poll(true);sonos_setup_snapshot(&s);assert(s.ready&&s.favorites.start==6);
 changed=true;assert(sonos_setup_page(0));sonos_setup_poll(true);sonos_setup_snapshot(&s);assert(!s.ready&&strstr(s.status,"identity changed"));
 changed=false;code=SONOS_GROUP_UNSUPPORTED;assert(sonos_setup_lookup("192.168.1.50"));sonos_setup_poll(true);sonos_setup_snapshot(&s);assert(!s.ready&&strstr(s.status,"ungrouped"));
 sonos_setup_disable();assert(!sonos_setup_lookup("192.168.1.50"));
 puts("PASS Sonos setup owner: queued lookup, offline/identity/group rejection, pages and disabled worker");}
