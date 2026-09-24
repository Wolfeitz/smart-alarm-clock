#pragma once
#include "media_backend.h"
/* HA wire-format helpers; generic owner and UI must not depend on these. */
bool media_ha_parse(const char *json,size_t size,const char *entity,media_player_t *out);
bool media_ha_body(const char *entity,const char *id,const char *type,char *out,size_t capacity);
