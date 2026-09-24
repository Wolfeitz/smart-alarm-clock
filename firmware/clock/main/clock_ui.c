#include "clock_ui.h"
#include "diagnostics.h"
#include "alarm_service.h"
#include "clock_service.h"
#include "local_time.h"
#include "board.h"
#include "audio.h"
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static lv_obj_t *root,*time_text,*date_text,*detail,*next_text,*status,*dim_text;
static lv_obj_t *slot,*hours,*minutes,*repeat,*enabled,*days[7],*year,*month,*day,*edit_status,*overlay,*overlay_detail;
static alarm_config_t draft;
static unsigned index_selected;
static bool editing,pending,time_editing;
static lv_obj_t *time_year,*time_month,*time_day,*time_hour,*time_minute,*time_status;
static uint32_t pending_ticket;
static uint8_t applied_brightness;
static lv_obj_t *label(lv_obj_t *parent,const char *text,int x,int y,int w,const lv_font_t *font)
{
    lv_obj_t *o=lv_label_create(parent);lv_label_set_text(o,text);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);
    lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_style_text_color(o,lv_color_hex(0xf6eddc),0);return o;
}
static lv_obj_t *button(lv_obj_t *parent,const char *text,int x,int y,int w,lv_event_cb_t cb,void *data)
{
    lv_obj_t *o=lv_button_create(parent);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,44);
    lv_obj_set_style_bg_color(o,lv_color_hex(0x203347),0);lv_obj_add_event_cb(o,cb,LV_EVENT_CLICKED,data);
    lv_obj_t *l=lv_label_create(o);lv_label_set_text(l,text);lv_obj_center(l);return o;
}
static void home(void);
static void show_editor(void);
static void show_time_editor(lv_event_t *e);
static void reset_screen(void)
{
    lv_obj_clean(root);lv_obj_set_style_bg_color(root,lv_color_hex(0x0b1119),0);
    lv_obj_remove_flag(root,LV_OBJ_FLAG_SCROLLABLE);
}
static void go_home(lv_event_t *e){(void)e;home();}
static void go_editor(lv_event_t *e){(void)e;index_selected=0;show_editor();}
static void sound(lv_event_t *e){(void)e;audio_test();}
static void dim(lv_event_t *e)
{
    (void)e;alarm_snapshot_t s;alarm_service_snapshot(&s);
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
    time_status=label(root,"America/New_York  /  seconds reset to 00",10,222,460,&lv_font_montserrat_16);
    button(root,"Cancel",50,260,160,go_home,NULL);button(root,"Save time",270,260,160,save_time,NULL);
}
static void snooze(lv_event_t *e){(void)e;alarm_service_snooze();}
static void dismiss(lv_event_t *e){(void)e;alarm_service_dismiss();}
static void home(void)
{
    editing=false;time_editing=false;pending=false;reset_screen();
    date_text=label(root,"Clock",15,20,315,&lv_font_montserrat_20);
    button(root,"Set time",350,12,115,show_time_editor,NULL);
    time_text=label(root,"--:--",90,80,300,&lv_font_montserrat_48);
    lv_obj_set_style_transform_pivot_x(time_text,LV_PCT(50),0);lv_obj_set_style_transform_pivot_y(time_text,LV_PCT(50),0);lv_obj_set_style_transform_scale(time_text,384,0);
    detail=label(root,"Set time to begin",20,159,440,&lv_font_montserrat_20);
    next_text=label(root,"No alarms enabled",20,199,440,&lv_font_montserrat_16);
    status=label(root,"",20,227,440,&lv_font_montserrat_16);
    lv_obj_t *b=button(root,"Dim",15,261,140,dim,NULL);dim_text=lv_obj_get_child(b,0);
    button(root,"Alarms",170,261,140,go_editor,NULL);button(root,"Test sound",325,261,140,sound,NULL);
}
void clock_ui_init(void){root=lv_screen_active();home();}
void clock_ui_update(void)
{
    alarm_snapshot_t s;alarm_service_snapshot(&s);
    if(s.settings.brightness!=applied_brightness){applied_brightness=s.settings.brightness;board_brightness(applied_brightness<80);}
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
    lv_label_set_text(dim_text,applied_brightness<80?"Brighten":"Dim");
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
    diagnostics_printf("UI_SCREEN name=%s overlay=%u pending=%u\n",time_editing?"time":editing?"alarm":"home",overlay!=NULL,pending);
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
