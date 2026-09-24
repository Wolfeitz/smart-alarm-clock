#include "ha_model.h"
#include "cJSON.h"
#include <string.h>
bool ha_entity_valid(const char *s)
{
    if(!s||strncmp(s,"light.",6)||!s[6]||strlen(s)>95)return false;
    for(s+=6;*s;s++)if(!((*s>='a'&&*s<='z')||(*s>='0'&&*s<='9')||*s=='_'))return false;
    return true;
}
bool ha_endpoint_valid(const char *s)
{
    if(!s||strlen(s)>191)return false;
    const char *host=!strncmp(s,"https://",8)?s+8:!strncmp(s,"http://",7)?s+7:NULL;
    if(!host||!*host)return false;
    /* Root endpoint only: no userinfo, query, fragment or path in credential target. */
    for(const char *p=host;*p;p++)if(!((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9')||*p=='.'||*p=='-'||*p==':'||*p=='['||*p==']'))return false;
    return true;
}
bool ha_token_valid(const char *s)
{
    if(!s||!s[0]||strlen(s)>511)return false;
    for(;*s;s++)if((unsigned char)*s<=32||(unsigned char)*s>=127)return false;
    return true;
}
bool ha_parse_light(const char *json,size_t length,const char *entity,ha_light_t *out)
{
    if(!json||!out||!ha_entity_valid(entity)||!length||length>12288)return false;
    const char *end=NULL;cJSON *root=cJSON_ParseWithLengthOpts(json,length,&end,false);if(!root)return false;
    while(end<json+length&&(*end==' '||*end=='\r'||*end=='\n'||*end=='\t'))end++;
    const cJSON *id=cJSON_GetObjectItemCaseSensitive(root,"entity_id"),*state=cJSON_GetObjectItemCaseSensitive(root,"state");
    bool ok=end==json+length&&cJSON_IsObject(root)&&cJSON_IsString(id)&&!strcmp(id->valuestring,entity)&&cJSON_IsString(state);
    ha_light_t parsed={.state=HA_UNKNOWN};
    if(ok){
        if(!strcmp(state->valuestring,"on"))parsed.state=HA_ON;
        else if(!strcmp(state->valuestring,"off"))parsed.state=HA_OFF;
        else if(!strcmp(state->valuestring,"unavailable"))parsed.state=HA_UNAVAILABLE;
        else if(strcmp(state->valuestring,"unknown"))ok=false;
        const cJSON *attrs=cJSON_GetObjectItemCaseSensitive(root,"attributes"),*name=cJSON_GetObjectItemCaseSensitive(attrs,"friendly_name");
        if(cJSON_IsString(name)&&strlen(name->valuestring)<sizeof(parsed.name)){
            strcpy(parsed.name,name->valuestring);
            for(char *p=parsed.name;*p;p++)if((unsigned char)*p<32)*p=' ';
        }
    }
    if(ok)*out=parsed;
    cJSON_Delete(root);return ok;
}
