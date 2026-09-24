#pragma once
#include <stdbool.h>
#include <time.h>
#include "driver/i2c_master.h"
void clock_init(i2c_master_bus_handle_t bus);
bool clock_valid(void);
const char *clock_source(void);
esp_err_t clock_set(time_t epoch);
esp_err_t clock_set_network(time_t epoch);
esp_err_t clock_rtc_epoch(time_t *epoch);
