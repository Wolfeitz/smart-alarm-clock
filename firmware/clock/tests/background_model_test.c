#include "background_model.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    struct legacy_config {background_source_t source;uint32_t interval_seconds;unsigned count;char images[8][384];char query[96];};
    assert(sizeof(struct legacy_config)==offsetof(background_config_t,options));
    struct legacy_config old={.source=BACKGROUND_WALLHAVEN,.interval_seconds=900,.query="forest"};
    background_config_t migrated={0};memcpy(&migrated,&old,sizeof(old));
    assert(background_config_valid(&migrated)&&!migrated.options.version);
    assert(!strcmp(migrated.query,"forest")&&migrated.interval_seconds==900);
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
    assert(!strcmp(candidates[0].id,"pomle9"));
    const char *selected="{\"data\":{\"id\":\"pomle9\",\"purity\":\"sfw\",\"thumbs\":{\"large\":\"https://th.wallhaven.cc/lg/po/pomle9.jpg\"}}}";
    assert(background_parse_search(selected,strlen(selected),candidates,8)==1);
    assert(!strcmp(candidates[0].id,"pomle9"));assert(!background_parse_search(fixture,strlen(fixture)-1,candidates,8));
    assert(!background_parse_search("{\"data\":[]}garbage",18,candidates,8));
    assert(background_parse_search_filtered(fixture,strlen(fixture),candidates,8,7)==2);
    assert(background_parse_search_filtered(fixture,strlen(fixture),candidates,8,1)==1);
    assert(!strcmp(candidates[0].id,"vpewrl"));
    c.options=background_options_default();c.options.categories=7;c.options.purity=3;
    c.options.sorting=5;c.options.top_range=3;c.options.ascending=1;
    strcpy(c.options.ratios,"16x9,3x2");strcpy(c.options.resolution,"1920x1080");
    assert(background_search_url_options(&c,url,sizeof(url)));
    assert(strstr(url,"purity=011&categories=111"));assert(strstr(url,"topRange=1w"));
    assert(strstr(url,"sorting=toplist&order=asc"));assert(strstr(url,"atleast=1920x1080"));
    c.options.categories=0;assert(!background_config_valid(&c));c.options.categories=7;
    strcpy(c.options.ratios,"16x9&purity=111");assert(!background_config_valid(&c));
    strcpy(c.options.ratios,"+16x9");assert(!background_config_valid(&c));
    strcpy(c.options.ratios,"16x9,");assert(!background_config_valid(&c));
    strcpy(c.options.ratios,"16x9");c.options.position=3;assert(!background_config_valid(&c));
    c.options.position=2;assert(background_config_valid(&c));
    assert(background_api_key_valid(""));assert(background_api_key_valid("synthetic-key_1"));
    assert(!background_api_key_valid("key\r\nX-Test: bad"));
    puts("PASS background URL bounds, configurable filters, purity parsing and key validation");
}
