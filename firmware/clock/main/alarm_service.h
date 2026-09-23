#pragma once
#include "settings_codec.h"
#include "esp_err.h"
typedef struct {
    clock_settings_t settings;
    uint8_t ringing,snoozed;
    esp_err_t storage_status;
    unsigned revision;
} alarm_snapshot_t;
void alarm_service_init(void);
void alarm_service_snapshot(alarm_snapshot_t *snapshot);
bool alarm_service_save(unsigned index,const alarm_config_t *alarm);
bool alarm_service_brightness(uint8_t brightness);
bool alarm_service_snooze(void);
bool alarm_service_dismiss(void);
