#include "media_model.h"
#include <string.h>
bool media_action_supported(const media_player_t *p,media_action_t action)
{
    if(!p||p->state==MEDIA_OFF||p->state==MEDIA_UNKNOWN||p->state==MEDIA_UNAVAILABLE)return false;
    return (unsigned)action<=MEDIA_START_SAVED&&(p->capabilities&(1u<<action))!=0;
}
const char *media_state_name(media_state_t state)
{const char *names[]={"Unknown","Off","On","Idle","Playing","Paused","Buffering","Unavailable"};return (unsigned)state<8?names[state]:"Unknown";}

bool media_selection_valid(const char *id,const char *type)
{
    if(!id||!type||strlen(id)>383||strlen(type)>47)return false;
    if(!*id||!*type)return !*id&&!*type;
    for(const unsigned char *p=(const unsigned char *)id;*p;p++)if(*p<32||*p==127)return false;
    for(const unsigned char *p=(const unsigned char *)type;*p;p++)
        if(!((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9')||strchr("/_-+.",*p)))return false;
    return true;
}
