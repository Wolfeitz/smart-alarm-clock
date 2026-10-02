#pragma once
#include <stddef.h>
/* Call once before starting network workers. */
void network_http_init(void);
/* Network-worker only. Credentials never logged; redirects never followed. */
int network_http_request(const char *url,const char *token,const char *post_body,char *buffer,size_t capacity,size_t *size);
