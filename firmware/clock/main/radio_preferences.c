#include "radio_preferences.h"
#include <stdint.h>
esp_err_t radio_preferences_load(nvs_handle_t storage,bool *enabled)
{
    uint8_t value=0;size_t size=sizeof(value);*enabled=false;
    esp_err_t err=nvs_get_blob(storage,"radio_enabled",&value,&size);
    if(err==ESP_ERR_NVS_NOT_FOUND){*enabled=true;return ESP_OK;}
    if(err!=ESP_OK)return err;
    if(size!=sizeof(value)||value>1)return ESP_ERR_INVALID_RESPONSE;
    *enabled=value!=0;return ESP_OK;
}
esp_err_t radio_preferences_save(nvs_handle_t storage,bool enabled)
{
    uint8_t value=enabled?1:0;
    esp_err_t err=nvs_set_blob(storage,"radio_enabled",&value,sizeof(value));
    return err==ESP_OK?nvs_commit(storage):err;
}
