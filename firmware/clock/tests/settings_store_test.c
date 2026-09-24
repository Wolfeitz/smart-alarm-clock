#include "settings_store.h"
#include "nvs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int init_error,open_error,get_error=ESP_ERR_NVS_NOT_FOUND,set_error,commit_error;
static unsigned opens,reads,writes,commits;
static unsigned char blob[SETTINGS_SIZE];
static void partition(const char *name){assert(!strcmp(name,"clockcfg"));}
esp_err_t nvs_flash_init_partition(const char *name){partition(name);return init_error;}
esp_err_t nvs_open_from_partition(const char *p,const char *name,int mode,nvs_handle_t *h)
{partition(p);assert(!strcmp(name,"clock")&&mode==NVS_READWRITE);opens++;*h=42;return open_error;}
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *data,size_t *size)
{assert(h==42&&!strcmp(key,"settings")&&*size==SETTINGS_SIZE);reads++;if(!get_error){memcpy(data,blob,sizeof(blob));*size=sizeof(blob);}return get_error;}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *data,size_t size)
{assert(h==42&&!strcmp(key,"settings")&&size==SETTINGS_SIZE);writes++;if(!set_error)memcpy(blob,data,size);return set_error;}
esp_err_t nvs_commit(nvs_handle_t h){assert(h==42);commits++;return commit_error;}
static void defaults(const clock_settings_t *s)
{clock_settings_t d;settings_defaults(&d);unsigned char a[SETTINGS_SIZE],b[SETTINGS_SIZE];assert(settings_encode(s,a)&&settings_encode(&d,b)&&!memcmp(a,b,sizeof(a)));}
int main(int argc,char **argv)
{
    assert(argc==2);const char *scenario=argv[1];clock_settings_t s;
    if(!strcmp(scenario,"init"))init_error=91;
    if(!strcmp(scenario,"open"))open_error=92;
    if(!strcmp(scenario,"read"))get_error=93;
    if(!strcmp(scenario,"corrupt"))get_error=ESP_OK;
    int err=settings_store_open(&s);defaults(&s);
    if(init_error||open_error){assert(err==(init_error?init_error:open_error));assert(reads==0);assert(opens==(init_error?0u:1u));assert(settings_store_save(&s)==ESP_ERR_INVALID_STATE);}
    else if(!strcmp(scenario,"read"))assert(err==93);
    else if(!strcmp(scenario,"corrupt"))assert(err==ESP_ERR_INVALID_RESPONSE);
    else{
        assert(err==ESP_OK);
        if(!strcmp(scenario,"write"))set_error=94;
        if(!strcmp(scenario,"commit"))commit_error=95;
        s.alarms[0].enabled=true;s.alarms[0].hour=6;s.alarms[0].minute=45;s.alarms[0].weekdays=62;
        s.alarms[0].consumed_date=20260924;s.phase[0]=ALARM_SNOOZED;s.deadline[0]=1790264000;
        s.brightness=25;
        err=settings_store_save(&s);
        assert(err==(set_error?set_error:commit_error));assert(writes==1);assert(commits==(set_error?0u:1u));
        if(!err){
            unsigned char expected[SETTINGS_SIZE],actual[SETTINGS_SIZE];assert(settings_encode(&s,expected));
            get_error=ESP_OK;clock_settings_t loaded;assert(settings_store_open(&loaded)==ESP_OK);
            assert(settings_encode(&loaded,actual)&&!memcmp(expected,actual,sizeof(actual)));
            loaded.brightness=0;assert(settings_store_save(&loaded)==ESP_ERR_INVALID_ARG);assert(writes==1&&commits==1);
        }
    }
    printf("PASS settings store %s: partition isolation, error propagation and reload boundaries\n",scenario);
}
