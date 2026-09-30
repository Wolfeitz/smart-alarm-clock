#include "media_ha.h"
#include "ha_service.h"
#include "cJSON.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
bool media_backend_target_valid(const char *s)
{
    if(!s||strncmp(s,"media_player.",13)||!s[13]||strlen(s)>95)return false;
    for(s+=13;*s;s++)if(!((*s>='a'&&*s<='z')||(*s>='0'&&*s<='9')||*s=='_'))return false;
    return true;
}
static void text_field(const cJSON *attrs,const char *key,char *out,size_t capacity)
{
    const cJSON *v=cJSON_GetObjectItemCaseSensitive(attrs,key);if(!cJSON_IsString(v))return;
    size_t n=strlen(v->valuestring);if(n>=capacity){n=capacity-1;while(n&&((unsigned char)v->valuestring[n]&0xc0)==0x80)n--;}
    memcpy(out,v->valuestring,n);out[n]=0;for(char *p=out;*p;p++)if((unsigned char)*p<32)*p=' ';
}
bool media_ha_parse(const char *json,size_t size,const char *entity,media_player_t *out)
{
    if(!json||!out||!media_backend_target_valid(entity)||!size||size>12288||memchr(json,0,size))return false;
    for(size_t i=0;i+6<=size;i++)if(!memcmp(json+i,"\\u0000",6))return false;
    const char *end; cJSON *root=cJSON_ParseWithLengthOpts(json,size,&end,false);if(!root)return false;
    while(end<json+size&&(*end==' '||*end=='\n'||*end=='\r'||*end=='\t'))end++;
    const cJSON *id=cJSON_GetObjectItemCaseSensitive(root,"entity_id"),*state=cJSON_GetObjectItemCaseSensitive(root,"state");
    bool ok=end==json+size&&cJSON_IsObject(root)&&cJSON_IsString(id)&&!strcmp(id->valuestring,entity)&&cJSON_IsString(state);
    media_player_t p={0};const char *names[]={"unknown","off","on","idle","playing","paused","buffering","unavailable"};
    if(ok){ok=false;for(unsigned i=0;i<8;i++)if(!strcmp(state->valuestring,names[i])){p.state=i;ok=true;break;}}
    if(ok){
        const cJSON *attrs=cJSON_GetObjectItemCaseSensitive(root,"attributes"),*features=cJSON_GetObjectItemCaseSensitive(attrs,"supported_features"),*volume=cJSON_GetObjectItemCaseSensitive(attrs,"volume_level");
        if(features){double f=features->valuedouble;ok=cJSON_IsNumber(features)&&isfinite(f)&&f>=0&&f<=UINT32_MAX&&floor(f)==f;if(ok){
            uint32_t flags=(uint32_t)f;const uint32_t wire[]={16,16384,1,32,1024,1024,512};
            for(unsigned i=0;i<7;i++)if(flags&wire[i])p.capabilities|=1u<<i;
            p.relative_volume=(flags&1024)!=0;
        }}
        const cJSON *muted=cJSON_GetObjectItemCaseSensitive(attrs,"is_volume_muted");
        if(cJSON_IsBool(muted)){p.muted_known=true;p.muted=cJSON_IsTrue(muted);}
        if(cJSON_IsNumber(volume)&&isfinite(volume->valuedouble)&&volume->valuedouble>=0&&volume->valuedouble<=1){p.volume=volume->valuedouble;p.volume_known=true;}
        if(ok&&cJSON_IsNumber(features)&&((uint32_t)features->valuedouble&4)&&p.volume_known)
            p.capabilities|=(1u<<MEDIA_QUIETER)|(1u<<MEDIA_LOUDER);
        const cJSON *content=cJSON_GetObjectItemCaseSensitive(attrs,"media_content_id");
        if(cJSON_IsString(content)&&strlen(content->valuestring)<sizeof(p.content_id))strcpy(p.content_id,content->valuestring);
        text_field(attrs,"friendly_name",p.name,sizeof(p.name));text_field(attrs,"media_title",p.title,sizeof(p.title));text_field(attrs,"media_artist",p.artist,sizeof(p.artist));
    }
    if(ok)*out=p;
    cJSON_Delete(root);return ok;
}
bool media_ha_body(const char *entity,const char *id,const char *type,char *out,size_t capacity)
{
    if(!out||capacity>2147483647||!media_backend_target_valid(entity)||!media_selection_valid(id,type)||!*id)return false;
    cJSON *root=cJSON_CreateObject();if(!root)return false;
    bool ok=cJSON_AddStringToObject(root,"entity_id",entity)&&cJSON_AddStringToObject(root,"media_content_id",id)&&
        cJSON_AddStringToObject(root,"media_content_type",type)&&cJSON_PrintPreallocated(root,out,(int)capacity,false);
    cJSON_Delete(root);return ok;
}

void media_backend_config(media_backend_config_t *out)
{ha_snapshot_t h;ha_service_snapshot(&h);out->configured=h.configured;strcpy(out->identity,h.endpoint);}
bool media_backend_identity_valid(const char *identity){return ha_endpoint_valid(identity);}
static int result(int http)
{return http==200?MEDIA_BACKEND_OK:http==401||http==403?MEDIA_BACKEND_DENIED:http==404?MEDIA_BACKEND_NOT_FOUND:MEDIA_BACKEND_ERROR;}
int media_backend_read(const char *target,media_player_t *out)
{
    if(!media_backend_target_valid(target))return MEDIA_BACKEND_ERROR;
    char *response=malloc(12289);if(!response)return MEDIA_BACKEND_ERROR;
    char path[164];snprintf(path,sizeof(path),"/api/states/%s",target);size_t size=0;
    int status=result(ha_service_request(path,NULL,response,12289,&size));
    if(status==MEDIA_BACKEND_OK&&!media_ha_parse(response,size,target,out))status=MEDIA_BACKEND_ERROR;
    free(response);return status;
}
int media_backend_action(const char *target,media_action_t action,const media_player_t *current,const char *content,const char *type)
{
    if(!media_backend_target_valid(target)||!media_action_supported(current,action))return MEDIA_BACKEND_ERROR;
    char *response=malloc(12289+1152);if(!response)return MEDIA_BACKEND_ERROR;
    char *body=response+12289;char path[164];size_t size=0;
    const char *actions[]={"media_previous_track","media_play","media_pause","media_next_track","volume_down","volume_up","play_media"};
    const char *name=actions[action];snprintf(body,1152,"{\"entity_id\":\"%s\"}",target);
    if((action==MEDIA_QUIETER||action==MEDIA_LOUDER)&&!current->relative_volume){
        name="volume_set";double volume=current->volume+(action==MEDIA_LOUDER?.05:-.05);if(volume<0)volume=0;if(volume>1)volume=1;
        snprintf(body,1152,"{\"entity_id\":\"%s\",\"volume_level\":%.3f}",target,volume);
    }
    if(action==MEDIA_START_SAVED&&!media_ha_body(target,content,type,body,1152)){free(response);return MEDIA_BACKEND_ERROR;}
    snprintf(path,sizeof(path),"/api/services/media_player/%s",name);
    int status=result(ha_service_request(path,body,response,12289,&size));free(response);return status;
}
