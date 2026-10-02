#include "sensor_model.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    battery_status_t battery=sensor_battery(true,0,0,true,true,100);assert(battery.known&&!battery.present&&!battery.level_known);
    battery=sensor_battery(true,8,0x20,true,true,73);assert(battery.present&&battery.charging&&battery.level_known&&battery.percent==73);
    battery=sensor_battery(true,8,0x40,true,true,0);assert(battery.present&&!battery.charging&&battery.level_known&&!battery.percent);
    battery=sensor_battery(true,8,0,true,true,255);assert(battery.present&&!battery.level_known);
    battery=sensor_battery(true,8,0,true,false,40);assert(!battery.level_known);
    battery=sensor_battery(false,8,0,true,true,40);assert(!battery.known);
    battery=sensor_battery(true,8,0,false,true,40);assert(!battery.known);
    puts("PASS battery absent, charging, empty, invalid level and disabled/read-error states");
    uint8_t known[]={0xbe,0xef};assert(sensor_crc(known,2)==0x92);
    uint8_t raw[]={0x66,0x66,0,0x80,0,0};raw[2]=sensor_crc(raw,2);raw[5]=sensor_crc(raw+3,2);
    float t=-999,h=-999;assert(sensor_environment(raw,&t,&h));assert(t>24.9&&t<25.1&&h>49.9&&h<50.1);
    raw[5]^=1;assert(!sensor_environment(raw,&t,&h));assert(t>24.9&&h>49.9);
    uint8_t acc[]={0,0x10,0,0xf0,0,0x80};int mg[3];sensor_acceleration(acc,mg);
    assert(mg[0]==1000&&mg[1]==-1000&&mg[2]==-8000);
    shake_detector_t s={0};int still[]={0,0,1000},bump[]={1500,0,1000};
    for(int now=0;now<2000;now+=20)assert(!sensor_shake(&s,now,true,true,still));
    assert(!sensor_shake(&s,2000,true,true,bump));
    for(int now=2020;now<4000;now+=20)assert(!sensor_shake(&s,now,true,true,still));
    unsigned events=0;
    for(int now=4000;now<4600;now+=20)events+=sensor_shake(&s,now,true,true,now%200==0?bump:still);
    assert(events==1);
    for(int now=4600;now<7000;now+=20)assert(!sensor_shake(&s,now,true,true,now%200==0?bump:still));
    for(int now=7000;now<9000;now+=20)assert(!sensor_shake(&s,now,false,true,bump));
    for(int now=9000;now<11000;now+=20)assert(!sensor_shake(&s,now,true,false,bump));
    /* Missing samples and constant orientation cannot count as three pulses. */
    for(int now=11000;now<15000;now+=500)assert(!sensor_shake(&s,now,true,true,bump));
    orientation_detector_t o={0};int upright[]={0,1000,0},reverse[]={0,-1000,0},flat[]={0,0,1000};
    for(int now=0;now<2000;now+=20)assert(!sensor_orientation(&o,now,true,upright));
    for(int now=2000;now<3400;now+=20)assert(!sensor_orientation(&o,now,true,reverse));
    for(int now=3400;now<4000;now+=20)sensor_orientation(&o,now,true,reverse);
    assert(o.flipped);
    for(int now=4000;now<8000;now+=20)assert(sensor_orientation(&o,now,true,flat));
    int nearflat[]={0,600,800};
    for(int now=8000;now<10000;now+=20)assert(sensor_orientation(&o,now,true,nearflat));
    for(int now=10000;now<12000;now+=20)assert(sensor_orientation(&o,now,false,upright));
    for(int now=12000;now<14000;now+=20)assert(sensor_orientation(&o,now,true,now%40?upright:reverse));
    for(int now=14000;now<16000;now+=20)sensor_orientation(&o,now,true,upright);
    assert(!o.flipped);
    int x=0,y=0;sensor_rotate_touch(true,&x,&y);assert(x==479&&y==319);
    sensor_rotate_touch(true,&x,&y);assert(!x&&!y);
    puts("PASS orientation stable delay, flat/near-flat hold, disabled lock, shaking and touch corner mapping");
    puts("PASS sensor CRC/conversion, shake arming, single bump, three pulses, cooldown, disabled and sample gaps");
}
