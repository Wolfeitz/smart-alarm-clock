#include "media_model.h"
#include "media_ha.h"
#include "ha_service.h"
#include <assert.h>
#include "cJSON.h"
#include <stdio.h>
#include <string.h>
int main(void)
{
    assert(media_backend_target_valid("media_player.bedroom"));assert(!media_backend_target_valid("light.bedroom"));assert(!media_backend_target_valid("media_player.a\"}"));
    const char *sonos="sonos:192.168.1.50:1400/RINCON_TEST";
    assert(media_backend_target_valid(sonos)&&!media_ha_target_valid(sonos));
    assert(!media_backend_target_valid("sonos:8.8.8.8:1400/RINCON_TEST"));
    media_backend_config_t config;media_backend_config_for(sonos,&config);
    assert(config.configured&&!strcmp(config.identity,"sonos-local-v1"));
    assert(media_backend_identity_valid(config.identity));
    media_player_t speaker;assert(media_backend_read(sonos,&speaker)==MEDIA_BACKEND_OK&&!strcmp(speaker.name,"Sonos fixture"));
    assert(media_backend_action(sonos,MEDIA_PLAY,&speaker,"","")==MEDIA_BACKEND_OK);
    media_backend_config_for("invalid",&config);assert(!config.configured&&!config.identity[0]);
    char invalid_body[64];assert(!media_ha_body(sonos,"id","music",invalid_body,sizeof(invalid_body)));
    const char *s="{\"entity_id\":\"media_player.bedroom\",\"state\":\"paused\",\"attributes\":{\"supported_features\":16437,\"volume_level\":0.35,\"media_title\":\"Example\"}}";
    media_player_t p;assert(media_ha_parse(s,strlen(s),"media_player.bedroom",&p));
    assert(p.state==MEDIA_PAUSED&&p.volume_known&&!strcmp(p.title,"Example"));
    for(unsigned i=0;i<6;i++)assert(media_action_supported(&p,i));
    assert(!media_ha_parse(s,strlen(s),"media_player.other",&p));assert(!media_ha_parse(s,strlen(s)-1,"media_player.bedroom",&p));
    p.capabilities=1u<<MEDIA_START_SAVED;assert(!media_action_supported(&p,MEDIA_PLAY));assert(media_action_supported(&p,MEDIA_START_SAVED)); /* PLAY_MEDIA is not PLAY */
    p.capabilities=0;p.volume_known=false;assert(!media_action_supported(&p,MEDIA_LOUDER));
    p.capabilities=(1u<<MEDIA_QUIETER)|(1u<<MEDIA_LOUDER);assert(media_action_supported(&p,MEDIA_LOUDER));
    p.state=MEDIA_UNAVAILABLE;assert(!media_action_supported(&p,MEDIA_LOUDER));
    const char *bad="{\"entity_id\":\"media_player.bedroom\",\"state\":\"paused\",\"attributes\":{\"supported_features\":-1}}";
    assert(!media_ha_parse(bad,strlen(bad),"media_player.bedroom",&p));
    assert(media_selection_valid("",""));assert(!media_selection_valid("id",""));
    assert(!media_selection_valid("","music"));assert(!media_selection_valid("a\nb","music"));
    char huge[385];memset(huge,'x',384);huge[384]=0;assert(!media_selection_valid(huge,"music"));
    assert(!media_selection_valid("id","bad type"));
    char body[1152];const char *id="https://example.test/a?name=\"quoted\"&path=\\test";
    assert(media_ha_body("media_player.bedroom",id,"music",body,sizeof(body)));
    cJSON *json=cJSON_Parse(body);assert(json&&cJSON_GetArraySize(json)==3);
    assert(!strcmp(cJSON_GetObjectItemCaseSensitive(json,"media_content_id")->valuestring,id));
    assert(!strcmp(cJSON_GetObjectItemCaseSensitive(json,"media_content_type")->valuestring,"music"));cJSON_Delete(json);
    assert(!media_ha_body("media_player.bedroom",id,"music",body,4));
    puts("PASS media parsing, exact entity, capability flags, volume availability and malformed response");
}

void ha_service_snapshot(ha_snapshot_t *s){memset(s,0,sizeof(*s));}
int ha_service_request(const char *path,const char *body,char *response,size_t capacity,size_t *size)
{(void)path;(void)body;(void)response;(void)capacity;(void)size;assert(0);return -1;}
