#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include "lvgl.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "driver/usb_serial_jtag.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "board.h"
#include "clock_service.h"
#include "audio.h"
static lv_obj_t *time_label,*date_label,*seconds_label,*source_label,*dim_label;
static uint32_t tick(void){return (uint32_t)(esp_timer_get_time()/1000);}
static void flush(lv_display_t *d,const lv_area_t *a,uint8_t *data)
{
    lv_draw_sw_rgb565_swap(data,(a->x2-a->x1+1)*(a->y2-a->y1+1));
    board_flush(a->x1,a->y1,a->x2+1,a->y2+1,data);lv_display_flush_ready(d);
}
static void touch_read(lv_indev_t *i,lv_indev_data_t *d)
{
    (void)i;int x,y;bool pressed=board_touch(&x,&y);
    d->state=pressed?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;
    if(pressed){d->point.x=x;d->point.y=y;}
}
static void sound_clicked(lv_event_t *e)
{
    (void)e;printf("AUDIO_TEST_REQUEST accepted=%d\n",audio_test());
}
static void dim_clicked(lv_event_t *e)
{
    (void)e;static bool dim;dim=!dim;board_brightness(dim);
    lv_label_set_text(dim_label,dim?"Brighten":"Dim screen");printf("CLOCK_BRIGHTNESS dim=%d\n",dim);
}
static lv_obj_t *label(const char *text,const lv_font_t *font,int y,uint32_t color)
{
    lv_obj_t *o=lv_label_create(lv_screen_active());lv_label_set_text(o,text);
    lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_color(o,lv_color_hex(color),0);
    lv_obj_align(o,LV_ALIGN_TOP_MID,0,y);return o;
}
static void update(void)
{
    if(!clock_valid())return;
    time_t now=time(NULL);struct tm local;localtime_r(&now,&local);char b[64];
    strftime(b,sizeof(b),"%I:%M",&local);lv_label_set_text(time_label,b[0]=='0'?b+1:b);
    strftime(b,sizeof(b),"%A, %B %d",&local);lv_label_set_text(date_label,b);
    strftime(b,sizeof(b),"%p   :%S   %Z",&local);lv_label_set_text(seconds_label,b);
    lv_label_set_text(source_label,clock_source());
}
static void serial_poll(void)
{
    static char line[48];static size_t n;static bool overflow;
    char b[64];int count=usb_serial_jtag_read_bytes(b,sizeof(b),0);
    for(int i=0;i<count;i++){
        if(b[i]=='\r')continue;
        if(b[i]=='\n'){
            line[n]=0;
            if(!overflow && strncmp(line,"TIME ",5)==0){
                char *end;errno=0;long long value=strtoll(line+5,&end,10);
                esp_err_t err=ESP_ERR_INVALID_ARG;
                if(end!=line+5 && *end==0 && !errno && value>=946684800LL && value<4102444800LL)
                    err=clock_set((time_t)value);
                printf("TIME_SET status=%s epoch=%lld\n",esp_err_to_name(err),value);update();
            }
            if(!overflow && strcmp(line,"SOUND")==0)printf("AUDIO_TEST_REQUEST accepted=%d\n",audio_test());
            n=0;overflow=false;
        }else if(n<sizeof(line)-1)line[n++]=b[i];else overflow=true;
    }
}
void app_main(void)
{
    board_init();clock_init(board_bus());audio_init(board_bus());
    usb_serial_jtag_driver_config_t usb=USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb));
    lv_init();lv_tick_set_cb(tick);
    lv_display_t *d=lv_display_create(480,320);lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB565);
    void *buf=heap_caps_malloc(480*20*2,MALLOC_CAP_DMA);if(!buf)abort();
    lv_display_set_buffers(d,buf,NULL,480*20*2,LV_DISPLAY_RENDER_MODE_PARTIAL);lv_display_set_flush_cb(d,flush);
    lv_indev_t *input=lv_indev_create();lv_indev_set_type(input,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(input,touch_read);
    lv_obj_set_style_bg_color(lv_screen_active(),lv_color_hex(0x0b1119),0);
    date_label=label("Clock",&lv_font_montserrat_20,30,0xa6b5c8);
    time_label=label("--:--",&lv_font_montserrat_48,103,0xf6eddc);
    lv_obj_set_style_transform_pivot_x(time_label,LV_PCT(50),0);
    lv_obj_set_style_transform_pivot_y(time_label,LV_PCT(50),0);
    lv_obj_set_style_transform_scale(time_label,384,0);
    seconds_label=label("Waiting for time",&lv_font_montserrat_20,186,0xa6b5c8);
    source_label=label("Set time via USB",&lv_font_montserrat_16,225,0x7c9eaa);
    lv_obj_t *button=lv_button_create(lv_screen_active());lv_obj_set_size(button,164,46);
    lv_obj_align(button,LV_ALIGN_BOTTOM_LEFT,52,-16);lv_obj_set_style_bg_color(button,lv_color_hex(0x203347),0);
    lv_obj_add_event_cb(button,dim_clicked,LV_EVENT_CLICKED,NULL);
    dim_label=lv_label_create(button);lv_label_set_text(dim_label,"Dim screen");lv_obj_center(dim_label);
    lv_obj_t *sound=lv_button_create(lv_screen_active());lv_obj_set_size(sound,164,46);
    lv_obj_align(sound,LV_ALIGN_BOTTOM_RIGHT,-52,-16);
    lv_obj_set_style_bg_color(sound,lv_color_hex(0x203347),0);
    lv_obj_add_event_cb(sound,sound_clicked,LV_EVENT_CLICKED,NULL);
    lv_obj_t *sound_label=lv_label_create(sound);lv_label_set_text(sound_label,"Test sound");lv_obj_center(sound_label);
    update();printf("CLOCK_READY lvgl=%d.%d.%d timezone=America/New_York\n",LVGL_VERSION_MAJOR,LVGL_VERSION_MINOR,LVGL_VERSION_PATCH);
    uint32_t last=0,report=0;
    for(;;){
        serial_poll();uint32_t now=tick();
        if(now-last>=1000){last=now;update();}
        if(now-report>=10000){
            report=now;time_t rtc_epoch=0;esp_err_t err=clock_rtc_epoch(&rtc_epoch);
            printf("CLOCK_ALIVE valid=%d epoch=%lld rtc=%lld rtc_status=%s heap=%lu\n",clock_valid(),(long long)time(NULL),(long long)rtc_epoch,esp_err_to_name(err),(unsigned long)esp_get_free_heap_size());
        }
        lv_timer_handler();vTaskDelay(pdMS_TO_TICKS(10));
    }
}
