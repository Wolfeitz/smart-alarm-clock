#include "json_memory.h"
#include "cJSON.h"
#include "esp_heap_caps.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
static unsigned external,internal,live;static int failure;
void *heap_caps_malloc(size_t n,unsigned caps){if(caps&MALLOC_CAP_SPIRAM){external++;if(failure)return NULL;}else{internal++;if(failure==2)return NULL;}void *p=malloc(n);if(p)live++;return p;}
void heap_caps_free(void *p){if(p){assert(live);live--;free(p);}}
int main(void){json_memory_init();cJSON *root=cJSON_Parse("{\"data\":[1,2,3]}");assert(root&&external&&!internal);cJSON_Delete(root);assert(!live);failure=1;root=cJSON_Parse("{\"data\":[1,2,3]}");assert(root&&internal);cJSON_Delete(root);assert(!live);failure=2;assert(!cJSON_Parse("{}")&&!live);puts("PASS JSON external allocation, internal fallback, failure and matching free");}
