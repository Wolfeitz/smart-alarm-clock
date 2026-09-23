#pragma once
#include "alarm_engine.h"
#include <stddef.h>
#define SETTINGS_V1_SIZE (16+16*ALARM_COUNT+4)
#define SETTINGS_SIZE (16+24*ALARM_COUNT+4)
typedef struct { alarm_config_t alarms[ALARM_COUNT];uint8_t brightness;
    uint8_t phase[ALARM_COUNT];
    uint32_t deadline[ALARM_COUNT]; } clock_settings_t;
void settings_defaults(clock_settings_t *settings);
bool settings_encode(const clock_settings_t *settings,uint8_t data[SETTINGS_SIZE]);
bool settings_decode(const uint8_t *data,size_t length,clock_settings_t *settings);
