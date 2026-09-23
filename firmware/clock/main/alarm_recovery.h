#pragma once
#include "settings_codec.h"
void alarm_capture(clock_settings_t *settings,const alarm_engine_t *engine,time_t now,uint64_t ms);
void alarm_restore(alarm_engine_t *engine,const clock_settings_t *settings,time_t now,uint64_t ms);
