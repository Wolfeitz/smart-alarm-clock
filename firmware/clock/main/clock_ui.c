#include "clock_ui.h"
#include "diagnostics.h"
#include "alarm_service.h"
#include "clock_service.h"
#include "local_time.h"
#include "board.h"
#include "audio.h"
#include "weather_service.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static lv_obj_t *root,*time_text,*date_text,*detail,*next_text,*status,*dim_text;
static lv_obj_t *home_place,*home_temperature,*home_forecast;
static lv_obj_t *slot,*hours,*minutes,*repeat,*enabled,*days[7],*year,*month,*day,*edit_status,*overlay,*overlay_detail;
static alarm_config_t draft;
static unsigned index_selected;
static bool editing,pending,time_editing;
static lv_obj_t *time_year,*time_month,*time_day,*time_hour,*time_minute,*time_status;
static uint32_t pending_ticket;
static uint8_t applied_brightness;
static uint64_t wake_until;
static bool settings_view,display_editing,display_pending;
static uint32_t display_ticket;
static lv_obj_t *night_enabled,*night_start_hour,*night_start_minute,*night_end_hour,*night_end_minute,*manual_level,*display_status;
static void show_display(lv_event_t *e);
static void show_settings(lv_event_t *e);
static bool weather_view,network_editing,network_error,location_editing,wifi_listing,network_connecting;
static unsigned shown_scan;
static lv_obj_t *network_list;
static weather_network_t shown_networks[WEATHER_NETWORK_COUNT],chosen_network;
static lv_obj_t *weather_title,*weather_now,*weather_today,*weather_status,*zone_button;
static lv_obj_t *network_password,*network_zip,*network_status,*keyboard;
static void show_weather(lv_event_t *e);

static lv_obj_t *label(lv_obj_t *parent,const char *text,int x,int y,int w,const lv_font_t *font)
{
    lv_obj_t *o=lv_label_create(parent);lv_label_set_text(o,text);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);
    lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_style_text_color(o,lv_color_hex(0xf6eddc),0);return o;
}
static lv_obj_t *button(lv_obj_t *parent,const char *text,int x,int y,int w,lv_event_cb_t cb,void *data)
{
    lv_obj_t *o=lv_button_create(parent);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,44);
    lv_obj_set_style_bg_color(o,lv_color_hex(0x203347),0);lv_obj_set_style_radius(o,12,0);lv_obj_set_style_shadow_width(o,0,0);lv_obj_add_event_cb(o,cb,LV_EVENT_CLICKED,data);
    lv_obj_t *l=lv_label_create(o);lv_label_set_text(l,text);lv_obj_center(l);return o;
}
static void home(void);
static void show_editor(void);
static void show_time_editor(lv_event_t *e);
static void reset_screen(void)
{
    settings_view=false;display_editing=false;display_pending=false;
    weather_view=false;network_editing=false;location_editing=false;wifi_listing=false;network_connecting=false;
    lv_obj_clean(root);lv_obj_set_style_bg_color(root,lv_color_hex(0x0b1119),0);
    lv_obj_remove_flag(root,LV_OBJ_FLAG_SCROLLABLE);
}
static void go_home(lv_event_t *e){(void)e;home();}
static void go_editor(lv_event_t *e){(void)e;index_selected=0;show_editor();}
static void sound(lv_event_t *e){(void)e;audio_test();}
static void dim(lv_event_t *e)
{
    (void)e;alarm_snapshot_t s;alarm_service_snapshot(&s);
    if(s.settings.display.enabled){show_display(NULL);return;}
    if(!alarm_service_brightness(s.settings.brightness<80?160:25))lv_label_set_text(status,"Busy - try again");
}
static void day_toggle(lv_event_t *e)
{
    unsigned i=(unsigned)(uintptr_t)lv_event_get_user_data(e);draft.weekdays^=1u<<i;
}
static void repeat_changed(lv_event_t *e)
{
    (void)e;bool once=lv_dropdown_get_selected(repeat)==1;
    for(unsigned i=0;i<7;i++){if(once)lv_obj_add_flag(days[i],LV_OBJ_FLAG_HIDDEN);else lv_obj_remove_flag(days[i],LV_OBJ_FLAG_HIDDEN);}
    lv_obj_t *dates[]={year,month,day};
    for(unsigned i=0;i<3;i++){if(once)lv_obj_remove_flag(dates[i],LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(dates[i],LV_OBJ_FLAG_HIDDEN);}
}
static void choose_slot(lv_event_t *e){(void)e;index_selected=lv_dropdown_get_selected(slot);show_editor();}
static lv_obj_t *dropdown(const char *options,int x,int y,int w)
{
    lv_obj_t *o=lv_dropdown_create(root);lv_dropdown_set_options(o,options);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,44);return o;
}
static lv_obj_t *number_list(int first,int last,int x,int y,int w)
{
    char options[512]={0};size_t n=0;
    for(int i=first;i<=last;i++)n+=snprintf(options+n,sizeof(options)-n,i==last?"%02d":"%02d\n",i);
    return dropdown(options,x,y,w);
}
static void show_settings(lv_event_t *e)
{
    (void)e;editing=false;time_editing=false;reset_screen();settings_view=true;
    label(root,"Settings",10,15,460,&lv_font_montserrat_20);
    button(root,"Time & date",80,75,320,show_time_editor,NULL);
    button(root,"Display & night mode",80,135,320,show_display,NULL);
    button(root,"Clock",150,260,180,go_home,NULL);
}
static void save_display(lv_event_t *e)
{
    (void)e;if(display_pending)return;
    display_schedule_t schedule={.enabled=lv_obj_has_state(night_enabled,LV_STATE_CHECKED),
        .start_minute=60*lv_dropdown_get_selected(night_start_hour)+lv_dropdown_get_selected(night_start_minute),
        .end_minute=60*lv_dropdown_get_selected(night_end_hour)+lv_dropdown_get_selected(night_end_minute)};
    if(!display_schedule_valid(&schedule)){lv_label_set_text(display_status,"Start and end must differ");return;}
    display_ticket=alarm_service_display(&schedule,lv_dropdown_get_selected(manual_level)?25:160);
    display_pending=display_ticket!=0;lv_label_set_text(display_status,display_pending?"Saving...":"Busy - try again");
}
static void show_display(lv_event_t *e)
{
    (void)e;alarm_snapshot_t state;alarm_service_snapshot(&state);editing=false;time_editing=false;reset_screen();display_editing=true;
    label(root,"Scheduled night mode",10,15,330,&lv_font_montserrat_20);
    night_enabled=lv_switch_create(root);lv_obj_set_pos(night_enabled,375,12);lv_obj_set_size(night_enabled,70,36);
    if(state.settings.display.enabled)lv_obj_add_state(night_enabled,LV_STATE_CHECKED);
    label(root,"Dim from",15,60,205,&lv_font_montserrat_16);label(root,"Brighten at",255,60,205,&lv_font_montserrat_16);
    night_start_hour=number_list(0,23,25,88,90);night_start_minute=number_list(0,59,125,88,90);
    night_end_hour=number_list(0,23,265,88,90);night_end_minute=number_list(0,59,365,88,90);
    lv_dropdown_set_selected(night_start_hour,state.settings.display.start_minute/60);lv_dropdown_set_selected(night_start_minute,state.settings.display.start_minute%60);
    lv_dropdown_set_selected(night_end_hour,state.settings.display.end_minute/60);lv_dropdown_set_selected(night_end_minute,state.settings.display.end_minute%60);
    label(root,"When schedule is off:",10,158,240,&lv_font_montserrat_16);
    manual_level=dropdown("Bright\nDim",265,145,190);lv_dropdown_set_selected(manual_level,state.settings.brightness<80);
    display_status=label(root,"Local time / touch wakes for 30 seconds",10,207,460,&lv_font_montserrat_16);
    button(root,"Cancel",45,264,170,show_settings,NULL);button(root,"Save",265,264,170,save_display,NULL);
}
static void save(lv_event_t *e)
{
    (void)e;if(pending)return;draft.hour=lv_dropdown_get_selected(hours);draft.minute=lv_dropdown_get_selected(minutes);
    draft.enabled=lv_obj_has_state(enabled,LV_STATE_CHECKED);
    if(lv_dropdown_get_selected(repeat)==1){
        draft.weekdays=0;draft.once_date=(2000+lv_dropdown_get_selected(year))*10000+(1+lv_dropdown_get_selected(month))*100+1+lv_dropdown_get_selected(day);
    }else if(!draft.weekdays){lv_label_set_text(edit_status,"Choose at least one day");return;}
    if(!alarm_config_valid(&draft)){lv_label_set_text(edit_status,"Check the date");return;}
    pending_ticket=alarm_service_save_tracked(index_selected,&draft);
    pending=pending_ticket!=0;lv_label_set_text(edit_status,pending?"Saving...":"Unable to queue save");
}
static void show_editor(void)
{
    alarm_snapshot_t s;alarm_service_snapshot(&s);draft=s.settings.alarms[index_selected];editing=true;time_editing=false;pending=false;reset_screen();
    label(root,"Alarm",15,12,240,&lv_font_montserrat_20);
    enabled=lv_switch_create(root);lv_obj_set_pos(enabled,380,10);lv_obj_set_size(enabled,70,36);
    if(draft.enabled)lv_obj_add_state(enabled,LV_STATE_CHECKED);
    slot=dropdown("Alarm 1\nAlarm 2\nAlarm 3\nAlarm 4\nAlarm 5\nAlarm 6\nAlarm 7\nAlarm 8",15,65,110);
    lv_dropdown_set_selected(slot,index_selected);lv_obj_add_event_cb(slot,choose_slot,LV_EVENT_VALUE_CHANGED,NULL);
    hours=number_list(0,23,145,65,75);minutes=number_list(0,59,230,65,75);
    lv_dropdown_set_selected(hours,draft.hour);lv_dropdown_set_selected(minutes,draft.minute);
    repeat=dropdown("Repeat\nOnce",325,65,140);lv_dropdown_set_selected(repeat,draft.weekdays?0:1);
    const char *names[]={"Su","Mo","Tu","We","Th","Fr","Sa"};
    for(unsigned i=0;i<7;i++){
        days[i]=button(root,names[i],15+i*65,142,58,day_toggle,(void*)(uintptr_t)i);
        lv_obj_add_flag(days[i],LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_style_bg_color(days[i],lv_color_hex(0x42797b),LV_STATE_CHECKED);
        if(draft.weekdays&(1u<<i))lv_obj_add_state(days[i],LV_STATE_CHECKED);
    }
    time_t now=time(NULL);struct tm local;localtime_r(&now,&local);
    unsigned date=draft.once_date?draft.once_date:clock_valid()?alarm_date(&local):20260101;
    year=number_list(2000,2099,35,142,125);month=number_list(1,12,180,142,110);day=number_list(1,31,310,142,110);
    lv_dropdown_set_selected(year,date/10000-2000);lv_dropdown_set_selected(month,date/100%100-1);lv_dropdown_set_selected(day,date%100-1);
    lv_obj_add_event_cb(repeat,repeat_changed,LV_EVENT_VALUE_CHANGED,NULL);repeat_changed(NULL);
    edit_status=label(root,"24-hour time  /  select days or a date",15,211,450,&lv_font_montserrat_16);
    button(root,"Cancel",50,260,160,go_home,NULL);button(root,"Save",270,260,160,save,NULL);
}
static void save_time(lv_event_t *e)
{
    (void)e;time_t epoch;
    if(!local_time_epoch(2000+lv_dropdown_get_selected(time_year),1+lv_dropdown_get_selected(time_month),
        1+lv_dropdown_get_selected(time_day),lv_dropdown_get_selected(time_hour),lv_dropdown_get_selected(time_minute),&epoch)){
        lv_label_set_text(time_status,"Invalid date or skipped DST time");return;
    }
    if(clock_set(epoch)!=ESP_OK){lv_label_set_text(time_status,"RTC write failed - please retry");return;}
    home();clock_ui_update();
}
static void show_time_editor(lv_event_t *e)
{
    (void)e;editing=false;pending=false;time_editing=true;reset_screen();
    label(root,"Set local time",15,10,450,&lv_font_montserrat_20);
    label(root,"Year             Month             Day",15,47,450,&lv_font_montserrat_16);
    time_year=number_list(2000,2099,30,72,130);time_month=number_list(1,12,180,72,120);time_day=number_list(1,31,320,72,120);
    label(root,"Hour (24h)           Minute",50,134,380,&lv_font_montserrat_16);
    time_hour=number_list(0,23,110,160,110);time_minute=number_list(0,59,260,160,110);
    struct tm local={.tm_year=126,.tm_mon=0,.tm_mday=1};
    if(clock_valid()){time_t now=time(NULL);localtime_r(&now,&local);}
    lv_dropdown_set_selected(time_year,local.tm_year-100);lv_dropdown_set_selected(time_month,local.tm_mon);
    lv_dropdown_set_selected(time_day,local.tm_mday-1);lv_dropdown_set_selected(time_hour,local.tm_hour);lv_dropdown_set_selected(time_minute,local.tm_min);
    time_status=label(root,weather_service_timezone(),10,222,460,&lv_font_montserrat_16);
    button(root,"Cancel",50,260,160,go_home,NULL);button(root,"Save time",270,260,160,save_time,NULL);
}
static void snooze(lv_event_t *e){(void)e;alarm_service_snooze();}
static void dismiss(lv_event_t *e){(void)e;alarm_service_dismiss();}
static void home(void)
{
    editing=false;time_editing=false;pending=false;reset_screen();
    date_text=label(root,"Clock",18,19,310,&lv_font_montserrat_20);
    lv_obj_set_style_text_align(date_text,LV_TEXT_ALIGN_LEFT,0);
    button(root,"Settings",350,12,115,show_settings,NULL);
    time_text=label(root,"--:--",22,76,270,&lv_font_montserrat_48);
    lv_obj_set_style_text_color(time_text,lv_color_hex(0xffd998),0);
    lv_obj_set_style_transform_pivot_x(time_text,LV_PCT(50),0);lv_obj_set_style_transform_pivot_y(time_text,LV_PCT(50),0);lv_obj_set_style_transform_scale(time_text,320,0);
    detail=label(root,"Set time to begin",20,143,270,&lv_font_montserrat_20);
    next_text=label(root,"No alarms enabled",20,192,270,&lv_font_montserrat_16);
    status=label(root,"",20,222,270,&lv_font_montserrat_16);
    lv_obj_set_style_text_color(status,lv_color_hex(0x95a8ba),0);
    lv_obj_t *card=button(root,"",310,74,155,show_weather,NULL);lv_obj_set_height(card,167);lv_obj_set_style_pad_all(card,0,0);
    lv_obj_set_style_bg_color(card,lv_color_hex(0x142a35),0);lv_obj_remove_flag(card,LV_OBJ_FLAG_SCROLLABLE);
    home_place=label(card,"Summerfield",4,12,147,&lv_font_montserrat_16);
    lv_obj_set_height(home_place,22);lv_label_set_long_mode(home_place,LV_LABEL_LONG_DOT);
    home_temperature=label(card,"-- F",4,45,147,&lv_font_montserrat_48);
    lv_obj_set_style_text_color(home_temperature,lv_color_hex(0x9edbd2),0);
    home_forecast=label(card,"Set up Wi-Fi\nfor weather",4,107,147,&lv_font_montserrat_16);
    lv_obj_t *b=button(root,"Dim",15,261,140,dim,NULL);dim_text=lv_obj_get_child(b,0);
    button(root,"Alarms",170,261,140,go_editor,NULL);button(root,"Weather",325,261,140,show_weather,NULL);
}
static void keyboard_event(lv_event_t *e)
{
    if(lv_event_get_code(e)==LV_EVENT_READY||lv_event_get_code(e)==LV_EVENT_CANCEL){
        lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);lv_keyboard_set_textarea(keyboard,NULL);
    }
}
static void field_focus(lv_event_t *e)
{
    network_error=false;lv_obj_t *target=lv_event_get_target(e);lv_keyboard_set_textarea(keyboard,target);
    lv_keyboard_set_mode(keyboard,location_editing?LV_KEYBOARD_MODE_NUMBER:LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_remove_flag(keyboard,LV_OBJ_FLAG_HIDDEN);lv_obj_move_foreground(keyboard);
}
static lv_obj_t *network_field(const char *caption,const char *text,int y,unsigned max,bool secret)
{
    label(root,caption,8,y+12,100,&lv_font_montserrat_16);
    lv_obj_t *o=lv_textarea_create(root);lv_obj_set_pos(o,112,y);lv_obj_set_size(o,355,42);
    lv_textarea_set_one_line(o,true);lv_textarea_set_max_length(o,max);lv_textarea_set_password_mode(o,secret);
    lv_textarea_set_text(o,text);lv_obj_add_event_cb(o,field_focus,LV_EVENT_CLICKED,NULL);return o;
}
static void show_network(lv_event_t *e);
static void save_network(lv_event_t *e)
{
    (void)e;const char *password=lv_textarea_get_text(network_password);
    bool ok=(!chosen_network.secured||strlen(password)>=8)&&weather_service_connect(chosen_network.ssid,password);
    lv_label_set_text(network_status,ok?"Connecting...":"Enter password (8+ characters), then Connect");
    network_error=!ok;network_connecting=ok;if(ok)lv_textarea_set_text(network_password,"");
}
static void show_password(lv_event_t *e)
{
    bool hidden=lv_textarea_get_password_mode(network_password);lv_textarea_set_password_mode(network_password,!hidden);
    lv_obj_t *button=lv_event_get_target(e);lv_label_set_text(lv_obj_get_child(button,0),hidden?"Hide":"Show");
}
static void choose_network(lv_event_t *e)
{
    unsigned index=(unsigned)(uintptr_t)lv_event_get_user_data(e);chosen_network=shown_networks[index];
    if(chosen_network.unsupported){lv_label_set_text(network_status,"This network's security is not supported yet");network_error=true;return;}
    reset_screen();network_editing=true;network_error=false;
    label(root,chosen_network.ssid,10,14,460,&lv_font_montserrat_20);
    label(root,chosen_network.secured?"Enter your Wi-Fi password":"Open network - no password needed",10,55,460,&lv_font_montserrat_16);
    network_password=network_field("Password","",93,63,true);
    lv_obj_set_width(network_password,260);button(root,"Show",380,92,85,show_password,NULL);
    if(!chosen_network.secured)lv_obj_add_state(network_password,LV_STATE_DISABLED);
    network_status=label(root,"",10,154,460,&lv_font_montserrat_16);
    button(root,"Back",45,264,170,show_network,NULL);button(root,"Connect",265,264,170,save_network,NULL);
    keyboard=lv_keyboard_create(root);lv_obj_set_size(keyboard,480,130);lv_obj_align(keyboard,LV_ALIGN_BOTTOM_MID,0,0);
    lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);lv_obj_add_event_cb(keyboard,keyboard_event,LV_EVENT_ALL,NULL);
}
static void rescan(lv_event_t *e)
{
    (void)e;network_error=false;if(!weather_service_scan()){lv_label_set_text(network_status,"Busy - please try Scan again");network_error=true;}
}
static void show_network(lv_event_t *e)
{
    (void)e;editing=false;time_editing=false;reset_screen();network_editing=true;wifi_listing=true;network_error=false;shown_scan=~0u;
    label(root,"Choose Wi-Fi",10,10,300,&lv_font_montserrat_20);button(root,"Scan",345,6,120,rescan,NULL);
    network_list=lv_list_create(root);lv_obj_set_pos(network_list,12,58);lv_obj_set_size(network_list,456,150);
    network_status=label(root,"Scanning nearby networks...",10,217,460,&lv_font_montserrat_16);
    button(root,"Back",20,264,130,show_weather,NULL);button(root,"Test sound",165,264,145,sound,NULL);
    button(root,"Clock",325,264,135,go_home,NULL);weather_service_scan();
}
static void save_location(lv_event_t *e)
{
    (void)e;bool ok=weather_service_location(lv_textarea_get_text(network_zip));
    lv_label_set_text(network_status,ok?"Location saved; updating...":"Enter a 5-digit ZIP, then Save");network_error=!ok;
}
static void estimate_location(lv_event_t *e)
{
    (void)e;bool ok=weather_service_location("");network_error=!ok;
    if(ok)lv_textarea_set_text(network_zip,"");
    lv_label_set_text(network_status,ok?"Estimating location; check suggested ZIP":"Busy - please retry");
}
static void show_location(lv_event_t *e)
{
    (void)e;weather_snapshot_t s;weather_service_snapshot(&s);editing=false;time_editing=false;reset_screen();location_editing=true;network_error=false;
    label(root,"Weather & time zone",10,10,290,&lv_font_montserrat_20);
    button(root,"Use network",315,5,150,estimate_location,NULL);
    label(root,s.manual_location?"Manual location - preserved across Wi-Fi changes":"Estimated from network - check or change ZIP",10,52,460,&lv_font_montserrat_16);
    network_zip=network_field("US ZIP",s.zip,95,5,false);lv_textarea_set_accepted_chars(network_zip,"0123456789");
    network_status=label(root,s.location.name,10,158,460,&lv_font_montserrat_16);
    button(root,"Back",45,264,170,show_weather,NULL);button(root,"Save location",265,264,170,save_location,NULL);
    keyboard=lv_keyboard_create(root);lv_obj_set_size(keyboard,480,130);lv_obj_align(keyboard,LV_ALIGN_BOTTOM_MID,0,0);
    lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);lv_obj_add_event_cb(keyboard,keyboard_event,LV_EVENT_ALL,NULL);
}
static void refresh_weather(lv_event_t *e){(void)e;weather_service_refresh();}
static void restart_zone(lv_event_t *e)
{
    (void)e;alarm_snapshot_t s;alarm_service_snapshot(&s);
    if(s.ringing||s.snoozed){lv_label_set_text(weather_status,"Dismiss active alarm before restarting");return;}
    esp_restart();
}
static void show_weather(lv_event_t *e)
{
    (void)e;editing=false;time_editing=false;reset_screen();weather_view=true;
    weather_title=label(root,"Local weather",15,12,315,&lv_font_montserrat_20);
    button(root,"Refresh",350,8,115,refresh_weather,NULL);
    weather_now=label(root,"Connect Wi-Fi to begin",15,72,450,&lv_font_montserrat_20);
    weather_today=label(root,"Today's forecast will appear here",15,123,450,&lv_font_montserrat_16);
    weather_status=label(root,"",15,177,450,&lv_font_montserrat_16);
    label(root,"Open-Meteo / CC BY 4.0",15,235,290,&lv_font_montserrat_16);
    button(root,"Clock",15,264,130,go_home,NULL);button(root,"Wi-Fi",165,264,140,show_network,NULL);button(root,"Location",325,264,140,show_location,NULL);
    zone_button=button(root,"Apply zone",325,211,140,restart_zone,NULL);lv_obj_add_flag(zone_button,LV_OBJ_FLAG_HIDDEN);
}
void clock_ui_init(void)
{
    lv_display_t *display=lv_display_get_default();
    lv_theme_t *theme=lv_theme_default_init(display,lv_color_hex(0x3f9c98),lv_color_hex(0xd5a565),true,&lv_font_montserrat_16);
    lv_display_set_theme(display,theme);root=lv_screen_active();home();
}
void clock_ui_update(void)
{
    alarm_snapshot_t s;alarm_service_snapshot(&s);
    time_t wall=time(NULL);struct tm local_now;localtime_r(&wall,&local_now);
    bool dimmed=display_should_dim(&s.settings.display,clock_valid(),local_now.tm_hour*60+local_now.tm_min,
        s.settings.brightness,s.ringing!=0,esp_timer_get_time()/1000,wake_until);
    uint8_t desired=dimmed?25:160;
    if(desired!=applied_brightness){applied_brightness=desired;board_brightness(dimmed);}
    bool active=s.ringing || s.snoozed;
    if(active && !overlay){
        overlay=lv_obj_create(lv_layer_top());lv_obj_set_size(overlay,460,230);lv_obj_center(overlay);
        lv_obj_set_style_bg_color(overlay,lv_color_hex(0x142c40),0);lv_obj_remove_flag(overlay,LV_OBJ_FLAG_SCROLLABLE);
        label(overlay,"Alarm",0,12,420,&lv_font_montserrat_48);
        overlay_detail=label(overlay,"",0,86,420,&lv_font_montserrat_20);
        button(overlay,"Snooze 5 min",5,140,195,snooze,NULL);button(overlay,"Dismiss",215,140,195,dismiss,NULL);
    }
    if(!active && overlay){lv_obj_delete(overlay);overlay=NULL;}
    if(overlay){
        lv_obj_t *title=lv_obj_get_child(overlay,0);lv_label_set_text(title,s.ringing?"Alarm":"Snoozed");
        char text[80];
        if(s.ringing){
            size_t n=0;n+=snprintf(text,sizeof(text),"Alarm ");
            for(unsigned i=0;i<ALARM_COUNT;i++)if(s.ringing&(1u<<i))n+=snprintf(text+n,sizeof(text)-n,"%s%u",n>6?", ":"",i+1);
        }else{
            uint32_t next=0;for(unsigned i=0;i<ALARM_COUNT;i++)if((s.snoozed&(1u<<i)) && (!next || s.settings.deadline[i]<next))next=s.settings.deadline[i];
            int64_t remaining=(int64_t)next-time(NULL);if(remaining<0)remaining=0;
            snprintf(text,sizeof(text),"Rings again in %02u:%02u",(unsigned)(remaining/60),(unsigned)(remaining%60));
        }
        lv_label_set_text(overlay_detail,text);
    }
    if(display_editing){
        if(display_pending&&s.save_ticket==display_ticket){
            display_pending=false;
            if(s.save_status==ESP_OK){home();return;}
            lv_label_set_text(display_status,"Save failed - settings unchanged");
        }
        return;
    }
    if(settings_view)return;
    if(weather_view||network_editing||location_editing){
        weather_snapshot_t w;weather_service_snapshot(&w);
        if(network_editing||location_editing){
            if(network_connecting&&!w.busy&&w.connected&&!strcmp(w.ssid,chosen_network.ssid)){show_weather(NULL);return;}
            if(location_editing&&!w.manual_location&&!lv_textarea_get_text(network_zip)[0]&&w.zip[0])lv_textarea_set_text(network_zip,w.zip);
            if(wifi_listing&&shown_scan!=w.scan_revision){
                shown_scan=w.scan_revision;lv_obj_clean(network_list);memcpy(shown_networks,w.networks,sizeof(shown_networks));
                for(unsigned i=0;i<w.network_count;i++){
                    char title[100],name[33];strcpy(name,shown_networks[i].ssid);
                    for(char *p=name;*p;p++)if((unsigned char)*p<32)*p=' ';
                    snprintf(title,sizeof(title),"%s   %s  %s",name,shown_networks[i].rssi>-60?"Strong":shown_networks[i].rssi>-75?"Good":"Weak",shown_networks[i].secured?"Secured":"Open");
                    lv_obj_t *b=lv_list_add_button(network_list,LV_SYMBOL_WIFI,title);lv_obj_add_event_cb(b,choose_network,LV_EVENT_CLICKED,(void*)(uintptr_t)i);
                }
                if(!w.network_count)lv_list_add_text(network_list,w.scanning?"Scanning...":"No networks listed. Tap Scan.");
            }
            if(network_error)return;
            if(w.scanning)lv_label_set_text(network_status,"Scanning nearby networks...");
            else if(w.busy)lv_label_set_text(network_status,"Working...");
            else lv_label_set_text(network_status,w.status);
            return;
        }
        char text[180];snprintf(text,sizeof(text),"%s  %s",w.location.name[0]?w.location.name:"ZIP",w.zip);lv_label_set_text(weather_title,text);
        if(w.has_data){
            snprintf(text,sizeof(text),"%.0f F  /  %s\nFeels like %.0f F",w.data.temperature,weather_condition(w.data.code),w.data.feels_like);lv_label_set_text(weather_now,text);
            struct tm date;time_t start=w.data.day_start;localtime_r(&start,&date);char day_text[32];strftime(day_text,sizeof(day_text),"%a %b %d",&date);
            snprintf(text,sizeof(text),"%s: %s\nHigh %.0f F   Low %.0f F   Rain %d%%",day_text,weather_condition(w.data.day_code),w.data.high,w.data.low,w.data.rain_percent);lv_label_set_text(weather_today,text);
        }else{lv_label_set_text(weather_now,"Weather not available yet");lv_label_set_text(weather_today,"Your clock and alarms work offline");}
        if(w.restart_for_zone)snprintf(text,sizeof(text),"%s\nApply zone restarts the clock",w.location.timezone);
        else if(w.has_data&&!weather_fresh(&w.data,time(NULL)))snprintf(text,sizeof(text),"OUTDATED weather - reconnect to update\n%s",w.status);
        else snprintf(text,sizeof(text),"%s\n%s",w.connected?"Wi-Fi connected":"Offline",w.status);
        lv_obj_set_width(weather_status,w.restart_for_zone?295:450);lv_label_set_text(weather_status,text);
        if(w.restart_for_zone)lv_obj_remove_flag(zone_button,LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(zone_button,LV_OBJ_FLAG_HIDDEN);
        return;
    }
    if(time_editing)return;
    if(editing){
        if(pending && s.save_ticket==pending_ticket){
            pending=false;
            if(s.save_status==ESP_OK)home();
            else lv_label_set_text(edit_status,"Save failed - settings unchanged");
        }
        return;
    }
    char b[80];
    if(clock_valid()){
        time_t now=time(NULL);struct tm local;localtime_r(&now,&local);
        strftime(b,sizeof(b),"%I:%M",&local);lv_label_set_text(time_text,b[0]=='0'?b+1:b);
        strftime(b,sizeof(b),"%A, %B %d",&local);lv_label_set_text(date_text,b);
        strftime(b,sizeof(b),"%p   :%S   %Z",&local);lv_label_set_text(detail,b);
    }
    time_t next=clock_valid()?alarm_next(s.settings.alarms,time(NULL)):0;
    if(next){struct tm local;localtime_r(&next,&local);strftime(b,sizeof(b),"Next: %a %I:%M %p",&local);}
    else snprintf(b,sizeof(b),"%s",clock_valid()?"No upcoming alarms":"Set time to arm alarms");
    lv_label_set_text(next_text,b);
    lv_label_set_text(status,s.storage_status==ESP_OK?clock_source():"Settings storage error");
    lv_label_set_text(dim_text,s.settings.display.enabled?"Display":applied_brightness<80?"Brighten":"Dim");
    weather_snapshot_t weather;weather_service_snapshot(&weather);
    lv_label_set_text(home_place,weather.location.name[0]?weather.location.name:"Local weather");
    if(weather.has_data){
        snprintf(b,sizeof(b),"%.0f°",weather.data.temperature);lv_label_set_text(home_temperature,b);
        snprintf(b,sizeof(b),"%s\nH %.0f°  L %.0f°",weather_fresh(&weather.data,time(NULL))?weather_condition(weather.data.code):"Outdated",weather.data.high,weather.data.low);
        lv_label_set_text(home_forecast,b);
    }else{lv_label_set_text(home_temperature,"--°");lv_label_set_text(home_forecast,weather.connected?"Updating...":"Set up Wi-Fi\nfor weather");}
}

static void dropdown_diagnostics(const char *name,lv_obj_t *o)
{
    diagnostics_printf("UI_SELECT name=%s selected=%u open=%u\n",name,(unsigned)lv_dropdown_get_selected(o),lv_dropdown_is_open(o));
    if(lv_dropdown_is_open(o)){
        lv_obj_t *list=lv_dropdown_get_list(o);lv_area_t area;lv_obj_get_coords(list,&area);
        diagnostics_printf("UI_LIST name=%s x1=%ld y1=%ld x2=%ld y2=%ld scroll=%ld\n",name,(long)area.x1,(long)area.y1,(long)area.x2,(long)area.y2,(long)lv_obj_get_scroll_y(list));
    }
}
void clock_ui_diagnostics(void)
{
    diagnostics_printf("UI_SCREEN name=%s overlay=%u pending=%u\n",display_editing?"display":settings_view?"settings":location_editing?"location":network_editing?"network":weather_view?"weather":time_editing?"time":editing?"alarm":"home",overlay!=NULL,pending);
    if(weather_view||network_editing||location_editing){weather_snapshot_t w;weather_service_snapshot(&w);
        diagnostics_printf("WEATHER_STATE online=%u valid=%u fresh=%u zip=%s zone=%s status=%s\n",w.connected,w.has_data,w.has_data&&weather_fresh(&w.data,time(NULL)),w.zip,w.location.timezone,w.status);
    }
    alarm_snapshot_t state;alarm_service_snapshot(&state);
    diagnostics_printf("DISPLAY_STATE auto=%u start=%u end=%u level=%u\n",state.settings.display.enabled,state.settings.display.start_minute,state.settings.display.end_minute,applied_brightness);
    if(overlay)diagnostics_printf("UI_OVERLAY text=%s\n",lv_label_get_text(overlay_detail));
    if(editing){
        diagnostics_printf("UI_ALARM slot=%u enabled=%u weekdays=%u message=%s\n",index_selected,lv_obj_has_state(enabled,LV_STATE_CHECKED),draft.weekdays,lv_label_get_text(edit_status));
        dropdown_diagnostics("slot",slot);dropdown_diagnostics("hour",hours);dropdown_diagnostics("minute",minutes);dropdown_diagnostics("repeat",repeat);
        if(lv_dropdown_get_selected(repeat)==1){dropdown_diagnostics("year",year);dropdown_diagnostics("month",month);dropdown_diagnostics("day",day);}
    }else if(time_editing){
        dropdown_diagnostics("year",time_year);dropdown_diagnostics("month",time_month);dropdown_diagnostics("day",time_day);
        dropdown_diagnostics("hour",time_hour);dropdown_diagnostics("minute",time_minute);
        diagnostics_printf("UI_MESSAGE text=%s\n",lv_label_get_text(time_status));
    }
}

void clock_ui_touch(void)
{
    wake_until=esp_timer_get_time()/1000+30000;
    alarm_snapshot_t s;alarm_service_snapshot(&s);
    if(s.settings.display.enabled&&applied_brightness<80){applied_brightness=160;board_brightness(false);}
}
