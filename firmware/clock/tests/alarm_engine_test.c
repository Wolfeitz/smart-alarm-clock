#define _POSIX_C_SOURCE 200809L
#include "alarm_engine.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static time_t at(int year,int month,int day,int hour,int minute,int second,int dst)
{
    struct tm t={.tm_year=year-1900,.tm_mon=month-1,.tm_mday=day,.tm_hour=hour,.tm_min=minute,.tm_sec=second,.tm_isdst=dst};
    return mktime(&t);
}
static alarm_engine_t daily(int hour,int minute)
{
    alarm_engine_t e={0};e.alarms[0]=(alarm_config_t){.enabled=true,.hour=hour,.minute=minute,.weekdays=127};return e;
}
int main(void)
{
    setenv("TZ","EST5EDT,M3.2.0/2,M11.1.0/2",1);tzset();
    alarm_engine_t e=daily(7,30);time_t now=at(2026,9,24,7,30,0,-1);
    assert(alarm_tick(&e,now,false,0)==0);
    assert(alarm_tick(&e,now,true,100)==1);assert(alarm_ringing(&e)==1);
    alarm_snooze(&e,500);assert(alarm_ringing(&e)==0);
    alarm_tick(&e,now+3600,true,ALARM_SNOOZE_MS+499);assert(alarm_ringing(&e)==0);
    alarm_tick(&e,now-3600,true,ALARM_SNOOZE_MS+500);assert(alarm_ringing(&e)==1);
    alarm_dismiss(&e);assert(alarm_ringing(&e)==0);assert(alarm_tick(&e,now,true,400000)==0);
    assert(alarm_tick(&e,now+86400,true,500000)==1);
    alarm_tick(&e,now+86400+600,true,500000+ALARM_RING_LIMIT_MS);assert(alarm_ringing(&e)==0);
    e=daily(7,30);assert(alarm_tick(&e,now+120,true,0)==1);
    e=daily(7,30);assert(alarm_tick(&e,now+121,true,0)==0);
    e=daily(7,30);e.alarms[0].weekdays=1;assert(alarm_tick(&e,now,true,0)==0);
    e=daily(7,30);e.alarms[0].weekdays=0;e.alarms[0].once_date=20260924;
    assert(alarm_tick(&e,now,true,0)==1);assert(!e.alarms[0].enabled);
    e=daily(2,30);assert(alarm_tick(&e,at(2026,3,8,3,0,0,-1),true,0)==0);
    assert(alarm_tick(&e,at(2026,3,8,3,30,0,-1),true,0)==0);
    e=daily(1,30);assert(alarm_tick(&e,at(2026,11,1,1,30,0,1),true,0)==1);
    alarm_dismiss(&e);assert(alarm_tick(&e,at(2026,11,1,1,30,0,0),true,4000000)==0);
    e=daily(7,30);e.alarms[1]=e.alarms[0];assert(alarm_tick(&e,now,true,0)==3);
    alarm_cancel(&e,0);assert(alarm_ringing(&e)==2);
    e.alarms[0].weekdays=0;e.alarms[0].once_date=20260229;assert(!alarm_config_valid(&e.alarms[0]));
    e=daily(2,30);time_t spring=at(2026,3,8,0,0,0,-1);
    assert(alarm_next(e.alarms,spring)==at(2026,3,9,2,30,0,-1));
    e=daily(1,30);time_t first=at(2026,11,1,1,30,0,1),second=at(2026,11,1,1,30,0,0);
    assert(alarm_next(e.alarms,first-1)==first);assert(alarm_next(e.alarms,first+1)==second);
    e.alarms[0].consumed_date=20261101;assert(alarm_next(e.alarms,first+1)==at(2026,11,2,1,30,0,-1));
    e=daily(7,30);assert(alarm_tick(&e,now,true,0)==1);alarm_dismiss(&e);
    alarm_config_t edit=e.alarms[0];edit.enabled=false;
    e.alarms[0]=alarm_merge_edit(&e.alarms[0],&edit);edit.enabled=true;
    e.alarms[0]=alarm_merge_edit(&e.alarms[0],&edit);assert(alarm_tick(&e,now,true,10)==0);
    edit.minute=35;e.alarms[0]=alarm_merge_edit(&e.alarms[0],&edit);
    assert(alarm_tick(&e,now+300,true,20)==1);alarm_dismiss(&e);
    edit=e.alarms[0];edit.once_date=20260925;
    e.alarms[0]=alarm_merge_edit(&e.alarms[0],&edit);assert(alarm_tick(&e,now+300,true,30)==0);
    puts("PASS alarm triggers, invalid time, weekdays/once, grace window, DST, duplicates, snooze, dismiss, timeout, simultaneous alarms");
}
