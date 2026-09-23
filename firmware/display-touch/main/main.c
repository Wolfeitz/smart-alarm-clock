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
static uint16_t *pixels;
static i2c_master_dev_handle_t touch;
static bool transfer_done(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *e, void *ctx)
{
    BaseType_t wake = pdFALSE;
    xSemaphoreGiveFromISR(done, &wake);
    return wake == pdTRUE;
}
static void rect(int x, int y, int w, int h, uint16_t color)
{
    if(x<0){w+=x;x=0;} if(y<0){h+=y;y=0;}
    if(x+w>W)w=W-x;
    if(y+h>H)h=H-y;
    if(w<=0 || h<=0)return;
    uint16_t wire=(color>>8)|(color<<8);
    for(int i=0;i<w*8;i++)pixels[i]=wire;
    for(int row=0;row<h;row+=8){
        int rows=h-row<8?h-row:8;
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel,x,y+row,x+w,y+row+rows,pixels));
        if(xSemaphoreTake(done,pdMS_TO_TICKS(1000))!=pdTRUE)abort();
    }
}
static const char alphabet[]=" ABCDEHILORSTUW";
static const uint8_t glyphs[][5]={
 {0,0,0,0,0},{126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},
 {127,65,65,34,28},{127,73,73,73,65},{127,8,8,8,127},{0,65,127,65,0},
 {127,64,64,64,64},{62,65,65,65,62},{127,9,25,41,70},{70,73,73,73,49},
 {1,1,127,1,1},{63,64,64,64,63},{63,64,56,64,63}};
static void text(int x,int y,const char *s,int scale)
{
    for(;*s;s++,x+=6*scale){const char *g=strchr(alphabet,*s);if(!g)continue;
        for(int c=0;c<5;c++)for(int r=0;r<7;r++)
            if(glyphs[g-alphabet][c]&(1<<r))rect(x+c*scale,y+r*scale,scale,scale,0xffff);
    }
}
static void screen(void)
{
    rect(0,0,W,H,0x1082);rect(0,0,W,48,0x001f);
    text(12,12,"TOUCH TEST",3);rect(350,5,120,38,0xf800);text(365,16,"CLEAR",2);
    text(140,155,"DRAW HERE",3);
    const int xs[]={10,450,10,450}; const int ys[]={60,60,290,290};
    for(int i=0;i<4;i++){rect(xs[i],ys[i],20,20,0x07e0);}
}
void app_main(void)
{
    i2c_master_bus_handle_t bus;
    i2c_master_bus_config_t bc={.i2c_port=I2C_NUM_0,.sda_io_num=27,.scl_io_num=26,
        .clk_source=I2C_CLK_SRC_DEFAULT,.glitch_ignore_cnt=7,.flags.enable_internal_pullup=true};
    ESP_ERROR_CHECK(i2c_new_master_bus(&bc,&bus));
    i2c_device_config_t dc={.dev_addr_length=I2C_ADDR_BIT_LEN_7,.device_address=0x38,.scl_speed_hz=100000};
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus,&dc,&touch));
    dc.device_address=0x24;i2c_master_dev_handle_t expander;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus,&dc,&expander));
    spi_bus_config_t spi={.mosi_io_num=7,.miso_io_num=2,.sclk_io_num=6,
        .quadwp_io_num=-1,.quadhd_io_num=-1,.max_transfer_sz=W*8*2};
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST,&spi,SPI_DMA_CH_AUTO));
    done=xSemaphoreCreateBinary();pixels=heap_caps_malloc(W*8*2,MALLOC_CAP_DMA);
    if(!done||!pixels)abort();
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
    screen();printf("DISPLAY_READY idf=%s size=480x320 SPI=20MHz\n",esp_get_idf_version());
    unsigned frames=0,events=0,errors=0;bool pressed=false;
    for(;;){
        uint8_t reg=2,data[7];esp_err_t err=i2c_master_transmit_receive(touch,&reg,1,data,sizeof(data),50);
        if(err==ESP_OK && (data[0]&15)>0 && (data[0]&15)<=2){
            unsigned x=((data[1]&15)<<8)|data[2],y=((data[3]&15)<<8)|data[4];
            if(!pressed || frames%5==0)printf("TOUCH raw_x=%u raw_y=%u event=%u\n",x,y,data[1]>>6);
            unsigned raw_x=x, raw_y=y;
            x=raw_y<480 ? 479-raw_y : 480; y=raw_x;
            if(x<W && y<H){
                if(y<48 && x>350){if(!pressed)screen();}
                else rect((int)x-4,(int)y-4,9,9,0xffe0);
            }
            events++;pressed=true;
        }else{if(err!=ESP_OK)errors++;pressed=false;}
        if(frames%25==0){rect(280,15,20,20,(frames/25)%2?0x07e0:0xffff);}
        if(frames%250==0)printf("DISPLAY_ALIVE frames=%u touch_samples=%u errors=%u\n",frames,events,errors);
        frames++;vTaskDelay(pdMS_TO_TICKS(20));
    }
}
