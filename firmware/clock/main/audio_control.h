#pragma once
#include <stdbool.h>
#include <stdint.h>
/* One token identifies an uninterrupted alarm/test playback context. */
uint32_t audio_control_token(void);
bool audio_control_is_alarm(uint32_t token);
bool audio_control_valid(uint32_t token);
void audio_control_alarm(bool active);
