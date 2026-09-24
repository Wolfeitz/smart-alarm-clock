#include "settings_codec.h"
#include <string.h>
static void put(uint8_t *p,uint32_t n){for(unsigned i=0;i<4;i++)p[i]=n>>(8*i);}
static uint32_t get(const uint8_t *p){uint32_t n=0;for(unsigned i=0;i<4;i++)n|=(uint32_t)p[i]<<(8*i);return n;}
static uint32_t crc(const uint8_t *p,size_t n)
{
    uint32_t c=~0u;
    for(size_t i=0;i<n;i++){
        c^=p[i];for(unsigned j=0;j<8;j++)c=(c>>1)^(0xedb88320u&-(c&1));
    }
    return ~c;
}
void settings_defaults(clock_settings_t *s)
{
    memset(s,0,sizeof(*s));s->brightness=160;s->display=(display_schedule_t){.start_minute=1320,.end_minute=420};
    for(unsigned i=0;i<ALARM_COUNT;i++){s->alarms[i].hour=7;s->alarms[i].weekdays=127;}
}
bool settings_encode(const clock_settings_t *s,uint8_t d[SETTINGS_SIZE])
{
    if(!s->brightness||!display_schedule_valid(&s->display))return false;
    memset(d,0,SETTINGS_SIZE);memcpy(d,"ESPC",4);d[4]=3;d[5]=ALARM_COUNT;d[6]=s->brightness;
    d[7]=s->display.enabled;d[8]=s->display.start_minute;d[9]=s->display.start_minute>>8;
    d[10]=s->display.end_minute;d[11]=s->display.end_minute>>8;
    for(unsigned i=0;i<ALARM_COUNT;i++){
        const alarm_config_t *a=&s->alarms[i];if(!alarm_config_valid(a))return false;
        uint8_t *p=d+16+24*i;p[0]=a->enabled;p[1]=a->hour;p[2]=a->minute;p[3]=a->weekdays;
        put(p+4,a->once_date);put(p+8,a->consumed_date);
        if(s->phase[i]>ALARM_SNOOZED || (s->phase[i] && (s->deadline[i]<946684800u || s->deadline[i]>=4102444800u)))return false;
        p[12]=s->phase[i];put(p+16,s->phase[i]?s->deadline[i]:0);
    }
    put(d+SETTINGS_SIZE-4,crc(d,SETTINGS_SIZE-4));return true;
}
bool settings_decode(const uint8_t *d,size_t n,clock_settings_t *s)
{
    if((n!=SETTINGS_SIZE && n!=SETTINGS_V1_SIZE) || memcmp(d,"ESPC",4) ||
       !((d[4]==1 && n==SETTINGS_V1_SIZE) || ((d[4]==2||d[4]==3) && n==SETTINGS_SIZE)) || d[5]!=ALARM_COUNT || !d[6] ||
       crc(d,n-4)!=get(d+n-4))return false;
    clock_settings_t next;settings_defaults(&next);next.brightness=d[6];
    if(d[4]==3){
        if(d[7]>1)return false;
        next.display=(display_schedule_t){.enabled=d[7],.start_minute=d[8]|((uint16_t)d[9]<<8),.end_minute=d[10]|((uint16_t)d[11]<<8)};
        if(!display_schedule_valid(&next.display))return false;
    }
    for(unsigned i=0;i<ALARM_COUNT;i++){
        const uint8_t *p=d+16+(d[4]==1?16:24)*i;alarm_config_t *a=&next.alarms[i];
        if(p[0]>1)return false;
        a->enabled=p[0];a->hour=p[1];a->minute=p[2];a->weekdays=p[3];
        a->once_date=get(p+4);a->consumed_date=get(p+8);
        if(!alarm_config_valid(a))return false;
        if(d[4]>=2){
            next.phase[i]=p[12];next.deadline[i]=get(p+16);
            if(next.phase[i]>ALARM_SNOOZED || (next.phase[i] && (next.deadline[i]<946684800u || next.deadline[i]>=4102444800u)))return false;
        }
    }
    *s=next;return true;
}
