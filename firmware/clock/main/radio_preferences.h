#pragma once
#include <stdbool.h>
#include "nvs.h"
esp_err_t radio_preferences_load(nvs_handle_t storage,bool *enabled);
esp_err_t radio_preferences_save(nvs_handle_t storage,bool enabled);
