/* Render the actual UI with synthetic service data; this is not hardware proof. */
#include "lvgl.h"
#include "clock_ui.h"
#include "alarm_service.h"
#include "weather_service.h"
#include "ha_service.h"
#include "media_service.h"
#include "clock_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
static ha_snapshot_t ha_state={.endpoint="http://192.168.1.232:8123",.status="Set up Home Assistant"};
void ha_service_snapshot(ha_snapshot_t *s){*s=ha_state;}
bool ha_service_configure(const char *url,const char *token,const char *entity){
    assert(!strcmp(token,"synthetic-preview-token"));
    snprintf(ha_state.endpoint,sizeof(ha_state.endpoint),"%s",url);
    snprintf(ha_state.entity,sizeof(ha_state.entity),"%s",entity);
    ha_state.configured=true;ha_state.fresh=true;ha_state.light.state=HA_OFF;return true;
}
bool ha_service_toggle(void){ha_state.light.state=ha_state.light.state==HA_ON?HA_OFF:HA_ON;return true;}
bool ha_service_refresh(void){return true;}
static media_snapshot_t media_state={.entity="media_player.bedroom",.configured=true,.fresh=true,
    .player={.state=MEDIA_PAUSED,.name="Bedroom speaker",.title="Example track",.artist="Example artist",.capabilities=127,.volume=.35,.volume_known=true},.status="Showing reported player state"};
void media_service_snapshot(media_snapshot_t *s){*s=media_state;}
bool media_service_configure(const char *entity){snprintf(media_state.entity,sizeof(media_state.entity),"%s",entity);return true;}
bool media_service_select(const char *entity,const char *content,const char *type){if(!media_selection_valid(content,type))return false;strcpy(media_state.content,content);strcpy(media_state.content_type,type);return media_service_configure(entity);}
bool media_service_action(media_action_t action){if(action==MEDIA_PLAY||action==MEDIA_START_SAVED)media_state.player.state=MEDIA_PLAYING;if(action==MEDIA_PAUSE)media_state.player.state=MEDIA_PAUSED;return true;}
bool media_service_refresh(void){return true;}
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
bool alarm_service_snooze(void){alarm_state.snooze_seconds=300;alarm_state.snoozed=alarm_state.ringing;alarm_state.ringing=0;return true;}
bool alarm_service_dismiss(void){alarm_state.ringing=0;alarm_state.snoozed=0;return true;}
void board_brightness(bool dim){backlight_dim=dim;}
static esp_err_t audio_error;
esp_err_t audio_status(void){return audio_error;}
bool audio_test(void){return audio_error==ESP_OK;}
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
        if(!strcmp(argv[3],"media")||!strcmp(argv[3],"media-test")||!strcmp(argv[3],"media-setup")){
            click_text(lv_screen_active(),"Settings");advance();click_text(lv_screen_active(),"Media");advance();
            if(!strcmp(argv[3],"media-test")){
                click_text(lv_screen_active(),"Play");advance();assert(media_state.player.state==MEDIA_PLAYING);
                click_text(lv_screen_active(),"Pause");advance();assert(media_state.player.state==MEDIA_PAUSED);
                click_text(lv_screen_active(),"Setup");advance();click_text(lv_screen_active(),"Cancel");advance();
                click_text(lv_screen_active(),"Setup");advance();
                lv_obj_t *fields[3]={0};unsigned count=0;
                for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
                    lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
                    if(lv_obj_check_type(o,&lv_textarea_class)){assert(count<3);fields[count++]=o;}
                }
                assert(count==3);lv_textarea_set_text(fields[1],"https://example.test/radio");lv_textarea_set_text(fields[2],"music");
                click_text(lv_screen_active(),"Save");advance();assert(!strcmp(media_state.content,"https://example.test/radio"));
                click_text(lv_screen_active(),"Start saved");advance();assert(media_state.player.state==MEDIA_PLAYING);
                media_state.fresh=false;advance();unsigned disabled=0;
                for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
                    lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
                    if(lv_obj_check_type(o,&lv_button_class)&&lv_obj_has_state(o,LV_STATE_DISABLED))disabled++;
                }
                assert(disabled==7);media_state.fresh=true;advance();
                puts("PASS media UI selection/save/start, play/pause, cancel and seven stale controls disabled");
            }
            if(!strcmp(argv[3],"media-setup")){click_text(lv_screen_active(),"Setup");advance();}
        }
        else if(!strcmp(argv[3],"alarms")||!strcmp(argv[3],"alarms-test")){
            alarm_state.settings.alarms[0].enabled=true;alarm_state.settings.alarms[0].weekdays=62;
            click_text(lv_screen_active(),"Alarms");advance();
            lv_obj_t *list=lv_obj_get_child(lv_screen_active(),1);assert(lv_obj_get_child_count(list)==ALARM_COUNT);
            if(!strcmp(argv[3],"alarms-test")){
                alarm_config_t before[ALARM_COUNT];memcpy(before,alarm_state.settings.alarms,sizeof(before));
                lv_obj_send_event(lv_obj_get_child(list,7),LV_EVENT_CLICKED,NULL);advance();
                lv_obj_t *hour=NULL;unsigned dropdowns=0;
                for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
                    lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
                    if(lv_obj_check_type(o,&lv_dropdown_class)){if(dropdowns++==1)hour=o;}
                }
                assert(hour);lv_dropdown_set_selected(hour,9);click_text(lv_screen_active(),"Save");advance();
                assert(alarm_state.settings.alarms[7].hour==9);
                assert(!memcmp(before,alarm_state.settings.alarms,7*sizeof(alarm_config_t)));
                list=lv_obj_get_child(lv_screen_active(),1);assert(lv_obj_get_child_count(list)==ALARM_COUNT);
                lv_obj_send_event(lv_obj_get_child(list,0),LV_EVENT_CLICKED,NULL);advance();
                click_text(lv_screen_active(),"Cancel");advance();
                assert(!memcmp(before,alarm_state.settings.alarms,7*sizeof(alarm_config_t)));
                assert(lv_obj_get_child_count(lv_obj_get_child(lv_screen_active(),1))==ALARM_COUNT);
                puts("PASS alarm overview: eight rows, slot8 save, other slots preserved, cancel return");
            }
        }
        else if(!strcmp(argv[3],"ha")||!strcmp(argv[3],"ha-setup")||!strcmp(argv[3],"ha-test")){
            click_text(lv_screen_active(),"Settings");advance();click_text(lv_screen_active(),"Home Assistant");advance();
            if(strcmp(argv[3],"ha")){
                click_text(lv_screen_active(),"Setup");advance();
                if(!strcmp(argv[3],"ha-test")){
                    lv_obj_t *fields[3]={0};unsigned n=0;
                    for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
                        lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
                        if(lv_obj_check_type(o,&lv_textarea_class)){assert(n<3);fields[n++]=o;}
                    }
                    assert(n==3&&lv_textarea_get_password_mode(fields[2]));
                    lv_textarea_set_text(fields[1],"light.bedside");lv_textarea_set_text(fields[2],"synthetic-preview-token");
                    click_text(lv_screen_active(),"Save");advance();
                    assert(ha_state.configured&&ha_state.light.state==HA_OFF);
                    click_text(lv_screen_active(),"Turn on");advance();assert(ha_state.light.state==HA_ON);
                    click_text(lv_screen_active(),"Turn off");advance();assert(ha_state.light.state==HA_OFF);
                    for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++)assert(!lv_obj_check_type(lv_obj_get_child(lv_screen_active(),i),&lv_textarea_class));
                    puts("PASS HA setup secret field, save exit and observed-state toggle UI");
                }
            }
        }
        else if(!strcmp(argv[3],"display")){click_text(lv_screen_active(),"Settings");advance();click_text(lv_screen_active(),"Display & night mode");advance();}
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
    if(argc>3&&!strcmp(argv[3],"audio-error")){
        click_text(lv_screen_active(),"Clock");advance();audio_error=1;advance();
        bool warning=false;
        for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
            lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
            if(lv_obj_check_type(o,&lv_label_class)&&!strcmp(lv_label_get_text(o),"Local audio unavailable"))warning=true;
        }
        assert(warning);alarm_state.ringing=1;advance();
        lv_obj_t *overlay=lv_obj_get_child(lv_layer_top(),0);assert(overlay);
        assert(!strcmp(lv_label_get_text(lv_obj_get_child(overlay,0)),"Audio error"));
        click_text(overlay,"Snooze 5 min");advance();assert(!alarm_state.ringing&&alarm_state.snoozed);assert(!strcmp(lv_label_get_text(lv_obj_get_child(overlay,1)),"Rings again in 05:00"));
        click_text(overlay,"Dismiss");advance();assert(!alarm_state.snoozed);
        assert(lv_obj_get_child_count(lv_layer_top())==0);
        puts("PASS audio fault visible; local snooze and dismiss remain operable");
    }
    if(argc>3&&!strcmp(argv[3],"repeat-test")){
        click_text(lv_screen_active(),"Clock");advance();
        const unsigned masks[]={127,62,65};
        for(unsigned selection=0;selection<3;selection++){
            click_text(lv_screen_active(),"Alarms");advance();
            lv_obj_t *list=lv_obj_get_child(lv_screen_active(),1);
            lv_obj_send_event(lv_obj_get_child(list,0),LV_EVENT_CLICKED,NULL);advance();
            lv_obj_t *repeat=NULL;unsigned n=0;
            for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
                lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
                if(lv_obj_check_type(o,&lv_dropdown_class)&&n++==3)repeat=o;
            }
            assert(repeat);lv_dropdown_set_selected(repeat,selection);lv_obj_send_event(repeat,LV_EVENT_VALUE_CHANGED,NULL);
            click_text(lv_screen_active(),"Save");advance();assert(alarm_state.settings.alarms[0].weekdays==masks[selection]);
            click_text(lv_screen_active(),"Clock");advance();
        }
        click_text(lv_screen_active(),"Alarms");advance();
        lv_obj_send_event(lv_obj_get_child(lv_obj_get_child(lv_screen_active(),1),0),LV_EVENT_CLICKED,NULL);advance();
        click_text(lv_screen_active(),"Mo");advance();click_text(lv_screen_active(),"Save");advance();
        assert(alarm_state.settings.alarms[0].weekdays==67);
        lv_obj_send_event(lv_obj_get_child(lv_obj_get_child(lv_screen_active(),1),0),LV_EVENT_CLICKED,NULL);advance();
        lv_obj_t *repeat=NULL;unsigned count=0;
        for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
            lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
            if(lv_obj_check_type(o,&lv_dropdown_class)&&count++==3)repeat=o;
        }
        assert(repeat);lv_dropdown_set_selected(repeat,4);lv_obj_send_event(repeat,LV_EVENT_VALUE_CHANGED,NULL);advance();
        count=0;
        for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
            lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
            if(lv_obj_check_type(o,&lv_dropdown_class)&&count++>=4)assert(!lv_obj_has_flag(o,LV_OBJ_FLAG_HIDDEN));
        }
        lv_dropdown_set_selected(repeat,0);lv_obj_send_event(repeat,LV_EVENT_VALUE_CHANGED,NULL);
        const char *days[]={"Su","Mo","Tu","We","Th","Fr","Sa"};
        for(unsigned i=0;i<7;i++)click_text(lv_screen_active(),days[i]);
        assert(lv_dropdown_get_selected(repeat)==3);
        uint32_t ticket=alarm_state.save_ticket;click_text(lv_screen_active(),"Save");advance();
        assert(alarm_state.save_ticket==ticket&&alarm_state.settings.alarms[0].weekdays==67);
        click_text(lv_screen_active(),"Cancel");advance();
        for(unsigned i=1;i<ALARM_COUNT;i++)assert(alarm_state.settings.alarms[i].weekdays==127&&alarm_state.settings.alarms[i].hour==7);
        puts("PASS actual alarm UI: presets, custom day edit, once fields, empty mask rejection and other-slot isolation");
    }
    FILE *f=fopen(argv[1],"wb");if(!f)return 3;fprintf(f,"P6\n480 320\n255\n");
    for(unsigned i=0;i<480*320;i++){uint16_t p=pixels[i];unsigned char rgb[]={((p>>11)&31)*255/31,((p>>5)&63)*255/63,(p&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);return 0;
}

uint32_t alarm_service_display(const display_schedule_t *s,uint8_t brightness)
{alarm_state.settings.display=*s;alarm_state.settings.brightness=brightness;return ++alarm_state.save_ticket;}
