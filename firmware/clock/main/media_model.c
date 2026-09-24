#include "media_model.h"
#include "cJSON.h"
#include <string.h>
#include <math.h>
bool media_entity_valid(const char *s)
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
bool media_parse(const char *json,size_t size,const char *entity,media_player_t *out)
{
    if(!json||!out||!media_entity_valid(entity)||!size||size>12288)return false;
    const char *end; cJSON *root=cJSON_ParseWithLengthOpts(json,size,&end,false);if(!root)return false;
    while(end<json+size&&(*end==' '||*end=='\n'||*end=='\r'||*end=='\t'))end++;
    const cJSON *id=cJSON_GetObjectItemCaseSensitive(root,"entity_id"),*state=cJSON_GetObjectItemCaseSensitive(root,"state");
    bool ok=end==json+size&&cJSON_IsObject(root)&&cJSON_IsString(id)&&!strcmp(id->valuestring,entity)&&cJSON_IsString(state);
    media_player_t p={0};const char *names[]={"unknown","off","on","idle","playing","paused","buffering","unavailable"};
    if(ok){ok=false;for(unsigned i=0;i<8;i++)if(!strcmp(state->valuestring,names[i])){p.state=i;ok=true;break;}}
    if(ok){
        const cJSON *attrs=cJSON_GetObjectItemCaseSensitive(root,"attributes"),*features=cJSON_GetObjectItemCaseSensitive(attrs,"supported_features"),*volume=cJSON_GetObjectItemCaseSensitive(attrs,"volume_level");
        if(features){double f=features->valuedouble;ok=cJSON_IsNumber(features)&&isfinite(f)&&f>=0&&f<=UINT32_MAX&&floor(f)==f;if(ok)p.features=(uint32_t)f;}
        if(cJSON_IsNumber(volume)&&isfinite(volume->valuedouble)&&volume->valuedouble>=0&&volume->valuedouble<=1){p.volume=volume->valuedouble;p.volume_known=true;}
        text_field(attrs,"friendly_name",p.name,sizeof(p.name));text_field(attrs,"media_title",p.title,sizeof(p.title));text_field(attrs,"media_artist",p.artist,sizeof(p.artist));
    }
    if(ok)*out=p;
    cJSON_Delete(root);return ok;
}
bool media_action_supported(const media_player_t *p,media_action_t action)
{
    if(!p||p->state==MEDIA_OFF||p->state==MEDIA_UNKNOWN||p->state==MEDIA_UNAVAILABLE)return false;
    /* HA2026.9.3 MediaPlayerEntityFeature constants; PLAY differs from PLAY_MEDIA. */
    const uint32_t flags[]={16,16384,1,32,1024,1024};
    if((unsigned)action>=sizeof(flags)/sizeof(*flags))return false;
    if(action>=MEDIA_QUIETER)return (p->features&1024)||((p->features&4)&&p->volume_known);
    return (p->features&flags[action])!=0;
}
const char *media_state_name(media_state_t state)
{const char *names[]={"Unknown","Off","On","Idle","Playing","Paused","Buffering","Unavailable"};return (unsigned)state<8?names[state]:"Unknown";}
