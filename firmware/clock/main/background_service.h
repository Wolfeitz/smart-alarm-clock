#pragma once
#include "background_model.h"
#include "lvgl.h"
/* Init after clockcfg NVS initialization; optional worker stays below alarm/UI. */
void background_service_init(void);
bool background_service_configure(const background_config_t *config);
/* Single UI save owner: receipt confirms persistence, not image download. */
uint32_t background_service_configure_tracked(const background_config_t *config);
bool background_service_save_result(uint32_t ticket,bool *success);
bool background_service_next(void);
void background_service_snapshot(background_config_t *config,char status[96],bool *busy);
/* UI thread only. Pins immutable pixels until the next UI acquisition. */
const lv_image_dsc_t *background_service_image(void);
void background_service_diagnostics(void);
