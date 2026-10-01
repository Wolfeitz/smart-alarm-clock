#include "background_model.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    background_config_t c={.source=BACKGROUND_LOCAL,.count=1,.images={"blue-hour"}};
    assert(background_config_valid(&c));strcpy(c.images[0],"../secret");assert(!background_config_valid(&c));
    c.source=BACKGROUND_SELECTED;strcpy(c.images[0],"https://example.test/image.jpg");assert(background_config_valid(&c));
    strcpy(c.images[0],"https://user:pass@example.test/a");assert(!background_config_valid(&c));
    assert(!background_https_url("http://example.test/a"));assert(!background_https_url("https://host/a\r\nx"));
    c.source=BACKGROUND_WALLHAVEN;c.count=0;c.interval_seconds=900;strcpy(c.query,"mountains & lake");assert(background_config_valid(&c));
    c.interval_seconds=1;assert(!background_config_valid(&c));c.interval_seconds=0;assert(background_config_valid(&c));
    char url[512];assert(background_search_url(c.query,url,sizeof(url)));assert(strstr(url,"mountains%20%26%20lake"));
    assert(strstr(url,"purity=100"));assert(!background_search_url(c.query,url,8));
    assert(background_selected_url("https://wallhaven.cc/w/pomle9",url,sizeof(url)));
    assert(!strcmp(url,"https://wallhaven.cc/api/v1/w/pomle9"));
    assert(!background_selected_url("https://wallhaven.cc/w/pomle9?apikey=x",url,sizeof(url)));
    const char *fixture="{\"data\":[{\"id\":\"pomle9\",\"purity\":\"sfw\",\"thumbs\":{\"large\":\"https://th.wallhaven.cc/lg/po/pomle9.jpg\"}},{\"id\":\"vpewrl\",\"purity\":\"nsfw\",\"thumbs\":{\"large\":\"https://th.wallhaven.cc/lg/vp/vpewrl.jpg\"}}]}";
    background_candidate_t candidates[8];assert(background_parse_search(fixture,strlen(fixture),candidates,8)==1);
    assert(!strcmp(candidates[0].id,"pomle9"));assert(!background_parse_search(fixture,strlen(fixture)-1,candidates,8));
    assert(!background_parse_search("{\"data\":[]}garbage",18,candidates,8));
    puts("PASS background sources, URL bounds, query encoding and SFW candidate parsing");
}
