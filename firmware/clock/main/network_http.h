#pragma once
#include <stddef.h>
#include <stdint.h>
/* Call once before starting network workers. */
void network_http_init(void);
/* Network-worker only. Credentials never logged; redirects never followed. */
int network_http_request(const char *url,const char *token,const char *post_body,char *buffer,size_t capacity,size_t *size);

/* Key sent only to Wallhaven API, never to image hosts or redirects. */
int network_http_wallhaven(const char *url,const char *key,char *buffer,size_t capacity,size_t *size);

/* Sonos network-owner callback: absolute esp_timer microsecond deadline shared
 * across an operation; private numeric endpoint, fixed paths, no credentials. */
int network_http_sonos(const char *endpoint,const char *path,const char *soap_action,const char *body,
    int64_t deadline,char *buffer,size_t capacity,size_t *size);
