#include "media_model.h"
#include <assert.h>
#include "cJSON.h"
#include <stdio.h>
#include <string.h>
int main(void)
{
    assert(media_entity_valid("media_player.bedroom"));assert(!media_entity_valid("light.bedroom"));assert(!media_entity_valid("media_player.a\"}"));
    const char *s="{\"entity_id\":\"media_player.bedroom\",\"state\":\"paused\",\"attributes\":{\"supported_features\":16437,\"volume_level\":0.35,\"media_title\":\"Example\"}}";
    media_player_t p;assert(media_parse(s,strlen(s),"media_player.bedroom",&p));
    assert(p.state==MEDIA_PAUSED&&p.volume_known&&!strcmp(p.title,"Example"));
    for(unsigned i=0;i<6;i++)assert(media_action_supported(&p,i));
    assert(!media_parse(s,strlen(s),"media_player.other",&p));assert(!media_parse(s,strlen(s)-1,"media_player.bedroom",&p));
    p.features=512;assert(!media_action_supported(&p,MEDIA_PLAY));assert(media_action_supported(&p,MEDIA_START_SAVED)); /* PLAY_MEDIA is not PLAY */
    p.features=4;p.volume_known=false;assert(!media_action_supported(&p,MEDIA_LOUDER));
    p.features=1024;assert(media_action_supported(&p,MEDIA_LOUDER));
    p.state=MEDIA_UNAVAILABLE;assert(!media_action_supported(&p,MEDIA_LOUDER));
    const char *bad="{\"entity_id\":\"media_player.bedroom\",\"state\":\"paused\",\"attributes\":{\"supported_features\":-1}}";
    assert(!media_parse(bad,strlen(bad),"media_player.bedroom",&p));
    assert(media_selection_valid("",""));assert(!media_selection_valid("id",""));
    assert(!media_selection_valid("","music"));assert(!media_selection_valid("a\nb","music"));
    char huge[385];memset(huge,'x',384);huge[384]=0;assert(!media_selection_valid(huge,"music"));
    assert(!media_selection_valid("id","bad type"));
    char body[1152];const char *id="https://example.test/a?name=\"quoted\"&path=\\test";
    assert(media_selection_body("media_player.bedroom",id,"music",body,sizeof(body)));
    cJSON *json=cJSON_Parse(body);assert(json&&cJSON_GetArraySize(json)==3);
    assert(!strcmp(cJSON_GetObjectItemCaseSensitive(json,"media_content_id")->valuestring,id));
    assert(!strcmp(cJSON_GetObjectItemCaseSensitive(json,"media_content_type")->valuestring,"music"));cJSON_Delete(json);
    assert(!media_selection_body("media_player.bedroom",id,"music",body,4));
    puts("PASS media parsing, exact entity, capability flags, volume availability and malformed response");
}
