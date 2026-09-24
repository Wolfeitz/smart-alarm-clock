#include "display_policy.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    display_schedule_t s={true,1320,420};assert(display_schedule_valid(&s));
    assert(!display_night(&s,true,1319));assert(display_night(&s,true,1320));
    assert(display_night(&s,true,0));assert(display_night(&s,true,419));assert(!display_night(&s,true,420));
    assert(!display_night(&s,false,0));
    assert(!display_should_dim(&s,true,0,160,false,29999,30000));
    assert(display_should_dim(&s,true,0,160,false,30000,30000));
    assert(!display_should_dim(&s,true,0,160,true,30001,30000));
    assert(display_should_dim(&s,false,0,25,false,0,0));
    s.start_minute=600;s.end_minute=900;assert(display_night(&s,true,600));assert(!display_night(&s,true,900));
    s.enabled=false;assert(display_should_dim(&s,true,700,25,false,0,0));
    s.end_minute=s.start_minute;assert(!display_schedule_valid(&s));
    s.end_minute=1440;assert(!display_schedule_valid(&s));
    puts("PASS night interval boundaries, touch wake timeout, alarm priority and invalid-clock/manual fallback");
}
