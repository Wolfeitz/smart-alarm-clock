#include "alarm_recovery.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    clock_settings_t s;settings_defaults(&s);alarm_engine_t e={0},restored={0};
    memcpy(e.alarms,s.alarms,sizeof(e.alarms));
    e.runtime[0]=(alarm_runtime_t){ALARM_RINGING,610000};
    e.runtime[1]=(alarm_runtime_t){ALARM_SNOOZED,310000};
    e.alarms[0].consumed_date=20260923;
    time_t now=1790200000;alarm_capture(&s,&e,now,10000);
    assert(s.deadline[0]==now+600 && s.deadline[1]==now+300);
    alarm_restore(&restored,&s,now+20,5000);
    assert(restored.runtime[0].phase==ALARM_RINGING && restored.runtime[0].deadline_ms==585000);
    assert(restored.runtime[1].phase==ALARM_SNOOZED && restored.runtime[1].deadline_ms==285000);
    alarm_restore(&restored,&s,now+310,0);assert(restored.runtime[1].phase==ALARM_RINGING);
    alarm_restore(&restored,&s,now+421,0);assert(restored.runtime[1].phase==ALARM_IDLE);
    alarm_restore(&restored,&s,now+601,0);assert(restored.runtime[0].phase==ALARM_IDLE);
    alarm_dismiss(&e);alarm_capture(&s,&e,now,10000);alarm_restore(&restored,&s,now,0);
    assert(alarm_ringing(&restored)==0 && s.phase[1]==0);
    puts("PASS active alarm recovery, elapsed snooze grace, stale events and durable dismissal");
}
