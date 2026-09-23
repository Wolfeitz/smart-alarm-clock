#include "clock_ui.h"
#include "alarm_service.h"
#include "clock_service.h"
#include "board.h"
#include "audio.h"
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static lv_obj_t *root,*time_text,*date_text,*detail,*next_text,*status,*dim_text;
static lv_obj_t *slot,*hours,*minutes,*repeat,*enabled,*days[7],*year,*month,*day,*edit_status,*overlay;
static alarm_config_t draft;
static unsigned index_selected;
static bool editing,pending;
static unsigned pending_revision;
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
    (void)e;draft.hour=lv_dropdown_get_selected(hours);draft.minute=lv_dropdown_get_selected(minutes);
    draft.enabled=lv_obj_has_state(enabled,LV_STATE_CHECKED);
    if(lv_dropdown_get_selected(repeat)==1){
        draft.weekdays=0;draft.once_date=(2000+lv_dropdown_get_selected(year))*10000+(1+lv_dropdown_get_selected(month))*100+1+lv_dropdown_get_selected(day);
    }else if(!draft.weekdays){lv_label_set_text(edit_status,"Choose at least one day");return;}
    if(!alarm_config_valid(&draft)){lv_label_set_text(edit_status,"Check the date");return;}
    alarm_snapshot_t s;alarm_service_snapshot(&s);pending_revision=s.revision;
    pending=alarm_service_save(index_selected,&draft);lv_label_set_text(edit_status,pending?"Saving...":"Unable to queue save");
}
static void show_editor(void)
{
    alarm_snapshot_t s;alarm_service_snapshot(&s);draft=s.settings.alarms[index_selected];editing=true;pending=false;reset_screen();
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
static void snooze(lv_event_t *e){(void)e;alarm_service_snooze();}
static void dismiss(lv_event_t *e){(void)e;alarm_service_dismiss();}
static void home(void)
{
    editing=false;pending=false;reset_screen();
    date_text=label(root,"Clock",20,23,440,&lv_font_montserrat_20);
    time_text=label(root,"--:--",90,80,300,&lv_font_montserrat_48);
    lv_obj_set_style_transform_pivot_x(time_text,LV_PCT(50),0);lv_obj_set_style_transform_pivot_y(time_text,LV_PCT(50),0);lv_obj_set_style_transform_scale(time_text,384,0);
    detail=label(root,"Set time via USB",20,159,440,&lv_font_montserrat_20);
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
        button(overlay,"Snooze 5 min",5,140,195,snooze,NULL);button(overlay,"Dismiss",215,140,195,dismiss,NULL);
    }
    if(!active && overlay){lv_obj_delete(overlay);overlay=NULL;}
    if(overlay){lv_obj_t *title=lv_obj_get_child(overlay,0);lv_label_set_text(title,s.ringing?"Alarm":"Snoozed");}
    if(editing){
        if(pending && s.revision>pending_revision)home();
        else if(pending && s.storage_status!=ESP_OK){lv_label_set_text(edit_status,"Save failed - settings unchanged");pending=false;}
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
