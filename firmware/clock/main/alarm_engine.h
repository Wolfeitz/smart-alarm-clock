#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#define ALARM_COUNT 8
#define ALARM_SNOOZE_MS (5ULL*60*1000)
#define ALARM_RING_LIMIT_MS (10ULL*60*1000)
typedef struct {
    bool enabled;
    uint8_t hour,minute,weekdays; /* bit0 Sunday; zero means once on once_date */
    uint32_t once_date;          /* YYYYMMDD */
    uint32_t consumed_date;      /* highest local date already triggered */
} alarm_config_t;
typedef enum {ALARM_IDLE,ALARM_RINGING,ALARM_SNOOZED} alarm_phase_t;
typedef struct {
    alarm_phase_t phase;
    uint64_t deadline_ms;
} alarm_runtime_t;
typedef struct {
    alarm_config_t alarms[ALARM_COUNT];
    alarm_runtime_t runtime[ALARM_COUNT];
} alarm_engine_t;
bool alarm_config_valid(const alarm_config_t *alarm);
uint32_t alarm_date(const struct tm *local);
/* Returns a bitmask of occurrences consumed: persist before starting sound. */
uint8_t alarm_tick(alarm_engine_t *engine,time_t epoch,bool valid,uint64_t mono_ms);
uint8_t alarm_ringing(const alarm_engine_t *engine);
void alarm_snooze(alarm_engine_t *engine,uint64_t mono_ms);
void alarm_dismiss(alarm_engine_t *engine);
void alarm_cancel(alarm_engine_t *engine,unsigned index);
