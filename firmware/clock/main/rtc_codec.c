#include "rtc_codec.h"
#include <string.h>
static int bcd(uint8_t v) { return (v&15)>9 || (v>>4)>9 ? -1 : (v>>4)*10+(v&15); }
static uint8_t enc(int v) { return ((v/10)<<4)|(v%10); }
static bool valid(const struct tm *t)
{
    if(t->tm_year<100 || t->tm_year>199 || t->tm_mon<0 || t->tm_mon>11 ||
       t->tm_hour<0 || t->tm_hour>23 || t->tm_min<0 || t->tm_min>59 ||
       t->tm_sec<0 || t->tm_sec>59 || t->tm_wday<0 || t->tm_wday>6)return false;
    const int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
    int year=t->tm_year+1900;
    int max=days[t->tm_mon]+(t->tm_mon==1 && year%4==0 && (year%100!=0 || year%400==0));
    return t->tm_mday>=1 && t->tm_mday<=max;
}
bool rtc_decode(const uint8_t r[7],struct tm *t)
{
    if(r[0]&0x80)return false;
    memset(t,0,sizeof(*t));
    t->tm_sec=bcd(r[0]);t->tm_min=bcd(r[1]);t->tm_hour=bcd(r[2]);
    t->tm_mday=bcd(r[3]);t->tm_wday=r[4];t->tm_mon=bcd(r[5])-1;t->tm_year=bcd(r[6])+100;
    return valid(t);
}
bool rtc_encode(const struct tm *t,uint8_t r[7])
{
    if(!valid(t))return false;
    r[0]=enc(t->tm_sec);r[1]=enc(t->tm_min);r[2]=enc(t->tm_hour);r[3]=enc(t->tm_mday);
    r[4]=t->tm_wday;r[5]=enc(t->tm_mon+1);r[6]=enc(t->tm_year-100);return true;
}
