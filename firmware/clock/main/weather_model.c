#include "weather_model.h"
#include "cJSON.h"
#include <math.h>
#include <string.h>
static const cJSON *field(const cJSON *o,const char *key){return cJSON_GetObjectItemCaseSensitive(o,key);}
static bool text(const cJSON *v,char *out,size_t n)
{
    if(!cJSON_IsString(v)||!v->valuestring||!v->valuestring[0]||strlen(v->valuestring)>=n)return false;
    for(const unsigned char *p=(const unsigned char *)v->valuestring;*p;p++)if(*p<32||*p==127)return false;
    strcpy(out,v->valuestring);return true;
}
static bool number(const cJSON *v,double low,double high,double *out)
{
    if(!cJSON_IsNumber(v)||!isfinite(v->valuedouble)||v->valuedouble<low||v->valuedouble>high)return false;
    *out=v->valuedouble;return true;
}
static bool integer(const cJSON *v,int64_t low,int64_t high,int64_t *out)
{
    double d;if(!number(v,(double)low,(double)high,&d)||floor(d)!=d)return false;*out=(int64_t)d;return true;
}
static cJSON *parse(const char *json,size_t size)
{
    if(!json||size==0||size>WEATHER_JSON_LIMIT)return NULL;
    const char *end=NULL;cJSON *root=cJSON_ParseWithLengthOpts(json,size,&end,false);
    if(!root)return NULL;
    while(end<json+size && (*end==' '||*end=='\n'||*end=='\r'||*end=='\t'))end++;
    if(end!=json+size||!cJSON_IsObject(root)){cJSON_Delete(root);return NULL;}
    return root;
}
bool weather_zip_valid(const char *zip)
{
    if(!zip||strlen(zip)!=5)return false;
    for(unsigned i=0;i<5;i++)if(zip[i]<'0'||zip[i]>'9')return false;
    return true;
}
bool weather_parse_location(const char *json,size_t size,const char *zip,weather_location_t *out)
{
    if(!out||!weather_zip_valid(zip))return false;
    cJSON *root=parse(json,size);if(!root)return false;
    const cJSON *results=field(root,"results"),*match=NULL,*v;unsigned matches=0;
    cJSON_ArrayForEach(v,results){
        const cJSON *country=field(v,"country_code"),*postal;
        if(!cJSON_IsString(country)||strcmp(country->valuestring,"US"))continue;
        cJSON_ArrayForEach(postal,field(v,"postcodes")){
            if(cJSON_IsString(postal)&&!strcmp(postal->valuestring,zip)){match=v;matches++;break;}
        }
    }
    weather_location_t loc={0};
    bool ok=matches==1 && text(field(match,"name"),loc.name,sizeof(loc.name)) &&
        text(field(match,"admin1"),loc.region,sizeof(loc.region)) &&
        text(field(match,"timezone"),loc.timezone,sizeof(loc.timezone)) &&
        number(field(match,"latitude"),-90,90,&loc.latitude) && number(field(match,"longitude"),-180,180,&loc.longitude);
    if(ok){strcpy(loc.zip,zip);*out=loc;}cJSON_Delete(root);return ok;
}
static const cJSON *first(const cJSON *o,const char *key){return cJSON_GetArrayItem(field(o,key),0);}
static bool unit(const cJSON *o,const char *key,const char *expected)
{const cJSON *v=field(o,key);return cJSON_IsString(v)&&!strcmp(v->valuestring,expected);}
bool weather_parse_forecast(const char *json,size_t size,int64_t now,const char *timezone,weather_data_t *out)
{
    if(!out||!timezone)return false;
    cJSON *root=parse(json,size);if(!root)return false;
    const cJSON *current=field(root,"current"),*daily=field(root,"daily");
    const cJSON *cu=field(root,"current_units"),*du=field(root,"daily_units");
    weather_data_t data={.fetched_at=now};int64_t code,day_code,rain;char tz[64];
    bool ok=text(field(root,"timezone"),tz,sizeof(tz))&&!strcmp(tz,timezone) &&
        unit(cu,"temperature_2m","°F")&&unit(cu,"apparent_temperature","°F")&&unit(cu,"time","unixtime")&&
        unit(du,"temperature_2m_max","°F")&&unit(du,"temperature_2m_min","°F")&&unit(du,"time","unixtime")&&unit(du,"precipitation_probability_max","%")&&
        integer(field(current,"time"),946684800,4102444799LL,&data.observed_at)&&
        integer(first(daily,"time"),946684800,4102444799LL,&data.day_start)&&
        number(field(current,"temperature_2m"),-150,150,&data.temperature)&&number(field(current,"apparent_temperature"),-200,200,&data.feels_like)&&
        number(first(daily,"temperature_2m_max"),-150,150,&data.high)&&number(first(daily,"temperature_2m_min"),-150,150,&data.low)&&data.low<=data.high&&
        integer(field(current,"weather_code"),0,99,&code)&&integer(first(daily,"weather_code"),0,99,&day_code)&&
        integer(first(daily,"precipitation_probability_max"),0,100,&rain)&&
        data.observed_at<=now+300&&data.observed_at>=now-7200&&data.day_start<=now&&data.day_start>now-90000;
    if(ok){data.code=code;data.day_code=day_code;data.rain_percent=rain;*out=data;}
    cJSON_Delete(root);return ok;
}
bool weather_fresh(const weather_data_t *d,int64_t now)
{return d&&d->fetched_at>0&&now>=d->fetched_at&&now-d->fetched_at<3600&&now>=d->observed_at&&now-d->observed_at<7200;}
const char *weather_condition(int c)
{
    switch(c){case 0:return "Clear";case 1:return "Mostly clear";case 2:return "Partly cloudy";case 3:return "Overcast";
    case 45:case 48:return "Fog";case 51:case 53:case 55:return "Drizzle";case 56:case 57:return "Freezing drizzle";
    case 61:case 63:case 65:return "Rain";case 66:case 67:return "Freezing rain";case 71:case 73:case 75:case 77:return "Snow";
    case 80:case 81:case 82:return "Rain showers";case 85:case 86:return "Snow showers";case 95:case 96:case 99:return "Thunderstorms";default:return "Unknown conditions";}
}
bool weather_should_locate(bool manual,const char *zip)
{return !manual&&zip&&!*zip;}
bool weather_parse_ip_zip(const char *json,size_t size,char zip[6])
{
    cJSON *root=parse(json,size);if(!root)return false;
    char candidate[6];bool ok=!cJSON_IsTrue(field(root,"error"))&&unit(root,"country_code","US")&&text(field(root,"postal"),candidate,sizeof(candidate))&&weather_zip_valid(candidate);
    if(ok)strcpy(zip,candidate);
    cJSON_Delete(root);return ok;
}
