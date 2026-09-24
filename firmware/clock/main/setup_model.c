#include "setup_model.h"
#include "ha_model.h"
#include "cJSON.h"
#include <string.h>
bool setup_parse(const char *json,size_t size,setup_request_t *out)
{
    if(!out)return false;
    memset(out,0,sizeof(*out));
    if(!json||!size||size>1800||memchr(json,0,size))return false;
    /* cJSON strings cannot represent the length beyond a decoded NUL safely. */
    for(size_t i=0;i+6<=size;i++)if(!memcmp(json+i,"\\u0000",6))return false;
    const char *end=NULL;cJSON *root=cJSON_ParseWithLengthOpts(json,size,&end,false);if(!root)return false;
    while(end<json+size&&(*end==' '||*end=='\r'||*end=='\n'||*end=='\t'))end++;
    const cJSON *tag=cJSON_GetObjectItemCaseSensitive(root,"tag"),*url=cJSON_GetObjectItemCaseSensitive(root,"url"),*token=cJSON_GetObjectItemCaseSensitive(root,"token"),*light=cJSON_GetObjectItemCaseSensitive(root,"light");
    bool ok=end==json+size&&cJSON_IsObject(root)&&cJSON_IsNumber(tag)&&tag->valuedouble>=1&&tag->valuedouble<=UINT32_MAX&&
        (uint32_t)tag->valuedouble==tag->valuedouble&&cJSON_IsString(url)&&ha_endpoint_valid(url->valuestring)&&
        cJSON_IsString(token)&&ha_token_valid(token->valuestring)&&cJSON_IsString(light)&&(!light->valuestring[0]||ha_entity_valid(light->valuestring));
    if(ok){out->tag=(uint32_t)tag->valuedouble;strcpy(out->endpoint,url->valuestring);strcpy(out->token,token->valuestring);strcpy(out->entity,light->valuestring);}
    cJSON_Delete(root);return ok;
}
