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
bool background_config_valid(const background_config_t *c)
{
    if(!c||c->source<BACKGROUND_LOCAL||c->source>BACKGROUND_WALLHAVEN||c->count>BACKGROUND_MAX_IMAGES||
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
bool background_search_url(const char *query,char *out,size_t capacity)
{
    if(!out||!capacity||!bounded(query,BACKGROUND_QUERY_SIZE))return false;
    char encoded[BACKGROUND_QUERY_SIZE*3];size_t n=0;
    const char hex[]="0123456789ABCDEF";
    for(const unsigned char *p=(const unsigned char *)query;*p;p++){
        if(*p<32||*p>=127)return false;
        if(isalnum(*p)||*p=='-'||*p=='_'||*p=='.'||*p=='~')encoded[n++]=(char)*p;
        else{encoded[n++]='%';encoded[n++]=hex[*p>>4];encoded[n++]=hex[*p&15];}
    }
    encoded[n]=0;
    return snprintf(out,capacity,"https://wallhaven.cc/api/v1/search?purity=100&categories=100&sorting=random&q=%s",encoded)<(int)capacity;
}
unsigned background_parse_search(const char *json,size_t size,background_candidate_t *out,unsigned capacity)
{
    if(!json||!out||!capacity||!size||size>BACKGROUND_JSON_LIMIT||memchr(json,0,size))return 0;
    const char *end=NULL;cJSON *root=cJSON_ParseWithLengthOpts(json,size,&end,false);if(!root)return 0;
    while(end<json+size&&isspace((unsigned char)*end))end++;
    if(end!=json+size){cJSON_Delete(root);return 0;}
    unsigned count=0;cJSON *data=cJSON_GetObjectItemCaseSensitive(root,"data"),*item=NULL;
    if(cJSON_IsArray(data))cJSON_ArrayForEach(item,data){
        cJSON *id=cJSON_GetObjectItemCaseSensitive(item,"id"),*purity=cJSON_GetObjectItemCaseSensitive(item,"purity");
        cJSON *thumbs=cJSON_GetObjectItemCaseSensitive(item,"thumbs"),*url=cJSON_GetObjectItemCaseSensitive(thumbs,"large");
        if(!cJSON_IsString(id)||!image_id(id->valuestring)||!cJSON_IsString(purity)||strcmp(purity->valuestring,"sfw")||!cJSON_IsString(url))continue;
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
