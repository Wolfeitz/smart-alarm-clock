#include "display_policy.h"
bool display_schedule_valid(const display_schedule_t *s)
{return s&&s->start_minute<1440&&s->end_minute<1440&&s->start_minute!=s->end_minute;}
bool display_night(const display_schedule_t *s,bool valid,unsigned minute)
{
    if(!display_schedule_valid(s)||!s->enabled||!valid||minute>=1440)return false;
    return s->start_minute<s->end_minute?minute>=s->start_minute&&minute<s->end_minute:
        minute>=s->start_minute||minute<s->end_minute;
}
bool display_should_dim(const display_schedule_t *s,bool valid,unsigned minute,uint8_t brightness,
                        bool ringing,uint64_t now,uint64_t wake_until)
{
    if(ringing)return false;
    if(!s->enabled||!valid)return brightness<80;
    return display_night(s,valid,minute)&&now>=wake_until;
}
