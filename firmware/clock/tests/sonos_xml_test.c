#include "sonos_xml.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    char text[80];sonos_xml_field_t field={.path="Envelope/Body/Response/Value",.value=text,.capacity=sizeof(text)};
    const char *xml="<s:Envelope xmlns:s='soap'><s:Body><Response><Value>A &amp; B<![CDATA[ <ok>]]></Value></Response></s:Body></s:Envelope>";
    assert(sonos_xml_fields(xml,strlen(xml),&field,1)&&field.found&&!strcmp(text,"A & B <ok>"));
    assert(!sonos_xml_fields(xml,strlen(xml)-2,&field,1));
    xml="<!DOCTYPE root [<!ENTITY x 'bad'>]><root>&x;</root>";assert(!sonos_xml_fields(xml,strlen(xml),&field,1));
    xml="<Envelope><Body><Fault/></Body></Envelope>";assert(!sonos_xml_fields(xml,strlen(xml),&field,1));
    xml="<Envelope><Body><Response><Value>1</Value><Value>2</Value></Response></Body></Envelope>";assert(!sonos_xml_fields(xml,strlen(xml),&field,1));
    xml="<ZoneGroups><ZoneGroup Coordinator='RINCON_1'><ZoneGroupMember UUID='RINCON_1'/></ZoneGroup></ZoneGroups>";
    assert(sonos_xml_standalone(xml,strlen(xml),"RINCON_1"));assert(!sonos_xml_standalone(xml,strlen(xml),"RINCON_2"));
    xml="<ZoneGroups><ZoneGroup Coordinator='RINCON_1'><ZoneGroupMember UUID='RINCON_1'/><ZoneGroupMember UUID='RINCON_2'/></ZoneGroup></ZoneGroups>";
    assert(!sonos_xml_standalone(xml,strlen(xml),"RINCON_1"));
    assert(sonos_xml_escape("A&B<'\"",text,sizeof(text)));assert(!strcmp(text,"A&amp;B&lt;&apos;&quot;"));assert(!sonos_xml_escape("&",text,3));
    unsigned items;char resource[1024];
    xml="<DIDL-Lite><item id='1'><title>First</title></item><item id='2'><title>Second &amp; last</title></item></DIDL-Lite>";
    field=(sonos_xml_field_t){.path="DIDL-Lite/item/title",.value=text,.capacity=sizeof(text)};
    assert(sonos_xml_item_fields(xml,strlen(xml),1,&field,1,&items)&&items==2&&field.found&&!strcmp(text,"Second & last"));
    xml="<DIDL-Lite><item><title>A</title><title>B</title></item></DIDL-Lite>";
    assert(!sonos_xml_item_fields(xml,strlen(xml),0,&field,1,&items));
    xml="<DIDL-Lite xmlns='urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/'><item><desc><![CDATA[</item>]]></desc></item></DIDL-Lite>";
    assert(sonos_xml_add_resource(xml,"https://example.test/a?b=1&c=2","http-get:*:audio/mpeg:*",resource,sizeof(resource)));
    field=(sonos_xml_field_t){.path="DIDL-Lite/item/res",.value=text,.capacity=sizeof(text)};
    assert(sonos_xml_fields(resource,strlen(resource),&field,1)&&field.found&&!strcmp(text,"https://example.test/a?b=1&c=2"));
    assert(!sonos_xml_add_resource(resource,"other","p",text,sizeof(text)));
    assert(!sonos_xml_add_resource("<DIDL-Lite><item/><item/></DIDL-Lite>","u","p",resource,sizeof(resource)));
    assert(!sonos_xml_add_resource(xml,"u","p",resource,10));
    assert(!sonos_xml_add_resource("<DIDL-Lite><item/></DIDL-Lite>","u","p",resource,sizeof(resource)));
    assert(!sonos_xml_item_fields("<wrong/>",8,0,NULL,0,&items));
    puts("PASS Sonos XML decoding, malformed/DTD/Fault/duplicate rejection, group isolation and escaping");
}
