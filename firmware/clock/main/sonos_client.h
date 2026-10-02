#pragma once
#include "media_model.h"
#define SONOS_RESPONSE_CAPACITY 32769
/* Transport must enforce a finite deadline and reject redirects/overflow. */
typedef int (*sonos_http_t)(void *context,const char *endpoint,const char *path,
    const char *soap_action,const char *body,char *response,size_t capacity,size_t *size);
typedef struct {
    sonos_http_t http;void *context;
    bool (*allowed)(void *context);void *allowed_context;
    char response[SONOS_RESPONSE_CAPACITY],body[24576];size_t response_size;
} sonos_client_t;
typedef struct {char endpoint[48],uuid[48],name[64];} sonos_device_t;
enum {SONOS_OK,SONOS_ERROR,SONOS_IDENTITY_CHANGED,SONOS_GROUP_UNSUPPORTED,SONOS_CANCELLED};
bool sonos_endpoint_valid(const char *endpoint);
bool sonos_target_parse(const char *target,sonos_device_t *device);
bool sonos_target_format(const sonos_device_t *device,char out[96]);
int sonos_probe(sonos_client_t *client,const char *endpoint,sonos_device_t *device);
int sonos_read(sonos_client_t *client,const sonos_device_t *device,media_player_t *player);
int sonos_action(sonos_client_t *client,const sonos_device_t *device,media_action_t action,
    const media_player_t *current,const char *uri,const char *metadata);
int sonos_stop(sonos_client_t *client,const sonos_device_t *device);
#define SONOS_FAVORITES_PAGE 6
typedef struct {char id[96],title[96];} sonos_favorite_t;
typedef struct {sonos_favorite_t items[SONOS_FAVORITES_PAGE];unsigned count,total,start;} sonos_favorites_t;
int sonos_favorites(sonos_client_t *client,const sonos_device_t *device,unsigned start,sonos_favorites_t *out);
int sonos_play_favorite(sonos_client_t *client,const sonos_device_t *device,const char *id);
