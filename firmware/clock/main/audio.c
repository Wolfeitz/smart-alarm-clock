#include "audio.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "driver/i2s_std.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
static i2s_chan_handle_t tx;
static QueueHandle_t requests;
static void play_task(void *unused)
{
    (void)unused;uint8_t command;int16_t pcm[256*2];
    for(;;){
        xQueueReceive(requests,&command,portMAX_DELAY);
        esp_err_t result=ESP_OK;size_t total=0;
        /* Four quiet pulses, with a 10ms envelope to avoid edge clicks. */
        for(unsigned frame=0;frame<22050*2;frame+=256){
            for(unsigned i=0;i<256;i++){
                unsigned n=frame+i,pos=n%11025;
                float envelope=pos<220?pos/220.0f:pos<5292?1.0f:pos<5512?(5512-pos)/220.0f:0;
                int16_t sample=(int16_t)(5000.0f*envelope*sinf(2.0f*3.14159265f*660.0f*n/22050.0f));
                pcm[2*i]=sample;pcm[2*i+1]=sample;
            }
            size_t written=0;result=i2s_channel_write(tx,pcm,sizeof(pcm),&written,1000);
            total+=written;if(result!=ESP_OK || written!=sizeof(pcm)){result=ESP_FAIL;break;}
        }
        memset(pcm,0,sizeof(pcm));size_t written=0;
        for(unsigned i=0;i<8;i++){
            esp_err_t err=i2s_channel_write(tx,pcm,sizeof(pcm),&written,1000);
            if(err!=ESP_OK){result=err;break;}
        }
        printf("AUDIO_TEST_DONE status=%s bytes=%u\n",esp_err_to_name(result),(unsigned)total);
    }
}
void audio_init(i2c_master_bus_handle_t bus)
{
    i2s_chan_config_t channel=I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0,I2S_ROLE_MASTER);
    channel.auto_clear=true;
    ESP_ERROR_CHECK(i2s_new_channel(&channel,&tx,NULL));
    i2s_std_config_t config={
        .clk_cfg=I2S_STD_CLK_DEFAULT_CONFIG(22050),
        .slot_cfg=I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,I2S_SLOT_MODE_STEREO),
        .gpio_cfg={.mclk=I2S_GPIO_UNUSED,.bclk=23,.ws=10,.dout=25,.din=I2S_GPIO_UNUSED},
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx,&config));
    ESP_ERROR_CHECK(i2s_channel_enable(tx));
    audio_codec_i2c_cfg_t control_cfg={.port=0,.addr=ES8311_CODEC_DEFAULT_ADDR,.bus_handle=bus};
    const audio_codec_ctrl_if_t *control=audio_codec_new_i2c_ctrl(&control_cfg);
    const audio_codec_gpio_if_t *gpio=audio_codec_new_gpio();
    if(!control || !gpio){printf("AUDIO_INIT failed=control\n");return;}
    es8311_codec_cfg_t codec_cfg={.ctrl_if=control,.gpio_if=gpio,.codec_mode=ESP_CODEC_DEV_WORK_MODE_DAC,
        .pa_pin=-1,.use_mclk=false,.hw_gain={.pa_voltage=5.0,.codec_dac_voltage=3.3}};
    const audio_codec_if_t *codec=es8311_codec_new(&codec_cfg);
    audio_codec_i2s_cfg_t data_cfg={.port=0,.tx_handle=tx};
    const audio_codec_data_if_t *data=audio_codec_new_i2s_data(&data_cfg);
    if(!control || !gpio || !codec || !data){printf("AUDIO_INIT failed=interfaces\n");return;}
    esp_codec_dev_cfg_t device_cfg={.dev_type=ESP_CODEC_DEV_TYPE_OUT,.codec_if=codec,.data_if=data};
    esp_codec_dev_handle_t device=esp_codec_dev_new(&device_cfg);
    if(!device){printf("AUDIO_INIT failed=device\n");return;}
    esp_codec_dev_sample_info_t sample={.sample_rate=22050,.channel=2,.bits_per_sample=16};
    int rc=esp_codec_dev_open(device,&sample);
    if(rc==0)rc=esp_codec_dev_set_out_vol(device,55);
    if(rc!=0){printf("AUDIO_INIT failed=codec rc=%d\n",rc);return;}
    const uint8_t regs[]={0x00,0x01,0x09,0x12,0x31,0x32,0xfd,0xfe,0xff};
    for(unsigned i=0;i<sizeof(regs);i++){
        uint8_t value=0;int status=control->read_reg(control,regs[i],1,&value,1);
        printf("AUDIO_REG reg=%02x value=%02x status=%d\n",regs[i],value,status);
    }
    requests=xQueueCreate(1,sizeof(uint8_t));
    if(!requests || xTaskCreate(play_task,"local_audio",4096,NULL,4,NULL)!=pdPASS)abort();
    printf("AUDIO_READY rate=22050 bits=16 codec=ES8311 volume=55\n");
}
bool audio_test(void)
{
    uint8_t command=1;return requests && xQueueSend(requests,&command,0)==pdTRUE;
}
