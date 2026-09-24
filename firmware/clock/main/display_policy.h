#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {bool enabled;uint16_t start_minute,end_minute;} display_schedule_t;
bool display_schedule_valid(const display_schedule_t *schedule);
bool display_night(const display_schedule_t *schedule,bool valid_time,unsigned minute);
bool display_should_dim(const display_schedule_t *schedule,bool valid_time,unsigned minute,
                        uint8_t manual_brightness,bool ringing,uint64_t now_ms,uint64_t wake_until_ms);
