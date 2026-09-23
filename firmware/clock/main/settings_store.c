#include "settings_store.h"
#include "nvs.h"
#include "nvs_flash.h"
static nvs_handle_t handle;
static bool opened;
esp_err_t settings_store_open(clock_settings_t *settings)
{
    settings_defaults(settings);
    esp_err_t err=nvs_flash_init_partition("clockcfg");
    if(err!=ESP_OK)return err;
    err=nvs_open_from_partition("clockcfg","clock",NVS_READWRITE,&handle);
    if(err!=ESP_OK)return err;
    opened=true;uint8_t bytes[SETTINGS_SIZE];size_t size=sizeof(bytes);
    err=nvs_get_blob(handle,"settings",bytes,&size);
    if(err==ESP_ERR_NVS_NOT_FOUND)return ESP_OK;
    if(err!=ESP_OK)return err;
    return settings_decode(bytes,size,settings)?ESP_OK:ESP_ERR_INVALID_RESPONSE;
}
esp_err_t settings_store_save(const clock_settings_t *settings)
{
    if(!opened)return ESP_ERR_INVALID_STATE;
    uint8_t bytes[SETTINGS_SIZE];if(!settings_encode(settings,bytes))return ESP_ERR_INVALID_ARG;
    esp_err_t err=nvs_set_blob(handle,"settings",bytes,sizeof(bytes));
    return err==ESP_OK?nvs_commit(handle):err;
}
