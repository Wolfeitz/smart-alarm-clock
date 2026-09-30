#include "alarm_output.h"
#include <stdatomic.h>
static atomic_bool enabled;
static atomic_uint desired,confirmed,lease;
static uint32_t sequence,session,started;
static uint8_t previous;
static bool fallback,was_confirmed;
void alarm_output_enable(bool value){atomic_store(&enabled,value);}
uint32_t alarm_output_session(void){return atomic_load(&desired);}
void alarm_output_confirm(uint32_t id,uint32_t now)
{
    if(id&&id==atomic_load(&desired)){
        atomic_store(&lease,now+3000);atomic_store(&confirmed,id);
    }
}
bool alarm_output_local(uint8_t ringing,uint32_t now)
{
    if(ringing!=previous){
        previous=ringing;session=0;fallback=true;was_confirmed=false;
        if(ringing&&atomic_load(&enabled)){
            do{sequence++;}while(!sequence);session=sequence;started=now;fallback=false;
        }
        atomic_store(&desired,session);
    }
    if(!ringing)return false;
    if(!atomic_load(&enabled)){session=0;atomic_store(&desired,0);fallback=true;}
    if(!session)return true;
    uint32_t until=atomic_load(&lease);
    bool fresh=atomic_load(&confirmed)==session&&(int32_t)(until-now)>0;
    if(!was_confirmed&&(uint32_t)((until-3000)-started)>=8000)fresh=false;
    if(fresh)was_confirmed=true;
    if((was_confirmed&&!fresh)||(!was_confirmed&&(uint32_t)(now-started)>=8000))fallback=true;
    return fallback;
}
