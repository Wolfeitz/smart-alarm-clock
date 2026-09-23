#include "local_time.h"
bool local_time_epoch(unsigned y,unsigned m,unsigned d,unsigned h,unsigned min,time_t *epoch)
{
    if(!epoch || y<2000 || y>2099 || m<1 || m>12 || d<1 || d>31 || h>23 || min>59)return false;
    bool found=false;time_t best=0;
    for(int dst=0;dst<=1;dst++){
        struct tm input={.tm_year=(int)y-1900,.tm_mon=(int)m-1,.tm_mday=d,.tm_hour=h,.tm_min=min,.tm_isdst=dst},check;
        time_t candidate=mktime(&input);
        if(candidate==(time_t)-1 || !localtime_r(&candidate,&check))continue;
        if(check.tm_year!=(int)y-1900 || check.tm_mon!=(int)m-1 || check.tm_mday!=(int)d || check.tm_hour!=(int)h || check.tm_min!=(int)min)continue;
        if(!found || candidate<best){found=true;best=candidate;}
    }
    if(found)*epoch=best;
    return found;
}
