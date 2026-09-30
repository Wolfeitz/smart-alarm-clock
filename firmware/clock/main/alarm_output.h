#pragma once
#include <stdbool.h>
#include <stdint.h>
/* Alarm task owns local(); network task owns enable()/confirm(). No blocking. */
void alarm_output_enable(bool enabled);
bool alarm_output_local(uint8_t ringing,uint32_t mono_ms);
uint32_t alarm_output_session(void);
void alarm_output_confirm(uint32_t session,uint32_t mono_ms);
