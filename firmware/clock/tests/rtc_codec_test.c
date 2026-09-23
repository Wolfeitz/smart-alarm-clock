#define _GNU_SOURCE
#include "rtc_codec.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
int main(void)
{
    uint8_t leap[]={0x59,0x59,0x23,0x29,4,0x02,0x24};struct tm t;
    assert(rtc_decode(leap,&t));assert(t.tm_year==124 && t.tm_mday==29);
    leap[6]=0x25;assert(!rtc_decode(leap,&t));leap[6]=0x24;
    leap[0]=0x80;assert(!rtc_decode(leap,&t));leap[0]=0x59;
    leap[1]=0x6a;assert(!rtc_decode(leap,&t));leap[1]=0x59;
    leap[2]=0x24;assert(!rtc_decode(leap,&t));leap[2]=0x23;
    leap[5]=0;assert(!rtc_decode(leap,&t));leap[5]=0x02;
    assert(rtc_decode(leap,&t));uint8_t r[7];assert(rtc_encode(&t,r));
    for(int i=0;i<7;i++)assert(r[i]==leap[i]);
    setenv("TZ","EST5EDT,M3.2.0/2,M11.1.0/2",1);tzset();
    struct tm utc={.tm_year=126,.tm_mon=2,.tm_mday=8,.tm_hour=6,.tm_min=59,.tm_sec=59};
    time_t epoch=timegm(&utc);struct tm local;localtime_r(&epoch,&local);
    assert(local.tm_hour==1 && local.tm_min==59 && local.tm_isdst==0);
    epoch++;localtime_r(&epoch,&local);assert(local.tm_hour==3 && local.tm_isdst==1);
    puts("PASS RTC invalid-data, leap-day, round-trip and timezone spring transition");
}
