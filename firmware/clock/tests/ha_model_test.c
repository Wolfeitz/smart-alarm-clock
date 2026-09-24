#include "ha_model.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    assert(ha_endpoint_valid("http://192.168.1.232:8123"));
    assert(ha_endpoint_valid("https://ha.example.test"));
    assert(!ha_endpoint_valid("http://user:pass@host"));
    assert(!ha_endpoint_valid("http://host/dashboard/home"));
    assert(!ha_endpoint_valid("http://host\r\nHeader:evil"));
    assert(!ha_entity_valid("switch.bedside"));
    assert(!ha_entity_valid("light.a\"}"));
    assert(ha_entity_valid("light.bedside_2"));
    assert(!ha_token_valid("abc\r\nx"));assert(!ha_token_valid(""));
    assert(ha_token_valid("synthetic.test.token"));
    ha_light_t light={0};
    const char *valid="{\"entity_id\":\"light.bedside\",\"state\":\"on\",\"attributes\":{\"friendly_name\":\"Bedside\"}}";
    assert(ha_parse_light(valid,strlen(valid),"light.bedside",&light));
    assert(light.state==HA_ON&&!strcmp(light.name,"Bedside"));
    assert(!ha_parse_light(valid,strlen(valid),"light.other",&light));
    assert(!ha_parse_light(valid,strlen(valid)-1,"light.bedside",&light));
    const char *bad[]={"{}","[]","{\"entity_id\":\"light.bedside\",\"state\":null}","{\"entity_id\":\"light.bedside\",\"state\":\"playing\"}","{\"entity_id\":\"light.bedside\",\"state\":\"on\"}garbage"};
    for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++)assert(!ha_parse_light(bad[i],strlen(bad[i]),"light.bedside",&light));
    const char *offline="{\"entity_id\":\"light.bedside\",\"state\":\"unavailable\"}";
    assert(ha_parse_light(offline,strlen(offline),"light.bedside",&light)&&light.state==HA_UNAVAILABLE);
    puts("PASS HA endpoint/header validation, exact entity and malformed/unavailable state");
}
