#pragma once
/* ESP32-C5 ROM exports jd_prepare/jd_decomp with a different JDEC ABI.
 * Namespace the complete pinned LVGL decoder, including its internal API. */
#define jd_prepare background_jd_prepare
#define jd_decomp background_jd_decomp
#define jd_mcu_load background_jd_mcu_load
#define jd_mcu_output background_jd_mcu_output
#define jd_restart background_jd_restart
#include "src/libs/tjpgd/tjpgd.h"
