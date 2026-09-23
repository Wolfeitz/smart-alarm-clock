#include "board.h"
#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7796.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define W 480
#define H 320
static esp_lcd_panel_handle_t panel;
static SemaphoreHandle_t done;
static i2c_master_bus_handle_t bus;
static i2c_master_dev_handle_t expander;
static i2c_master_dev_handle_t touch;
static bool transfer_done(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *e, void *ctx)
{
    BaseType_t wake = pdFALSE;
    xSemaphoreGiveFromISR(done, &wake);
    return wake == pdTRUE;
}

void board_init(void)
{
    i2c_master_bus_config_t bc={.i2c_port=I2C_NUM_0,.sda_io_num=27,.scl_io_num=26,
        .clk_source=I2C_CLK_SRC_DEFAULT,.glitch_ignore_cnt=7,.flags.enable_internal_pullup=true};
    ESP_ERROR_CHECK(i2c_new_master_bus(&bc,&bus));
    i2c_device_config_t dc={.dev_addr_length=I2C_ADDR_BIT_LEN_7,.device_address=0x38,.scl_speed_hz=100000};
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus,&dc,&touch));
    dc.device_address=0x24;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus,&dc,&expander));
    /* Exact board factory expander sequence; see docs/hardware/RECOVERED-MAP.md.
     * Command 2: direction (1=output); command 3: output latch.
     * Keep factory bit 6 as input. Net labels are not yet schematic-verified. */
    const uint8_t startup[][2]={{2,0xff},{3,0x00},{2,0xbf},{3,0x03},{3,0x00},{3,0x23}};
    for (unsigned i=0;i<sizeof(startup)/sizeof(startup[0]);i++) {
        ESP_ERROR_CHECK(i2c_master_transmit(expander,startup[i],2,100));
        if(i==3 || i==4) vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    printf("BOARD_RESET_DONE touch_probe=%s\n",esp_err_to_name(i2c_master_probe(bus,0x38,100)));
    spi_bus_config_t spi={.mosi_io_num=7,.miso_io_num=2,.sclk_io_num=6,
        .quadwp_io_num=-1,.quadhd_io_num=-1,.max_transfer_sz=W*20*2};
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST,&spi,SPI_DMA_CH_AUTO));
    done=xSemaphoreCreateBinary();
    if(!done)abort();
    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_io_spi_config_t ic={.cs_gpio_num=8,.dc_gpio_num=5,.spi_mode=0,
        .pclk_hz=20000000,.trans_queue_depth=1,.lcd_cmd_bits=8,.lcd_param_bits=8,
        .on_color_trans_done=transfer_done};
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST,&ic,&io));
    esp_lcd_panel_dev_config_t pc={.reset_gpio_num=-1,.rgb_ele_order=LCD_RGB_ELEMENT_ORDER_BGR,.bits_per_pixel=16};
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io,&pc,&panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel,true));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel,true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel,true,true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel,true));
    const uint8_t brightness[]={5,160};
    ESP_ERROR_CHECK(i2c_master_transmit(expander,brightness,sizeof(brightness),100));
}

i2c_master_bus_handle_t board_bus(void) { return bus; }
void board_flush(int x1,int y1,int x2,int y2,void *data)
{
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel,x1,y1,x2,y2,data));
    if(xSemaphoreTake(done,pdMS_TO_TICKS(1000))!=pdTRUE)abort();
}
bool board_touch(int *x,int *y)
{
    uint8_t reg=2,data[7];
    if(i2c_master_transmit_receive(touch,&reg,1,data,sizeof(data),50)!=ESP_OK)return false;
    if((data[0]&15)==0 || (data[0]&15)>2)return false;
    int rx=((data[1]&15)<<8)|data[2],ry=((data[3]&15)<<8)|data[4];
    if(rx>=320 || ry>=480)return false;
    *x=479-ry;*y=rx;return true;
}
void board_brightness(bool dim)
{
    uint8_t data[]={5,dim?25:160};
    ESP_ERROR_CHECK(i2c_master_transmit(expander,data,sizeof(data),100));
}
