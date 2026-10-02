#include "sensor_service.h"
/* Render the actual UI with synthetic service data; this is not hardware proof. */
#include "lvgl.h"
#include "clock_ui.h"
#include "alarm_service.h"
#include "weather_service.h"
#include "background_service.h"
#include "ha_service.h"
#include "media_service.h"
#include "sonos_setup.h"
#include "clock_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
static sensor_snapshot_t sensor_mock={.available=true,.fresh=true,.imu_ready=true,.celsius=23.5,.humidity=45};
void sensor_service_snapshot(sensor_snapshot_t *out){*out=sensor_mock;}
bool sensor_service_configure(bool shake,bool rotate,bool wallpaper){sensor_mock.wallpaper_shake=wallpaper;sensor_mock.shake_enabled=shake;sensor_mock.auto_rotate=rotate;return true;}
static bool preview_flipped;
bool board_rotation(bool flipped){preview_flipped=flipped;return true;}
extern const lv_image_dsc_t home_wallpaper;
static background_config_t preview_background={.source=BACKGROUND_LOCAL,.count=1,.images={"blue-hour"}};
static unsigned background_next_count;
void background_service_init(void){}
void background_service_diagnostics(void){}
const lv_image_dsc_t *background_service_image(void){return &home_wallpaper;}
void background_service_snapshot(background_config_t *config,char text[96],bool *busy){if(config)*config=preview_background;if(text)strcpy(text,"Background settings saved");if(busy)*busy=false;}
bool background_service_configure(const background_config_t *config){if(!background_config_valid(config))return false;preview_background=*config;return true;}
static uint32_t preview_save_ticket;
static bool preview_save_done,preview_save_ok=true;
static background_config_t preview_save_draft;
uint32_t background_service_configure_tracked(const background_config_t *config){
    if(!background_config_valid(config))return 0;
    preview_save_draft=*config;preview_save_done=false;return ++preview_save_ticket;
}
static bool preview_key_present;
uint32_t background_service_configure_credentials(const background_config_t *config,const char *key){
    if(key&&!background_api_key_valid(key))return 0;
    uint32_t ticket=background_service_configure_tracked(config);
    if(ticket&&key)preview_key_present=*key!=0;
    return ticket;
}
bool background_service_has_key(void){return preview_key_present;}
bool background_service_notice(unsigned *revision,bool *error){(void)revision;(void)error;return false;}
bool background_service_save_result(uint32_t ticket,bool *success){
    if(ticket!=preview_save_ticket||!preview_save_done)return false;
    *success=preview_save_ok;if(*success)preview_background=preview_save_draft;return true;
}
bool background_service_next(void){background_next_count++;return true;}
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
bool media_service_select_alarm(const char *entity,const char *content,const char *type,bool remote){if(remote&&!*content)return false;if(!media_service_select(entity,content,type))return false;media_state.remote_alarm=remote;return true;}
bool media_service_action(media_action_t action){if(action==MEDIA_PLAY||action==MEDIA_START_SAVED)media_state.player.state=MEDIA_PLAYING;if(action==MEDIA_PAUSE)media_state.player.state=MEDIA_PAUSED;return true;}
bool media_service_refresh(void){return true;}
static unsigned media_save_requests;
uint32_t media_service_select_alarm_tracked(const char *entity,const char *content,const char *type,bool remote)
{if(!media_service_select_alarm(entity,content,type,remote))return 0;media_save_requests++;return media_save_requests;}
static sonos_setup_snapshot_t setup_mock;
void sonos_setup_snapshot(sonos_setup_snapshot_t *s){*s=setup_mock;}
bool sonos_setup_lookup(const char *address)
{
    if(strcmp(address,"192.168.1.50")&&strcmp(address,"192.168.1.50:1400"))return false;
    setup_mock=(sonos_setup_snapshot_t){.ready=true,.revision=setup_mock.revision+1,.target="sonos:192.168.1.50:1400/RINCON_TEST",.name="Duncan room",.status="Choose a favorite",.favorites={.count=1,.total=7,.items={{.id="FV:2/1",.title="Morning music"}}}};return true;
}
bool sonos_setup_page(unsigned start){if(start>=7)return false;setup_mock.revision++;setup_mock.favorites.start=start;strcpy(setup_mock.favorites.items[0].title,start?"Evening radio":"Morning music");return true;}

static uint16_t pixels[480*320];
static uint32_t ticks;
static bool has_weather,backlight_dim,result_unavailable;
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
bool weather_service_radio(bool enabled){weather.radio_paused=!enabled;return true;}
bool weather_service_refresh(void){return true;}
const char *weather_service_timezone(void){return "America/New_York";}
static bool pointer_pressed;static int pointer_x,pointer_y;
static void pointer_read(lv_indev_t *device,lv_indev_data_t *data)
{(void)device;data->state=pointer_pressed?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;data->point=(lv_point_t){pointer_x,pointer_y};}
static void advance(void){for(unsigned i=0;i<20;i++){ticks+=20;lv_timer_handler();}clock_ui_update();lv_refr_now(NULL);}
static void pointer_tap(int x,int y)
{pointer_x=x;pointer_y=y;pointer_pressed=true;advance();pointer_pressed=false;advance();}
static void click_text(lv_obj_t *parent,const char *text)
{
    for(unsigned i=0;i<lv_obj_get_child_count(parent);i++){
        lv_obj_t *child=lv_obj_get_child(parent,i);
        if(lv_obj_check_type(child,&lv_label_class)&&!strcmp(lv_label_get_text(child),text)){
            lv_obj_send_event(lv_obj_get_parent(child),LV_EVENT_CLICKED,NULL);return;
        }
        if(lv_obj_check_type(child,&lv_button_class)){
            /* Navigation keeps stable names independently of icon presentation. */
            const char *name=lv_obj_get_user_data(child);
            if(name&&!strcmp(name,text)){lv_obj_send_event(child,LV_EVENT_CLICKED,NULL);return;}
            lv_obj_t *l=lv_obj_get_child(child,0);
            if(l&&lv_obj_check_type(l,&lv_label_class)&&!strcmp(lv_label_get_text(l),text)){lv_obj_send_event(child,LV_EVENT_CLICKED,NULL);return;}
        }
    }
}
static bool has_text(lv_obj_t *parent,const char *text)
{
    if(lv_obj_check_type(parent,&lv_label_class)&&!strcmp(lv_label_get_text(parent),text))return true;
    for(unsigned i=0;i<lv_obj_get_child_count(parent);i++)if(has_text(lv_obj_get_child(parent,i),text))return true;
    return false;
}
static lv_obj_t *named(lv_obj_t *parent,const char *name)
{
    const char *value=lv_obj_get_user_data(parent);
    if(value&&!strcmp(value,name))return parent;
    for(unsigned i=0;i<lv_obj_get_child_count(parent);i++){
        lv_obj_t *found=named(lv_obj_get_child(parent,i),name);if(found)return found;
    }
    return NULL;
}
int main(int argc,char **argv)
{
    if(argc<2)return 2;has_weather=argc>2&&!strcmp(argv[2],"weather");
    setenv("TZ","EST5EDT,M3.2.0/2,M11.1.0/2",1);tzset();settings_defaults(&alarm_state.settings);
    weather=(weather_snapshot_t){.zip="27358",.location={.name="Summerfield",.timezone="America/New_York"},.manual_location=true,.has_data=has_weather,.connected=has_weather,.signal_known=has_weather,.signal_dbm=-48,.internet_verified=has_weather,.internet_age_seconds=30,.status="Set up Wi-Fi for local weather",
        .network_count=3,.scan_revision=1,.networks={{.ssid="Home Wi-Fi",.rssi=-42,.secured=true},{.ssid="Guest Network",.rssi=-65,.secured=true},{.ssid="Another network",.rssi=-78,.secured=true}}};
    time_t now=time(NULL);struct tm date;localtime_r(&now,&date);date.tm_hour=0;date.tm_min=0;date.tm_sec=0;
    weather.data=(weather_data_t){.temperature=72,.feels_like=71,.high=77,.low=58,.code=2,.day_code=2,.rain_percent=10,.observed_at=now,.fetched_at=now,.day_start=mktime(&date)};
    if(has_weather)strcpy(weather.status,"Weather updated");
    lv_init();lv_tick_set_cb(tick);lv_display_t *d=lv_display_create(480,320);lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB565);
    static uint16_t buffer[480*40];lv_display_set_buffers(d,buffer,NULL,sizeof(buffer),LV_DISPLAY_RENDER_MODE_PARTIAL);lv_display_set_flush_cb(d,flush);
    lv_indev_t *pointer=lv_indev_create();lv_indev_set_type(pointer,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(pointer,pointer_read);
    clock_ui_init();advance();
    if(argc>3&&!strcmp(argv[3],"pointer-test")){
        for(unsigned i=0;i<3;i++){
            pointer_tap(410,288);assert(has_text(lv_screen_active(),"Make it yours"));
            pointer_tap(355,174);assert(has_text(lv_screen_active(),"Media"));
            pointer_tap(66,288);
            pointer_tap(180,288);assert(has_text(lv_screen_active(),"Alarms"));
            pointer_tap(66,288);
        }
        puts("PASS pointer navigation Home-Settings-Media-Home-Alarms");
    }
    if(argc>3&&!strcmp(argv[3],"sonos-test")){
        click_text(lv_screen_active(),"Settings");advance();click_text(lv_screen_active(),"Media");advance();
        click_text(lv_screen_active(),"Setup");advance();click_text(lv_screen_active(),"Sonos");advance();
        assert(has_text(lv_screen_active(),"Sonos on your Wi-Fi"));
        click_text(lv_screen_active(),"Save");advance();assert(has_text(lv_screen_active(),"Find a speaker first"));
        lv_obj_t *address=NULL,*choices=NULL;
        for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
            lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
            if(lv_obj_check_type(o,&lv_textarea_class))address=o;
            if(lv_obj_check_type(o,&lv_dropdown_class))choices=o;
        }
        assert(address&&choices);lv_textarea_set_text(address,"192.168.1.50");
        click_text(lv_screen_active(),"Find");advance();assert(strstr(lv_dropdown_get_options(choices),"Morning music"));
        lv_textarea_set_text(address,"192.168.1.51");advance();click_text(lv_screen_active(),"Save");advance();
        assert(!media_save_requests&&has_text(lv_screen_active(),"Find a speaker first"));
        lv_textarea_set_text(address,"192.168.1.50");click_text(lv_screen_active(),"Find");advance();
        click_text(lv_screen_active(),"Next");advance();assert(strstr(lv_dropdown_get_options(choices),"Evening radio"));
        click_text(lv_screen_active(),"Previous");advance();lv_dropdown_set_selected(choices,1);
        click_text(lv_screen_active(),"Save");advance();assert(media_save_requests==1&&has_text(lv_screen_active(),"Saving speaker..."));
        click_text(lv_screen_active(),"Save");advance();assert(media_save_requests==1);
        media_state.saved_ticket=1;media_state.save_failed=true;advance();assert(has_text(lv_screen_active(),"Save failed; previous player retained"));
        click_text(lv_screen_active(),"Save");advance();media_state.saved_ticket=2;media_state.save_failed=false;advance();
        assert(has_text(lv_screen_active(),"Media")&&!strcmp(media_state.content_type,"sonos-favorite"));
        click_text(lv_screen_active(),"Setup");advance();click_text(lv_screen_active(),"Sonos");advance();
        puts("PASS Sonos UI lookup, pagination, missing lookup, delayed/failed save, duplicate tap suppression and confirmed navigation");
    }
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
                assert(count==3);
                for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
                    lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
                    if(lv_obj_check_type(o,&lv_checkbox_class))lv_obj_add_state(o,LV_STATE_CHECKED);
                }
                lv_textarea_set_text(fields[1],"https://example.test/radio");lv_textarea_set_text(fields[2],"music");
                click_text(lv_screen_active(),"Save");advance();assert(!strcmp(media_state.content,"https://example.test/radio")&&media_state.remote_alarm);
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
        else if(!strcmp(argv[3],"settings")||!strcmp(argv[3],"navigation-test")){click_text(lv_screen_active(),"Settings");advance();}
        else if(!strcmp(argv[3],"display")){click_text(lv_screen_active(),"Settings");advance();click_text(lv_screen_active(),"Display & night mode");advance();}
        else {click_text(lv_screen_active(),"Weather");advance();}
        if(!strcmp(argv[3],"wifi")||!strcmp(argv[3],"radio-test")||!strcmp(argv[3],"connect")){
            click_text(lv_screen_active(),"Wi-Fi");advance();
            if(!strcmp(argv[3],"radio-test")){
                click_text(lv_screen_active(),"Turn off");advance();assert(weather.radio_paused);
                click_text(lv_screen_active(),"Turn on");advance();assert(!weather.radio_paused);
                puts("PASS actual Wi-Fi screen on/off control and label updates");
            }
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
    if(argc>3&&!strcmp(argv[3],"snooze-clock-test")){
        click_text(lv_screen_active(),"Clock");advance();
        alarm_state.ringing=1;advance();
        lv_obj_t *panel=lv_obj_get_child(lv_layer_top(),0);assert(panel);
        click_text(panel,"Snooze 5 min");advance();
        assert(has_text(panel,"Show clock")&&alarm_state.snoozed==1);
        click_text(panel,"Show clock");advance();
        assert(lv_obj_has_flag(panel,LV_OBJ_FLAG_HIDDEN)&&alarm_state.snoozed==1);
        assert(has_text(lv_screen_active(),"Rings again in 05:00"));
        click_text(lv_screen_active(),"Settings");advance();
        assert(has_text(lv_screen_active(),"Time & date"));
        click_text(lv_screen_active(),"Clock");alarm_state.snooze_seconds=217;advance();
        assert(has_text(lv_screen_active(),"Rings again in 03:37"));
        lv_obj_t *card=NULL;
        for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
            lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
            if(lv_obj_check_type(o,&lv_button_class)&&lv_obj_get_user_data(o)&&!strcmp(lv_obj_get_user_data(o),"Upcoming alarms"))card=o;
        }
        assert(card);lv_obj_send_event(card,LV_EVENT_CLICKED,NULL);advance();
        assert(!lv_obj_has_flag(panel,LV_OBJ_FLAG_HIDDEN));
        click_text(panel,"Show clock");advance();
        alarm_state.ringing=2;advance();
        assert(!lv_obj_has_flag(panel,LV_OBJ_FLAG_HIDDEN)&&has_text(panel,"Snooze 5 min")&&!backlight_dim);
        click_text(panel,"Dismiss");advance();
        assert(!alarm_state.snoozed&&!alarm_state.ringing&&lv_obj_get_child_count(lv_layer_top())==0);
        assert(has_text(lv_screen_active(),LV_SYMBOL_BELL));
        alarm_state.ringing=1;advance();panel=lv_obj_get_child(lv_layer_top(),0);
        click_text(panel,"Snooze 5 min");advance();click_text(panel,"Show clock");advance();
        puts("PASS snooze clock: countdown, navigation, reopen, new ring priority and dismissal");
    }
    if(argc>3&&!strcmp(argv[3],"load-failure-test")){
        click_text(lv_screen_active(),"Clock");advance();
        alarm_state.load_failed=true;advance();
        assert(has_text(lv_screen_active(),"Saved alarms unavailable - review Alarms"));
        click_text(lv_screen_active(),"Alarms");advance();
        lv_obj_send_event(lv_obj_get_child(lv_obj_get_child(lv_screen_active(),1),0),LV_EVENT_CLICKED,NULL);advance();
        assert(has_text(lv_screen_active(),"Saved alarms unavailable. Saving replaces\nthe previous alarm configuration."));
        puts("PASS actual UI: failed-load warning on home and explicit replacement notice before alarm save");
    }
    if(argc>3&&!strcmp(argv[3],"save-timeout-test")){
        click_text(lv_screen_active(),"Clock");advance();
        click_text(lv_screen_active(),"Alarms");advance();
        lv_obj_send_event(lv_obj_get_child(lv_obj_get_child(lv_screen_active(),1),0),LV_EVENT_CLICKED,NULL);advance();
        result_unavailable=true;click_text(lv_screen_active(),"Save");advance();
        assert(has_text(lv_screen_active(),"Saving..."));
        ticks+=10000;advance();
        assert(has_text(lv_screen_active(),"Result unavailable; reopen to check alarm"));
        click_text(lv_screen_active(),"Cancel");advance();
        click_text(lv_screen_active(),"Clock");advance();
        click_text(lv_screen_active(),"Settings");advance();
        click_text(lv_screen_active(),"Display & night mode");advance();
        click_text(lv_screen_active(),"Save");advance();
        assert(has_text(lv_screen_active(),"Saving..."));
        ticks+=10000;advance();
        assert(has_text(lv_screen_active(),"Result unavailable; reopen to check settings"));
        puts("PASS actual UI: missing alarm/display receipts time out without false success");
    }
    if(argc>3&&!strcmp(argv[3],"navigation-test")){
        alarm_config_t before[ALARM_COUNT];memcpy(before,alarm_state.settings.alarms,sizeof(before));
        for(unsigned cycle=0;cycle<25;cycle++){
            click_text(lv_screen_active(),"Settings");advance();
            assert(has_text(lv_screen_active(),"Make it yours"));
            click_text(lv_screen_active(),"Wi-Fi");advance();
            assert(has_text(lv_screen_active(),"Scan"));
            click_text(lv_screen_active(),"Back");advance();
            assert(has_text(lv_screen_active(),"Make it yours"));
            click_text(lv_screen_active(),"Location");advance();
            assert(has_text(lv_screen_active(),"Save location"));
            click_text(lv_screen_active(),"Back");advance();
            assert(has_text(lv_screen_active(),"Make it yours"));
            click_text(lv_screen_active(),"Time & date");advance();
            click_text(lv_screen_active(),"Cancel");advance();
            assert(has_text(lv_screen_active(),"Make it yours"));
            click_text(lv_screen_active(),"Weather");advance();
            assert(has_text(lv_screen_active(),"Refresh"));
            click_text(lv_screen_active(),"Wi-Fi");advance();
            click_text(lv_screen_active(),"Back");advance();
            assert(has_text(lv_screen_active(),"Refresh"));
            click_text(lv_screen_active(),"Alarms");advance();
            assert(lv_obj_get_child_count(lv_obj_get_child(lv_screen_active(),1))==ALARM_COUNT);
            click_text(lv_screen_active(),"Clock");advance();
            assert(has_text(lv_screen_active(),LV_SYMBOL_BELL));
        }
        assert(!memcmp(before,alarm_state.settings.alarms,sizeof(before)));
        puts("PASS main navigation: 25 cycles, setup returns to origin, no alarm changes");
    }
    if(argc>3&&!strcmp(argv[3],"background-test")){
        for(unsigned scenario=0;scenario<3;scenario++){
            click_text(lv_screen_active(),"Clock");advance();click_text(lv_screen_active(),"Settings");advance();
            click_text(lv_screen_active(),"Backgrounds");advance();
            lv_obj_t *source=NULL,*period=NULL,*input=NULL;
            for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
                lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
                if(lv_obj_check_type(o,&lv_dropdown_class)){if(!source)source=o;else period=o;}
                if(lv_obj_check_type(o,&lv_textarea_class))input=o;
            }
            assert(source&&period&&input);
            click_text(lv_screen_active(),"Next image");advance();assert(background_next_count==scenario+1);
            lv_dropdown_set_selected(source,scenario==0?2:scenario==1?1:0);
            lv_obj_send_event(source,LV_EVENT_VALUE_CHANGED,NULL);advance();
            if(scenario==0){lv_textarea_set_text(input,"forest lake");lv_dropdown_set_selected(period,2);lv_obj_send_event(period,LV_EVENT_VALUE_CHANGED,NULL);}
            if(scenario==1){
                lv_textarea_set_text(input,"http://bad.example/a");click_text(lv_screen_active(),"Save");advance();
                assert(has_text(lv_screen_active(),"Invalid source or busy; check entries"));
                assert(preview_background.source==BACKGROUND_WALLHAVEN);
                lv_textarea_set_text(input,"https://example.test/a.jpg\nhttps://wallhaven.cc/w/pomle9");
            }
            click_text(lv_screen_active(),"Save");advance();assert(has_text(lv_screen_active(),"Saving..."));
            uint32_t ticket=preview_save_ticket;click_text(lv_screen_active(),"Save");advance();assert(preview_save_ticket==ticket);
            if(scenario==1){
                preview_save_ok=false;preview_save_done=true;advance();
                assert(has_text(lv_screen_active(),"Save failed; please retry"));
                assert(!strcmp(lv_textarea_get_text(input),"https://example.test/a.jpg\nhttps://wallhaven.cc/w/pomle9"));
                assert(preview_background.source==BACKGROUND_WALLHAVEN);
                click_text(lv_screen_active(),"Save");advance();assert(has_text(lv_screen_active(),"Saving..."));
            }
            preview_save_ok=true;preview_save_done=true;advance();
            assert(!has_text(lv_screen_active(),"Backgrounds"));assert(!has_text(lv_screen_active(),"Saving..."));
            assert(preview_background.source==(scenario==0?BACKGROUND_WALLHAVEN:scenario==1?BACKGROUND_SELECTED:BACKGROUND_LOCAL));
            if(scenario==0){assert(preview_background.interval_seconds==900);assert(!strcmp(preview_background.query,"forest lake"));}
            if(scenario==1)assert(preview_background.count==2);
        }
        puts("PASS background confirmed-save return, delayed receipt, duplicate suppression, failure/retry, validation and Next");
    }
    if(argc>3&&!strcmp(argv[3],"battery-test")){
        click_text(lv_screen_active(),"Clock");advance();
        lv_obj_t *indicator=named(lv_screen_active(),"Battery status");assert(indicator&&lv_obj_has_flag(indicator,LV_OBJ_FLAG_HIDDEN));
        sensor_mock.battery=(battery_status_t){.known=true,.present=true,.level_known=true,.percent=73,.charging=true};advance();
        assert(!lv_obj_has_flag(indicator,LV_OBJ_FLAG_HIDDEN));assert(has_text(lv_screen_active(),LV_SYMBOL_CHARGE " 73%"));
        sensor_mock.battery.level_known=false;advance();assert(has_text(lv_screen_active(),LV_SYMBOL_BATTERY_EMPTY " ?"));
        sensor_mock.battery.present=false;advance();assert(lv_obj_has_flag(indicator,LV_OBJ_FLAG_HIDDEN));
        sensor_mock.battery=(battery_status_t){.known=true,.present=true,.level_known=true,.percent=73,.charging=true};advance();
        puts("PASS battery hidden when absent, charging percent, unknown and removal");
    }
    if(argc>3&&!strcmp(argv[3],"rotation-test")){
        click_text(lv_screen_active(),"Clock");advance();
        sensor_mock.auto_rotate=true;sensor_mock.flipped=true;
        clock_ui_touch();clock_ui_update();assert(!preview_flipped);
        advance();assert(!preview_flipped);advance();assert(preview_flipped);
        sensor_mock.auto_rotate=false;sensor_mock.flipped=false;advance();assert(preview_flipped);
        sensor_mock.auto_rotate=true;advance();assert(!preview_flipped);
        puts("PASS paired rotation request, active-touch deferral and disabled orientation lock");
    }
    if(argc>3&&!strcmp(argv[3],"sensors-test")){
        click_text(lv_screen_active(),"Clock");advance();
        click_text(lv_screen_active(),"Settings");advance();click_text(lv_screen_active(),"Sensors & gestures");advance();
        assert(has_text(lv_screen_active(),"Inside case 74.3 F / 45% RH\nMotion sensor: Ready"));
        lv_obj_t *toggle=NULL;
        for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
            lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);if(lv_obj_check_type(o,&lv_checkbox_class)&&!strcmp(lv_checkbox_get_text(o),"Shake to snooze"))toggle=o;
        }
        assert(toggle&&!lv_obj_has_state(toggle,LV_STATE_CHECKED));lv_obj_add_state(toggle,LV_STATE_CHECKED);
        lv_obj_t *wallpaper_toggle=NULL;
        for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
            lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
            if(lv_obj_check_type(o,&lv_checkbox_class)&&!strcmp(lv_checkbox_get_text(o),"Shake to change wallpaper"))wallpaper_toggle=o;
        }
        assert(wallpaper_toggle&&!lv_obj_has_state(wallpaper_toggle,LV_STATE_CHECKED));
        lv_obj_add_state(wallpaper_toggle,LV_STATE_CHECKED);
        click_text(lv_screen_active(),"Save");advance();assert(sensor_mock.shake_enabled&&sensor_mock.wallpaper_shake);
        assert(has_text(lv_screen_active(),"Make it yours"));
        click_text(lv_screen_active(),"Sensors & gestures");advance();
        sensor_mock.fresh=false;advance();assert(has_text(lv_screen_active(),"Inside case 74.3 F / 45% RH (stale)\nMotion sensor: Ready"));
        alarm_state.ringing=1;advance();assert(has_text(lv_layer_top(),"Dismiss"));
        click_text(lv_obj_get_child(lv_layer_top(),0),"Dismiss");advance();assert(!alarm_state.ringing);
        puts("PASS indoor reading, stale state, opt-in saved shake setting and alarm foreground");
    }
    if(argc>3&&!strcmp(argv[3],"background-advanced-test")){
        click_text(lv_screen_active(),"Clock");advance();click_text(lv_screen_active(),"Settings");advance();
        click_text(lv_screen_active(),"Backgrounds");advance();click_text(lv_screen_active(),"Advanced");advance();
        lv_obj_t *general=named(lv_screen_active(),"General"),*nsfw=named(lv_screen_active(),"NSFW");
        assert(general&&nsfw);assert(lv_obj_has_state(general,LV_STATE_CHECKED));
        lv_obj_t *source=NULL;
        for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
            lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
            if(lv_obj_check_type(o,&lv_dropdown_class)){source=o;break;}
        }
        assert(source);lv_dropdown_set_selected(source,2);lv_obj_send_event(source,LV_EVENT_VALUE_CHANGED,NULL);
        lv_obj_add_state(nsfw,LV_STATE_CHECKED);click_text(lv_screen_active(),"Save");advance();
        assert(has_text(lv_screen_active(),"Wallhaven requires an API key for NSFW"));assert(!preview_save_ticket);
        lv_obj_t *key=named(lv_screen_active(),"Wallhaven API key (masked)");assert(key&&lv_textarea_get_password_mode(key));
        lv_textarea_set_text(key,"synthetic-test-key");
        lv_obj_add_state(named(lv_screen_active(),"Anime"),LV_STATE_CHECKED);
        lv_obj_add_state(named(lv_screen_active(),"Sketchy"),LV_STATE_CHECKED);
        lv_dropdown_set_selected(named(lv_screen_active(),"Positioning"),1);
        lv_dropdown_set_selected(named(lv_screen_active(),"Sort"),5);
        lv_textarea_set_text(named(lv_screen_active(),"Rotation seconds (0=fixed)"),"1200");
        lv_textarea_set_text(named(lv_screen_active(),"Ratios: e.g. 16x9,3x2 (blank=any)"),"16x9,3x2");
        click_text(lv_screen_active(),"Save");advance();assert(has_text(lv_screen_active(),"Saving..."));
        preview_save_done=true;preview_save_ok=true;advance();
        assert(preview_background.options.categories==6&&preview_background.options.purity==7);
        assert(preview_background.options.position==1&&preview_background.options.sorting==5);
        assert(preview_background.interval_seconds==1200);
        click_text(lv_screen_active(),"Settings");advance();click_text(lv_screen_active(),"Backgrounds");advance();
        click_text(lv_screen_active(),"Advanced");advance();
        assert(lv_obj_has_state(named(lv_screen_active(),"NSFW"),LV_STATE_CHECKED));
        assert(!*lv_textarea_get_text(named(lv_screen_active(),"Wallhaven API key (masked)")));
        assert(!strcmp(lv_textarea_get_text(named(lv_screen_active(),"Rotation seconds (0=fixed)")),"1200"));
        puts("PASS advanced editable filters, missing-key feedback, masked credentials and saved options round trip");
    }
    if(argc>3&&!strcmp(argv[3],"calendar-test")){
        click_text(lv_screen_active(),"Clock");advance();
        click_text(lv_screen_active(),"Calendar");advance();
        lv_obj_t *cal=NULL;
        for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++){
            lv_obj_t *o=lv_obj_get_child(lv_screen_active(),i);
            if(lv_obj_check_type(o,&lv_calendar_class))cal=o;
        }
        assert(cal);const lv_calendar_date_t today=*lv_calendar_get_today_date(cal);
        lv_calendar_set_month_shown(cal,2028,12);
        click_text(lv_screen_active(),LV_SYMBOL_RIGHT);advance();
        assert(lv_calendar_get_showed_date(cal)->year==2029&&lv_calendar_get_showed_date(cal)->month==1);
        click_text(lv_screen_active(),LV_SYMBOL_LEFT);advance();assert(lv_calendar_get_showed_date(cal)->month==12);
        click_text(lv_screen_active(),"Today");advance();
        assert(lv_calendar_get_showed_date(cal)->month==today.month&&lv_calendar_get_showed_date(cal)->year==today.year);
        alarm_state.ringing=1;advance();assert(has_text(lv_layer_top(),"Dismiss"));
        click_text(lv_obj_get_child(lv_layer_top(),0),"Dismiss");advance();assert(!alarm_state.ringing);
        assert(has_text(lv_screen_active(),"Local calendar / event accounts not connected"));
        puts("PASS calendar year rollover, Today and alarm foreground priority");
    }
    if(argc>3&&!strcmp(argv[3],"signal-test")){
        assert(weather_signal_bars(-60)==3&&weather_signal_bars(-61)==2);
        assert(weather_signal_bars(-75)==2&&weather_signal_bars(-76)==1);
        click_text(lv_screen_active(),"Clock");advance();
        weather.signal_dbm=-81;weather.signal_channel=1;
        click_text(lv_screen_active(),"Wi-Fi shortcut");advance();
        assert(has_text(lv_screen_active(),"Connected: Weak (-81 dBm) / channel 1"));
        assert(has_text(lv_screen_active(),"Home Wi-Fi\nBest nearby: Strong (-42 dBm) / Secured"));
        weather.signal_dbm=-48;weather.signal_channel=36;advance();
        assert(has_text(lv_screen_active(),"Connected: Strong (-48 dBm) / channel 36"));
        puts("PASS shared RSSI boundaries and separate live/nearby signals");
    }
    if(argc>3&&!strcmp(argv[3],"brightness-test")){
        click_text(lv_screen_active(),"Clock");advance();
        alarm_state.settings.display.enabled=false;alarm_state.settings.brightness=160;advance();
        click_text(lv_screen_active(),"Brightness");advance();assert(alarm_state.settings.brightness==25);
        click_text(lv_screen_active(),"Brightness");advance();assert(alarm_state.settings.brightness==160);
        alarm_state.settings.display.enabled=true;advance();
        click_text(lv_screen_active(),"Brightness");advance();assert(has_text(lv_screen_active(),"Scheduled night mode"));
        puts("PASS brightness icon: dim/bright toggle and scheduled display settings");
    }
    if(argc>3&&!strcmp(argv[3],"connectivity-test")){
        click_text(lv_screen_active(),"Clock");advance();
        weather.connected=true;weather.signal_known=true;weather.signal_dbm=-48;
        weather.internet_verified=false;advance();
        assert(has_text(lv_screen_active(),LV_SYMBOL_WARNING "  Internet not verified"));
        weather.internet_verified=true;weather.internet_age_seconds=30;advance();
        assert(has_text(lv_screen_active(),LV_SYMBOL_OK "  Internet checked just now"));
        weather.internet_age_seconds=120;advance();
        assert(has_text(lv_screen_active(),LV_SYMBOL_OK "  Internet checked 2 min ago"));
        weather.internet_age_seconds=1860;advance();
        assert(has_text(lv_screen_active(),LV_SYMBOL_WARNING "  Internet not verified"));
        weather.connected=false;advance();
        assert(has_text(lv_screen_active(),LV_SYMBOL_WIFI "  Wi-Fi disconnected"));
        weather.radio_paused=true;advance();
        assert(has_text(lv_screen_active(),LV_SYMBOL_WIFI "  Wi-Fi off"));
        click_text(lv_screen_active(),"Wi-Fi shortcut");advance();
        assert(has_text(lv_screen_active(),"Scan"));
        click_text(lv_screen_active(),"Back");advance();
        assert(has_text(lv_screen_active(),LV_SYMBOL_BELL));
        audio_error=1;advance();assert(has_text(lv_screen_active(),"Local audio unavailable"));
        puts("PASS connectivity: independent Wi-Fi/HTTPS evidence, age, offline, shortcut/back, warning priority");
    }
    FILE *f=fopen(argv[1],"wb");if(!f)return 3;fprintf(f,"P6\n480 320\n255\n");
    for(unsigned i=0;i<480*320;i++){uint16_t p=pixels[i];unsigned char rgb[]={((p>>11)&31)*255/31,((p>>5)&63)*255/63,(p&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);return 0;
}

uint32_t alarm_service_display(const display_schedule_t *s,uint8_t brightness)
{alarm_state.settings.display=*s;alarm_state.settings.brightness=brightness;return ++alarm_state.save_ticket;}

bool alarm_service_result(uint32_t ticket,alarm_save_result_t *out)
{
    if(result_unavailable||!ticket||ticket!=alarm_state.save_ticket)return false;
    *out=(alarm_save_result_t){.ticket=ticket,.status=alarm_state.save_status};
    return true;
}
