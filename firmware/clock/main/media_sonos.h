#pragma once
#include "media_backend.h"
int media_sonos_read(const char *target,media_player_t *out);
int media_sonos_action(const char *target,media_action_t action,const media_player_t *current,const char *content,const char *type);

int media_sonos_action_guarded(const char *target,media_action_t action,const media_player_t *current,
    const char *content,const char *type,bool (*allowed)(void *),void *context);
int media_sonos_stop(const char *target);
