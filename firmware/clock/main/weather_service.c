#include "weather_service.h"
#include "network_http.h"
#include "ha_service.h"
#include "timezone_rules.h"
#include "diagnostics.h"
#include "clock_service.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_timer.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <math.h>
typedef struct {uint32_t version;char ssid[33],password[64],zip[6];weather_location_t location;} preferences_t;
typedef struct {unsigned kind;char ssid[33],password[64],zip[6];} command_t;
/* Version 1 = manual ZIP (including legacy records); 2 = network estimate. */
static preferences_t prefs;
static weather_snapshot_t state;
static SemaphoreHandle_t lock;
static QueueHandle_t commands,time_updates;
static nvs_handle_t storage;
static bool storage_open,radio_started;
static atomic_bool online,available,radio_paused;
static atomic_int disconnect_reason;
static char active_zone[64]="America/New_York";
/* If lock allocation fails no worker starts, so the fallback snapshot is immutable. */
static void state_lock(void){if(lock)xSemaphoreTake(lock,portMAX_DELAY);}
static void state_unlock(void){if(lock)xSemaphoreGive(lock);}
static void status(const char *message)
{state_lock();snprintf(state.status,sizeof(state.status),"%s",message);state_unlock();}
static void publish(void)
{
    state_lock();state.location=prefs.location;
    state.manual_location=prefs.version==1;
    strcpy(state.ssid,prefs.ssid);strcpy(state.zip,prefs.zip);state.connected=online;
    state.restart_for_zone=prefs.location.timezone[0]&&strcmp(active_zone,prefs.location.timezone)!=0;
    state_unlock();
}
static bool valid_preferences(const preferences_t *p)
{
    return (p->version==1||p->version==2)&&memchr(p->ssid,0,sizeof(p->ssid))&&memchr(p->password,0,sizeof(p->password))&&
        memchr(p->zip,0,sizeof(p->zip))&&(weather_zip_valid(p->zip)||(p->version==2&&!p->zip[0]))&&
        memchr(p->location.name,0,sizeof(p->location.name))&&memchr(p->location.region,0,sizeof(p->location.region))&&
        memchr(p->location.timezone,0,sizeof(p->location.timezone))&&memchr(p->location.zip,0,sizeof(p->location.zip))&&
        (!p->location.timezone[0]||(timezone_rule(p->location.timezone)&&!strcmp(p->zip,p->location.zip)&&
        isfinite(p->location.latitude)&&fabs(p->location.latitude)<=90&&isfinite(p->location.longitude)&&fabs(p->location.longitude)<=180));
}
static esp_err_t save_preferences(const preferences_t *p)
{
    if(!storage_open)return ESP_ERR_INVALID_STATE;
    esp_err_t err=nvs_set_str(storage,"fallback_zone",active_zone);
    if(err==ESP_OK)err=nvs_set_blob(storage,"preferences",p,sizeof(*p));
    return err==ESP_OK?nvs_commit(storage):err;
}
static void wifi_event(void *arg,esp_event_base_t base,int32_t id,void *data)
{
    (void)arg;(void)data;
    if(base==IP_EVENT&&id==IP_EVENT_STA_GOT_IP&&!radio_paused)online=true;
    if(base==WIFI_EVENT&&id==WIFI_EVENT_STA_DISCONNECTED){
        online=false;wifi_event_sta_disconnected_t *event=data;disconnect_reason=event->reason;
        diagnostics_printf("WIFI_DISCONNECTED reason=%u\n",event->reason);
    }
}
static void time_sync(struct timeval *tv)
{time_t value=tv->tv_sec;if(value>=946684800&&value<4102444800LL)xQueueOverwrite(time_updates,&value);}
static esp_err_t radio_init(void)
{
    esp_err_t err=esp_netif_init();if(err!=ESP_OK)return err;
    err=esp_event_loop_create_default();if(err!=ESP_OK)return err;
    /* The convenience factory asserts on allocation/attach failure. Networking
     * is optional, so use the same SDK steps with explicit error handling. */
    esp_netif_config_t netif_config=ESP_NETIF_DEFAULT_WIFI_STA();
    esp_netif_t *station=esp_netif_new(&netif_config);if(!station)return ESP_ERR_NO_MEM;
    err=esp_netif_attach_wifi_station(station);
    if(err==ESP_OK)err=esp_wifi_set_default_wifi_sta_handlers();
    if(err!=ESP_OK){esp_netif_destroy_default_wifi(station);return err;}
    wifi_init_config_t config=WIFI_INIT_CONFIG_DEFAULT();config.nvs_enable=false;
    err=esp_wifi_init(&config);if(err!=ESP_OK)return err;
    err=esp_wifi_set_storage(WIFI_STORAGE_RAM);if(err!=ESP_OK)return err;
    err=esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_event,NULL);if(err!=ESP_OK)return err;
    err=esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,wifi_event,NULL);if(err!=ESP_OK)return err;
    err=esp_wifi_set_mode(WIFI_MODE_STA);if(err!=ESP_OK)return err;
    esp_sntp_config_t ntp=ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");ntp.sync_cb=time_sync;
    err=esp_netif_sntp_init(&ntp);if(err!=ESP_OK)return err;
    err=esp_wifi_start();if(err==ESP_OK)radio_started=true;return err;
}
static esp_err_t connect_wifi(void)
{
    online=false;
    if(radio_started)esp_wifi_disconnect();
    wifi_config_t config={0};memcpy(config.sta.ssid,prefs.ssid,strlen(prefs.ssid));
    memcpy(config.sta.password,prefs.password,strlen(prefs.password));
    config.sta.pmf_cfg.capable=true;
    config.sta.sae_pwe_h2e=WPA3_SAE_PWE_BOTH;
    config.sta.threshold.authmode=prefs.password[0]?WIFI_AUTH_WPA2_PSK:WIFI_AUTH_OPEN;
    esp_err_t err=esp_wifi_set_config(WIFI_IF_STA,&config);memset(&config,0,sizeof(config));
    if(err!=ESP_OK)return err;
    if(!radio_started){err=esp_wifi_start();if(err!=ESP_OK)return err;radio_started=true;}
    return esp_wifi_connect();
}
static bool fetch(const char *url,char *buffer,size_t *size)
{
    int status=network_http_request(url,NULL,NULL,buffer,WEATHER_JSON_LIMIT+1,size);
    diagnostics_printf("WEATHER_HTTP status=%d bytes=%u\n",status,(unsigned)*size);return status==200;
}
static bool update_weather(void)
{
    char *json=malloc(WEATHER_JSON_LIMIT+1);if(!json){status("Weather needs more memory; retrying");return false;}
    char url[768];size_t size;bool ok=false;
    if(weather_should_locate(prefs.version==1,prefs.zip)){
        status("Estimating location from public IP...");char zip[6];
        if(!fetch("https://ipapi.co/json/",json,&size)||!weather_parse_ip_zip(json,size,zip)){
            status("Location estimate unavailable; set ZIP manually");goto done;
        }
        preferences_t next=prefs;strcpy(next.zip,zip);
        if(save_preferences(&next)!=ESP_OK){status("Location estimate could not be saved");goto done;}
        prefs=next;publish();
    }
    if(!prefs.location.timezone[0]){
        status("Finding ZIP location...");
        snprintf(url,sizeof(url),"https://geocoding-api.open-meteo.com/v1/search?name=%s&count=10&language=en&format=json&countryCode=US",prefs.zip);
        weather_location_t loc;
        if(!fetch(url,json,&size)){status("Location service unavailable; retrying");goto done;}
        if(!weather_parse_location(json,size,prefs.zip,&loc)||!timezone_rule(loc.timezone)){
            status("ZIP not uniquely resolved; check location");goto done;
        }
        preferences_t next=prefs;next.location=loc;
        if(save_preferences(&next)!=ESP_OK){status("Location could not be saved");goto done;}
        prefs=next;publish();
    }
    status("Updating weather...");
    snprintf(url,sizeof(url),"https://api.open-meteo.com/v1/forecast?latitude=%.5f&longitude=%.5f&current=temperature_2m,apparent_temperature,weather_code&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max&temperature_unit=fahrenheit&timezone=auto&timeformat=unixtime&forecast_days=1",prefs.location.latitude,prefs.location.longitude);
    weather_data_t data;
    if(!fetch(url,json,&size)){status("Weather unavailable; cached data retained");goto done;}
    if(!weather_parse_forecast(json,size,time(NULL),prefs.location.timezone,&data)){status("Weather response invalid or outdated");goto done;}
    state_lock();state.data=data;state.has_data=true;state_unlock();
    status("Weather updated");ok=true;
 done:free(json);return ok;
}
static void scan_networks(void)
{
    state_lock();state.scanning=true;state_unlock();
    status("Scanning for nearby Wi-Fi...");
    wifi_scan_config_t scan={.show_hidden=false,.scan_type=WIFI_SCAN_TYPE_ACTIVE,.scan_time.active={.min=30,.max=90}};
    esp_err_t err=esp_wifi_scan_start(&scan,true);
    wifi_ap_record_t *records=calloc(32,sizeof(*records));uint16_t count=32;
    if(!records)err=ESP_ERR_NO_MEM;
    if(err==ESP_OK)err=esp_wifi_scan_get_ap_records(&count,records);
    if(err!=ESP_OK)esp_wifi_clear_ap_list();
    state_lock();state.network_count=0;
    if(err==ESP_OK)for(unsigned i=0;i<count&&state.network_count<WEATHER_NETWORK_COUNT;i++){
        char name[33]={0};memcpy(name,records[i].ssid,32);if(!name[0])continue;
        bool duplicate=false;for(unsigned j=0;j<state.network_count;j++)if(!strcmp(name,state.networks[j].ssid))duplicate=true;
        if(duplicate)continue;
        weather_network_t *n=&state.networks[state.network_count++];strcpy(n->ssid,name);n->rssi=records[i].rssi;
        n->secured=records[i].authmode!=WIFI_AUTH_OPEN;
        wifi_auth_mode_t mode=records[i].authmode;
        n->unsupported=mode!=WIFI_AUTH_OPEN&&mode!=WIFI_AUTH_WPA2_PSK&&mode!=WIFI_AUTH_WPA_WPA2_PSK&&mode!=WIFI_AUTH_WPA3_PSK&&mode!=WIFI_AUTH_WPA2_WPA3_PSK;
    }
    state.scanning=false;state.scan_revision++;state_unlock();free(records);
    status(err==ESP_OK?"Select your Wi-Fi network":"Scan unavailable; tap Scan again");
    diagnostics_printf("WIFI_SCAN result=%s count=%u\n",esp_err_to_name(err),err==ESP_OK?count:0);
}
static void worker(void *arg)
{
    (void)arg;esp_err_t radio=radio_init();
    if(radio!=ESP_OK){available=false;state_lock();state.busy=false;state_unlock();ha_service_disable();status("Wi-Fi initialization failed; clock works offline");vTaskDelete(NULL);return;}
    int64_t retry=0,weather_at=0;bool was_online=false;
    if(prefs.ssid[0]){status("Connecting to Wi-Fi...");connect_wifi();retry=esp_timer_get_time()+30000000;}
    else status("Set up Wi-Fi to get local weather");
    for(;;){
        command_t command;
        if(xQueueReceive(commands,&command,pdMS_TO_TICKS(200))==pdTRUE){
            if(command.kind==1||command.kind==2){
                preferences_t next=prefs;
                if(command.kind==2){
                    if(strcmp(next.zip,command.zip))memset(&next.location,0,sizeof(next.location));
                    strcpy(next.zip,command.zip);next.version=command.zip[0]?1:2;
                }else{
                    if(next.version==2&&strcmp(next.ssid,command.ssid)){next.zip[0]=0;memset(&next.location,0,sizeof(next.location));}
                    strcpy(next.password,command.password);strcpy(next.ssid,command.ssid);
                }
                if(save_preferences(&next)!=ESP_OK)status("Save failed; previous settings retained");
                else{
                    bool changed=strcmp(prefs.zip,next.zip)!=0;prefs=next;
                    if(changed){state_lock();state.has_data=false;state_unlock();}
                    if(command.kind==1){radio_paused=false;status("Wi-Fi saved; connecting...");connect_wifi();retry=esp_timer_get_time()+30000000;}
                    else status("Location saved; looking up weather...");
                    weather_at=0;
                }
            }else if(command.kind==5||command.kind==6){
                bool enable=command.kind==6;esp_err_t result=ESP_OK;
                if(enable){
                    if(!radio_started){result=esp_wifi_start();if(result==ESP_OK)radio_started=true;}
                    if(result==ESP_OK){radio_paused=false;if(prefs.ssid[0])result=connect_wifi();retry=esp_timer_get_time()+30000000;}
                    status(result==ESP_OK?"Wi-Fi resumed":"Wi-Fi resume failed");weather_at=0;
                }else{
                    radio_paused=true;
                    if(radio_started)result=esp_wifi_stop();
                    if(result==ESP_OK){radio_started=false;online=false;status("Wi-Fi paused; clock works offline");}
                    else{radio_paused=false;status("Wi-Fi pause failed");}
                }
                diagnostics_printf("NETWORK_RADIO enabled=%u status=%s\n",enable,esp_err_to_name(result));
            }else if(command.kind==3){if(!radio_paused)scan_networks();else status("Wi-Fi paused; resume before scanning");}else weather_at=0;
            memset(&command,0,sizeof(command));state_lock();state.busy=false;state_unlock();publish();
        }
        int64_t now=esp_timer_get_time();int reason=atomic_exchange(&disconnect_reason,0);
        if(reason&&!online&&!radio_paused){char message[96];snprintf(message,sizeof(message),"Wi-Fi connection failed (%d); check password",reason);status(message);}
        if(online!=was_online){was_online=online;publish();if(online){status("Wi-Fi connected");weather_at=0;}}
        if(!radio_paused&&!online&&prefs.ssid[0]&&now>=retry){status("Wi-Fi unavailable; reconnecting...");esp_wifi_connect();retry=now+30000000;}
        ha_service_poll(online);
        if(online&&now>=weather_at){
            if(clock_valid()){bool ok=update_weather();weather_at=esp_timer_get_time()+(ok?1800000000LL:60000000);}
            else{status("Waiting for network time...");weather_at=now+5000000;}
        }
    }
}
void weather_service_init(void)
{
    lock=xSemaphoreCreateMutex();commands=xQueueCreate(1,sizeof(command_t));time_updates=xQueueCreate(1,sizeof(time_t));

    prefs=(preferences_t){.version=1,.zip="27358",.location={.zip="27358",.name="Summerfield",.region="North Carolina",.timezone="America/New_York",.latitude=36.20875,.longitude=-79.90476}};
    esp_err_t err=nvs_flash_init_partition("clockcfg");
    if(err==ESP_OK)err=nvs_open_from_partition("clockcfg","network",NVS_READWRITE,&storage);
    if(err==ESP_OK){storage_open=true;preferences_t saved;size_t size=sizeof(saved);
        if(nvs_get_blob(storage,"preferences",&saved,&size)==ESP_OK&&size==sizeof(saved)&&valid_preferences(&saved))prefs=saved;
    }
    if(prefs.location.timezone[0])strcpy(active_zone,prefs.location.timezone);
    else if(storage_open){char prior[64];size_t length=sizeof(prior);
        if(nvs_get_str(storage,"fallback_zone",prior,&length)==ESP_OK&&timezone_rule(prior))strcpy(active_zone,prior);
    }
    const char *rule=timezone_rule(active_zone);if(rule){setenv("TZ",rule,1);tzset();}
    ha_service_init();publish();status("Starting weather service...");
    if(!lock||!commands||!time_updates){
        status("Network memory unavailable; clock works offline");ha_service_disable();return;
    }
    available=true;
    if(xTaskCreate(worker,"weather",8192,NULL,2,NULL)!=pdPASS){available=false;ha_service_disable();status("Weather task unavailable; clock works offline");}
}
void weather_service_snapshot(weather_snapshot_t *out)
{state_lock();*out=state;out->connected=online;state_unlock();}
static bool submit(command_t *c)
{
    if(!available)return false;
    state_lock();
    if(!available||state.busy){state_unlock();return false;}
    bool ok=xQueueSend(commands,c,0)==pdTRUE;state.busy=ok;
    state_unlock();return ok;
}
bool weather_service_connect(const char *ssid,const char *password)
{
    if(!ssid||!password||strlen(ssid)<1||strlen(ssid)>32||strlen(password)>63)return false;
    if(password[0]&&strlen(password)<8)return false;
    command_t c={.kind=1};strcpy(c.ssid,ssid);strcpy(c.password,password);
    bool ok=submit(&c);memset(&c,0,sizeof(c));return ok;
}
bool weather_service_location(const char *zip)
{
    if(!zip||(*zip&&!weather_zip_valid(zip)))return false;
    command_t c={.kind=2};strcpy(c.zip,zip);return submit(&c);
}
bool weather_service_scan(void){command_t c={.kind=3};return submit(&c);}
bool weather_service_refresh(void){command_t c={.kind=4};return submit(&c);}
bool weather_service_take_time(time_t *epoch){return time_updates&&xQueueReceive(time_updates,epoch,0)==pdTRUE;}
const char *weather_service_timezone(void){return active_zone;}

bool weather_service_radio(bool enabled){command_t c={.kind=enabled?6:5};return submit(&c);}
