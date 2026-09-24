#include "media_model.h"
#include <assert.h>
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
    p.features=512;assert(!media_action_supported(&p,MEDIA_PLAY)); /* PLAY_MEDIA is not PLAY */
    p.features=4;p.volume_known=false;assert(!media_action_supported(&p,MEDIA_LOUDER));
    p.features=1024;assert(media_action_supported(&p,MEDIA_LOUDER));
    p.state=MEDIA_UNAVAILABLE;assert(!media_action_supported(&p,MEDIA_LOUDER));
    const char *bad="{\"entity_id\":\"media_player.bedroom\",\"state\":\"paused\",\"attributes\":{\"supported_features\":-1}}";
    assert(!media_parse(bad,strlen(bad),"media_player.bedroom",&p));
    puts("PASS media parsing, exact entity, capability flags, volume availability and malformed response");
}
