#pragma once
#include "media_model.h"
typedef struct {char entity[96],status[96];media_player_t player;bool configured,fresh,busy;} media_snapshot_t;
void media_service_init(void);
void media_service_disable(void);
void media_service_poll(bool online);
void media_service_snapshot(media_snapshot_t *out);
bool media_service_configure(const char *entity);
bool media_service_action(media_action_t action);
bool media_service_refresh(void);

bool media_service_configure_tagged(const char *entity,uint32_t tag);
