#pragma once
#include <stddef.h>
/* Call once before starting network workers. */
void network_http_init(void);
/* Network-worker only. Credentials never logged; redirects never followed. */
int network_http_request(const char *url,const char *token,const char *post_body,char *buffer,size_t capacity,size_t *size);

/* Key sent only to Wallhaven API, never to image hosts or redirects. */
int network_http_wallhaven(const char *url,const char *key,char *buffer,size_t capacity,size_t *size);
