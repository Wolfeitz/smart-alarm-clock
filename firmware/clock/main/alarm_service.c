#include "diagnostics.h"
#include "alarm_service.h"
#include "settings_store.h"
#include "alarm_recovery.h"
#include <stdlib.h>
#include <stdatomic.h>
#include "clock_service.h"
#include "audio.h"
#include "alarm_output.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include <string.h>
typedef struct {unsigned kind,index;uint32_t ticket;uint64_t expected_session;unsigned expected_revision;alarm_config_t alarm;uint8_t brightness;display_schedule_t display;} command_t;
static QueueHandle_t commands;
static SemaphoreHandle_t lock;
static alarm_snapshot_t published;
static atomic_uint next_ticket;
static uint64_t owner_session;
static alarm_save_result_t results[16];
static unsigned result_cursor;
static void complete(uint32_t ticket,esp_err_t status,bool conflict,unsigned revision)
{
    if(!ticket)return;
    xSemaphoreTake(lock,portMAX_DELAY);
    results[result_cursor++%16]=(alarm_save_result_t){ticket,status,conflict,revision,owner_session};
    xSemaphoreGive(lock);
}
bool alarm_service_result(uint32_t ticket,alarm_save_result_t *out)
{
    if(!ticket||!out||!lock)return false;
    bool found=false;xSemaphoreTake(lock,portMAX_DELAY);
    for(unsigned i=0;i<16;i++)if(results[i].ticket==ticket){*out=results[i];found=true;break;}
    xSemaphoreGive(lock);return found;
}
static void run(void *unused)
{
    (void)unused;clock_settings_t settings;alarm_engine_t engine={0};
    esp_err_t storage=settings_store_open(&settings);unsigned revision=0;
    bool settings_loaded=storage==ESP_OK;
    uint8_t prior_ringing=0,prior_snoozed=0;
    uint32_t save_ticket=0;esp_err_t save_status=ESP_OK;bool save_conflict=false;
    memcpy(engine.alarms,settings.alarms,sizeof(engine.alarms));
    diagnostics_printf("SETTINGS_LOAD status=%s brightness=%u\n",esp_err_to_name(storage),settings.brightness);
    bool restored=false,retry=false;uint64_t last_ms=0,retry_at=0;time_t last_epoch=0;
    for(;;){
        uint64_t ms=esp_timer_get_time()/1000;time_t now=time(NULL);bool valid=clock_valid(),dirty=false;command_t c;
        if(!restored && valid){
            alarm_restore(&engine,&settings,now,ms);restored=true;dirty=true;
            diagnostics_printf("ALARM_RECOVERY restored=1\n");
        }
        while(xQueueReceive(commands,&c,0)==pdTRUE){
            if(c.kind==6){
                if(c.expected_session!=owner_session || c.expected_revision!=revision || dirty){
                    save_ticket=c.ticket;save_status=ESP_ERR_INVALID_STATE;save_conflict=true;
                    complete(c.ticket,save_status,true,revision);
                    diagnostics_printf("ALARM_CONFLICT ticket=%lu\n",(unsigned long)c.ticket);
                    continue;
                }
                c.kind=1;
            }
            if(c.kind==1 || c.kind==2 || c.kind==5){
                clock_settings_t next=settings;
                if(restored)alarm_capture(&next,&engine,now,ms);
                if(c.kind==1){
                    c.alarm=alarm_merge_edit(&engine.alarms[c.index],&c.alarm);
                    next.alarms[c.index]=c.alarm;next.phase[c.index]=0;next.deadline[c.index]=0;
                }else{next.brightness=c.brightness;if(c.kind==5)next.display=c.display;}
                /* Brightness must not replace alarms after a failed startup read.
                 * Only a deliberate alarm edit can establish a new configuration. */
                if(settings_loaded || c.kind==1)storage=settings_store_save(&next);
                if(storage==ESP_OK){
                    settings_loaded=true;settings=next;memcpy(engine.alarms,settings.alarms,sizeof(engine.alarms));
                    if(c.kind==1)alarm_cancel(&engine,c.index);
                    revision++;
                }
                if(c.ticket){save_ticket=c.ticket;save_status=storage;save_conflict=false;complete(c.ticket,storage,false,revision);}
                diagnostics_printf("SETTINGS_SAVE status=%s kind=%u index=%u revision=%u\n",esp_err_to_name(storage),c.kind,c.index,revision);
            }else if(c.kind==3){alarm_snooze(&engine,ms);dirty=true;}
            else if(c.kind==4){
                alarm_dismiss(&engine);memset(settings.phase,0,sizeof(settings.phase));
                memset(settings.deadline,0,sizeof(settings.deadline));dirty=true;
            }
        }
        uint8_t consumed=alarm_tick(&engine,now,valid,ms);
        if(restored){
            for(unsigned i=0;i<ALARM_COUNT;i++)if(settings.phase[i]!=engine.runtime[i].phase)dirty=true;
            if(last_ms && llabs((long long)(now-last_epoch)-(long long)((ms-last_ms)/1000))>2)dirty=true;
        }
        if(settings_loaded && (consumed || dirty || (retry && ms>=retry_at))){
            if(restored)alarm_capture(&settings,&engine,now,ms);
            storage=settings_store_save(&settings);retry=storage!=ESP_OK;retry_at=ms+5000;revision++;
            diagnostics_printf("ALARM_CHECKPOINT status=%s\n",esp_err_to_name(storage));
        }
        if(consumed)diagnostics_printf("ALARM_TRIGGER mask=%u persistence=%s\n",consumed,esp_err_to_name(storage));
        last_ms=ms;last_epoch=now;
        uint8_t ringing=alarm_ringing(&engine),snoozed=0;
        for(unsigned i=0;i<ALARM_COUNT;i++)if(engine.runtime[i].phase==ALARM_SNOOZED)snoozed|=1u<<i;
        if(ringing!=prior_ringing || snoozed!=prior_snoozed){
            diagnostics_printf("ALARM_PHASE ringing=%u snoozed=%u\n",ringing,snoozed);
            prior_ringing=ringing;prior_snoozed=snoozed;
        }
        bool local_sound=alarm_output_local(ringing,(uint32_t)ms);audio_alarm(local_sound);
        xSemaphoreTake(lock,portMAX_DELAY);
        published=(alarm_snapshot_t){.settings=settings,.ringing=ringing,.snoozed=snoozed,.snooze_seconds=alarm_snooze_seconds(&engine,ms),.local_sound=local_sound,.storage_status=storage,.load_failed=!settings_loaded,.revision=revision,.session=owner_session,.save_ticket=save_ticket,.save_status=save_status,.save_conflict=save_conflict};
        xSemaphoreGive(lock);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void alarm_service_init(void)
{
    settings_defaults(&published.settings);
    do{owner_session=((uint64_t)esp_random()<<32)|esp_random();}while(!owner_session);
    lock=xSemaphoreCreateMutex();commands=xQueueCreate(8,sizeof(command_t));
    if(!lock || !commands || xTaskCreate(run,"alarm_owner",4096,NULL,5,NULL)!=pdPASS)abort();
}
void alarm_service_snapshot(alarm_snapshot_t *s)
{ xSemaphoreTake(lock,portMAX_DELAY);*s=published;xSemaphoreGive(lock); }
bool alarm_service_save(unsigned i,const alarm_config_t *a)
{
    if(i>=ALARM_COUNT || !alarm_config_valid(a))return false;
    command_t c={.kind=1,.index=i,.alarm=*a};return xQueueSend(commands,&c,0)==pdTRUE;
}
uint32_t alarm_service_save_tracked(unsigned i,const alarm_config_t *a)
{
    if(i>=ALARM_COUNT || !alarm_config_valid(a))return 0;
    uint32_t ticket;
    do{ticket=atomic_fetch_add(&next_ticket,1)+1;}while(!ticket);
    command_t c={.kind=1,.index=i,.ticket=ticket,.alarm=*a};
    return xQueueSend(commands,&c,0)==pdTRUE?ticket:0;
}
bool alarm_service_brightness(uint8_t b)
{ if(!b)return false;command_t c={.kind=2,.brightness=b};return xQueueSend(commands,&c,0)==pdTRUE; }
bool alarm_service_snooze(void)
{ command_t c={.kind=3};return xQueueSend(commands,&c,0)==pdTRUE; }
bool alarm_service_dismiss(void)
{ command_t c={.kind=4};return xQueueSend(commands,&c,0)==pdTRUE; }

uint32_t alarm_service_display(const display_schedule_t *schedule,uint8_t brightness)
{
    if(!brightness||!display_schedule_valid(schedule))return 0;
    uint32_t ticket;do{ticket=atomic_fetch_add(&next_ticket,1)+1;}while(!ticket);
    command_t c={.kind=5,.ticket=ticket,.display=*schedule,.brightness=brightness};
    return xQueueSend(commands,&c,0)==pdTRUE?ticket:0;
}

uint32_t alarm_service_save_conditional(unsigned i,const alarm_config_t *a,
                                        uint64_t expected_session,unsigned expected_revision)
{
    if(i>=ALARM_COUNT || !a || !alarm_config_valid(a) || !expected_session)return 0;
    uint32_t ticket;
    do{ticket=atomic_fetch_add(&next_ticket,1)+1;}while(!ticket);
    command_t c={.kind=6,.index=i,.ticket=ticket,.alarm=*a,
                 .expected_session=expected_session,.expected_revision=expected_revision};
    return xQueueSend(commands,&c,0)==pdTRUE?ticket:0;
}
