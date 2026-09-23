#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
bool rtc_decode(const uint8_t regs[7], struct tm *utc);
bool rtc_encode(const struct tm *utc, uint8_t regs[7]);
