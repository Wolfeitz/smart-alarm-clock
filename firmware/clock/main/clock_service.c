#include "diagnostics.h"
#include "clock_service.h"
#include "rtc_codec.h"
#include <stdlib.h>
#include <stdatomic.h>
#include <stdio.h>
#include <sys/time.h>
static i2c_master_dev_handle_t rtc;
static atomic_bool synchronized;
static const char *source="Set time to begin";
static esp_err_t read_regs(uint8_t reg,uint8_t *v,size_t n)
{ return i2c_master_transmit_receive(rtc,&reg,1,v,n,100); }
static esp_err_t write_reg(uint8_t reg,uint8_t v)
{ uint8_t b[]={reg,v};return i2c_master_transmit(rtc,b,2,100); }
esp_err_t clock_rtc_epoch(time_t *epoch)
{
    uint8_t r[11];esp_err_t e=read_regs(0,r,sizeof(r));if(e!=ESP_OK)return e;
    struct tm utc;
    if((r[0]&0xa2) || r[3]!=0xa7 || !rtc_decode(r+4,&utc))return ESP_ERR_INVALID_STATE;
    *epoch=timegm(&utc);return *epoch<0?ESP_ERR_INVALID_STATE:ESP_OK;
}
void clock_init(i2c_master_bus_handle_t bus)
{
    setenv("TZ","EST5EDT,M3.2.0/2,M11.1.0/2",1);tzset();
    i2c_device_config_t cfg={.dev_addr_length=I2C_ADDR_BIT_LEN_7,.device_address=0x51,.scl_speed_hz=100000};
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus,&cfg,&rtc));
    time_t epoch;esp_err_t e=clock_rtc_epoch(&epoch);
    if(e==ESP_OK){struct timeval tv={.tv_sec=epoch};settimeofday(&tv,NULL);synchronized=true;source="RTC / offline";}
    diagnostics_printf("CLOCK_INIT rtc=%s source=%s\n",esp_err_to_name(e),source);
}
bool clock_valid(void){return synchronized;}
const char *clock_source(void){return source;}
esp_err_t clock_set(time_t epoch)
{
    struct tm utc;gmtime_r(&epoch,&utc);uint8_t data[8]={4};
    if(!rtc_encode(&utc,data+1))return ESP_ERR_INVALID_ARG;
    uint8_t control;esp_err_t e=read_regs(0,&control,1);if(e!=ESP_OK)return e;
    e=write_reg(3,0);if(e!=ESP_OK)return e;
    /* Preserve crystal load and correction IRQ; normal mode, stopped, 24-hour. */
    control&=0x05;
    e=write_reg(0,control|0x20);if(e!=ESP_OK)return e;
    e=i2c_master_transmit(rtc,data,sizeof(data),100);
    esp_err_t resume=write_reg(0,control);if(e!=ESP_OK)return e;if(resume!=ESP_OK)return resume;
    uint8_t verify[7];e=read_regs(4,verify,7);if(e!=ESP_OK)return e;
    struct tm decoded;if(!rtc_decode(verify,&decoded))return ESP_ERR_INVALID_RESPONSE;
    time_t check=timegm(&decoded);if(check<epoch || check>epoch+1)return ESP_ERR_INVALID_RESPONSE;
    e=write_reg(3,0xa7);if(e!=ESP_OK)return e;
    struct timeval tv={.tv_sec=check};if(settimeofday(&tv,NULL)!=0)return ESP_FAIL;
    synchronized=true;source="Time saved / offline";return ESP_OK;
}
