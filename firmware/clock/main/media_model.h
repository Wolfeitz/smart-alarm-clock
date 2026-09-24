#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef enum {MEDIA_UNKNOWN,MEDIA_OFF,MEDIA_ON,MEDIA_IDLE,MEDIA_PLAYING,MEDIA_PAUSED,MEDIA_BUFFERING,MEDIA_UNAVAILABLE} media_state_t;
typedef enum {MEDIA_PREVIOUS,MEDIA_PLAY,MEDIA_PAUSE,MEDIA_NEXT,MEDIA_QUIETER,MEDIA_LOUDER,MEDIA_START_SAVED} media_action_t;
typedef struct {media_state_t state;char name[64],title[96],artist[64];uint32_t capabilities;double volume;bool volume_known,relative_volume;} media_player_t;
bool media_action_supported(const media_player_t *player,media_action_t action);
const char *media_state_name(media_state_t state);

bool media_selection_valid(const char *id,const char *type);
