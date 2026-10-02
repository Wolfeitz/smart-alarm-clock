#include "clock_ui.h"
#include "diagnostics.h"
#include "alarm_service.h"
#include "clock_service.h"
#include "local_time.h"
#include "board.h"
#include "audio.h"
#include "weather_service.h"
#include "background_service.h"
#include "ha_service.h"
#include "media_service.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern const lv_image_dsc_t home_wallpaper;
static bool scenic_home,background_editing,background_error;
static lv_obj_t *home_image,*background_source,*background_interval,*background_input,*background_status;
static background_config_t background_draft;
static void show_backgrounds(lv_event_t *e);
static lv_obj_t *root,*time_text,*date_text,*detail,*next_text,*status;
static lv_obj_t *home_place,*home_temperature,*home_forecast,*alarm_caption;
static bool snooze_collapsed,calendar_view;
static lv_obj_t *calendar,*calendar_heading,*weather_art;
static int artwork_code=-999;
static void show_calendar(lv_event_t *e);
static lv_obj_t *slot,*hours,*minutes,*repeat,*enabled,*days[7],*year,*month,*day,*edit_status,*overlay,*overlay_detail;
static alarm_config_t draft;
static unsigned index_selected;
static bool editing,pending,time_editing,alarm_list_view;
static lv_obj_t *alarm_rows[ALARM_COUNT];
static void show_alarms(lv_event_t *e);
static lv_obj_t *time_year,*time_month,*time_day,*time_hour,*time_minute,*time_status;
static uint32_t pending_ticket;
static int64_t pending_since,display_since;
static uint8_t applied_brightness;
static uint64_t wake_until;
static bool settings_view,display_editing,display_pending;
static uint32_t display_ticket;
static lv_obj_t *night_enabled,*night_start_hour,*night_start_minute,*night_end_hour,*night_end_minute,*manual_level,*display_status;
static void show_display(lv_event_t *e);
static void show_settings(lv_event_t *e);
static bool weather_view,network_editing,network_error,location_editing,wifi_listing,network_connecting;
static unsigned shown_scan;
static unsigned setup_origin; /* 0 weather, 1 settings, 2 clock */
static lv_obj_t *wifi_indicator,*wifi_dot,*wifi_strength[3];
static lv_obj_t *network_list,*network_link;
static weather_network_t shown_networks[WEATHER_NETWORK_COUNT],chosen_network;
static lv_obj_t *weather_title,*weather_now,*weather_today,*weather_status,*zone_button;
static lv_obj_t *network_password,*network_zip,*network_status,*keyboard,*radio_button;
static void show_weather(lv_event_t *e);
static void show_network(lv_event_t *e);
static void home_network(lv_event_t *e){setup_origin=2;show_network(e);}
static void show_location(lv_event_t *e);
static void show_ha(lv_event_t *e);
static void show_media(lv_event_t *e);
static bool media_view,media_editing,media_error;
static lv_obj_t *media_name,*media_title,*media_artist,*media_info,*media_status,*media_entity,*media_content,*media_type,*media_remote,*media_controls[7];
static bool ha_view,ha_editing,ha_error;
static lv_obj_t *ha_name,*ha_state,*ha_status,*ha_toggle,*ha_url,*ha_entity,*ha_token;

static lv_obj_t *label(lv_obj_t *parent,const char *text,int x,int y,int w,const lv_font_t *font)
{
    lv_obj_t *o=lv_label_create(parent);lv_label_set_text(o,text);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);
    lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_style_text_color(o,lv_color_hex(0xf3eee5),0);return o;
}
static lv_obj_t *button(lv_obj_t *parent,const char *text,int x,int y,int w,lv_event_cb_t cb,void *data)
{
    lv_obj_t *o=lv_button_create(parent);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,44);
    lv_obj_set_style_bg_color(o,lv_color_hex(0x202d36),0);lv_obj_set_style_radius(o,12,0);lv_obj_set_style_shadow_width(o,0,0);lv_obj_set_style_border_width(o,1,0);lv_obj_set_style_border_color(o,lv_color_hex(0x33434e),0);lv_obj_set_style_bg_color(o,lv_color_hex(0x365961),LV_STATE_PRESSED);lv_obj_add_event_cb(o,cb,LV_EVENT_CLICKED,data);
    lv_obj_t *l=lv_label_create(o);lv_label_set_text(l,text);lv_obj_center(l);return o;
}
static void home(void);
static void show_editor(void);
static void show_time_editor(lv_event_t *e);
static void reset_screen(void)
{
    scenic_home=false;background_editing=false;calendar_view=false;weather_art=NULL;artwork_code=-999;alarm_list_view=false;settings_view=false;display_editing=false;display_pending=false;ha_view=false;ha_editing=false;ha_error=false;media_view=false;media_editing=false;media_error=false;
    weather_view=false;network_editing=false;location_editing=false;wifi_listing=false;network_connecting=false;
    lv_obj_clean(root);lv_obj_set_style_bg_color(root,lv_color_hex(0x0c141b),0);
    lv_obj_remove_flag(root,LV_OBJ_FLAG_SCROLLABLE);
}
static void go_home(lv_event_t *e){(void)e;home();}
/* Weather is not included in LVGL's built-in symbol font. Construct a small
 * sun/cloud silhouette with native objects, without a bitmap or emoji font. */
static void weather_nav_icon(lv_obj_t *button, bool selected)
{
    const uint32_t ink=selected?0xd7fff0:0xa8bac7;
    const int circles[][3]={{49,8,12},{39,20,10},{45,14,15},{55,19,11}};
    for(unsigned i=0;i<4;i++){
        lv_obj_t *o=lv_obj_create(button);
        lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_pos(o,circles[i][0],circles[i][1]);
        lv_obj_set_size(o,circles[i][2],circles[i][2]);
        lv_obj_set_style_radius(o,LV_RADIUS_CIRCLE,0);
        lv_obj_set_style_border_width(o,i==0?2:0,0);
        lv_obj_set_style_border_color(o,lv_color_hex(ink),0);
        lv_obj_set_style_bg_color(o,lv_color_hex(ink),0);
        if(i==0)lv_obj_set_style_bg_opa(o,LV_OPA_TRANSP,0);
    }
    lv_obj_t *base=lv_obj_create(button);
    lv_obj_remove_flag(base,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(base,43,24);lv_obj_set_size(base,19,6);
    lv_obj_set_style_border_width(base,0,0);lv_obj_set_style_radius(base,2,0);
    lv_obj_set_style_bg_color(base,lv_color_hex(ink),0);
}
/* Main destinations have identical positions; editors retain explicit Save/Cancel. */
static void navigation(unsigned selected)
{
    const char *names[]={"Clock","Alarms","Weather","Settings"};
    const char *icons[]={LV_SYMBOL_HOME,LV_SYMBOL_BELL,"",LV_SYMBOL_SETTINGS};
    lv_event_cb_t callbacks[]={go_home,show_alarms,show_weather,show_settings};
    for(unsigned i=0;i<4;i++){
        lv_obj_t *b=button(root,icons[i],12+i*116,266,108,callbacks[i],NULL);
        lv_obj_set_user_data(b,(void *)names[i]);
        lv_obj_set_style_pad_all(b,0,0);
        lv_obj_set_style_text_font(b,&lv_font_montserrat_20,0);
        lv_obj_set_style_radius(b,10,0);
        lv_obj_set_style_bg_color(b,lv_color_hex(i==selected?0x2e5558:0x121e27),0);
        lv_obj_set_style_border_color(b,lv_color_hex(i==selected?0x7dbbb0:0x24343f),0);
        lv_obj_set_style_text_color(b,lv_color_hex(i==selected?0xd7fff0:0xa8bac7),0);
        if(scenic_home){
            lv_obj_set_style_bg_color(b,lv_color_hex(i==selected?0x455b70:0x101a2d),0);
            lv_obj_set_style_bg_opa(b,i==selected?LV_OPA_70:LV_OPA_50,0);
            lv_obj_set_style_border_color(b,lv_color_hex(0xb5c5d7),0);
            lv_obj_set_style_border_opa(b,i==selected?LV_OPA_50:LV_OPA_20,0);
        }
        if(i==2)weather_nav_icon(b,i==selected);
    }
}
static void select_alarm(lv_event_t *e)
{index_selected=(unsigned)(uintptr_t)lv_event_get_user_data(e);show_editor();}
static void update_alarm_rows(const alarm_snapshot_t *s)
{
    for(unsigned i=0;i<ALARM_COUNT;i++){
        const alarm_config_t *a=&s->settings.alarms[i];char schedule[64]={0},text[128];
        if(!a->weekdays)snprintf(schedule,sizeof(schedule),"Once: %04u-%02u-%02u",(unsigned)(a->once_date/10000),(unsigned)(a->once_date/100%100),(unsigned)(a->once_date%100));
        else if(a->weekdays==127)strcpy(schedule,"Every day");
        else if(a->weekdays==62)strcpy(schedule,"Weekdays");
        else if(a->weekdays==65)strcpy(schedule,"Weekends");
        else{
            const char *names[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};size_t n=0;
            for(unsigned d=0;d<7;d++)if(a->weekdays&(1u<<d))n+=snprintf(schedule+n,sizeof(schedule)-n,"%s%s",n?" ":"",names[d]);
        }
        snprintf(text,sizeof(text),"%u   %02u:%02u   %s\n%s",i+1,a->hour,a->minute,a->enabled?"ON":"OFF",schedule);
        lv_label_set_text(alarm_rows[i],text);
    }
}
static void show_alarms(lv_event_t *e)
{
    (void)e;editing=false;time_editing=false;pending=false;reset_screen();alarm_list_view=true;
    label(root,"Alarms",15,12,450,&lv_font_montserrat_20);
    lv_obj_t *list=lv_obj_create(root);lv_obj_set_pos(list,15,52);lv_obj_set_size(list,450,198);
    lv_obj_set_style_pad_all(list,6,0);lv_obj_set_style_border_width(list,0,0);
    lv_obj_set_style_bg_color(list,lv_color_hex(0x0c141b),0);
    lv_obj_set_scroll_dir(list,LV_DIR_VER);
    for(unsigned i=0;i<ALARM_COUNT;i++){
        lv_obj_t *row=button(list,"",0,i*72,422,select_alarm,(void *)(uintptr_t)i);lv_obj_set_height(row,64);
        alarm_rows[i]=lv_obj_get_child(row,0);lv_obj_set_width(alarm_rows[i],394);
        lv_obj_set_style_text_align(alarm_rows[i],LV_TEXT_ALIGN_LEFT,0);lv_obj_center(alarm_rows[i]);
    }
    navigation(1);
    alarm_snapshot_t s;alarm_service_snapshot(&s);update_alarm_rows(&s);
}
static void sound(lv_event_t *e)
{
    bool accepted=audio_test();lv_obj_t *button=lv_event_get_target(e);
    lv_label_set_text(lv_obj_get_child(button,0),accepted?"Sound queued":"Sound unavailable");
}
static void dim(lv_event_t *e)
{
    (void)e;alarm_snapshot_t s;alarm_service_snapshot(&s);
    if(s.settings.display.enabled){show_display(NULL);return;}
    if(!alarm_service_brightness(s.settings.brightness<80?160:25))lv_label_set_text(status,"Busy - try again");
}
enum {REPEAT_DAILY,REPEAT_WEEKDAYS,REPEAT_WEEKENDS,REPEAT_CUSTOM,REPEAT_ONCE};
static unsigned repeat_choice(uint8_t mask)
{return mask==127?REPEAT_DAILY:mask==62?REPEAT_WEEKDAYS:mask==65?REPEAT_WEEKENDS:REPEAT_CUSTOM;}
static void day_toggle(lv_event_t *e)
{
    unsigned i=(unsigned)(uintptr_t)lv_event_get_user_data(e);draft.weekdays^=1u<<i;
    lv_dropdown_set_selected(repeat,repeat_choice(draft.weekdays));
}
static void repeat_changed(lv_event_t *e)
{
    unsigned selection=lv_dropdown_get_selected(repeat);bool once=selection==REPEAT_ONCE;
    if(e&&selection<=REPEAT_WEEKENDS){
        const uint8_t masks[]={127,62,65};draft.weekdays=masks[selection];
        for(unsigned i=0;i<7;i++){
            if(draft.weekdays&(1u<<i))lv_obj_add_state(days[i],LV_STATE_CHECKED);
            else lv_obj_remove_state(days[i],LV_STATE_CHECKED);
        }
    }
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
    label(root,"Make it yours",20,16,440,&lv_font_montserrat_20);
    button(root,"Time & date",15,52,218,show_time_editor,NULL);
    button(root,"Display & night mode",247,52,218,show_display,NULL);
    button(root,"Wi-Fi",15,102,218,show_network,NULL);
    button(root,"Location",247,102,218,show_location,NULL);
    button(root,"Home Assistant",15,152,218,show_ha,NULL);
    button(root,"Media",247,152,218,show_media,NULL);
    button(root,"Backgrounds",15,202,450,show_backgrounds,NULL);
    navigation(3);
}
static void save_display(lv_event_t *e)
{
    (void)e;if(display_pending)return;
    display_schedule_t schedule={.enabled=lv_obj_has_state(night_enabled,LV_STATE_CHECKED),
        .start_minute=60*lv_dropdown_get_selected(night_start_hour)+lv_dropdown_get_selected(night_start_minute),
        .end_minute=60*lv_dropdown_get_selected(night_end_hour)+lv_dropdown_get_selected(night_end_minute)};
    if(!display_schedule_valid(&schedule)){lv_label_set_text(display_status,"Start and end must differ");return;}
    display_ticket=alarm_service_display(&schedule,lv_dropdown_get_selected(manual_level)?25:160);
    display_pending=display_ticket!=0;display_since=esp_timer_get_time();lv_label_set_text(display_status,display_pending?"Saving...":"Busy - try again");
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
    if(lv_dropdown_get_selected(repeat)==REPEAT_ONCE){
        draft.weekdays=0;draft.once_date=(2000+lv_dropdown_get_selected(year))*10000+(1+lv_dropdown_get_selected(month))*100+1+lv_dropdown_get_selected(day);
    }else if(!draft.weekdays){lv_label_set_text(edit_status,"Choose at least one day");return;}
    if(!alarm_config_valid(&draft)){lv_label_set_text(edit_status,"Check the date");return;}
    pending_ticket=alarm_service_save_tracked(index_selected,&draft);
    pending=pending_ticket!=0;pending_since=esp_timer_get_time();lv_label_set_text(edit_status,pending?"Saving...":"Unable to queue save");
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
    repeat=dropdown("Every day\nWeekdays\nWeekends\nCustom\nOnce",325,65,140);lv_dropdown_set_selected(repeat,draft.weekdays?repeat_choice(draft.weekdays):REPEAT_ONCE);
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
    edit_status=label(root,s.load_failed?"Saved alarms unavailable. Saving replaces\nthe previous alarm configuration.":"24-hour time  /  select days or a date",15,211,450,&lv_font_montserrat_16);
    button(root,"Cancel",50,260,160,show_alarms,NULL);button(root,"Save",270,260,160,save,NULL);
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
    button(root,"Cancel",50,260,160,show_settings,NULL);button(root,"Save time",270,260,160,save_time,NULL);
}
static void snooze(lv_event_t *e)
{
    (void)e;alarm_snapshot_t s;alarm_service_snapshot(&s);
    if(s.ringing)alarm_service_snooze();
    else if(s.snoozed){snooze_collapsed=true;home();clock_ui_update();}
}
static void home_alarm(lv_event_t *e)
{
    alarm_snapshot_t s;alarm_service_snapshot(&s);
    if(s.snoozed){snooze_collapsed=false;clock_ui_update();}
    else show_alarms(e);
}
static void dismiss(lv_event_t *e){(void)e;alarm_service_dismiss();}
static void brightness_icon(lv_obj_t *parent)
{
    lv_obj_t *bulb=lv_obj_create(parent);lv_obj_remove_style_all(bulb);
    lv_obj_set_pos(bulb,13,6);lv_obj_set_size(bulb,22,25);
    lv_obj_set_style_border_width(bulb,2,0);lv_obj_set_style_radius(bulb,LV_RADIUS_CIRCLE,0);
    lv_obj_set_style_border_color(bulb,lv_color_hex(0xffdfaa),0);
    lv_obj_set_style_clip_corner(bulb,true,0);
    lv_obj_remove_flag(bulb,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *lit=lv_obj_create(bulb);lv_obj_remove_style_all(lit);
    lv_obj_set_pos(lit,0,0);lv_obj_set_size(lit,9,21);
    lv_obj_set_style_bg_color(lit,lv_color_hex(0xffdfaa),0);
    lv_obj_set_style_bg_opa(lit,LV_OPA_COVER,0);
    lv_obj_remove_flag(lit,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    for(int i=0;i<2;i++){
        lv_obj_t *base=lv_obj_create(parent);lv_obj_remove_style_all(base);
        lv_obj_set_pos(base,19+i,31+i*4);lv_obj_set_size(base,10-i*2,2);
        lv_obj_set_style_bg_color(base,lv_color_hex(0xffdfaa),0);
        lv_obj_set_style_bg_opa(base,LV_OPA_COVER,0);lv_obj_set_style_radius(base,1,0);
        lv_obj_remove_flag(base,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    }
}
/* A local month view; no fabricated events or dependency on a provider. */
static void calendar_refresh_heading(void)
{
    const lv_calendar_date_t *d=lv_calendar_get_showed_date(calendar);
    struct tm date={.tm_year=d->year-1900,.tm_mon=d->month-1,.tm_mday=1};
    char text[40];strftime(text,sizeof(text),"%B %Y",&date);lv_label_set_text(calendar_heading,text);
}
static void calendar_step(lv_event_t *e)
{
    const lv_calendar_date_t *d=lv_calendar_get_showed_date(calendar);
    int year=d->year,month=d->month+(int)(intptr_t)lv_event_get_user_data(e);
    if(month==0){month=12;year--;}else if(month==13){month=1;year++;}
    if(year<1970||year>2099)return;
    lv_calendar_set_month_shown(calendar,year,month);calendar_refresh_heading();
}
static void calendar_today(lv_event_t *e)
{
    (void)e;if(!clock_valid())return;
    time_t now=time(NULL);struct tm date;localtime_r(&now,&date);
    lv_calendar_set_today_date(calendar,date.tm_year+1900,date.tm_mon+1,date.tm_mday);
    lv_calendar_set_month_shown(calendar,date.tm_year+1900,date.tm_mon+1);calendar_refresh_heading();
}
static void show_calendar(lv_event_t *e)
{
    (void)e;editing=false;time_editing=false;reset_screen();calendar_view=true;
    calendar_heading=label(root,"Calendar",65,17,250,&lv_font_montserrat_20);
    button(root,LV_SYMBOL_LEFT,12,6,44,calendar_step,(void *)(intptr_t)-1);
    button(root,LV_SYMBOL_RIGHT,310,6,44,calendar_step,(void *)(intptr_t)1);
    button(root,"Today",366,6,102,calendar_today,NULL);
    calendar=lv_calendar_create(root);lv_obj_set_pos(calendar,12,56);lv_obj_set_size(calendar,456,177);
    lv_obj_set_style_bg_color(calendar,lv_color_hex(0x172b38),0);
    calendar_today(NULL);
    label(root,clock_valid()?"Local calendar / event accounts not connected":"Set time in Settings to show today's date",12,240,456,&lv_font_montserrat_16);
    navigation(0);
}
static lv_obj_t *weather_shape(int x,int y,int w,int h,uint32_t color,int radius)
{
    lv_obj_t *o=lv_obj_create(weather_art);lv_obj_remove_style_all(o);
    lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
    lv_obj_set_style_bg_color(o,lv_color_hex(color),0);lv_obj_set_style_bg_opa(o,LV_OPA_COVER,0);
    lv_obj_set_style_radius(o,radius,0);lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);return o;
}
static void update_weather_art(int code,bool fresh)
{
    if(!weather_art)return;
    lv_obj_set_style_opa(weather_art,fresh?LV_OPA_COVER:LV_OPA_40,0);
    if(code==artwork_code)return;
    artwork_code=code;lv_obj_clean(weather_art);
    if(code<0){label(weather_art,"--",0,12,60,&lv_font_montserrat_20);return;}
    bool snow=(code>=71&&code<=77)||code==85||code==86;
    bool wet=(code>=51&&code<=67)||(code>=80&&code<=82);
    bool storm=code>=95, fog=code==45||code==48;
    if(code<=2){
        weather_shape(29,3,26,26,0xffce73,LV_RADIUS_CIRCLE);
        if(code==0){weather_shape(40,0,3,2,0xffce73,1);weather_shape(58,14,4,3,0xffce73,1);weather_shape(18,14,5,3,0xffce73,1);weather_shape(40,33,3,4,0xffce73,1);return;}
    }
    weather_shape(9,18,41,17,0xc9dde4,8);weather_shape(16,9,23,24,0xc9dde4,LV_RADIUS_CIRCLE);
    weather_shape(32,17,24,19,0xc9dde4,LV_RADIUS_CIRCLE);
    if(fog){weather_shape(4,38,51,2,0x97b6c2,1);weather_shape(12,44,46,2,0x97b6c2,1);}
    else if(storm){label(weather_art,LV_SYMBOL_CHARGE,13,29,42,&lv_font_montserrat_20);}
    else if(snow||wet)for(int i=0;i<3;i++){
        weather_shape(15+i*14,39,3,snow?3:8,snow?0xe5f6ff:0x69c7ff,2);
        if(snow)weather_shape(13+i*14,40,7,1,0xe5f6ff,0);
    }
}
static void home(void)
{
    editing=false;time_editing=false;pending=false;reset_screen();scenic_home=true;
    lv_obj_t *wallpaper=lv_image_create(root);home_image=wallpaper;lv_image_set_src(wallpaper,background_service_image());
    lv_obj_set_pos(wallpaper,0,0);lv_obj_remove_flag(wallpaper,LV_OBJ_FLAG_CLICKABLE);
    /* Native image data is read from flash; scrim protects contrast without blur. */
    lv_obj_t *scrim=lv_obj_create(root);lv_obj_remove_style_all(scrim);
    lv_obj_set_size(scrim,480,320);lv_obj_set_style_bg_color(scrim,lv_color_hex(0x07101f),0);
    lv_obj_set_style_bg_opa(scrim,LV_OPA_20,0);
    lv_obj_remove_flag(scrim,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *date_button=button(root,"",15,8,335,show_calendar,NULL);
    lv_obj_set_user_data(date_button,"Calendar");lv_obj_set_height(date_button,48);
    lv_obj_set_style_pad_all(date_button,0,0);lv_obj_set_style_bg_opa(date_button,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(date_button,0,0);
    date_text=label(date_button,"Calendar",5,12,325,&lv_font_montserrat_20);
    lv_obj_set_style_text_color(date_text,lv_color_hex(0xe6e9f2),0);
    lv_obj_set_style_text_align(date_text,LV_TEXT_ALIGN_LEFT,0);
    lv_obj_set_height(date_text,22);lv_label_set_long_mode(date_text,LV_LABEL_LONG_DOT);
    wifi_indicator=button(root,"",358,10,48,home_network,NULL);
    lv_obj_set_user_data(wifi_indicator,"Wi-Fi shortcut");
    lv_obj_set_style_bg_opa(wifi_indicator,LV_OPA_50,0);lv_obj_set_style_border_opa(wifi_indicator,LV_OPA_20,0);
    lv_obj_set_style_pad_all(wifi_indicator,0,0);
    for(unsigned i=0;i<3;i++){
        int radius=8+i*6;
        wifi_strength[i]=lv_arc_create(wifi_indicator);
        lv_obj_remove_style_all(wifi_strength[i]);
        lv_obj_remove_flag(wifi_strength[i],LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_pos(wifi_strength[i],24-radius,32-radius);
        lv_obj_set_size(wifi_strength[i],radius*2,radius*2);
        lv_arc_set_bg_angles(wifi_strength[i],225,315);
        lv_obj_set_style_arc_width(wifi_strength[i],3,LV_PART_MAIN);
        lv_obj_set_style_arc_rounded(wifi_strength[i],true,LV_PART_MAIN);
        lv_obj_set_style_arc_opa(wifi_strength[i],LV_OPA_TRANSP,LV_PART_INDICATOR);
    }
    wifi_dot=lv_obj_create(wifi_indicator);
    lv_obj_remove_flag(wifi_dot,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(wifi_dot,21,30);lv_obj_set_size(wifi_dot,6,6);
    lv_obj_set_style_radius(wifi_dot,LV_RADIUS_CIRCLE,0);lv_obj_set_style_border_width(wifi_dot,0,0);
    lv_obj_t *b=button(root,"",417,10,48,dim,NULL);
    lv_obj_set_style_bg_opa(b,LV_OPA_50,0);lv_obj_set_style_border_opa(b,LV_OPA_20,0);
    lv_obj_set_user_data(b,"Brightness");lv_obj_set_style_pad_all(b,0,0);brightness_icon(b);
    time_text=label(root,"--:--",22,80,270,&lv_font_montserrat_48);
    lv_obj_set_style_text_color(time_text,lv_color_hex(0xfff6ec),0);
    lv_obj_set_style_transform_pivot_x(time_text,LV_PCT(50),0);lv_obj_set_style_transform_pivot_y(time_text,LV_PCT(50),0);lv_obj_set_style_transform_scale(time_text,432,0);
    detail=label(root,"Set time to begin",20,148,270,&lv_font_montserrat_16);
    lv_obj_set_style_text_color(detail,lv_color_hex(0xd5deeb),0);
    lv_obj_t *alarm_card=button(root,"",70,190,340,home_alarm,NULL);
    lv_obj_set_user_data(alarm_card,"Upcoming alarms");lv_obj_set_height(alarm_card,44);lv_obj_set_style_pad_all(alarm_card,0,0);
    lv_obj_set_style_bg_color(alarm_card,lv_color_hex(0x122136),0);
    lv_obj_set_style_bg_opa(alarm_card,LV_OPA_40,0);
    lv_obj_set_style_border_width(alarm_card,0,0);
    alarm_caption=label(alarm_card,LV_SYMBOL_BELL,8,12,28,&lv_font_montserrat_16);
    lv_obj_set_style_text_color(alarm_caption,lv_color_hex(0x9cd4bb),0);
    next_text=label(alarm_card,"No alarms enabled",43,12,288,&lv_font_montserrat_16);
    status=label(root,"",15,242,450,&lv_font_montserrat_16);
    lv_obj_set_style_text_color(status,lv_color_hex(0xcbd8e5),0);
    lv_obj_t *card=button(root,"",310,64,155,show_weather,NULL);
    lv_obj_set_height(card,112);lv_obj_set_style_pad_all(card,0,0);
    lv_obj_set_style_bg_color(card,lv_color_hex(0x102038),0);
    lv_obj_set_style_bg_opa(card,LV_OPA_60,0);lv_obj_set_style_border_color(card,lv_color_hex(0xbdcadb),0);
    lv_obj_set_style_border_opa(card,LV_OPA_30,0);lv_obj_remove_flag(card,LV_OBJ_FLAG_SCROLLABLE);
    home_place=label(card,"Local weather",4,6,147,&lv_font_montserrat_16);
    lv_obj_set_height(home_place,20);lv_label_set_long_mode(home_place,LV_LABEL_LONG_DOT);
    weather_art=lv_obj_create(card);lv_obj_remove_style_all(weather_art);
    lv_obj_set_pos(weather_art,3,30);lv_obj_set_size(weather_art,64,50);
    lv_obj_remove_flag(weather_art,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    home_temperature=label(card,"-- F",66,34,86,&lv_font_montserrat_48);
    lv_obj_set_style_text_color(home_temperature,lv_color_hex(0xb4d8e9),0);
    home_forecast=label(card,"Set up Wi-Fi",4,81,147,&lv_font_montserrat_16);
    lv_obj_set_height(home_forecast,22);lv_label_set_long_mode(home_forecast,LV_LABEL_LONG_DOT);
    navigation(0);
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
static void background_focus(lv_event_t *e)
{
    lv_keyboard_set_textarea(keyboard,lv_event_get_target(e));lv_obj_remove_flag(keyboard,LV_OBJ_FLAG_HIDDEN);lv_obj_move_foreground(keyboard);
}
static void background_source_changed(lv_event_t *e)
{
    (void)e;unsigned source=lv_dropdown_get_selected(background_source);
    lv_textarea_set_placeholder_text(background_input,source==BACKGROUND_WALLHAVEN?"Search: mountains, forest, space...":"One HTTPS image or Wallhaven link per line");
    if(source==BACKGROUND_LOCAL)lv_obj_add_flag(background_input,LV_OBJ_FLAG_HIDDEN);
    else lv_obj_remove_flag(background_input,LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);lv_keyboard_set_textarea(keyboard,NULL);
}
static void background_save(lv_event_t *e)
{
    (void)e;background_error=true;memset(&background_draft,0,sizeof(background_draft));
    background_draft.source=lv_dropdown_get_selected(background_source);
    const unsigned seconds[]={0,300,900,3600};background_draft.interval_seconds=seconds[lv_dropdown_get_selected(background_interval)];
    const char *text=lv_textarea_get_text(background_input);
    if(background_draft.source==BACKGROUND_LOCAL){background_draft.count=1;strcpy(background_draft.images[0],"blue-hour");}
    else if(background_draft.source==BACKGROUND_WALLHAVEN){
        if(strlen(text)>=sizeof(background_draft.query)){lv_label_set_text(background_status,"Search is too long");return;}strcpy(background_draft.query,text);
    }else{
        while(*text){
            const char *end=strchr(text,'\n');size_t n=end?(size_t)(end-text):strlen(text);
            if(n){if(n>=BACKGROUND_URL_SIZE||background_draft.count==BACKGROUND_MAX_IMAGES){lv_label_set_text(background_status,"Up to 8 links; each under 384 characters");return;}
                memcpy(background_draft.images[background_draft.count++],text,n);}
            if(!end)break;
            text=end+1;
        }
    }
    bool ok=background_service_configure(&background_draft);
    lv_label_set_text(background_status,ok?"Saving...":"Invalid source or busy; check entries");
    if(ok){background_error=false;lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);lv_keyboard_set_textarea(keyboard,NULL);}
}
static void background_next(lv_event_t *e){(void)e;background_service_next();}
static void show_backgrounds(lv_event_t *e)
{
    (void)e;editing=false;time_editing=false;reset_screen();background_editing=true;background_error=false;
    char message[96];bool busy;background_service_snapshot(&background_draft,message,&busy);
    label(root,"Backgrounds",12,10,456,&lv_font_montserrat_20);
    background_source=dropdown("Local: Blue hour\nSelected image links\nWallhaven search",12,45,270);
    lv_dropdown_set_selected(background_source,background_draft.source);
    background_interval=dropdown("Keep fixed\nEvery 5 min\nEvery 15 min\nEvery hour",294,45,174);
    unsigned interval=background_draft.interval_seconds;lv_dropdown_set_selected(background_interval,interval==300?1:interval==900?2:interval?3:0);
    background_input=lv_textarea_create(root);lv_obj_set_pos(background_input,12,96);lv_obj_set_size(background_input,456,96);
    lv_textarea_set_max_length(background_input,BACKGROUND_MAX_IMAGES*BACKGROUND_URL_SIZE);
    if(background_draft.source==BACKGROUND_WALLHAVEN)lv_textarea_set_text(background_input,background_draft.query);
    else{lv_textarea_set_text(background_input,"");if(background_draft.source==BACKGROUND_SELECTED)for(unsigned i=0;i<background_draft.count;i++){
        if(i)lv_textarea_add_text(background_input,"\n");
        lv_textarea_add_text(background_input,background_draft.images[i]);}}
    lv_obj_add_event_cb(background_input,background_focus,LV_EVENT_CLICKED,NULL);
    background_status=label(root,message,12,202,456,&lv_font_montserrat_16);
    label(root,"JPEG links / Wallhaven public SFW",12,235,456,&lv_font_montserrat_16);
    button(root,"Back",12,267,144,show_settings,NULL);button(root,"Next image",168,267,144,background_next,NULL);button(root,"Save",324,267,144,background_save,NULL);
    keyboard=lv_keyboard_create(root);lv_obj_set_size(keyboard,480,150);lv_obj_align(keyboard,LV_ALIGN_BOTTOM_MID,0,0);
    lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);lv_obj_add_event_cb(keyboard,keyboard_event,LV_EVENT_ALL,NULL);
    lv_obj_add_event_cb(background_source,background_source_changed,LV_EVENT_VALUE_CHANGED,NULL);background_source_changed(NULL);
}
static lv_obj_t *network_field(const char *caption,const char *text,int y,unsigned max,bool secret)
{
    label(root,caption,8,y+12,100,&lv_font_montserrat_16);
    lv_obj_t *o=lv_textarea_create(root);lv_obj_set_pos(o,112,y);lv_obj_set_size(o,355,42);
    lv_textarea_set_one_line(o,true);lv_textarea_set_max_length(o,max);lv_textarea_set_password_mode(o,secret);
    lv_textarea_set_text(o,text);lv_obj_add_event_cb(o,field_focus,LV_EVENT_CLICKED,NULL);return o;
}
static void ha_save(lv_event_t *e)
{
    (void)e;bool ok=ha_service_configure(lv_textarea_get_text(ha_url),lv_textarea_get_text(ha_token),lv_textarea_get_text(ha_entity));
    ha_error=!ok;
    if(ok){lv_textarea_set_text(ha_token,"");show_ha(NULL);}
    else lv_label_set_text(ha_status,"Check URL / light entity, or try again when idle");
}
static void ha_setup(lv_event_t *e)
{
    (void)e;ha_snapshot_t s;ha_service_snapshot(&s);reset_screen();ha_editing=true;
    label(root,"Home Assistant setup",10,5,460,&lv_font_montserrat_20);
    ha_url=network_field("Server",s.endpoint,34,191,false);
    ha_entity=network_field("Light",s.entity,80,95,false);
    ha_token=network_field("Token","",126,511,true);
    lv_textarea_set_password_show_time(ha_token,0);
    lv_textarea_set_placeholder_text(ha_entity,"Optional: light.bedside");
    lv_textarea_set_placeholder_text(ha_token,s.configured?"Blank keeps saved token":"Long-lived access token");
    ha_status=label(root,"Use the server address, without a dashboard path",10,181,460,&lv_font_montserrat_16);
    button(root,"Cancel",40,260,180,show_ha,NULL);button(root,"Save",260,260,180,ha_save,NULL);
    keyboard=lv_keyboard_create(root);lv_obj_set_size(keyboard,480,130);lv_obj_align(keyboard,LV_ALIGN_BOTTOM_MID,0,0);
    lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);lv_obj_add_event_cb(keyboard,keyboard_event,LV_EVENT_ALL,NULL);
}
static void ha_switch(lv_event_t *e)
{
    (void)e;ha_error=!ha_service_toggle();
    if(ha_error)lv_label_set_text(ha_status,"Wait for a fresh light state, then try again");
}
static void ha_refresh(lv_event_t *e)
{
    (void)e;ha_error=!ha_service_refresh();
    if(ha_error)lv_label_set_text(ha_status,"Request in progress");
}
static void show_ha(lv_event_t *e)
{
    (void)e;editing=false;time_editing=false;reset_screen();ha_view=true;
    label(root,"Home Assistant",10,12,320,&lv_font_montserrat_20);
    button(root,"Setup",350,8,115,ha_setup,NULL);
    ha_name=label(root,"Bedside light",15,75,450,&lv_font_montserrat_20);
    ha_state=label(root,"Not connected",15,110,450,&lv_font_montserrat_20);
    ha_status=label(root,"",15,165,450,&lv_font_montserrat_16);
    button(root,"Back",15,260,140,show_settings,NULL);
    ha_toggle=button(root,"Turn on",170,260,140,ha_switch,NULL);
    lv_obj_add_state(ha_toggle,LV_STATE_DISABLED);
    button(root,"Refresh",325,260,140,ha_refresh,NULL);
}
static void media_save(lv_event_t *e)
{
    (void)e;bool ok=media_service_select_alarm(lv_textarea_get_text(media_entity),lv_textarea_get_text(media_content),lv_textarea_get_text(media_type),lv_obj_has_state(media_remote,LV_STATE_CHECKED));
    if(ok)show_media(NULL);else lv_label_set_text(media_status,"Check HA setup, player and media ID/type");
}
static void media_setup(lv_event_t *e)
{
    (void)e;media_snapshot_t s;media_service_snapshot(&s);reset_screen();media_editing=true;
    label(root,"External player",10,12,310,&lv_font_montserrat_20);
    media_entity=network_field("Player",s.entity,58,95,false);
    media_content=network_field("Media ID",s.content,104,383,false);
    media_type=network_field("Type",s.content_type,150,47,false);
    lv_textarea_set_placeholder_text(media_type,"music or playlist (optional)");
    lv_textarea_set_placeholder_text(media_entity,"media_player.bedroom");
    media_status=label(root,"Leave media ID and type blank for controls only",10,196,460,&lv_font_montserrat_16);
    button(root,"HA setup",350,8,115,ha_setup,NULL);
    media_remote=lv_checkbox_create(root);lv_checkbox_set_text(media_remote,"Use for alarms (local fallback)");
    lv_obj_set_pos(media_remote,30,230);if(s.remote_alarm)lv_obj_add_state(media_remote,LV_STATE_CHECKED);
    button(root,"Cancel",40,265,180,show_media,NULL);button(root,"Save",260,265,180,media_save,NULL);
    keyboard=lv_keyboard_create(root);lv_obj_set_size(keyboard,480,130);lv_obj_align(keyboard,LV_ALIGN_BOTTOM_MID,0,0);
    lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);lv_obj_add_event_cb(keyboard,keyboard_event,LV_EVENT_ALL,NULL);
}
static void media_action(lv_event_t *e)
{
    media_error=!media_service_action((media_action_t)(uintptr_t)lv_event_get_user_data(e));
    if(media_error)lv_label_set_text(media_status,"Action unavailable; refresh player state");
}
static void media_refresh(lv_event_t *e)
{(void)e;media_error=!media_service_refresh();if(media_error)lv_label_set_text(media_status,"Request unavailable or already in progress");}
static void show_media(lv_event_t *e)
{
    (void)e;editing=false;time_editing=false;reset_screen();media_view=true;
    label(root,"Media",10,12,160,&lv_font_montserrat_20);button(root,"Setup",350,8,115,media_setup,NULL);
    media_controls[6]=button(root,"Start saved",180,8,155,media_action,(void*)MEDIA_START_SAVED);
    media_name=label(root,"Choose a player",15,60,450,&lv_font_montserrat_20);
    media_title=label(root,"",15,92,450,&lv_font_montserrat_16);media_artist=label(root,"",15,116,450,&lv_font_montserrat_16);
    lv_obj_t *one_line[]={media_name,media_title,media_artist};
    for(unsigned i=0;i<3;i++){lv_obj_set_height(one_line[i],24);lv_label_set_long_mode(one_line[i],LV_LABEL_LONG_DOT);}
    media_info=label(root,"",15,144,450,&lv_font_montserrat_16);
    media_controls[0]=button(root,"Previous",15,180,100,media_action,(void*)MEDIA_PREVIOUS);
    media_controls[1]=button(root,"Play",125,180,105,media_action,(void*)MEDIA_PLAY);
    media_controls[2]=button(root,"Pause",240,180,105,media_action,(void*)MEDIA_PAUSE);
    media_controls[3]=button(root,"Next",355,180,110,media_action,(void*)MEDIA_NEXT);
    media_status=label(root,"",15,232,450,&lv_font_montserrat_16);
    button(root,"Clock",15,270,100,go_home,NULL);
    media_controls[4]=button(root,"Quieter",125,270,105,media_action,(void*)MEDIA_QUIETER);
    media_controls[5]=button(root,"Louder",240,270,105,media_action,(void*)MEDIA_LOUDER);
    button(root,"Refresh",355,270,110,media_refresh,NULL);
    for(unsigned i=0;i<7;i++)lv_obj_add_state(media_controls[i],LV_STATE_DISABLED);
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
static void toggle_radio(lv_event_t *e)
{
    (void)e;weather_snapshot_t s;weather_service_snapshot(&s);network_error=false;
    if(!weather_service_radio(s.radio_paused)){lv_label_set_text(network_status,"Busy; try again");network_error=true;}
}
static void setup_back(lv_event_t *e)
{if(setup_origin==2)home();else if(setup_origin==1)show_settings(e);else show_weather(e);}
static void show_network(lv_event_t *e)
{
    if(settings_view||weather_view)setup_origin=settings_view?1:0;
    (void)e;editing=false;time_editing=false;reset_screen();network_editing=true;wifi_listing=true;network_error=false;shown_scan=~0u;
    label(root,"Wi-Fi",10,10,180,&lv_font_montserrat_20);button(root,"Scan",345,6,120,rescan,NULL);
    weather_snapshot_t w;weather_service_snapshot(&w);
    radio_button=button(root,w.radio_paused?"Turn on":"Turn off",210,6,125,toggle_radio,NULL);
    network_link=label(root,"Not connected",12,55,456,&lv_font_montserrat_16);
    network_list=lv_list_create(root);lv_obj_set_pos(network_list,12,82);lv_obj_set_size(network_list,456,126);
    network_status=label(root,"Scanning nearby networks...",10,217,460,&lv_font_montserrat_16);
    button(root,"Back",20,264,130,setup_back,NULL);button(root,"Test sound",165,264,145,sound,NULL);
    button(root,"Clock",325,264,135,go_home,NULL);if(!w.radio_paused)weather_service_scan();
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
    if(settings_view||weather_view)setup_origin=settings_view?1:0;
    (void)e;weather_snapshot_t s;weather_service_snapshot(&s);editing=false;time_editing=false;reset_screen();location_editing=true;network_error=false;
    label(root,"Weather & time zone",10,10,290,&lv_font_montserrat_20);
    button(root,"Use network",315,5,150,estimate_location,NULL);
    label(root,s.manual_location?"Manual location - preserved across Wi-Fi changes":"Estimated from network - check or change ZIP",10,52,460,&lv_font_montserrat_16);
    network_zip=network_field("US ZIP",s.zip,95,5,false);lv_textarea_set_accepted_chars(network_zip,"0123456789");
    network_status=label(root,s.location.name,10,158,460,&lv_font_montserrat_16);
    button(root,"Back",45,264,170,setup_back,NULL);button(root,"Save location",265,264,170,save_location,NULL);
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
    weather_now=label(root,"Connect Wi-Fi to begin",15,59,450,&lv_font_montserrat_20);
    weather_today=label(root,"Today's forecast will appear here",15,112,450,&lv_font_montserrat_16);
    weather_status=label(root,"",15,158,450,&lv_font_montserrat_16);
    label(root,"Open-Meteo / CC BY 4.0",15,198,290,&lv_font_montserrat_16);
    button(root,"Wi-Fi",15,218,140,show_network,NULL);button(root,"Location",170,218,140,show_location,NULL);navigation(2);
    zone_button=button(root,"Apply zone",325,218,140,restart_zone,NULL);lv_obj_add_flag(zone_button,LV_OBJ_FLAG_HIDDEN);
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
    if(s.ringing || !active)snooze_collapsed=false;
    if(active && !overlay){
        overlay=lv_obj_create(lv_layer_top());lv_obj_set_size(overlay,460,230);lv_obj_center(overlay);
        lv_obj_set_style_bg_color(overlay,lv_color_hex(0x142c40),0);lv_obj_remove_flag(overlay,LV_OBJ_FLAG_SCROLLABLE);
        label(overlay,"Alarm",0,12,420,&lv_font_montserrat_48);
        overlay_detail=label(overlay,"",0,86,420,&lv_font_montserrat_20);
        button(overlay,"Snooze 5 min",5,140,195,snooze,NULL);button(overlay,"Dismiss",215,140,195,dismiss,NULL);
    }
    if(!active && overlay){lv_obj_delete(overlay);overlay=NULL;}
    if(overlay){
        if(snooze_collapsed)lv_obj_add_flag(overlay,LV_OBJ_FLAG_HIDDEN);
        else lv_obj_remove_flag(overlay,LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(lv_obj_get_child(lv_obj_get_child(overlay,2),0),s.ringing?"Snooze 5 min":"Show clock");
        lv_obj_t *title=lv_obj_get_child(overlay,0);lv_label_set_text(title,s.ringing?(audio_status()==ESP_OK?"Alarm":"Audio error"):"Snoozed");
        char text[80];
        if(s.ringing){
            size_t n=0;n+=snprintf(text,sizeof(text),"Alarm ");
            for(unsigned i=0;i<ALARM_COUNT;i++)if(s.ringing&(1u<<i))n+=snprintf(text+n,sizeof(text)-n,"%s%u",n>6?", ":"",i+1);
            if(!s.local_sound)snprintf(text+n,sizeof(text)-n,"\nExternal speaker; fallback armed");
        }else{
            uint32_t remaining=s.snooze_seconds;
            snprintf(text,sizeof(text),"Rings again in %02u:%02u",(unsigned)(remaining/60),(unsigned)(remaining%60));
        }
        lv_label_set_text(overlay_detail,text);
    }
    if(display_editing){
        alarm_save_result_t result;
        if(display_pending&&alarm_service_result(display_ticket,&result)){
            display_pending=false;
            if(result.status==ESP_OK){home();return;}
            lv_label_set_text(display_status,"Save failed - settings unchanged");
        }else if(display_pending&&esp_timer_get_time()-display_since>=10000000){
            display_pending=false;lv_label_set_text(display_status,"Result unavailable; reopen to check settings");
        }
        return;
    }
    if(alarm_list_view){update_alarm_rows(&s);return;}
    if(media_editing)return;
    if(media_view){
        media_snapshot_t m;media_service_snapshot(&m);
        lv_label_set_text(media_name,m.player.name[0]?m.player.name:m.entity[0]?m.entity:"Choose a player in Setup");
        lv_label_set_text(media_title,m.player.title);lv_label_set_text(media_artist,m.player.artist);
        char info[96];
        if(m.fresh&&m.player.volume_known)snprintf(info,sizeof(info),"%s  /  Volume %.0f%%",media_state_name(m.player.state),m.player.volume*100);
        else snprintf(info,sizeof(info),"%s",m.fresh?media_state_name(m.player.state):"State unavailable");
        lv_label_set_text(media_info,info);if(!media_error)lv_label_set_text(media_status,m.status);
        for(unsigned i=0;i<7;i++){
            if(m.configured&&m.fresh&&!m.busy&&media_action_supported(&m.player,i)&&(i!=MEDIA_START_SAVED||m.content[0]))lv_obj_remove_state(media_controls[i],LV_STATE_DISABLED);
            else lv_obj_add_state(media_controls[i],LV_STATE_DISABLED);
        }
        return;
    }
    if(ha_editing)return;
    if(ha_view){
        ha_snapshot_t h;ha_service_snapshot(&h);
        lv_label_set_text(ha_name,h.light.name[0]?h.light.name:h.entity[0]?h.entity:"Choose a light in Setup");
        lv_label_set_text(ha_state,!h.configured?"Not configured":!h.fresh?"State unavailable":h.light.state==HA_ON?"On":h.light.state==HA_OFF?"Off":"Unavailable");
        if(!ha_error)lv_label_set_text(ha_status,h.status);
        lv_label_set_text(lv_obj_get_child(ha_toggle,0),h.light.state==HA_ON?"Turn off":"Turn on");
        if(h.configured&&h.fresh&&!h.busy&&(h.light.state==HA_ON||h.light.state==HA_OFF))lv_obj_remove_state(ha_toggle,LV_STATE_DISABLED);
        else lv_obj_add_state(ha_toggle,LV_STATE_DISABLED);
        return;
    }
    if(settings_view)return;
    if(weather_view||network_editing||location_editing){
        weather_snapshot_t w;weather_service_snapshot(&w);
        if(network_editing||location_editing){
            if(network_connecting&&!w.busy&&w.connected&&!strcmp(w.ssid,chosen_network.ssid)){setup_back(NULL);return;}
            if(location_editing&&!w.manual_location&&!lv_textarea_get_text(network_zip)[0]&&w.zip[0])lv_textarea_set_text(network_zip,w.zip);
            if(wifi_listing)lv_label_set_text(lv_obj_get_child(radio_button,0),w.radio_paused?"Turn on":"Turn off");
            if(wifi_listing){
                char link[96];
                if(w.connected&&w.signal_known)snprintf(link,sizeof(link),"Connected: %s (%d dBm) / channel %u",weather_signal_name(w.signal_dbm),w.signal_dbm,w.signal_channel);
                else snprintf(link,sizeof(link),"%s",w.connected?"Connected: measuring signal...":"Not connected");
                lv_label_set_text(network_link,link);
            }
            if(wifi_listing&&shown_scan!=w.scan_revision){
                shown_scan=w.scan_revision;lv_obj_clean(network_list);memcpy(shown_networks,w.networks,sizeof(shown_networks));
                for(unsigned i=0;i<w.network_count;i++){
                    char title[100],name[33];strcpy(name,shown_networks[i].ssid);
                    for(char *p=name;*p;p++)if((unsigned char)*p<32)*p=' ';
                    snprintf(title,sizeof(title),"%s\nBest nearby: %s (%d dBm) / %s",name,weather_signal_name(shown_networks[i].rssi),shown_networks[i].rssi,shown_networks[i].secured?"Secured":"Open");
                    lv_obj_t *b=lv_list_add_button(network_list,LV_SYMBOL_WIFI,title);
                    lv_label_set_long_mode(lv_obj_get_child(b,1),LV_LABEL_LONG_WRAP);lv_obj_add_event_cb(b,choose_network,LV_EVENT_CLICKED,(void*)(uintptr_t)i);
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
    if(background_editing){if(background_error)return;char text[96];bool busy; background_service_snapshot(NULL,text,&busy);lv_label_set_text(background_status,text);return;}
    if(calendar_view)return;
    if(time_editing)return;
    if(editing){
        alarm_save_result_t result;
        if(pending && alarm_service_result(pending_ticket,&result)){
            pending=false;
            if(result.status==ESP_OK)show_alarms(NULL);
            else lv_label_set_text(edit_status,result.conflict?"Alarm changed; reopen to review":"Save failed - settings unchanged");
        }else if(pending&&esp_timer_get_time()-pending_since>=10000000){
            pending=false;lv_label_set_text(edit_status,"Result unavailable; reopen to check alarm");
        }
        return;
    }
    const lv_image_dsc_t *image=background_service_image();
    if(lv_image_get_src(home_image)!=image)lv_image_set_src(home_image,image);
    char b[80];
    if(clock_valid()){
        time_t now=time(NULL);struct tm local;localtime_r(&now,&local);
        strftime(b,sizeof(b),"%I:%M",&local);lv_label_set_text(time_text,b[0]=='0'?b+1:b);
        strftime(b,sizeof(b),"%a, %B %d  " LV_SYMBOL_RIGHT,&local);lv_label_set_text(date_text,b);
        strftime(b,sizeof(b),"%p   :%S   %Z",&local);lv_label_set_text(detail,b);
    }
    time_t next=clock_valid()?alarm_next(s.settings.alarms,time(NULL)):0;
    if(next){struct tm local;localtime_r(&next,&local);strftime(b,sizeof(b),"Next: %a %I:%M %p",&local);}
    else snprintf(b,sizeof(b),"%s",clock_valid()?"No upcoming alarms":"Set time to arm alarms");
    if(s.snoozed){
        lv_label_set_text(alarm_caption,LV_SYMBOL_PAUSE);
        snprintf(b,sizeof(b),"Rings again in %02u:%02u",(unsigned)(s.snooze_seconds/60),(unsigned)(s.snooze_seconds%60));
    }else lv_label_set_text(alarm_caption,LV_SYMBOL_BELL);
    lv_label_set_text(next_text,b);


    weather_snapshot_t weather;weather_service_snapshot(&weather);
    unsigned bars=weather.connected&&weather.signal_known?weather_signal_bars(weather.signal_dbm):0;
    lv_obj_set_style_bg_color(wifi_dot,lv_color_hex(weather.connected?0x9cd4bb:0x71818d),0);
    for(unsigned i=0;i<3;i++)lv_obj_set_style_arc_color(wifi_strength[i],lv_color_hex(i<bars?0x9cd4bb:0x33434e),LV_PART_MAIN);
    if(weather.radio_paused)snprintf(b,sizeof(b),LV_SYMBOL_WIFI "  Wi-Fi off");
    else if(!weather.connected)snprintf(b,sizeof(b),LV_SYMBOL_WIFI "  Wi-Fi disconnected");
    else if(weather.internet_verified&&weather.internet_age_seconds<1860){
        if(weather.internet_age_seconds<60)snprintf(b,sizeof(b),LV_SYMBOL_OK "  Internet checked just now");
        else snprintf(b,sizeof(b),LV_SYMBOL_OK "  Internet checked %u min ago",(unsigned)(weather.internet_age_seconds/60));
    }else snprintf(b,sizeof(b),LV_SYMBOL_WARNING "  Internet not verified");
    lv_label_set_text(status,audio_status()!=ESP_OK?"Local audio unavailable":s.load_failed?"Saved alarms unavailable - review Alarms":s.storage_status!=ESP_OK?"Settings storage error":!clock_valid()?"Set time to enable alarms":b);
    lv_label_set_text(home_place,weather.location.name[0]?weather.location.name:"Local weather");
    update_weather_art(weather.has_data?weather.data.code:-1,weather.has_data&&weather_fresh(&weather.data,time(NULL)));
    if(weather.has_data){
        snprintf(b,sizeof(b),"%.0f°",weather.data.temperature);lv_label_set_text(home_temperature,b);
        snprintf(b,sizeof(b),"%s",weather_fresh(&weather.data,time(NULL))?weather_condition(weather.data.code):"Outdated");
        lv_label_set_text(home_forecast,b);
    }else{lv_label_set_text(home_temperature,"--°");lv_label_set_text(home_forecast,weather.connected?"Updating...":"Set up Wi-Fi");}
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
    diagnostics_printf("UI_SCREEN name=%s overlay=%u pending=%u\n",background_editing?"backgrounds":calendar_view?"calendar":media_editing?"media_setup":media_view?"media":alarm_list_view?"alarms":ha_editing?"ha_setup":ha_view?"ha":display_editing?"display":settings_view?"settings":location_editing?"location":network_editing?"network":weather_view?"weather":time_editing?"time":editing?"alarm":"home",overlay!=NULL&&!lv_obj_has_flag(overlay,LV_OBJ_FLAG_HIDDEN),pending);
    if(weather_view||network_editing||location_editing){weather_snapshot_t w;weather_service_snapshot(&w);
        diagnostics_printf("WEATHER_STATE online=%u valid=%u fresh=%u zip=%s zone=%s status=%s\n",w.connected,w.has_data,w.has_data&&weather_fresh(&w.data,time(NULL)),w.zip,w.location.timezone,w.status);
    }
    weather_snapshot_t link;weather_service_snapshot(&link);
    diagnostics_printf("UI_CONNECTIVITY wifi=%u signal_known=%u dbm=%d internet_verified=%u age=%lu channel=%u\n",
        link.connected,link.signal_known,link.signal_dbm,link.internet_verified,(unsigned long)link.internet_age_seconds,link.signal_channel);
    background_service_diagnostics();
    alarm_snapshot_t state;alarm_service_snapshot(&state);
    diagnostics_printf("AUDIO_STATE status=%d\n",(int)audio_status());
    diagnostics_printf("DISPLAY_STATE auto=%u start=%u end=%u level=%u\n",state.settings.display.enabled,state.settings.display.start_minute,state.settings.display.end_minute,applied_brightness);
    if(overlay)diagnostics_printf("UI_OVERLAY text=%s\n",lv_label_get_text(overlay_detail));
    if(editing){
        diagnostics_printf("UI_ALARM slot=%u enabled=%u weekdays=%u message=%s\n",index_selected,lv_obj_has_state(enabled,LV_STATE_CHECKED),draft.weekdays,lv_label_get_text(edit_status));
        dropdown_diagnostics("slot",slot);dropdown_diagnostics("hour",hours);dropdown_diagnostics("minute",minutes);dropdown_diagnostics("repeat",repeat);
        if(lv_dropdown_get_selected(repeat)==REPEAT_ONCE){dropdown_diagnostics("year",year);dropdown_diagnostics("month",month);dropdown_diagnostics("day",day);}
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
