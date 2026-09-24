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
time_t alarm_next(const alarm_config_t alarms[ALARM_COUNT],time_t now)
{
    struct tm today;if(!localtime_r(&now,&today))return 0;
    time_t best=0;
    for(unsigned i=0;i<ALARM_COUNT;i++){
        const alarm_config_t *a=&alarms[i];if(!a->enabled || !alarm_config_valid(a))continue;
        for(unsigned offset=0;offset<(a->weekdays?8u:1u);offset++){
            struct tm date=today;date.tm_hour=12;date.tm_min=0;date.tm_sec=0;date.tm_isdst=-1;
            if(a->weekdays)date.tm_mday+=offset;
            else{date.tm_year=a->once_date/10000-1900;date.tm_mon=a->once_date/100%100-1;date.tm_mday=a->once_date%100;}
            if(mktime(&date)==(time_t)-1)continue;
            uint32_t key=alarm_date(&date);
            if(key<=a->consumed_date || (a->weekdays && !(a->weekdays&(1u<<date.tm_wday))))continue;
            for(int dst=0;dst<=1;dst++){
                struct tm wanted=date;wanted.tm_hour=a->hour;wanted.tm_min=a->minute;wanted.tm_isdst=dst;
                time_t candidate=mktime(&wanted);struct tm actual;
                if(candidate<now || !localtime_r(&candidate,&actual))continue;
                if(alarm_date(&actual)!=key || actual.tm_hour!=a->hour || actual.tm_min!=a->minute)continue;
                if(!best || candidate<best)best=candidate;
            }
        }
    }
    return best;
}

alarm_config_t alarm_merge_edit(const alarm_config_t *old,const alarm_config_t *requested)
{
    alarm_config_t result=*requested;
    bool same=old->hour==requested->hour && old->minute==requested->minute && old->weekdays==requested->weekdays &&
        (requested->weekdays || old->once_date==requested->once_date);
    result.consumed_date=same?old->consumed_date:0;
    return result;
}
