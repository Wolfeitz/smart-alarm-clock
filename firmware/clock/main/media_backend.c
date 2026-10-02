#include "media_backend.h"
#include "media_ha.h"
#include "media_sonos.h"
#include "sonos_client.h"
#include <string.h>
static bool sonos(const char *target)
{sonos_device_t device;return sonos_target_parse(target,&device);}
void media_backend_config_for(const char *target,media_backend_config_t *out)
{
    memset(out,0,sizeof(*out));
    if(sonos(target)){out->configured=true;strcpy(out->identity,"sonos-local-v1");}
    else if(media_ha_target_valid(target))media_ha_config(out);
}
bool media_backend_identity_valid(const char *identity)
{return identity&&(!strcmp(identity,"sonos-local-v1")||media_ha_identity_valid(identity));}
bool media_backend_target_valid(const char *target)
{return sonos(target)||media_ha_target_valid(target);}
int media_backend_read(const char *target,media_player_t *out)
{
    if(!out)return MEDIA_BACKEND_ERROR;
    return sonos(target)?media_sonos_read(target,out):media_ha_read(target,out);
}
int media_backend_action(const char *target,media_action_t action,const media_player_t *current,const char *content,const char *type)
{return sonos(target)?media_sonos_action(target,action,current,content,type):media_ha_action(target,action,current,content,type);}

int media_backend_action_guarded(const char *target,media_action_t action,const media_player_t *current,
    const char *content,const char *type,bool (*allowed)(void *),void *context)
{
    if(allowed&&!allowed(context))return MEDIA_BACKEND_ERROR;
    return sonos(target)?media_sonos_action_guarded(target,action,current,content,type,allowed,context):
        media_ha_action(target,action,current,content,type);
}
int media_backend_stop(const char *target,const media_player_t *current)
{return sonos(target)?media_sonos_stop(target):media_ha_action(target,MEDIA_PAUSE,current,"","");}
