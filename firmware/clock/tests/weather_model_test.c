#include "weather_model.h"
#include "timezone_rules.h"
#include "cJSON.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static char *read_file(const char *name)
{
    FILE *f=fopen(name,"rb");assert(f);char *b=calloc(1,WEATHER_JSON_LIMIT+1);assert(b);
    size_t n=fread(b,1,WEATHER_JSON_LIMIT,f);assert(n>0&&!ferror(f)&&feof(f));fclose(f);return b;
}
static bool forecast(const cJSON *root,int64_t now,weather_data_t *data)
{
    char *s=cJSON_PrintUnformatted(root);assert(s);bool ok=weather_parse_forecast(s,strlen(s),now,"America/New_York",data);free(s);return ok;
}
int main(int argc,char **argv)
{
    assert(!weather_should_locate(true,"27358"));assert(!weather_should_locate(true,""));
    assert(weather_should_locate(false,""));assert(!weather_should_locate(false,"27358"));
    char zip[6];const char *ip="{\"country_code\":\"US\",\"postal\":\"27358\"}";
    assert(weather_parse_ip_zip(ip,strlen(ip),zip)&&!strcmp(zip,"27358"));
    ip="{\"country_code\":\"US\",\"postal\":null}";assert(!weather_parse_ip_zip(ip,strlen(ip),zip));
    ip="{\"country_code\":\"CA\",\"postal\":\"27358\"}";assert(!weather_parse_ip_zip(ip,strlen(ip),zip));
    assert(argc==3);char *geo=read_file(argv[1]),*raw=read_file(argv[2]);weather_location_t loc;
    assert(weather_parse_location(geo,strlen(geo),"27358",&loc));assert(!strcmp(loc.name,"Summerfield"));assert(!strcmp(loc.timezone,"America/New_York"));
    assert(!weather_parse_location(geo,strlen(geo),"00000",&loc));assert(!weather_parse_location(geo,strlen(geo),"2735",&loc));
    cJSON *g=cJSON_Parse(geo),*results=cJSON_GetObjectItemCaseSensitive(g,"results");
    cJSON_AddItemToArray(results,cJSON_Duplicate(cJSON_GetArrayItem(results,0),true));char *ambiguous=cJSON_PrintUnformatted(g);
    assert(!weather_parse_location(ambiguous,strlen(ambiguous),"27358",&loc));free(ambiguous);cJSON_Delete(g);
    assert(!weather_parse_location("{}",2,"27358",&loc));assert(!weather_zip_valid("27358&x=1"));
    cJSON *root=cJSON_Parse(raw),*current=cJSON_GetObjectItemCaseSensitive(root,"current");
    int64_t now=(int64_t)cJSON_GetObjectItemCaseSensitive(current,"time")->valuedouble+60;
    weather_data_t data;assert(forecast(root,now,&data));assert(data.rain_percent==83);assert(weather_fresh(&data,now));
    assert(!weather_fresh(&data,now+3601));assert(!weather_fresh(&data,now-1));
    assert(!forecast(root,now+7200,&data));assert(!forecast(root,now-1000,&data));
    cJSON *units=cJSON_GetObjectItemCaseSensitive(root,"current_units");
    cJSON_ReplaceItemInObjectCaseSensitive(units,"temperature_2m",cJSON_CreateString("°C"));assert(!forecast(root,now,&data));
    cJSON_ReplaceItemInObjectCaseSensitive(units,"temperature_2m",cJSON_CreateString("°F"));
    cJSON_ReplaceItemInObjectCaseSensitive(current,"temperature_2m",cJSON_CreateNull());assert(!forecast(root,now,&data));
    cJSON_Delete(root);root=cJSON_Parse(raw);
    cJSON *daily=cJSON_GetObjectItemCaseSensitive(root,"daily");cJSON_DeleteItemFromObjectCaseSensitive(daily,"precipitation_probability_max");assert(!forecast(root,now,&data));
    assert(!weather_parse_forecast(raw,strlen(raw)-4,now,"America/New_York",&data));
    assert(!weather_parse_forecast(raw,strlen(raw),now,"America/Chicago",&data));
    char *junk=malloc(strlen(raw)+5);sprintf(junk,"%sJUNK",raw);assert(!weather_parse_forecast(junk,strlen(junk),now,"America/New_York",&data));free(junk);
    assert(timezone_rule("America/New_York"));assert(!timezone_rule("Made/Up"));
    setenv("TZ",timezone_rule("America/New_York"),1);tzset();
    struct tm winter={.tm_year=126,.tm_mon=0,.tm_mday=15,.tm_hour=12,.tm_isdst=-1};
    struct tm summer={.tm_year=126,.tm_mon=6,.tm_mday=15,.tm_hour=12,.tm_isdst=-1};
    assert(mktime(&winter)>0&&winter.tm_isdst==0);assert(mktime(&summer)>0&&summer.tm_isdst==1);
    cJSON_Delete(root);free(geo);free(raw);puts("PASS weather validation, ambiguity, missing data, units, freshness and timezone DST");
}
