/* Render the actual UI with synthetic service data; this is not hardware proof. */
#include "lvgl.h"
#include "clock_ui.h"
#include "alarm_service.h"
#include "weather_service.h"
#include "clock_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
static uint16_t pixels[480*320];
static uint32_t ticks;
static bool has_weather,backlight_dim;
static weather_snapshot_t weather;
static alarm_snapshot_t alarm_state;
static uint32_t tick(void){return ticks;}
int64_t esp_timer_get_time(void){return (int64_t)ticks*1000;}
static void flush(lv_display_t *d,const lv_area_t *a,uint8_t *data)
{
    uint16_t *src=(uint16_t *)data;
    for(int y=a->y1;y<=a->y2;y++)for(int x=a->x1;x<=a->x2;x++)pixels[y*480+x]=*src++;
    lv_display_flush_ready(d);
}
void diagnostics_printf(const char *format,...){(void)format;}
void alarm_service_snapshot(alarm_snapshot_t *s){*s=alarm_state;}
uint32_t alarm_service_save_tracked(unsigned i,const alarm_config_t *a){alarm_state.settings.alarms[i]=*a;alarm_state.save_ticket++;return alarm_state.save_ticket;}
bool alarm_service_brightness(uint8_t b){alarm_state.settings.brightness=b;return true;}
bool alarm_service_snooze(void){return true;}
bool alarm_service_dismiss(void){return true;}
void board_brightness(bool dim){backlight_dim=dim;}
bool audio_test(void){return true;}
bool clock_valid(void){return true;}
const char *clock_source(void){return "RTC / offline";}
esp_err_t clock_set(time_t epoch){(void)epoch;return ESP_OK;}
void esp_restart(void){}
void weather_service_snapshot(weather_snapshot_t *s){*s=weather;}
bool weather_service_connect(const char *ssid,const char *password){(void)password;strcpy(weather.ssid,ssid);weather.connected=true;return true;}
bool weather_service_location(const char *zip){(void)zip;return true;}
bool weather_service_scan(void){return true;}
bool weather_service_refresh(void){return true;}
const char *weather_service_timezone(void){return "America/New_York";}
static void advance(void){for(unsigned i=0;i<20;i++){ticks+=20;lv_timer_handler();}clock_ui_update();lv_refr_now(NULL);}
static void click_text(lv_obj_t *parent,const char *text)
{
    for(unsigned i=0;i<lv_obj_get_child_count(parent);i++){
        lv_obj_t *child=lv_obj_get_child(parent,i);
        if(lv_obj_check_type(child,&lv_label_class)&&!strcmp(lv_label_get_text(child),text)){
            lv_obj_send_event(lv_obj_get_parent(child),LV_EVENT_CLICKED,NULL);return;
        }
        if(lv_obj_check_type(child,&lv_button_class)){
            lv_obj_t *l=lv_obj_get_child(child,0);
            if(l&&lv_obj_check_type(l,&lv_label_class)&&!strcmp(lv_label_get_text(l),text)){lv_obj_send_event(child,LV_EVENT_CLICKED,NULL);return;}
        }
    }
}
int main(int argc,char **argv)
{
    if(argc<2)return 2;has_weather=argc>2&&!strcmp(argv[2],"weather");
    setenv("TZ","EST5EDT,M3.2.0/2,M11.1.0/2",1);tzset();settings_defaults(&alarm_state.settings);
    weather=(weather_snapshot_t){.zip="27358",.location={.name="Summerfield",.timezone="America/New_York"},.manual_location=true,.has_data=has_weather,.connected=has_weather,.status="Set up Wi-Fi for local weather",
        .network_count=3,.scan_revision=1,.networks={{.ssid="Home Wi-Fi",.rssi=-42,.secured=true},{.ssid="Guest Network",.rssi=-65,.secured=true},{.ssid="Another network",.rssi=-78,.secured=true}}};
    time_t now=time(NULL);struct tm date;localtime_r(&now,&date);date.tm_hour=0;date.tm_min=0;date.tm_sec=0;
    weather.data=(weather_data_t){.temperature=72,.feels_like=71,.high=77,.low=58,.code=2,.day_code=2,.rain_percent=10,.observed_at=now,.fetched_at=now,.day_start=mktime(&date)};
    if(has_weather)strcpy(weather.status,"Weather updated");
    lv_init();lv_tick_set_cb(tick);lv_display_t *d=lv_display_create(480,320);lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB565);
    static uint16_t buffer[480*40];lv_display_set_buffers(d,buffer,NULL,sizeof(buffer),LV_DISPLAY_RENDER_MODE_PARTIAL);lv_display_set_flush_cb(d,flush);
    clock_ui_init();advance();
    if(argc>3){
        if(!strcmp(argv[3],"display")){click_text(lv_screen_active(),"Settings");advance();click_text(lv_screen_active(),"Display & night mode");advance();}
        else {click_text(lv_screen_active(),"Weather");advance();}
        if(!strcmp(argv[3],"wifi")||!strcmp(argv[3],"connect")){
            click_text(lv_screen_active(),"Wi-Fi");advance();
            if(!strcmp(argv[3],"connect")){
                lv_obj_t *screen=lv_screen_active(),*list=NULL,*password=NULL;
                for(unsigned i=0;i<lv_obj_get_child_count(screen);i++)if(lv_obj_check_type(lv_obj_get_child(screen,i),&lv_list_class))list=lv_obj_get_child(screen,i);
                assert(list);lv_obj_send_event(lv_obj_get_child(list,0),LV_EVENT_CLICKED,NULL);advance();
                for(unsigned i=0;i<lv_obj_get_child_count(screen);i++)if(lv_obj_check_type(lv_obj_get_child(screen,i),&lv_textarea_class))password=lv_obj_get_child(screen,i);
                assert(password);lv_textarea_set_text(password,"fake-preview-password");click_text(screen,"Connect");advance();
                for(unsigned i=0;i<lv_obj_get_child_count(screen);i++)assert(!lv_obj_check_type(lv_obj_get_child(screen,i),&lv_textarea_class));
                puts("PASS connection success leaves password form");
            }
        }
        if(!strcmp(argv[3],"location")){click_text(lv_screen_active(),"Location");advance();}
    }
    if(argc>3&&!strcmp(argv[3],"night-test")){
        time_t current=time(NULL);struct tm local;localtime_r(&current,&local);unsigned minute=local.tm_hour*60+local.tm_min;
        alarm_state.settings.display=(display_schedule_t){true,(minute+1439)%1440,(minute+2)%1440};
        clock_ui_update();assert(backlight_dim);clock_ui_touch();assert(!backlight_dim);
        ticks+=29999;clock_ui_update();assert(!backlight_dim);ticks+=1;clock_ui_update();assert(backlight_dim);
        alarm_state.ringing=1;clock_ui_update();assert(!backlight_dim);
        puts("PASS UI night dim, immediate touch wake, expiry and ringing backlight");
    }
    FILE *f=fopen(argv[1],"wb");if(!f)return 3;fprintf(f,"P6\n480 320\n255\n");
    for(unsigned i=0;i<480*320;i++){uint16_t p=pixels[i];unsigned char rgb[]={((p>>11)&31)*255/31,((p>>5)&63)*255/63,(p&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);return 0;
}

uint32_t alarm_service_display(const display_schedule_t *s,uint8_t brightness)
{alarm_state.settings.display=*s;alarm_state.settings.brightness=brightness;return ++alarm_state.save_ticket;}
