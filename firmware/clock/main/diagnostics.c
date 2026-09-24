#include "diagnostics.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/usb_serial_jtag.h"
#include "esp_log.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
typedef struct {char text[192];} record_t;
static QueueHandle_t records;
static void drain(void *unused)
{
    (void)unused;record_t record;
    for(;;){
        if(xQueueReceive(records,&record,portMAX_DELAY)==pdTRUE)
            usb_serial_jtag_write_bytes(record.text,strlen(record.text),pdMS_TO_TICKS(20));
    }
}
static int sdk_log(const char *format,va_list args)
{
    record_t record;int size=vsnprintf(record.text,sizeof(record.text),format,args);
    if(records)xQueueSend(records,&record,0);
    return size;
}
void diagnostics_init(void)
{
    usb_serial_jtag_driver_config_t usb=USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    /* Hold a complete bounded setup frame while a display flush delays polling. */
    usb.rx_buffer_size=4096;
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb));
    usb_serial_jtag_vfs_use_driver();
    records=xQueueCreate(32,sizeof(record_t));
    if(!records || xTaskCreate(drain,"diagnostics",3072,NULL,1,NULL)!=pdPASS)abort();
    esp_log_set_vprintf(sdk_log);esp_log_level_set("wifi",ESP_LOG_WARN);esp_log_level_set("wifi_init",ESP_LOG_WARN);
}
void diagnostics_printf(const char *format,...)
{
    record_t record;va_list args;va_start(args,format);
    vsnprintf(record.text,sizeof(record.text),format,args);va_end(args);
    /* Best effort only: device operation never waits for a diagnostic reader. */
    if(records)xQueueSend(records,&record,0);
}
