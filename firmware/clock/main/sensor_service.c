#include "sensor_service.h"
#include "sensor_model.h"
#include "board.h"
#include "alarm_service.h"
#include "diagnostics.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "nvs.h"
#include <string.h>
static SemaphoreHandle_t lock;
static QueueHandle_t changes;
static sensor_snapshot_t state;
static i2c_master_dev_handle_t environment,imu;
static bool running;
void sensor_service_snapshot(sensor_snapshot_t *out)
{
    memset(out,0,sizeof(*out));if(!running)return;
    xSemaphoreTake(lock,portMAX_DELAY);*out=state;xSemaphoreGive(lock);
    out->fresh=out->available&&esp_timer_get_time()/1000-out->updated_ms<30000;
}
bool sensor_service_set_shake(bool enabled)
{
    if(!running)return false;
    xSemaphoreTake(lock,portMAX_DELAY);
    bool ok=!state.pending&&xQueueSend(changes,&enabled,0)==pdTRUE;
    if(ok){state.pending=true;state.save_failed=false;}
    xSemaphoreGive(lock);return ok;
}
static bool write_reg(i2c_master_dev_handle_t dev,uint8_t reg,uint8_t value)
{uint8_t data[]={reg,value};return i2c_master_transmit(dev,data,2,20)==ESP_OK;}
static bool read_reg(i2c_master_dev_handle_t dev,uint8_t reg,uint8_t *data,unsigned size)
{return i2c_master_transmit_receive(dev,&reg,1,data,size,20)==ESP_OK;}
static bool command(uint16_t value)
{uint8_t data[]={value>>8,value&255};return environment&&i2c_master_transmit(environment,data,2,20)==ESP_OK;}
static bool environment_read(float *t,float *h)
{
    if(!command(0x3517))return false;
    vTaskDelay(pdMS_TO_TICKS(10));
    bool ok=command(0x7866);uint8_t bytes[6];
    if(ok){vTaskDelay(pdMS_TO_TICKS(20));ok=i2c_master_receive(environment,bytes,6,20)==ESP_OK;}
    command(0xb098);
    return ok&&sensor_environment(bytes,t,h);
}
static bool imu_start(void)
{
    if(!imu)return false;
    uint8_t id=0;if(!read_reg(imu,0,&id,1)||id!=5)return false;
    /* Vendor register definitions: disable, auto-increment, +/-8g 125Hz,
     * acceleration low-pass, no FIFO, accelerometer only (gyro unneeded). */
    return write_reg(imu,8,0)&&write_reg(imu,2,0x60)&&write_reg(imu,3,0x26)&&
           write_reg(imu,6,3)&&write_reg(imu,20,0)&&write_reg(imu,8,1);
}
static void worker(void *arg)
{
    (void)arg;nvs_handle_t storage;bool stored=nvs_open_from_partition("clockcfg","sensors",NVS_READWRITE,&storage)==ESP_OK;
    uint8_t value=0;if(stored&&nvs_get_u8(storage,"shake",&value)!=ESP_OK)value=0;
    bool enabled=value==1,imu_ok=imu_start();
    xSemaphoreTake(lock,portMAX_DELAY);state.shake_enabled=enabled;state.imu_ready=imu_ok;xSemaphoreGive(lock);
    diagnostics_printf("SENSORS_INIT environment=%u imu=%u shake=%u\n",environment!=NULL,imu_ok,enabled);
    shake_detector_t detector={0};int64_t due=0,retry=0;
    for(;;){
        bool next;
        if(xQueueReceive(changes,&next,0)==pdTRUE){
            esp_err_t err=stored?nvs_set_u8(storage,"shake",next):ESP_ERR_INVALID_STATE;
            if(err==ESP_OK)err=nvs_commit(storage);
            if(err==ESP_OK)enabled=next;
            memset(&detector,0,sizeof(detector));
            xSemaphoreTake(lock,portMAX_DELAY);state.shake_enabled=enabled;state.pending=false;state.save_failed=err!=ESP_OK;xSemaphoreGive(lock);
        }
        int64_t now=esp_timer_get_time()/1000;
        if(now>=due){
            float t,h;bool ok=environment_read(&t,&h);
            xSemaphoreTake(lock,portMAX_DELAY);
            if(ok){state.celsius=t;state.humidity=h;state.updated_ms=esp_timer_get_time()/1000;state.available=true;}
            xSemaphoreGive(lock);due=now+10000;
        }
        if(!imu_ok&&now>=retry){imu_ok=imu_start();retry=now+10000;}
        if(imu_ok){
            uint8_t status=0,raw[6];
            bool ok=read_reg(imu,0x2e,&status,1);
            if(ok&&(status&1)){
                ok=read_reg(imu,0x35,raw,6);
                if(ok){
                    int mg[3];sensor_acceleration(raw,mg);
                    alarm_snapshot_t alarm;alarm_service_snapshot(&alarm);
                    bool shake=sensor_shake(&detector,esp_timer_get_time()/1000,enabled,alarm.ringing!=0,mg);
                    bool accepted=shake&&alarm_service_snooze();
                    xSemaphoreTake(lock,portMAX_DELAY);memcpy(state.acceleration,mg,sizeof(mg));state.samples++;if(accepted)state.gestures++;xSemaphoreGive(lock);
                    if(shake)diagnostics_printf("SHAKE_SNOOZE accepted=%u\n",accepted);
                }
            }
            if(!ok){imu_ok=false;memset(&detector,0,sizeof(detector));}
        }
        xSemaphoreTake(lock,portMAX_DELAY);state.imu_ready=imu_ok;xSemaphoreGive(lock);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
void sensor_service_init(void)
{
    lock=xSemaphoreCreateMutex();changes=xQueueCreate(1,sizeof(bool));if(!lock||!changes)return;
    i2c_device_config_t cfg={.dev_addr_length=I2C_ADDR_BIT_LEN_7,.scl_speed_hz=100000,.device_address=0x70};
    if(i2c_master_bus_add_device(board_bus(),&cfg,&environment)!=ESP_OK)environment=NULL;
    for(unsigned addr=0x6a;addr<=0x6b;addr++){
        if(i2c_master_probe(board_bus(),addr,20)!=ESP_OK)continue;
        cfg.device_address=addr;
        if(i2c_master_bus_add_device(board_bus(),&cfg,&imu)==ESP_OK)break;
    }
    running=true;if(xTaskCreate(worker,"sensors",4096,NULL,2,NULL)!=pdPASS)running=false;
}
void sensor_service_diagnostics(void)
{
    sensor_snapshot_t s;sensor_service_snapshot(&s);
    diagnostics_printf("SENSOR_STATE available=%u fresh=%u temperature_c=%.2f humidity=%.2f imu=%u samples=%u mg=%d,%d,%d shake=%u gestures=%u pending=%u save_failed=%u\n",s.available,s.fresh,s.celsius,s.humidity,s.imu_ready,s.samples,s.acceleration[0],s.acceleration[1],s.acceleration[2],s.shake_enabled,s.gestures,s.pending,s.save_failed);
}
