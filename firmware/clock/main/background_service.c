#include "background_service.h"
#include "background_decode.h"
#include "network_http.h"
#include "weather_service.h"
#include "alarm_service.h"
#include "diagnostics.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
extern const lv_image_dsc_t home_wallpaper;
static SemaphoreHandle_t lock;
static QueueHandle_t changes;
static background_config_t current={.source=BACKGROUND_LOCAL,.count=1,.images={"blue-hour"}};
static char message[96]="Local wallpaper";
static bool busy;
static atomic_bool ready,next_requested;
static lv_image_dsc_t frames[3];
static int published=-1,displayed=-1;
static unsigned revision;
static void status(const char *text){xSemaphoreTake(lock,portMAX_DELAY);snprintf(message,sizeof(message),"%s",text);xSemaphoreGive(lock);}
static void local_image(void){xSemaphoreTake(lock,portMAX_DELAY);published=-1;revision++;xSemaphoreGive(lock);}
void background_service_snapshot(background_config_t *config,char out[96],bool *working)
{
    if(!atomic_load(&ready)){if(config)*config=current;if(out)snprintf(out,96,"Background worker unavailable");if(working)*working=false;return;}
    xSemaphoreTake(lock,portMAX_DELAY);if(config)*config=current;if(out)memcpy(out,message,96);if(working)*working=busy;xSemaphoreGive(lock);
}
const lv_image_dsc_t *background_service_image(void)
{
    if(!atomic_load(&ready))return &home_wallpaper;
    xSemaphoreTake(lock,portMAX_DELAY);displayed=published;
    const lv_image_dsc_t *image=displayed<0?&home_wallpaper:&frames[displayed];
    xSemaphoreGive(lock);return image;
}
bool background_service_configure(const background_config_t *config)
{
    if(!atomic_load(&ready)||!background_config_valid(config))return false;
    if(config->source==BACKGROUND_LOCAL&&(config->count!=1||strcmp(config->images[0],"blue-hour")))return false;
    return xQueueSend(changes,config,0)==pdTRUE;
}
bool background_service_next(void){if(!atomic_load(&ready))return false;atomic_store(&next_requested,true);return true;}
static bool proceed(void *context)
{
    int64_t deadline=*(int64_t *)context;
    if(esp_timer_get_time()>deadline||uxQueueMessagesWaiting(changes))return false;
    taskYIELD();return true;
}
static bool request(const char *url,uint8_t *data,size_t capacity,size_t *size)
{
    int code=network_http_request(url,NULL,NULL,(char *)data,capacity,size);
    diagnostics_printf("BACKGROUND_HTTP status=%d bytes=%u\n",code,(unsigned)*size);
    return code==200&&*size>0;
}
static bool fetch_image(const background_config_t *config,unsigned *cursor,char last_id[7],uint8_t *data,void *scratch)
{
    char url[BACKGROUND_URL_SIZE];size_t size=0;
    if(config->source==BACKGROUND_WALLHAVEN){
        if(!background_search_url(config->query,url,sizeof(url))||!request(url,data,BACKGROUND_JSON_LIMIT+1,&size))return false;
    }else if(!background_selected_url(config->images[(*cursor)++%config->count],url,sizeof(url)))return false;
    else if(strncmp(url,"https://wallhaven.cc/api/v1/w/",strlen("https://wallhaven.cc/api/v1/w/")))goto download;
    else if(!request(url,data,BACKGROUND_JSON_LIMIT+1,&size))return false;
    {
        background_candidate_t candidates[BACKGROUND_MAX_IMAGES];
        unsigned count=background_parse_search((char *)data,size,candidates,BACKGROUND_MAX_IMAGES);
        if(!count)return false;
        unsigned pick=0;while(pick+1<count&&!strcmp(candidates[pick].id,last_id))pick++;
        strcpy(url,candidates[pick].url);strcpy(last_id,candidates[pick].id);
    }
 download:
    if(uxQueueMessagesWaiting(changes)||!request(url,data,BACKGROUND_JPEG_LIMIT+1,&size))return false;
    int target=-1;xSemaphoreTake(lock,portMAX_DELAY);
    for(int i=0;i<3;i++)if(i!=published&&i!=displayed){target=i;break;}
    xSemaphoreGive(lock);if(target<0)return false;
    int64_t deadline=esp_timer_get_time()+10000000;
    if(!background_decode_jpeg(data,size,(uint16_t *)frames[target].data,scratch,8192,proceed,&deadline))return false;
    if(uxQueueMessagesWaiting(changes))return false;
    xSemaphoreTake(lock,portMAX_DELAY);published=target;revision++;unsigned shown=revision;xSemaphoreGive(lock);
    diagnostics_printf("BACKGROUND_READY revision=%u source=%u bytes=%u id=%s\n",shown,config->source,(unsigned)size,last_id);return true;
}
static void worker(void *arg)
{
    (void)arg;uint8_t *data=heap_caps_malloc(BACKGROUND_JPEG_LIMIT+1,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    void *scratch=heap_caps_malloc(8192,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    bool memory=data&&scratch;
    for(int i=0;i<3;i++){
        uint8_t *pixels=heap_caps_malloc(BACKGROUND_PIXELS*2,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
        frames[i]=(lv_image_dsc_t){.header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_RGB565,.w=480,.h=320,.stride=960},.data_size=BACKGROUND_PIXELS*2,.data=pixels};
        if(!pixels)memory=false;
    }
    nvs_handle_t storage;bool saved=nvs_open_from_partition("clockcfg","background",NVS_READWRITE,&storage)==ESP_OK;
    background_config_t config=current,incoming;unsigned cursor=0;char last_id[7]={0};
    if(saved){size_t size=sizeof(incoming);uint32_t version=0;
        if(nvs_get_u32(storage,"version",&version)==ESP_OK&&version==1&&nvs_get_blob(storage,"config",&incoming,&size)==ESP_OK&&size==sizeof(incoming)&&background_config_valid(&incoming)&&
           (incoming.source!=BACKGROUND_LOCAL||(incoming.count==1&&!strcmp(incoming.images[0],"blue-hour"))))config=incoming;
    }
    xSemaphoreTake(lock,portMAX_DELAY);current=config;xSemaphoreGive(lock);
    int64_t due=0;bool waiting_for_network=false;
    for(;;){
        if(xQueueReceive(changes,&incoming,pdMS_TO_TICKS(250))==pdTRUE){
            esp_err_t err=saved?nvs_set_blob(storage,"config",&incoming,sizeof(incoming)):ESP_ERR_INVALID_STATE;
            if(err==ESP_OK)err=nvs_set_u32(storage,"version",1);
            if(err==ESP_OK)err=nvs_commit(storage);
            if(err==ESP_OK){config=incoming;cursor=0;last_id[0]=0;due=0;
                xSemaphoreTake(lock,portMAX_DELAY);current=config;xSemaphoreGive(lock);
                status("Background settings saved");if(config.source==BACKGROUND_LOCAL)local_image();
            }else status("Save failed; previous background retained");
        }
        if(atomic_exchange(&next_requested,false))due=0;
        if(config.source==BACKGROUND_LOCAL)continue;
        if(!memory){status("Image memory unavailable; local background retained");continue;}
        weather_snapshot_t network;weather_service_snapshot(&network);
        if(waiting_for_network&&network.connected){due=0;waiting_for_network=false;}
        if(esp_timer_get_time()<due)continue;
        if(!network.connected){status("Wi-Fi offline; current background retained");waiting_for_network=true;due=esp_timer_get_time()+30000000;continue;}
        alarm_snapshot_t alarm;alarm_service_snapshot(&alarm);if(alarm.ringing||alarm.snoozed)continue;
        xSemaphoreTake(lock,portMAX_DELAY);busy=true;xSemaphoreGive(lock);status("Loading background...");
        bool ok=fetch_image(&config,&cursor,last_id,data,scratch);
        status(ok?"Background updated":"Image unavailable; current background retained");
        xSemaphoreTake(lock,portMAX_DELAY);busy=false;xSemaphoreGive(lock);
        due=ok?(config.interval_seconds?esp_timer_get_time()+(int64_t)config.interval_seconds*1000000:INT64_MAX):esp_timer_get_time()+60000000;
    }
}
void background_service_init(void)
{
    lock=xSemaphoreCreateMutex();changes=xQueueCreate(1,sizeof(background_config_t));
    if(!lock||!changes)return;
    atomic_store(&ready,true);
    if(xTaskCreate(worker,"background",16384,NULL,1,NULL)!=pdPASS)atomic_store(&ready,false);
}
void background_service_diagnostics(void)
{
    if(!atomic_load(&ready)){diagnostics_printf("BACKGROUND_STATE ready=0\n");return;}
    xSemaphoreTake(lock,portMAX_DELAY);
    diagnostics_printf("BACKGROUND_STATE ready=1 source=%u revision=%u busy=%u image=%d displayed=%d internal_free=%u status=%s\n",current.source,revision,busy,published,displayed,(unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),message);
    xSemaphoreGive(lock);
}
