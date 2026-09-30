/* Execute the production owner loop; only RTOS/clock/storage/audio are fake. */
#include "alarm_service.h"
#include "settings_store.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static jmp_buf finished;
static void (*owner)(void *);
static unsigned char queue[8][256];
static size_t item_size;static unsigned head,count,iterations,saves;
static uint64_t ms=1000;static time_t epoch;
static clock_settings_t durable;static int storage_error=93;
static bool sounding,edit_case;static uint32_t ticket,retry_ticket;
static unsigned successful_saves;
static unsigned recovery_case;
static bool clock_ready=true;
int64_t esp_timer_get_time(void){return ms*1000;}
time_t time(time_t *out){time_t value=epoch+ms/1000;if(out)*out=value;return value;}
bool clock_valid(void){return clock_ready;}
void audio_alarm(bool active){sounding=active;}
const char *esp_err_to_name(esp_err_t error){return error?"injected failure":"ESP_OK";}
void diagnostics_printf(const char *format,...){(void)format;}
esp_err_t settings_store_open(clock_settings_t *out){*out=durable;return ESP_OK;}
esp_err_t settings_store_save(const clock_settings_t *in)
{saves++;if(storage_error)return storage_error;durable=*in;successful_saves++;return ESP_OK;}
QueueHandle_t xQueueCreate(unsigned n,size_t size){assert(n==8&&size<=sizeof(queue[0]));item_size=size;return queue;}
int xQueueSend(QueueHandle_t q,const void *v,unsigned wait)
{(void)q;(void)wait;if(count==8)return 0;memcpy(queue[(head+count)%8],v,item_size);count++;return 1;}
int xQueueReceive(QueueHandle_t q,void *v,unsigned wait)
{(void)q;(void)wait;if(!count)return 0;memcpy(v,queue[head],item_size);head=(head+1)%8;count--;return 1;}
SemaphoreHandle_t xSemaphoreCreateMutex(void){return (void *)1;}
int xSemaphoreTake(SemaphoreHandle_t s,unsigned wait){(void)s;(void)wait;return 1;}
int xSemaphoreGive(SemaphoreHandle_t s){(void)s;return 1;}
int xTaskCreate(void (*entry)(void *),const char *name,unsigned stack,void *arg,unsigned priority,void *handle)
{assert(!strcmp(name,"alarm_owner")&&stack>=4096&&priority==5);(void)arg;(void)handle;owner=entry;return 1;}
void vTaskDelay(unsigned ticks)
{
    assert(ticks==100);iterations++;alarm_snapshot_t s;alarm_service_snapshot(&s);
    if(recovery_case){
        assert(!sounding&&!s.ringing);
        if(iterations==1){
            assert(!s.snoozed&&saves==0&&durable.phase[0]==ALARM_SNOOZED);
            if(recovery_case==1)assert(alarm_service_brightness(25));
            else if(recovery_case==2)assert(alarm_service_dismiss());
            else {alarm_config_t edit=s.settings.alarms[0];edit.hour=8;
                ticket=alarm_service_save_tracked(0,&edit);assert(ticket);}
        }else if(iterations==2){
            assert(!s.snoozed&&saves==1);
            if(recovery_case==1){assert(durable.brightness==25&&durable.phase[0]==ALARM_SNOOZED);}
            else assert(durable.phase[0]==ALARM_IDLE&&durable.deadline[0]==0);
            if(recovery_case==3)assert(s.save_ticket==ticket&&s.save_status==ESP_OK&&durable.alarms[0].hour==8);
            clock_ready=true;
        }else if(iterations==3){
            if(recovery_case==1){
                assert(s.snoozed==1&&s.snooze_seconds>0&&s.snooze_seconds<=60);
                assert(s.settings.brightness==25&&durable.phase[0]==ALARM_SNOOZED);
                assert(alarm_service_dismiss());
            }else {assert(!s.snoozed&&durable.phase[0]==ALARM_IDLE);longjmp(finished,1);}
        }else {
            assert(!s.snoozed&&durable.phase[0]==ALARM_IDLE&&durable.deadline[0]==0);
            longjmp(finished,1);
        }
    }else if(edit_case){
        if(iterations==1){
            assert(s.save_ticket==ticket&&s.save_status==93&&s.settings.alarms[0].hour==7);
            assert(durable.alarms[0].hour==7&&!sounding);storage_error=0;
            alarm_config_t edit=s.settings.alarms[0];edit.hour=8;
            retry_ticket=alarm_service_save_tracked(0,&edit);assert(retry_ticket&&retry_ticket!=ticket);
        }else{
            assert(s.save_ticket==retry_ticket&&s.save_status==ESP_OK);
            assert(s.settings.alarms[0].hour==8&&durable.alarms[0].hour==8&&!sounding);
            longjmp(finished,1);
        }
    }else{
        if(iterations==1){
            assert(s.ringing==1&&sounding&&s.storage_status==93);
            assert(durable.alarms[0].consumed_date==0&&s.settings.alarms[0].consumed_date==20260101);
            assert(alarm_service_snooze());
        }else if(iterations==2){
            assert(!s.ringing&&s.snoozed==1&&!sounding&&s.snooze_seconds==300&&s.storage_status==93);
            assert(alarm_service_dismiss());
        }else{
            assert(!s.ringing&&!s.snoozed&&!sounding);
            if(iterations==3){assert(s.storage_status==93);storage_error=0;}
            if(iterations==60){
                assert(s.storage_status==ESP_OK&&successful_saves==1);
                assert(durable.alarms[0].consumed_date==20260101&&durable.phase[0]==ALARM_IDLE);
                assert(saves==4);longjmp(finished,1);
            }
        }
    }
    ms+=ticks;
}
int main(int argc,char **argv)
{
    edit_case=argc>1&&!strcmp(argv[1],"edit");setenv("TZ","UTC0",1);tzset();
    struct tm t={.tm_year=126,.tm_mon=0,.tm_mday=1,.tm_hour=6,.tm_min=59,.tm_sec=59};epoch=mktime(&t);
    settings_defaults(&durable);durable.alarms[0]=(alarm_config_t){.enabled=!edit_case,.hour=7,.weekdays=127};
    if(argc>1){
        if(!strcmp(argv[1],"invalid-brightness"))recovery_case=1;
        if(!strcmp(argv[1],"invalid-dismiss"))recovery_case=2;
        if(!strcmp(argv[1],"invalid-edit"))recovery_case=3;
    }
    if(recovery_case){
        clock_ready=false;storage_error=0;
        durable.alarms[0].consumed_date=20260101;
        durable.phase[0]=ALARM_SNOOZED;durable.deadline[0]=(uint32_t)time(NULL)+60;
    }
    alarm_service_init();assert(owner);
    if(edit_case){alarm_config_t edit=durable.alarms[0];edit.hour=8;ticket=alarm_service_save_tracked(0,&edit);assert(ticket);}
    if(!setjmp(finished))owner(NULL);
    if(recovery_case){puts("PASS invalid-time startup: pending snooze preserved by brightness, canceled by dismiss/edit before RTC recovery");return 0;}
    puts(edit_case?"PASS alarm owner tracked failed edit preserves settings; successful retry persists":"PASS alarm owner rings despite checkpoint failure; snooze/dismiss work; bounded retry persists without retrigger");
}
