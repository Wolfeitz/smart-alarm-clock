#include "radio_preferences.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
static int read_error=ESP_ERR_NVS_NOT_FOUND,write_error,commit_error;
static uint8_t stored;static size_t stored_size=1;static unsigned commits;
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *size)
{assert(h==7&&!strcmp(key,"radio_enabled"));if(read_error)return read_error;*(uint8_t*)out=stored;*size=stored_size;return ESP_OK;}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *in,size_t size)
{assert(h==7&&!strcmp(key,"radio_enabled")&&size==1);if(write_error)return write_error;stored=*(const uint8_t*)in;return ESP_OK;}
esp_err_t nvs_commit(nvs_handle_t h){assert(h==7);commits++;return commit_error;}
int main(void){bool enabled=false;
assert(radio_preferences_load(7,&enabled)==ESP_OK&&enabled);
assert(radio_preferences_save(7,false)==ESP_OK);read_error=0;
assert(radio_preferences_load(7,&enabled)==ESP_OK&&!enabled);
assert(radio_preferences_save(7,true)==ESP_OK);
assert(radio_preferences_load(7,&enabled)==ESP_OK&&enabled);
stored=2;assert(radio_preferences_load(7,&enabled)==ESP_ERR_INVALID_RESPONSE&&!enabled);
stored=1;stored_size=0;assert(radio_preferences_load(7,&enabled)==ESP_ERR_INVALID_RESPONSE&&!enabled);
read_error=91;assert(radio_preferences_load(7,&enabled)==91&&!enabled);
write_error=92;unsigned before=commits;assert(radio_preferences_save(7,false)==92&&commits==before);
write_error=0;commit_error=93;assert(radio_preferences_save(7,false)==93);
puts("PASS radio preference: legacy enabled, off/on reload, corruption/read errors fail closed, write/commit failures");}
