#include "sensor_service.h"
#include "diagnostics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include "lvgl.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_psram.h"
#include "driver/usb_serial_jtag.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "board.h"
#include "clock_service.h"
#include "audio.h"
#include "alarm_service.h"
#include "clock_ui.h"
#include "weather_service.h"
#include "background_service.h"
#include "json_memory.h"
#include "network_http.h"
#include "ha_service.h"
#include "media_service.h"
#include "sonos_setup.h"
#include "setup_model.h"
static bool test_touch;static int test_x,test_y;static uint32_t test_until;
static uint32_t tick(void){return (uint32_t)(esp_timer_get_time()/1000);}
static void flush(lv_display_t *d,const lv_area_t *a,uint8_t *data)
{
    lv_draw_sw_rgb565_swap(data,(a->x2-a->x1+1)*(a->y2-a->y1+1));
    board_flush(a->x1,a->y1,a->x2+1,a->y2+1,data);lv_display_flush_ready(d);
}
static void touch_read(lv_indev_t *i,lv_indev_data_t *d)
{
    (void)i;int x,y;bool pressed;
    if(test_touch){
        if(!test_until){test_until=tick()+120;diagnostics_printf("UI_INPUT phase=press x=%d y=%d\n",test_x,test_y);}
        x=test_x;y=test_y;pressed=(int32_t)(test_until-tick())>0;
        if(!pressed){test_touch=false;diagnostics_printf("UI_INPUT phase=release x=%d y=%d\n",test_x,test_y);}
    }else pressed=board_touch(&x,&y);
    d->state=pressed?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;
    if(pressed){clock_ui_touch();d->point.x=x;d->point.y=y;}
}
static void serial_poll(void)
{
    static char line[2048];static size_t n;static bool overflow;
    char b[64];int count=usb_serial_jtag_read_bytes(b,sizeof(b),0);
    for(int i=0;i<count;i++){
        if(b[i]=='\r')continue;
        if(b[i]=='\n'){
            line[n]=0;
            if(!overflow&&!strcmp(line,"SETUP?"))diagnostics_printf("SETUP_READY version=1\n");
            if(!overflow&&!strncmp(line,"HA_SETUP ",9)){
                setup_request_t request;
                if(setup_parse(line+9,n-9,&request))diagnostics_printf("SETUP_HA tag=%lu accepted=%u\n",(unsigned long)request.tag,
                    ha_service_configure_tagged(request.endpoint,request.token,request.entity,request.tag));
                else diagnostics_printf("SETUP_ERROR invalid_request\n");
                memset(&request,0,sizeof(request));
            }
            if(!overflow&&!strncmp(line,"MEDIA_SETUP ",12)){
                uint32_t tag=0;char entity[96],extra;
                bool valid=sscanf(line+12,"%"SCNu32" %95s %c",&tag,entity,&extra)==2&&tag;
                if(valid)diagnostics_printf("SETUP_MEDIA tag=%lu accepted=%u\n",(unsigned long)tag,media_service_configure_tagged(entity,tag));
                else diagnostics_printf("SETUP_ERROR invalid_request\n");
            }
            if(overflow)diagnostics_printf("SETUP_ERROR line_too_long\n");
            if(!overflow&&!strncmp(line,"BACKGROUND ",11)){
                static background_config_t config;memset(&config,0,sizeof(config));bool accepted=false;
                if(!strcmp(line+11,"NEXT"))accepted=background_service_next();
                else if(!strcmp(line+11,"LOCAL")){config.source=BACKGROUND_LOCAL;config.count=1;strcpy(config.images[0],"blue-hour");accepted=background_service_configure(&config);}
                else if(!strncmp(line+11,"WALLHAVEN ",10)&&strlen(line+21)<BACKGROUND_QUERY_SIZE){
                    config.source=BACKGROUND_WALLHAVEN;config.interval_seconds=900;strcpy(config.query,line+21);accepted=background_service_configure(&config);
                }
                diagnostics_printf("BACKGROUND_REQUEST accepted=%u\n",accepted);
            }
            if(!overflow&&!strncmp(line,"SONOS_LOOKUP ",13))
                diagnostics_printf("SONOS_LOOKUP accepted=%u\n",sonos_setup_lookup(line+13));
            if(!overflow&&!strcmp(line,"SONOS_STATE")){
                static sonos_setup_snapshot_t setup;sonos_setup_snapshot(&setup);
                diagnostics_printf("SONOS_STATE busy=%u ready=%u revision=%u count=%u total=%u status=%s\n",
                    setup.busy,setup.ready,setup.revision,setup.favorites.count,setup.favorites.total,setup.status);
            }
            if(!overflow && strcmp(line,"UI")==0)clock_ui_diagnostics();
            if(!overflow && strncmp(line,"TAP ",4)==0){
                int x,y;char extra;bool ok=!test_touch && sscanf(line+4,"%d %d %c",&x,&y,&extra)==2 && x>=0 && x<480 && y>=0 && y<320;
                if(ok){test_x=x;test_y=y;test_until=0;test_touch=true;}
                diagnostics_printf("UI_TAP accepted=%d\n",ok);
            }
            if(!overflow && strncmp(line,"TIME ",5)==0){
                char *end;errno=0;long long value=strtoll(line+5,&end,10);
                esp_err_t err=ESP_ERR_INVALID_ARG;
                if(end!=line+5 && *end==0 && !errno && value>=946684800LL && value<4102444800LL)
                    err=clock_set((time_t)value);
                diagnostics_printf("TIME_SET status=%s epoch=%lld\n",esp_err_to_name(err),value);clock_ui_update();
            }
            if(!overflow && strcmp(line,"SOUND")==0)diagnostics_printf("AUDIO_TEST_REQUEST accepted=%d\n",audio_test());
            if(!overflow && strncmp(line,"ALARM ",6)==0){
                unsigned i,h,m,days,date,on;char extra;
                bool ok=sscanf(line+6,"%u %u %u %u %u %u %c",&i,&h,&m,&days,&date,&on,&extra)==6;
                if(ok && i<ALARM_COUNT && h<24 && m<60 && days<128 && on<2){
                    alarm_config_t a={.enabled=on,.hour=h,.minute=m,.weekdays=days,.once_date=date};
                    ok=alarm_service_save(i,&a);
                }else ok=false;
                diagnostics_printf("ALARM_EDIT accepted=%d\n",ok);
            }
            if(!overflow && strcmp(line,"SNOOZE")==0)diagnostics_printf("ALARM_SNOOZE accepted=%d\n",alarm_service_snooze());
            if(!overflow && strcmp(line,"DISMISS")==0)diagnostics_printf("ALARM_DISMISS accepted=%d\n",alarm_service_dismiss());
            if(!overflow && strncmp(line,"BRIGHT ",7)==0){
                unsigned value;char extra;
                bool ok=sscanf(line+7,"%u %c",&value,&extra)==1 && value>0 && value<=255;
                diagnostics_printf("BRIGHT_SET accepted=%d\n",ok && alarm_service_brightness(value));
            }
            if(!overflow && (!strcmp(line,"NETWORK OFF")||!strcmp(line,"NETWORK ON")))
                diagnostics_printf("NETWORK_REQUEST accepted=%u\n",weather_service_radio(!strcmp(line,"NETWORK ON")));
            if(!overflow && strcmp(line,"STATE")==0){
                sensor_service_diagnostics();
                alarm_snapshot_t a;alarm_service_snapshot(&a);
                diagnostics_printf("ALARM_STATE ringing=%u snoozed=%u brightness=%u storage=%s revision=%u\n",a.ringing,a.snoozed,a.settings.brightness,esp_err_to_name(a.storage_status),a.revision);
                for(unsigned j=0;j<ALARM_COUNT;j++){
                    alarm_config_t *c=&a.settings.alarms[j];
                    diagnostics_printf("ALARM_SLOT index=%u enabled=%u hour=%u minute=%u days=%u date=%lu consumed=%lu\n",j,c->enabled,c->hour,c->minute,c->weekdays,(unsigned long)c->once_date,(unsigned long)c->consumed_date);
                }
            }
            memset(line,0,sizeof(line));n=0;overflow=false;
        }else if(n<sizeof(line)-1)line[n++]=b[i];else overflow=true;
    }
}
void app_main(void)
{
    /* UI/RTC owner stays above HTTPS work; alarm owner remains higher still. */
    vTaskPrioritySet(NULL,3);json_memory_init();network_http_init();diagnostics_init();
    diagnostics_printf("IMAGE_MEMORY psram=%u external_free=%lu internal_free=%lu\n",esp_psram_is_initialized(),(unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM),(unsigned long)heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    board_init();clock_init(board_bus());audio_init(board_bus());weather_service_init();alarm_service_init();background_service_init();sensor_service_init();
    lv_init();lv_tick_set_cb(tick);
    lv_display_t *d=lv_display_create(480,320);lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB565);
    void *buf=heap_caps_malloc(480*20*2,MALLOC_CAP_DMA);if(!buf)abort();
    lv_display_set_buffers(d,buf,NULL,480*20*2,LV_DISPLAY_RENDER_MODE_PARTIAL);lv_display_set_flush_cb(d,flush);
    lv_indev_t *input=lv_indev_create();lv_indev_set_type(input,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(input,touch_read);
    clock_ui_init();clock_ui_update();diagnostics_printf("CLOCK_READY lvgl=%d.%d.%d timezone=%s\n",LVGL_VERSION_MAJOR,LVGL_VERSION_MINOR,LVGL_VERSION_PATCH,weather_service_timezone());
    uint32_t last=0,report=0;
    for(;;){
        serial_poll();time_t network_time;if(weather_service_take_time(&network_time))diagnostics_printf("NETWORK_TIME rtc=%s\n",esp_err_to_name(clock_set_network(network_time)));uint32_t now=tick();
        if(now-last>=1000){last=now;clock_ui_update();}
        if(now-report>=10000){
            report=now;time_t rtc_epoch=0;esp_err_t err=clock_rtc_epoch(&rtc_epoch);
            diagnostics_printf("CLOCK_ALIVE valid=%d epoch=%lld rtc=%lld rtc_status=%s heap=%lu\n",clock_valid(),(long long)time(NULL),(long long)rtc_epoch,esp_err_to_name(err),(unsigned long)esp_get_free_heap_size());
        }
        lv_timer_handler();vTaskDelay(pdMS_TO_TICKS(10));
    }
}
