#include "background_model.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
static bool bounded(const char *s,size_t n){if(!s)return false;for(size_t i=0;i<n;i++)if(!s[i])return true;return false;}
bool background_https_url(const char *url)
{
    if(!bounded(url,BACKGROUND_URL_SIZE)||strncmp(url,"https://",8))return false;
    const char *host=url+8,*end=strchr(host,'/');if(!end)end=host+strlen(host);
    if(host==end||*host=='.'||*host=='-'||*host==':')return false;
    for(const char *p=host;p<end;p++)if(!(isalnum((unsigned char)*p)||*p=='.'||*p=='-'||*p==':'))return false;
    for(const char *p=end;*p;p++)if((unsigned char)*p<=32||(unsigned char)*p>=127||*p=='\\'||*p=='#')return false;
    return true;
}
static bool image_id(const char *s)
{
    if(!s||strlen(s)!=6)return false;
    for(unsigned i=0;i<6;i++)if(!((s[i]>='a'&&s[i]<='z')||(s[i]>='0'&&s[i]<='9')))return false;
    return true;
}
background_options_t background_options_default(void)
{return (background_options_t){.version=1,.categories=4,.purity=4,.retries=3,.retry_seconds=5};}
bool background_api_key_valid(const char *key)
{
    if(!bounded(key,129))return false;
    for(const unsigned char *p=(const unsigned char *)key;*p;p++)if(!isalnum(*p)&&*p!='-'&&*p!='_')return false;
    return true;
}
static bool dimensions(const char *text,size_t limit,bool list)
{
    if(!bounded(text,limit))return false;
    if(!*text)return true;
    for(const char *v=text;*v;v++)if(!isdigit((unsigned char)*v)&&*v!='x'&&*v!=',')return false;
    unsigned a=0,b=0;
    const char *p=text;
    while(*p){
        int n=0;if(sscanf(p,"%ux%u%n",&a,&b,&n)!=2||!n||!a||!b||a>16384||b>16384)return false;
        p+=n;if(!*p)return true;if(!list||*p++!=','||!*p)return false;
    }
    return false;
}
bool background_options_valid(const background_options_t *o)
{
    if(!o->version)return true;
    if(o->version!=1||!o->categories||o->categories>7||!o->purity||o->purity>7||o->sorting>5||o->ascending>1||o->top_range>6||o->position>2||o->retries>10||o->retry_seconds<2||o->retry_seconds>3600||o->notify_refresh>1||o->notify_error>1||o->exact_resolution>1)return false;
    if(!dimensions(o->resolution,sizeof(o->resolution),o->exact_resolution)||!dimensions(o->ratios,sizeof(o->ratios),true)||!bounded(o->color,sizeof(o->color))||!bounded(o->seed,sizeof(o->seed)))return false;
    if(*o->color){if(strlen(o->color)!=6)return false;for(const char *p=o->color;*p;p++)if(!isxdigit((unsigned char)*p))return false;}
    if(*o->seed){if(strlen(o->seed)!=6)return false;for(const char *p=o->seed;*p;p++)if(!isalnum((unsigned char)*p))return false;}
    return true;
}
bool background_config_valid(const background_config_t *c)
{
    if(!c||!background_options_valid(&c->options)||c->source<BACKGROUND_LOCAL||c->source>BACKGROUND_WALLHAVEN||c->count>BACKGROUND_MAX_IMAGES||
       (c->interval_seconds&&(c->interval_seconds<60||c->interval_seconds>86400))||!bounded(c->query,sizeof(c->query)))return false;
    for(const unsigned char *p=(const unsigned char *)c->query;*p;p++)if(*p<32||*p>=127)return false;
    if(c->source==BACKGROUND_WALLHAVEN)return c->count==0;
    if(!c->count)return false;
    for(unsigned i=0;i<c->count;i++){
        if(!bounded(c->images[i],sizeof(c->images[i]))||!c->images[i][0])return false;
        if(c->source==BACKGROUND_SELECTED){if(!background_https_url(c->images[i]))return false;}
        else for(const unsigned char *p=(const unsigned char *)c->images[i];*p;p++)
            if(!(isalnum(*p)||*p=='_'||*p=='-'))return false;
    }
    return true;
}
bool background_selected_url(const char *input,char *out,size_t capacity)
{
    if(!out||!capacity||!background_https_url(input))return false;
    const char *prefix="https://wallhaven.cc/w/";
    if(!strncmp(input,prefix,strlen(prefix))){
        const char *id=input+strlen(prefix);if(!image_id(id))return false;
        return snprintf(out,capacity,"https://wallhaven.cc/api/v1/w/%s",id)<(int)capacity;
    }
    return snprintf(out,capacity,"%s",input)<(int)capacity;
}
bool background_search_url_options(const background_config_t *config,char *out,size_t capacity)
{
    if(!config||!background_options_valid(&config->options))return false;
    const char *query=config->query;
    if(!out||!capacity||!bounded(query,BACKGROUND_QUERY_SIZE))return false;
    char encoded[BACKGROUND_QUERY_SIZE*3];size_t n=0;
    const char hex[]="0123456789ABCDEF";
    for(const unsigned char *p=(const unsigned char *)query;*p;p++){
        if(*p<32||*p>=127)return false;
        if(isalnum(*p)||*p=='-'||*p=='_'||*p=='.'||*p=='~')encoded[n++]=(char)*p;
        else{encoded[n++]='%';encoded[n++]=hex[*p>>4];encoded[n++]=hex[*p&15];}
    }
    encoded[n]=0;
    background_options_t o=config->options.version?config->options:background_options_default();
    const char *sort[]={"random","date_added","relevance","views","favorites","toplist"};
    const char *ranges[]={"1M","1d","3d","1w","3M","6M","1y"};
    int written=snprintf(out,capacity,"https://wallhaven.cc/api/v1/search?purity=%u%u%u&categories=%u%u%u&sorting=%s&order=%s&q=%s%s%s%s%s%s%s%s%s%s%s",
        !!(o.purity&4),!!(o.purity&2),!!(o.purity&1),!!(o.categories&4),!!(o.categories&2),!!(o.categories&1),sort[o.sorting],o.ascending?"asc":"desc",encoded,
        *o.resolution?(o.exact_resolution?"&resolutions=":"&atleast="):"",o.resolution,
        *o.ratios?"&ratios=":"",o.ratios,*o.color?"&colors=":"",o.color,
        *o.seed?"&seed=":"",o.seed,o.sorting==5?"&topRange=":"",o.sorting==5?ranges[o.top_range]:"");
    return written>=0&&(size_t)written<capacity;
}
bool background_search_url(const char *query,char *out,size_t capacity)
{
    if(!bounded(query,BACKGROUND_QUERY_SIZE))return false;
    background_config_t config={0};strcpy(config.query,query);return background_search_url_options(&config,out,capacity);
}
unsigned background_parse_search_filtered(const char *json,size_t size,background_candidate_t *out,unsigned capacity,unsigned allowed)
{
    if(!json||!out||!capacity||!size||size>BACKGROUND_JSON_LIMIT||memchr(json,0,size))return 0;
    const char *end=NULL;cJSON *root=cJSON_ParseWithLengthOpts(json,size,&end,false);if(!root)return 0;
    while(end<json+size&&isspace((unsigned char)*end))end++;
    if(end!=json+size){cJSON_Delete(root);return 0;}
    unsigned count=0;cJSON *data=cJSON_GetObjectItemCaseSensitive(root,"data"),*item=NULL;
    for(item=cJSON_IsArray(data)?data->child:cJSON_IsObject(data)?data:NULL;item;item=cJSON_IsArray(data)?item->next:NULL){
        cJSON *id=cJSON_GetObjectItemCaseSensitive(item,"id"),*purity=cJSON_GetObjectItemCaseSensitive(item,"purity");
        cJSON *thumbs=cJSON_GetObjectItemCaseSensitive(item,"thumbs"),*url=cJSON_GetObjectItemCaseSensitive(thumbs,"large");
        if(!cJSON_IsString(id)||!image_id(id->valuestring)||!cJSON_IsString(purity)||!cJSON_IsString(url))continue;
        unsigned flag=!strcmp(purity->valuestring,"sfw")?4:!strcmp(purity->valuestring,"sketchy")?2:!strcmp(purity->valuestring,"nsfw")?1:0;
        if(!(flag&allowed))continue;
        /* Construct the expected provider CDN URL; never trust a response redirect. */
        char expected[96];snprintf(expected,sizeof(expected),"https://th.wallhaven.cc/lg/%.2s/%s.jpg",id->valuestring,id->valuestring);
        if(strcmp(url->valuestring,expected))continue;
        bool duplicate=false;for(unsigned i=0;i<count;i++)if(!strcmp(out[i].id,id->valuestring))duplicate=true;
        if(duplicate)continue;
        strcpy(out[count].id,id->valuestring);strcpy(out[count].url,expected);
        if(++count==capacity)break;
    }
    cJSON_Delete(root);return count;
}

unsigned background_parse_search(const char *json,size_t size,background_candidate_t *out,unsigned capacity)
{return background_parse_search_filtered(json,size,out,capacity,4);}
