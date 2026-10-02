#include "media_sonos.h"
#include "sonos_network.h"
#include <stdlib.h>
#include <string.h>
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#endif
static sonos_client_t *client_new(void)
{
#ifdef ESP_PLATFORM
    return heap_caps_calloc(1,sizeof(sonos_client_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
#else
    return calloc(1,sizeof(sonos_client_t));
#endif
}
static int result(int code)
{return code==SONOS_OK?MEDIA_BACKEND_OK:code==SONOS_IDENTITY_CHANGED?MEDIA_BACKEND_NOT_FOUND:MEDIA_BACKEND_ERROR;}
int media_sonos_read(const char *target,media_player_t *out)
{
    sonos_device_t device;
    if(!out||!sonos_target_parse(target,&device))return MEDIA_BACKEND_ERROR;
    sonos_client_t *client=client_new();if(!client)return MEDIA_BACKEND_ERROR;
    sonos_network_t network;sonos_network_begin(client,&network,NULL,NULL);
    int code=sonos_read(client,&device,out);free(client);return result(code);
}
int media_sonos_action_guarded(const char *target,media_action_t action,const media_player_t *current,const char *content,const char *type,bool (*allowed)(void *),void *context)
{
    sonos_device_t device;
    if(!sonos_target_parse(target,&device)||!media_action_supported(current,action))return MEDIA_BACKEND_ERROR;
    /* URI is an explicit interim selection type; favorites are resolved separately,
     * never treated as arbitrary Spotify metadata or silently sent as empty DIDL. */
    if(action==MEDIA_START_SAVED&&(!media_selection_valid(content,type)||!content[0]||(strcmp(type,"uri")&&strcmp(type,"sonos-favorite"))))return MEDIA_BACKEND_ERROR;
    sonos_client_t *client=client_new();if(!client)return MEDIA_BACKEND_ERROR;
    sonos_network_t network;sonos_network_begin(client,&network,allowed,context);
    int code=action==MEDIA_START_SAVED&&!strcmp(type,"sonos-favorite")?
        sonos_play_favorite(client,&device,content):sonos_action(client,&device,action,current,content,"");free(client);return result(code);
}

int media_sonos_action(const char *target,media_action_t action,const media_player_t *current,const char *content,const char *type)
{return media_sonos_action_guarded(target,action,current,content,type,NULL,NULL);}
int media_sonos_stop(const char *target)
{
    sonos_device_t device;if(!sonos_target_parse(target,&device))return MEDIA_BACKEND_ERROR;
    sonos_client_t *client=client_new();if(!client)return MEDIA_BACKEND_ERROR;
    sonos_network_t network;sonos_network_begin(client,&network,NULL,NULL);
    int code=sonos_stop(client,&device);free(client);return result(code);
}
