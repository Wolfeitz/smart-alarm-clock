#include "audio_control.h"
#include <stdatomic.h>
/* Bit zero is alarm activity; upper bits change on every transition. */
static atomic_uint state;
uint32_t audio_control_token(void){return atomic_load(&state);}
bool audio_control_is_alarm(uint32_t token){return (token&1u)!=0;}
bool audio_control_valid(uint32_t token){return token==atomic_load(&state);}
void audio_control_alarm(bool active)
{
    unsigned old=atomic_load(&state);
    while(audio_control_is_alarm(old)!=active){
        unsigned next=((old+2u)&~1u)|(active?1u:0u);
        if(atomic_compare_exchange_weak(&state,&old,next))return;
    }
}
