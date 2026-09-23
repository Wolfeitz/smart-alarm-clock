#include "local_time.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
int main(void)
{
    setenv("TZ","EST5EDT,M3.2.0/2,M11.1.0/2",1);tzset();time_t t;struct tm check;
    assert(local_time_epoch(2028,2,29,12,34,&t));localtime_r(&t,&check);assert(check.tm_mday==29 && check.tm_min==34);
    assert(!local_time_epoch(2026,2,29,12,0,&t));
    assert(!local_time_epoch(2026,4,31,12,0,&t));
    assert(!local_time_epoch(2026,3,8,2,30,&t));
    assert(local_time_epoch(2026,11,1,1,30,&t));localtime_r(&t,&check);assert(check.tm_isdst==1);
    assert(!local_time_epoch(2100,1,1,0,0,&t));
    assert(!local_time_epoch(2026,1,1,24,0,&t));
    puts("PASS local calendar validation, leap day, DST gap and earlier repeated hour");
}
