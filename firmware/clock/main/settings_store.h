#pragma once
#include "esp_err.h"
#include "settings_codec.h"
/* Never erases a partition on error. Missing key yields disabled defaults. */
esp_err_t settings_store_open(clock_settings_t *settings);
esp_err_t settings_store_save(const clock_settings_t *settings);
