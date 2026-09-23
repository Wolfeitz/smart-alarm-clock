#include "alarm_engine.h"
static bool valid_date(uint32_t date)
{
    unsigned y=date/10000,m=date/100%100,d=date%100;
    if(y<2000 || y>2099 || m<1 || m>12)return false;
    const unsigned days[]={31,28,31,30,31,30,31,31,30,31,30,31};
    unsigned max=days[m-1]+(m==2 && y%4==0 && (y%100!=0 || y%400==0));
    return d>=1 && d<=max;
}
bool alarm_config_valid(const alarm_config_t *a)
{
    return a->hour<24 && a->minute<60 && a->weekdays<=127 &&
        (a->weekdays || valid_date(a->once_date)) &&
        (!a->consumed_date || valid_date(a->consumed_date));
}
uint32_t alarm_date(const struct tm *t)
{ return (t->tm_year+1900)*10000+(t->tm_mon+1)*100+t->tm_mday; }
uint8_t alarm_ringing(const alarm_engine_t *e)
{
    uint8_t mask=0;
    for(unsigned i=0;i<ALARM_COUNT;i++)if(e->runtime[i].phase==ALARM_RINGING)mask|=1u<<i;
    return mask;
}
uint8_t alarm_tick(alarm_engine_t *e,time_t epoch,bool valid,uint64_t ms)
{
    uint8_t consumed=0;
    for(unsigned i=0;i<ALARM_COUNT;i++){
        alarm_runtime_t *r=&e->runtime[i];
        if(r->phase!=ALARM_IDLE && ms>=r->deadline_ms){
            if(r->phase==ALARM_SNOOZED){r->phase=ALARM_RINGING;r->deadline_ms=ms+ALARM_RING_LIMIT_MS;}
            else r->phase=ALARM_IDLE;
        }
    }
    if(!valid)return 0;
    /* Search the bounded grace window using real instants, not mktime
     * normalization: a nonexistent DST minute never becomes a later alarm. */
    for(unsigned ago=0;ago<=120;ago++){
        time_t candidate=epoch-ago;struct tm t;
        if(!localtime_r(&candidate,&t) || t.tm_sec!=0)continue;
        uint32_t date=alarm_date(&t);
        for(unsigned i=0;i<ALARM_COUNT;i++){
            alarm_config_t *a=&e->alarms[i];alarm_runtime_t *r=&e->runtime[i];
            if(!a->enabled || !alarm_config_valid(a) || r->phase!=ALARM_IDLE || date<=a->consumed_date)continue;
            if(a->hour!=t.tm_hour || a->minute!=t.tm_min)continue;
            if(a->weekdays ? !(a->weekdays&(1u<<t.tm_wday)) : a->once_date!=date)continue;
            a->consumed_date=date;if(!a->weekdays)a->enabled=false;
            r->phase=ALARM_RINGING;r->deadline_ms=ms+ALARM_RING_LIMIT_MS;consumed|=1u<<i;
        }
    }
    return consumed;
}
void alarm_snooze(alarm_engine_t *e,uint64_t ms)
{
    for(unsigned i=0;i<ALARM_COUNT;i++)if(e->runtime[i].phase==ALARM_RINGING){
        e->runtime[i].phase=ALARM_SNOOZED;e->runtime[i].deadline_ms=ms+ALARM_SNOOZE_MS;
    }
}
void alarm_dismiss(alarm_engine_t *e)
{ for(unsigned i=0;i<ALARM_COUNT;i++)e->runtime[i].phase=ALARM_IDLE; }
void alarm_cancel(alarm_engine_t *e,unsigned i)
{ if(i<ALARM_COUNT)e->runtime[i].phase=ALARM_IDLE; }
