/* Router-only fixture. Sonos protocol/transport have separate production tests. */
#include "media_sonos.h"
#include <assert.h>
#include <string.h>
static void target_check(const char *target){assert(!strcmp(target,"sonos:192.168.1.50:1400/RINCON_TEST"));}
int media_sonos_read(const char *target,media_player_t *out)
{target_check(target);*out=(media_player_t){.state=MEDIA_PAUSED,.capabilities=1u<<MEDIA_PLAY};strcpy(out->name,"Sonos fixture");return MEDIA_BACKEND_OK;}
int media_sonos_action(const char *target,media_action_t action,const media_player_t *current,const char *content,const char *type)
{target_check(target);assert(action==MEDIA_PLAY&&current->state==MEDIA_PAUSED&&!content[0]&&!type[0]);return MEDIA_BACKEND_OK;}

int media_sonos_action_guarded(const char *target,media_action_t action,const media_player_t *current,
    const char *content,const char *type,bool (*allowed)(void *),void *context)
{if(allowed&&!allowed(context))return MEDIA_BACKEND_ERROR;return media_sonos_action(target,action,current,content,type);}
int media_sonos_stop(const char *target){target_check(target);return MEDIA_BACKEND_OK;}
