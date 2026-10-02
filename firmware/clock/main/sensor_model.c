#include "sensor_model.h"
#include <string.h>
uint8_t sensor_crc(const uint8_t *data,unsigned size)
{
    uint8_t crc=0xff;
    for(unsigned i=0;i<size;i++){
        crc^=data[i];for(unsigned b=0;b<8;b++)crc=(crc&0x80)?(crc<<1)^0x31:crc<<1;
    }
    return crc;
}
bool sensor_environment(const uint8_t raw[6],float *celsius,float *humidity)
{
    if(sensor_crc(raw,2)!=raw[2]||sensor_crc(raw+3,2)!=raw[5])return false;
    *celsius=-45.f+175.f*((raw[0]<<8)|raw[1])/65535.f;
    *humidity=100.f*((raw[3]<<8)|raw[4])/65535.f;return true;
}
void sensor_acceleration(const uint8_t raw[6],int mg[3])
{
    for(unsigned i=0;i<3;i++){
        unsigned word=raw[2*i]|((unsigned)raw[2*i+1]<<8);
        int value=word>=32768?(int)word-65536:(int)word;
        mg[i]=value*1000/4096; /* QMI8658 +/-8g, 4096 LSB/g. */
    }
}
bool sensor_shake(shake_detector_t *s,int64_t now,bool enabled,bool ringing,const int mg[3])
{
    if(!enabled||!ringing){memset(s,0,sizeof(*s));return false;}
    if(!s->started||now<=s->last||now-s->last>250){
        int64_t cooldown=s->cooldown;memset(s,0,sizeof(*s));s->cooldown=cooldown;
        s->started=true;s->armed=now+1000;memcpy(s->gravity,mg,sizeof(s->gravity));
    }
    s->last=now;int64_t motion=0;
    for(unsigned i=0;i<3;i++){
        int d=mg[i]-s->gravity[i];motion+=(int64_t)d*d;s->gravity[i]+=d/8;
    }
    if(now<s->armed||now<s->cooldown)return false;
    if(s->pulses&&now-s->first>1600)s->pulses=0;
    if(motion<450*450)s->high=false;
    if(motion<900*900||s->high)return false;
    s->high=true;
    if(s->pulses&&now-s->pulse<120)return false;
    if(!s->pulses)s->first=now;
    s->pulse=now;
    if(++s->pulses<3)return false;
    s->pulses=0;s->cooldown=now+5000;return true;
}

bool sensor_orientation(orientation_detector_t *s,int64_t now,bool enabled,const int mg[3])
{
    int64_t magnitude=0,movement=0;
    for(unsigned i=0;i<3;i++){
        magnitude+=(int64_t)mg[i]*mg[i];int d=mg[i]-s->previous[i];movement+=(int64_t)d*d;
    }
    bool valid=enabled&&s->started&&now>s->last&&now-s->last<=250&&
        magnitude>=800*800&&magnitude<=1200*1200&&movement<=150*150&&
        mg[2]>=-500&&mg[2]<=500&&mg[0]>=-500&&mg[0]<=500&&
        (mg[1]>=750||mg[1]<=-750);
    memcpy(s->previous,mg,sizeof(s->previous));s->last=now;s->started=true;
    bool candidate=mg[1]<0;
    if(!valid||candidate!=s->candidate){s->since=now;s->candidate=candidate;}
    else if(now-s->since>=1500)s->flipped=candidate;
    return s->flipped;
}
void sensor_rotate_touch(bool flipped,int *x,int *y)
{if(flipped){*x=479-*x;*y=319-*y;}}

battery_status_t sensor_battery(bool ok,uint8_t status1,uint8_t status2,bool detection,bool gauge,int percent)
{
    battery_status_t s={0};s.known=ok&&detection;
    if(!s.known)return s;
    s.present=(status1&8)!=0;if(!s.present)return s;
    s.charging=(status2>>5)==1;
    s.level_known=gauge&&percent>=0&&percent<=100;
    if(s.level_known)s.percent=percent;
    return s;
}
