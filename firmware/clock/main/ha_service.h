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
