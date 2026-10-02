#pragma once
#include "media_backend.h"
/* HA wire-format helpers; generic owner and UI must not depend on these. */
bool media_ha_parse(const char *json,size_t size,const char *entity,media_player_t *out);
bool media_ha_body(const char *entity,const char *id,const char *type,char *out,size_t capacity);

void media_ha_config(media_backend_config_t *out);
bool media_ha_identity_valid(const char *identity);
bool media_ha_target_valid(const char *target);
int media_ha_read(const char *target,media_player_t *out);
int media_ha_action(const char *target,media_action_t action,const media_player_t *current,const char *content,const char *type);
