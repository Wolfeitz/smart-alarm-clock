#pragma once
#include <stddef.h>
/* Network-worker only. Credentials never logged; redirects never followed. */
int network_http_request(const char *url,const char *token,const char *post_body,char *buffer,size_t capacity,size_t *size);
