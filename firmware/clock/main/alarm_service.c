#include "alarm_service.h"
#include "settings_store.h"
#include "clock_service.h"
#include "audio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include <string.h>
typedef struct {unsigned kind,index;alarm_config_t alarm;uint8_t brightness;} command_t;
static QueueHandle_t commands;
static SemaphoreHandle_t lock;
static alarm_snapshot_t published;
static void run(void *unused)
{
    (void)unused;clock_settings_t settings;alarm_engine_t engine={0};
    esp_err_t storage=settings_store_open(&settings);unsigned revision=0;
    uint8_t prior_ringing=0,prior_snoozed=0;
    memcpy(engine.alarms,settings.alarms,sizeof(engine.alarms));
    printf("SETTINGS_LOAD status=%s brightness=%u\n",esp_err_to_name(storage),settings.brightness);
    for(;;){
        uint64_t ms=esp_timer_get_time()/1000;command_t c;
        while(xQueueReceive(commands,&c,0)==pdTRUE){
            if(c.kind==1 || c.kind==2){
                clock_settings_t next=settings;
                if(c.kind==1){
                    /* Editing preserves consumed dates: cannot re-ring a dismissed occurrence. */
                    c.alarm.consumed_date=engine.alarms[c.index].consumed_date;
                    next.alarms[c.index]=c.alarm;
                }else next.brightness=c.brightness;
                storage=settings_store_save(&next);
                if(storage==ESP_OK){
                    settings=next;memcpy(engine.alarms,settings.alarms,sizeof(engine.alarms));
                    if(c.kind==1)alarm_cancel(&engine,c.index);
                    revision++;
                }
                printf("SETTINGS_SAVE status=%s kind=%u index=%u revision=%u\n",esp_err_to_name(storage),c.kind,c.index,revision);
            }else if(c.kind==3)alarm_snooze(&engine,ms);
            else if(c.kind==4)alarm_dismiss(&engine);
        }
        uint8_t consumed=alarm_tick(&engine,time(NULL),clock_valid(),ms);
        if(consumed){
            memcpy(settings.alarms,engine.alarms,sizeof(engine.alarms));
            storage=settings_store_save(&settings);revision++;
            printf("ALARM_TRIGGER mask=%u persistence=%s\n",consumed,esp_err_to_name(storage));
        }
        uint8_t ringing=alarm_ringing(&engine),snoozed=0;
        for(unsigned i=0;i<ALARM_COUNT;i++)if(engine.runtime[i].phase==ALARM_SNOOZED)snoozed|=1u<<i;
        if(ringing!=prior_ringing || snoozed!=prior_snoozed){
            printf("ALARM_PHASE ringing=%u snoozed=%u\n",ringing,snoozed);
            prior_ringing=ringing;prior_snoozed=snoozed;
        }
        audio_alarm(ringing!=0);
        xSemaphoreTake(lock,portMAX_DELAY);
        published=(alarm_snapshot_t){.settings=settings,.ringing=ringing,.snoozed=snoozed,.storage_status=storage,.revision=revision};
        xSemaphoreGive(lock);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void alarm_service_init(void)
{
    settings_defaults(&published.settings);
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
bool alarm_service_brightness(uint8_t b)
{ if(!b)return false;command_t c={.kind=2,.brightness=b};return xQueueSend(commands,&c,0)==pdTRUE; }
bool alarm_service_snooze(void)
{ command_t c={.kind=3};return xQueueSend(commands,&c,0)==pdTRUE; }
bool alarm_service_dismiss(void)
{ command_t c={.kind=4};return xQueueSend(commands,&c,0)==pdTRUE; }
