#pragma once
#include "ha_model.h"
typedef struct {
    char endpoint[192],entity[96],status[96];
    ha_light_t light;
    bool configured,busy,fresh;
} ha_snapshot_t;
void ha_service_init(void);
/* Network owner is unavailable; reject commands instead of leaving them queued. */
void ha_service_disable(void);
/* Called exclusively by the existing network worker; no second HTTP task. */
void ha_service_poll(bool online);
void ha_service_snapshot(ha_snapshot_t *snapshot);
bool ha_service_configure(const char *endpoint,const char *token,const char *entity);
bool ha_service_toggle(void);
bool ha_service_refresh(void);

/* Existing network worker only; never exposes a token to callers. */
int ha_service_request(const char *path,const char *body,char *response,size_t capacity,size_t *size);

bool ha_service_configure_tagged(const char *endpoint,const char *token,const char *entity,uint32_t tag);
