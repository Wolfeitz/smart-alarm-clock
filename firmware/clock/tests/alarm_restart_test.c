#define _POSIX_C_SOURCE 200809L
#include "alarm_recovery.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Cross the actual persisted-byte boundary, with a fresh engine and uptime. */
static alarm_engine_t restart(const alarm_engine_t *before,time_t saved,
                              uint64_t uptime,time_t boot)
{
    clock_settings_t settings,loaded;settings_defaults(&settings);
    alarm_capture(&settings,before,saved,uptime);
    uint8_t bytes[SETTINGS_SIZE];assert(settings_encode(&settings,bytes));
    assert(settings_decode(bytes,sizeof(bytes),&loaded));
    alarm_engine_t after={0};alarm_restore(&after,&loaded,boot,0);return after;
}
int main(void)
{
    setenv("TZ","UTC0",1);tzset();
    struct tm t={.tm_year=126,.tm_mon=8,.tm_mday=24,.tm_hour=7,.tm_min=30};
    time_t due=mktime(&t);alarm_engine_t engine={0};
    clock_settings_t defaults;settings_defaults(&defaults);
    memcpy(engine.alarms,defaults.alarms,sizeof(engine.alarms));
    engine.alarms[0]=(alarm_config_t){.enabled=true,.hour=7,.minute=30,.once_date=20260924};
    assert(alarm_tick(&engine,due,true,1000)==1);
    assert(!engine.alarms[0].enabled);
    alarm_engine_t boot=restart(&engine,due,1000,due+10);
    assert(alarm_ringing(&boot)==1);
    assert(boot.runtime[0].deadline_ms==590000);
    assert(boot.alarms[0].consumed_date==20260924 && !boot.alarms[0].enabled);
    assert(alarm_tick(&boot,due+10,true,0)==0);

    alarm_snooze(&boot,2000);
    engine=restart(&boot,due+12,2000,due+42);
    assert(engine.runtime[0].phase==ALARM_SNOOZED);
    assert(engine.runtime[0].deadline_ms==270000);
    /* A later invalid or corrected wall clock cannot lengthen runtime snooze. */
    assert(alarm_tick(&engine,0,false,269999)==0 && !alarm_ringing(&engine));
    assert(alarm_tick(&engine,0,false,270000)==0 && alarm_ringing(&engine)==1);

    engine=restart(&boot,due+12,2000,due+312+120);
    assert(alarm_ringing(&engine)==1);
    engine=restart(&boot,due+12,2000,due+312+121);
    assert(!alarm_ringing(&engine) && engine.runtime[0].phase==ALARM_IDLE);
    engine=restart(&boot,due+12,2000,due+11);
    assert(engine.runtime[0].phase==ALARM_IDLE); /* future beyond full snooze */

    alarm_dismiss(&boot);
    engine=restart(&boot,due+13,3000,due+14);
    assert(!alarm_ringing(&engine) && engine.runtime[0].phase==ALARM_IDLE);
    assert(alarm_tick(&engine,due+14,true,0)==0);
    assert(alarm_tick(&engine,due,true,1)==0); /* correction cannot re-ring */
    puts("PASS encoded alarm restart: once consumption, ring, snooze, expiry bounds, invalid time and dismissal");
}
