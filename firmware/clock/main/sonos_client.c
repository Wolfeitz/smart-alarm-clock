#include "sonos_client.h"
#include "sonos_xml.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
static bool uuid_valid(const char *id)
{
    if(!id||strncmp(id,"RINCON_",7)||strlen(id)<8||strlen(id)>47)return false;
    for(const char *p=id;*p;p++)if(!isalnum((unsigned char)*p)&&*p!='_')return false;
    return true;
}
bool sonos_endpoint_valid(const char *endpoint)
{
    if(!endpoint||strlen(endpoint)>47)return false;
    for(const char *p=endpoint;*p;p++)if(!isdigit((unsigned char)*p)&&*p!='.'&&*p!=':')return false;
    unsigned a,b,c,d,port;int n=0;
    if(sscanf(endpoint,"%u.%u.%u.%u:%u%n",&a,&b,&c,&d,&port,&n)!=5||endpoint[n]||a>255||b>255||c>255||d>255||!port||port>65535)return false;
    char canonical[48];snprintf(canonical,sizeof(canonical),"%u.%u.%u.%u:%u",a,b,c,d,port);
    if(strcmp(canonical,endpoint))return false;
    return a==10||a==127||(a==172&&b>=16&&b<=31)||(a==192&&b==168)||(a==169&&b==254);
}
bool sonos_target_parse(const char *target,sonos_device_t *out)
{
    if(!target||!out||strncmp(target,"sonos:",6)||strlen(target)>=96)return false;
    const char *slash=strchr(target+6,'/');if(!slash||slash-target-6>=48)return false;
    sonos_device_t device={0};memcpy(device.endpoint,target+6,slash-target-6);
    if(!sonos_endpoint_valid(device.endpoint)||!uuid_valid(slash+1))return false;
    strcpy(device.uuid,slash+1);*out=device;return true;
}
bool sonos_target_format(const sonos_device_t *device,char out[96])
{
    if(!device||!sonos_endpoint_valid(device->endpoint)||!uuid_valid(device->uuid))return false;
    int n=snprintf(out,96,"sonos:%s/%s",device->endpoint,device->uuid);return n>0&&n<96;
}
static int request(sonos_client_t *c,const char *endpoint,const char *path,const char *action,const char *body)
{
    if(!c||!c->http||!sonos_endpoint_valid(endpoint))return SONOS_ERROR;
    if(c->allowed&&!c->allowed(c->allowed_context))return SONOS_CANCELLED;
    c->response_size=0;c->response[0]=0;
    int status=c->http(c->context,endpoint,path,action,body,c->response,sizeof(c->response),&c->response_size);
    if(status!=200||!c->response_size||c->response_size>=sizeof(c->response)||memchr(c->response,0,c->response_size))return SONOS_ERROR;
    c->response[c->response_size]=0;return SONOS_OK;
}
static bool append(char *out,size_t capacity,const char *text)
{size_t n=strlen(out),m=strlen(text);if(m>=capacity-n)return false;memcpy(out+n,text,m+1);return true;}
static bool parameter(char *out,size_t capacity,const char *name,const char *value)
{
    if(!append(out,capacity,"<")||!append(out,capacity,name)||!append(out,capacity,">"))return false;
    size_t n=strlen(out);if(!sonos_xml_escape(value,out+n,capacity-n))return false;
    return append(out,capacity,"</")&&append(out,capacity,name)&&append(out,capacity,">");
}
typedef struct {const char *name,*value;} argument_t;
static int soap(sonos_client_t *c,const sonos_device_t *d,const char *service,const char *action,const argument_t *args,unsigned count)
{
    char path[128],header[160];
    snprintf(path,sizeof(path),"/%s/%s/Control",!strcmp(service,"AVTransport")||!strcmp(service,"RenderingControl")?"MediaRenderer":"",service);
    if(!strcmp(service,"ZoneGroupTopology"))strcpy(path,"/ZoneGroupTopology/Control");
    if(!strcmp(service,"ContentDirectory"))strcpy(path,"/MediaServer/ContentDirectory/Control");
    snprintf(header,sizeof(header),"urn:schemas-upnp-org:service:%s:1#%s",service,action);
    snprintf(c->body,sizeof(c->body),"<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:%s xmlns:u=\"urn:schemas-upnp-org:service:%s:1\">",action,service);
    for(unsigned i=0;i<count;i++)if(!parameter(c->body,sizeof(c->body),args[i].name,args[i].value))return SONOS_ERROR;
    char end[128];snprintf(end,sizeof(end),"</u:%s></s:Body></s:Envelope>",action);
    if(!append(c->body,sizeof(c->body),end))return SONOS_ERROR;
    int result=request(c,d->endpoint,path,header,c->body);if(result!=SONOS_OK)return result;
    char response_path[128];snprintf(response_path,sizeof(response_path),"Envelope/Body/%sResponse",action);
    /* Detect the actual response element, not merely an HTTP acknowledgment. */
    sonos_xml_field_t field={.path=response_path};
    /* Response text is whitespace; querying its existence separately tolerates it. */
    char whitespace[512];field.value=whitespace;field.capacity=sizeof(whitespace);
    return sonos_xml_fields(c->response,c->response_size,&field,1)&&field.found?SONOS_OK:SONOS_ERROR;
}
static bool field(sonos_client_t *c,const char *action,const char *name,char *out,size_t capacity)
{
    char path[160];snprintf(path,sizeof(path),"Envelope/Body/%sResponse/%s",action,name);
    sonos_xml_field_t f={.path=path,.value=out,.capacity=capacity};
    return sonos_xml_fields(c->response,c->response_size,&f,1)&&f.found;
}
static bool number(const char *text,unsigned max,unsigned *value)
{
    if(!*text)return false;
    unsigned n=0;
    for(;*text;text++){if(*text<'0'||*text>'9'||n>max/10)return false;n=n*10+*text-'0';if(n>max)return false;}
    *value=n;return true;
}
int sonos_probe(sonos_client_t *c,const char *endpoint,sonos_device_t *out)
{
    int result=request(c,endpoint,"/xml/device_description.xml",NULL,NULL);if(result)return result;
    char udn[64],maker[80],kind[96],name[64];
    sonos_xml_field_t fields[]={
        {.path="root/device/UDN",.value=udn,.capacity=sizeof(udn)},
        {.path="root/device/manufacturer",.value=maker,.capacity=sizeof(maker)},
        {.path="root/device/deviceType",.value=kind,.capacity=sizeof(kind)},
        {.path="root/device/roomName",.value=name,.capacity=sizeof(name)}};
    if(!sonos_xml_fields(c->response,c->response_size,fields,4)||!fields[0].found||!fields[1].found||!fields[2].found||
        strncmp(udn,"uuid:",5)||!uuid_valid(udn+5)||!strstr(maker,"Sonos")||strcmp(kind,"urn:schemas-upnp-org:device:ZonePlayer:1"))return SONOS_ERROR;
    sonos_device_t d={0};strcpy(d.endpoint,endpoint);strcpy(d.uuid,udn+5);snprintf(d.name,sizeof(d.name),"%s",fields[3].found&&*name?name:"Sonos speaker");*out=d;return SONOS_OK;
}
static int verify(sonos_client_t *c,const sonos_device_t *d)
{
    if(!uuid_valid(d->uuid))return SONOS_ERROR;
    sonos_device_t actual;int result=sonos_probe(c,d->endpoint,&actual);if(result)return result;
    if(strcmp(actual.uuid,d->uuid))return SONOS_IDENTITY_CHANGED;
    result=soap(c,d,"ZoneGroupTopology","GetZoneGroupState",NULL,0);if(result)return result;
    /* Keep the decoded topology outside response, which remains the source. */
    if(!field(c,"GetZoneGroupState","ZoneGroupState",c->body,sizeof(c->body)))return SONOS_ERROR;
    return sonos_xml_standalone(c->body,strlen(c->body),d->uuid)?SONOS_OK:SONOS_GROUP_UNSUPPORTED;
}
int sonos_read(sonos_client_t *c,const sonos_device_t *d,media_player_t *out)
{
    int result=verify(c,d);if(result)return result;
    media_player_t p={0};snprintf(p.name,sizeof(p.name),"%s",d->name[0]?d->name:"Sonos speaker");
    argument_t instance[]={ {"InstanceID","0"} };char value[80];
    result=soap(c,d,"AVTransport","GetTransportInfo",instance,1);if(result)return result;
    if(!field(c,"GetTransportInfo","CurrentTransportState",value,sizeof(value)))return SONOS_ERROR;
    p.state=!strcmp(value,"PLAYING")?MEDIA_PLAYING:!strcmp(value,"PAUSED_PLAYBACK")?MEDIA_PAUSED:!strcmp(value,"STOPPED")?MEDIA_IDLE:!strcmp(value,"TRANSITIONING")?MEDIA_BUFFERING:MEDIA_UNKNOWN;
    argument_t channel[]={ {"InstanceID","0"},{"Channel","Master"} };unsigned n;
    result=soap(c,d,"RenderingControl","GetVolume",channel,2);if(result)return result;
    if(!field(c,"GetVolume","CurrentVolume",value,sizeof(value))||!number(value,100,&n))return SONOS_ERROR;
    p.volume=n/100.;p.volume_known=true;
    result=soap(c,d,"RenderingControl","GetMute",channel,2);if(result)return result;
    if(!field(c,"GetMute","CurrentMute",value,sizeof(value))||!number(value,1,&n))return SONOS_ERROR;
    p.muted=n;p.muted_known=true;
    result=soap(c,d,"AVTransport","GetMediaInfo",instance,1);if(result)return result;
    if(!field(c,"GetMediaInfo","CurrentURI",p.content_id,sizeof(p.content_id)))return SONOS_ERROR;
    p.capabilities=127;*out=p;return SONOS_OK;
}
static int transport(sonos_client_t *c,const sonos_device_t *d,const char *action)
{argument_t args[]={{"InstanceID","0"},{"Speed","1"}};return soap(c,d,"AVTransport",action,args,!strcmp(action,"Play")?2:1);}
int sonos_stop(sonos_client_t *c,const sonos_device_t *d)
{int result=verify(c,d);return result?result:transport(c,d,"Stop");}
int sonos_action(sonos_client_t *c,const sonos_device_t *d,media_action_t action,const media_player_t *current,const char *uri,const char *metadata)
{
    if(action<MEDIA_PREVIOUS||action>MEDIA_START_SAVED)return SONOS_ERROR;
    int result=verify(c,d);if(result)return result;
    if(action==MEDIA_QUIETER||action==MEDIA_LOUDER){
        if(!current||!current->volume_known||!isfinite(current->volume)||current->volume<0||current->volume>1)return SONOS_ERROR;
        int volume=(int)(current->volume*100+.5)+(action==MEDIA_LOUDER?5:-5);if(volume<0)volume=0;if(volume>100)volume=100;
        char text[8];snprintf(text,sizeof(text),"%d",volume);
        argument_t args[]={{"InstanceID","0"},{"Channel","Master"},{"DesiredVolume",text}};
        return soap(c,d,"RenderingControl","SetVolume",args,3);
    }
    if(action==MEDIA_START_SAVED){
        if(!uri||!*uri||strlen(uri)>=384||!metadata||strlen(metadata)>4096)return SONOS_ERROR;
        argument_t args[]={{"InstanceID","0"},{"CurrentURI",uri},{"CurrentURIMetaData",metadata}};
        result=soap(c,d,"AVTransport","SetAVTransportURI",args,3);if(result)return result;
        return transport(c,d,"Play");
    }
    const char *actions[]={"Previous","Play","Pause","Next"};return transport(c,d,actions[action]);
}
static bool favorite_id(const char *id)
{
    if(!id||strncmp(id,"FV:2/",5)||!id[5]||strlen(id)>=96)return false;
    for(const char *p=id+5;*p;p++)if(!isalnum((unsigned char)*p)&&*p!='_'&&*p!='-')return false;
    return true;
}
static int browse(sonos_client_t *c,const sonos_device_t *d,const char *object,bool metadata,unsigned start,unsigned count,unsigned *returned,unsigned *total)
{
    char offset[16],limit[16];snprintf(offset,sizeof(offset),"%u",start);snprintf(limit,sizeof(limit),"%u",count);
    argument_t args[]={{"ObjectID",object},{"BrowseFlag",metadata?"BrowseMetadata":"BrowseDirectChildren"},
        {"Filter","*"},{"StartingIndex",offset},{"RequestedCount",limit},{"SortCriteria",""}};
    int result=soap(c,d,"ContentDirectory","Browse",args,6);if(result)return result;
    char value[16];
    if(!field(c,"Browse","NumberReturned",value,sizeof(value))||!number(value,count,returned)||
       !field(c,"Browse","TotalMatches",value,sizeof(value))||!number(value,10000,total)||*returned>*total||
       !field(c,"Browse","Result",c->body,sizeof(c->body)))return SONOS_ERROR;
    return SONOS_OK;
}
int sonos_favorites(sonos_client_t *c,const sonos_device_t *d,unsigned start,sonos_favorites_t *out)
{
    if(!out||start>10000)return SONOS_ERROR;
    int result=verify(c,d);if(result)return result;
    unsigned count,total;
    result=browse(c,d,"FV:2",false,start,SONOS_FAVORITES_PAGE,&count,&total);if(result)return result;
    sonos_favorites_t page={.count=count,.total=total,.start=start};
    for(unsigned i=0;i<count;i++){
        sonos_favorite_t *f=&page.items[i];unsigned items;
        sonos_xml_field_t fields[]={
            {.path="DIDL-Lite/item",.attribute="id",.value=f->id,.capacity=sizeof(f->id)},
            {.path="DIDL-Lite/item/title",.value=f->title,.capacity=sizeof(f->title)}};
        if(!sonos_xml_item_fields(c->body,strlen(c->body),i,fields,2,&items)||items!=count||
           !fields[0].found||!fields[1].found||!favorite_id(f->id)||!f->title[0])return SONOS_ERROR;
        for(unsigned j=0;j<i;j++)if(!strcmp(page.items[j].id,f->id))return SONOS_ERROR;
    }
    if(!count){unsigned items;if(!sonos_xml_item_fields(c->body,strlen(c->body),0,NULL,0,&items)||items)return SONOS_ERROR;}
    *out=page;return SONOS_OK;
}
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#define favorite_alloc(n) heap_caps_calloc(1,n,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT)
#else
#define favorite_alloc(n) calloc(1,n)
#endif
int sonos_play_favorite(sonos_client_t *c,const sonos_device_t *d,const char *id)
{
    if(!favorite_id(id))return SONOS_ERROR;
    int result=verify(c,d);if(result)return result;
    unsigned count,total;
    result=browse(c,d,id,true,0,1,&count,&total);if(result)return result;
    if(count!=1||total!=1)return SONOS_ERROR;
    typedef struct {char id[96],uri[384],protocol[256],raw[4097],metadata[4097];} selection_t;
    selection_t *s=favorite_alloc(sizeof(*s));if(!s)return SONOS_ERROR;
    sonos_xml_field_t fields[]={
        {.path="DIDL-Lite/item",.attribute="id",.value=s->id,.capacity=sizeof(s->id)},
        {.path="DIDL-Lite/item/res",.value=s->uri,.capacity=sizeof(s->uri)},
        {.path="DIDL-Lite/item/res",.attribute="protocolInfo",.value=s->protocol,.capacity=sizeof(s->protocol)},
        {.path="DIDL-Lite/item/resMD",.value=s->raw,.capacity=sizeof(s->raw)}};
    unsigned items;result=SONOS_ERROR;
    if(!sonos_xml_item_fields(c->body,strlen(c->body),0,fields,4,&items)||items!=1)goto done;
    for(unsigned i=0;i<4;i++)if(!fields[i].found)goto done;
    if(strcmp(s->id,id)||!s->uri[0]||!s->protocol[0]||!sonos_xml_add_resource(s->raw,s->uri,s->protocol,s->metadata,sizeof(s->metadata)))goto done;
    bool radio=!strncmp(s->uri,"x-sonosapi-stream:",17)||!strncmp(s->uri,"x-sonosapi-radio:",16)||
        !strncmp(s->uri,"x-rincon-mp3radio:",17)||!strncmp(s->uri,"hls-radio:",10);
    if(radio){
        argument_t args[]={{"InstanceID","0"},{"CurrentURI",s->uri},{"CurrentURIMetaData",s->metadata}};
        result=soap(c,d,"AVTransport","SetAVTransportURI",args,3);
    }else{
        argument_t args[]={{"InstanceID","0"},{"EnqueuedURI",s->uri},{"EnqueuedURIMetaData",s->metadata},
            {"DesiredFirstTrackNumberEnqueued","0"},{"EnqueueAsNext","0"}};
        result=soap(c,d,"AVTransport","AddURIToQueue",args,5);if(result)goto done;
        char position[16];unsigned track;
        if(!field(c,"AddURIToQueue","FirstTrackNumberEnqueued",position,sizeof(position))||!number(position,100000,&track)||!track){result=SONOS_ERROR;goto done;}
        char queue_uri[96];snprintf(queue_uri,sizeof(queue_uri),"x-rincon-queue:%s#0",d->uuid);
        argument_t set[]={{"InstanceID","0"},{"CurrentURI",queue_uri},{"CurrentURIMetaData",""}};
        result=soap(c,d,"AVTransport","SetAVTransportURI",set,3);if(result)goto done;
        argument_t seek[]={{"InstanceID","0"},{"Unit","TRACK_NR"},{"Target",position}};
        result=soap(c,d,"AVTransport","Seek",seek,3);
    }
    if(result==SONOS_OK)result=transport(c,d,"Play");
 done:
    free(s);return result;
}
