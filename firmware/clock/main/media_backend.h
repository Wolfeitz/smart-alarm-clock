#pragma once
#include "media_model.h"
/* Config snapshots/validation are safe for UI callers. Reads/actions run only on
 * the existing network owner; no backend may call the UI or alarm services. */
typedef struct {bool configured;char identity[192];} media_backend_config_t;
enum {MEDIA_BACKEND_OK,MEDIA_BACKEND_ERROR,MEDIA_BACKEND_DENIED,MEDIA_BACKEND_NOT_FOUND};
void media_backend_config_for(const char *target,media_backend_config_t *out);
bool media_backend_identity_valid(const char *identity);
bool media_backend_target_valid(const char *target);
int media_backend_read(const char *target,media_player_t *out);
int media_backend_action(const char *target,media_action_t action,const media_player_t *current,const char *content,const char *type);

/* Network owner supplies session cancellation; backends do not own alarm state. */
int media_backend_action_guarded(const char *target,media_action_t action,const media_player_t *current,
    const char *content,const char *type,bool (*allowed)(void *),void *context);
int media_backend_stop(const char *target,const media_player_t *current);
