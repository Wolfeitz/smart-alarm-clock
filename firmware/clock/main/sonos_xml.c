#include "sonos_xml.h"
#include <expat.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
static void *xml_alloc(size_t n){return heap_caps_malloc(n,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);}
static void *xml_resize(void *p,size_t n){return heap_caps_realloc(p,n,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);}
#else
#define xml_alloc malloc
#define xml_resize realloc
#endif
#define XML_LIMIT 32768
static const char *local(const char *name){const char *p=strrchr(name,'|');return p?p+1:name;}
typedef struct {
    XML_Parser parser;bool error;
    char path[512];size_t lengths[20];unsigned depth;
    sonos_xml_field_t *fields;unsigned count;
    const char *uuid;unsigned groups,members;bool in_group;
    bool select_item,didl_root;unsigned item_index,items;long item_end;
} context_t;
static void fail(context_t *c){c->error=true;XML_StopParser(c->parser,XML_FALSE);}
static const char *attribute(const char **attrs,const char *name)
{for(unsigned i=0;attrs[i];i+=2)if(!strcmp(local(attrs[i]),name))return attrs[i+1];return NULL;}
static bool copy(char *out,size_t cap,const char *value)
{size_t n=strlen(value);if(n>=cap)return false;memcpy(out,value,n+1);return true;}
static void XMLCALL start(void *user,const char *name,const char **attrs)
{
    context_t *c=user;name=local(name);size_t n=strlen(c->path),m=strlen(name);
    if(c->depth>=20||n+m+2>=sizeof(c->path)){fail(c);return;}
    c->lengths[c->depth++]=n;if(n)c->path[n++]='/';memcpy(c->path+n,name,m+1);
    if(c->depth==1&&!strcmp(c->path,"DIDL-Lite"))c->didl_root=true;
    if(!strcmp(c->path,"DIDL-Lite/item"))c->items++;
    if(!strcmp(name,"Fault")){fail(c);return;}
    if(c->uuid){
        if(!strcmp(c->path,"ZoneGroups/ZoneGroup")){
            const char *coordinator=attribute(attrs,"Coordinator");
            c->in_group=coordinator&&!strcmp(coordinator,c->uuid);
            if(c->in_group)c->groups++;
        }else if(c->in_group&&(!strcmp(name,"ZoneGroupMember")||!strcmp(name,"Satellite"))){
            c->members++;const char *id=attribute(attrs,"UUID");if(!id||strcmp(id,c->uuid))fail(c);
        }
    }
    if(c->select_item&&c->items!=c->item_index+1)return;
    for(unsigned i=0;i<c->count;i++){
        sonos_xml_field_t *f=&c->fields[i];if(strcmp(f->path,c->path))continue;
        if(f->found){fail(c);return;}f->found=true;f->value[0]=0;
        if(f->attribute){const char *value=attribute(attrs,f->attribute);if(!value||!copy(f->value,f->capacity,value)){fail(c);return;}}
    }
}
static void XMLCALL end(void *user,const char *name)
{
    (void)name;context_t *c=user;if(!c->depth){fail(c);return;}
    if(!strcmp(c->path,"DIDL-Lite/item"))c->item_end=XML_GetCurrentByteCount(c->parser)>0?XML_GetCurrentByteIndex(c->parser):-1;
    if(!strcmp(c->path,"ZoneGroups/ZoneGroup"))c->in_group=false;
    c->path[c->lengths[--c->depth]]=0;
}
static void XMLCALL text_data(void *user,const char *text,int count)
{
    context_t *c=user;
    if(c->select_item&&c->items!=c->item_index+1)return;
    for(unsigned i=0;i<c->count;i++){
        sonos_xml_field_t *f=&c->fields[i];if(f->attribute||strcmp(c->path,f->path))continue;
        size_t n=strlen(f->value);
        if((size_t)count>=f->capacity-n){fail(c);return;}
        memcpy(f->value+n,text,count);f->value[n+count]=0;
    }
}
static void XMLCALL doctype(void *user,const char *name,const char *sys,const char *pub,int subset)
{(void)name;(void)sys;(void)pub;(void)subset;fail(user);}
static bool parse(context_t *c,const char *xml,size_t size)
{
    if(!xml||!size||size>XML_LIMIT||memchr(xml,0,size))return false;
    XML_Memory_Handling_Suite memory={xml_alloc,xml_resize,free};
    c->parser=XML_ParserCreate_MM(NULL,&memory,"|");if(!c->parser)return false;
    XML_SetUserData(c->parser,c);XML_SetElementHandler(c->parser,start,end);XML_SetCharacterDataHandler(c->parser,text_data);
    XML_SetStartDoctypeDeclHandler(c->parser,doctype);XML_SetParamEntityParsing(c->parser,XML_PARAM_ENTITY_PARSING_NEVER);
    bool ok=XML_Parse(c->parser,xml,(int)size,XML_TRUE)==XML_STATUS_OK&&!c->error&&!c->depth;
    XML_ParserFree(c->parser);return ok;
}
bool sonos_xml_fields(const char *xml,size_t size,sonos_xml_field_t *fields,unsigned count)
{
    for(unsigned i=0;i<count;i++){if(!fields[i].value||!fields[i].capacity)return false;fields[i].found=false;fields[i].value[0]=0;}
    context_t c={.fields=fields,.count=count};return parse(&c,xml,size);
}
bool sonos_xml_standalone(const char *xml,size_t size,const char *uuid)
{context_t c={.uuid=uuid};return parse(&c,xml,size)&&c.groups==1&&c.members==1;}
bool sonos_xml_escape(const char *text,char *out,size_t capacity)
{
    if(!text||!out||!capacity)return false;
    size_t n=0;
    for(const unsigned char *p=(const unsigned char *)text;*p;p++){
        if(*p<32&&*p!='\n'&&*p!='\r'&&*p!='\t')return false;
        const char *entity=*p=='&'?"&amp;":*p=='<'?"&lt;":*p=='>'?"&gt;":*p=='\"'?"&quot;":*p=='\''?"&apos;":NULL;
        size_t size=entity?strlen(entity):1;if(size>=capacity-n)return false;
        if(entity)memcpy(out+n,entity,size);else out[n]=*p;n+=size;
    }
    out[n]=0;return true;
}

bool sonos_xml_item_fields(const char *xml,size_t size,unsigned index,sonos_xml_field_t *fields,unsigned count,unsigned *items)
{
    if(!items)return false;
    *items=0;
    for(unsigned i=0;i<count;i++){if(!fields[i].value||!fields[i].capacity)return false;fields[i].found=false;fields[i].value[0]=0;}
    context_t c={.fields=fields,.count=count,.select_item=true,.item_index=index};
    if(!parse(&c,xml,size)||!c.didl_root)return false;
    *items=c.items;return true;
}
bool sonos_xml_add_resource(const char *xml,const char *uri,const char *protocol,char *out,size_t capacity)
{
    if(!xml||!uri||!protocol||!out||out==xml)return false;
    char existing[384];sonos_xml_field_t f={.path="DIDL-Lite/item/res",.value=existing,.capacity=sizeof(existing)};
    context_t c={.fields=&f,.count=1};size_t size=strlen(xml);
    if(!parse(&c,xml,size)||c.items!=1||f.found||c.item_end<0||(size_t)c.item_end>=size||xml[c.item_end]!='<')return false;
    size_t n=(size_t)c.item_end;
    const char *start="<res xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" protocolInfo=\"";
    if(n+strlen(start)>=capacity)return false;
    memcpy(out,xml,n);strcpy(out+n,start);n+=strlen(start);
    if(!sonos_xml_escape(protocol,out+n,capacity-n))return false;
    n+=strlen(out+n);if(n+2>=capacity)return false;strcpy(out+n,"\">");n+=2;
    if(!sonos_xml_escape(uri,out+n,capacity-n))return false;
    n+=strlen(out+n);if(n+6+size-(size_t)c.item_end>=capacity)return false;
    strcpy(out+n,"</res>");n+=6;strcpy(out+n,xml+c.item_end);
    return true;
}
