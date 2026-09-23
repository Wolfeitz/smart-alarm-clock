#include "settings_codec.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    clock_settings_t s,read;settings_defaults(&s);uint8_t data[SETTINGS_SIZE];
    assert(settings_encode(&s,data));assert(settings_decode(data,sizeof(data),&read));
    assert(read.brightness==160 && !read.alarms[0].enabled);
    s.alarms[3]=(alarm_config_t){.enabled=true,.hour=23,.minute=59,.weekdays=0,.once_date=20280229,.consumed_date=20260201};
    assert(settings_encode(&s,data));assert(settings_decode(data,sizeof(data),&read));
    assert(read.alarms[3].once_date==20280229 && read.alarms[3].consumed_date==20260201);
    for(unsigned i=0;i<sizeof(data);i++){
        data[i]^=1;assert(!settings_decode(data,sizeof(data),&read));data[i]^=1;
    }
    for(unsigned n=0;n<sizeof(data);n++)assert(!settings_decode(data,n,&read));
    s.alarms[0].hour=24;assert(!settings_encode(&s,data));
    const uint8_t legacy[]={69,83,80,67,1,8,25,0,0,0,0,0,0,0,0,0,1,6,43,62,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,127,0,0,0,0,0,0,0,0,0,0,0,0,97,25,116,121};
    assert(settings_decode(legacy,sizeof(legacy),&read));
    assert(read.brightness==25 && read.alarms[0].enabled && read.alarms[0].minute==43 && read.phase[0]==0);
    assert(settings_encode(&read,data) && data[4]==2);
    puts("PASS settings defaults, durable alarm fields, corruption and truncation rejection");
}
