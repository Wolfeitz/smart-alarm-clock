#include "alarm_recovery.h"
#include <string.h>
void alarm_capture(clock_settings_t *s,const alarm_engine_t *e,time_t now,uint64_t ms)
{
    memcpy(s->alarms,e->alarms,sizeof(s->alarms));
    for(unsigned i=0;i<ALARM_COUNT;i++){
        s->phase[i]=e->runtime[i].phase;s->deadline[i]=0;
        if(s->phase[i]){
            uint64_t delta=e->runtime[i].deadline_ms>ms?e->runtime[i].deadline_ms-ms:0;
            s->deadline[i]=(uint32_t)(now+(delta+999)/1000);
        }
    }
}
void alarm_restore(alarm_engine_t *e,const clock_settings_t *s,time_t now,uint64_t ms)
{
    memcpy(e->alarms,s->alarms,sizeof(e->alarms));
    memset(e->runtime,0,sizeof(e->runtime));
    for(unsigned i=0;i<ALARM_COUNT;i++){
        if(!s->phase[i])continue;
        int64_t delta=(int64_t)s->deadline[i]-now;
        uint64_t limit=s->phase[i]==ALARM_SNOOZED?ALARM_SNOOZE_MS:ALARM_RING_LIMIT_MS;
        if(delta>0 && (uint64_t)delta*1000<=limit){
            e->runtime[i].phase=s->phase[i];e->runtime[i].deadline_ms=ms+delta*1000;
        }else if(delta<=0 && delta>=-120 && s->phase[i]==ALARM_SNOOZED){
            e->runtime[i].phase=ALARM_RINGING;e->runtime[i].deadline_ms=ms+ALARM_RING_LIMIT_MS;
        }
    }
}
