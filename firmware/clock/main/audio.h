#pragma once
#include <stdbool.h>
#include "driver/i2c_master.h"
void audio_init(i2c_master_bus_handle_t bus);
bool audio_test(void);
void audio_alarm(bool ringing);

esp_err_t audio_status(void);
