#pragma once
#include "settings_codec.h"
#include "esp_err.h"
typedef struct {
    clock_settings_t settings;
    uint8_t ringing,snoozed;
    uint32_t snooze_seconds;
    bool local_sound;
    esp_err_t storage_status;
    unsigned revision;
    uint64_t session; /* Boot-scoped concurrency token; zero until owner publishes. */
    uint32_t save_ticket;
    esp_err_t save_status;
    bool save_conflict;
} alarm_snapshot_t;
typedef struct {
    uint32_t ticket;
    esp_err_t status;
    bool conflict;
    unsigned revision;
    uint64_t session;
} alarm_save_result_t;
/* Last 16 completions this boot; false means pending, unknown or evicted.
 * Never infer failure or retry automatically from an unavailable receipt. */
bool alarm_service_result(uint32_t ticket,alarm_save_result_t *result);
void alarm_service_init(void);
void alarm_service_snapshot(alarm_snapshot_t *snapshot);
bool alarm_service_save(unsigned index,const alarm_config_t *alarm);
uint32_t alarm_service_save_tracked(unsigned index,const alarm_config_t *alarm);
/* Compare inside the owner. A queued ticket is not a persistence acknowledgment. */
uint32_t alarm_service_save_conditional(unsigned index,const alarm_config_t *alarm,
                                        uint64_t expected_session,unsigned expected_revision);
bool alarm_service_brightness(uint8_t brightness);
bool alarm_service_snooze(void);
bool alarm_service_dismiss(void);

uint32_t alarm_service_display(const display_schedule_t *schedule,uint8_t manual_brightness);
