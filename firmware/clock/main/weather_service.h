#pragma once
#include "weather_model.h"
#include <time.h>
#define WEATHER_NETWORK_COUNT 16
typedef struct {char ssid[33];int rssi;bool secured,unsupported;} weather_network_t;
typedef struct {
    weather_location_t location;
    weather_data_t data;
    char ssid[33],zip[6],status[96];
    bool connected,has_data,restart_for_zone,busy,scanning,manual_location;
    unsigned scan_revision,network_count;
    weather_network_t networks[WEATHER_NETWORK_COUNT];
} weather_snapshot_t;
/* Init before alarm/UI tasks: loads timezone once; no runtime TZ mutation. */
void weather_service_init(void);
void weather_service_snapshot(weather_snapshot_t *out);
bool weather_service_connect(const char *ssid,const char *password);
bool weather_service_location(const char *zip);
bool weather_service_scan(void);
bool weather_service_refresh(void);
bool weather_service_take_time(time_t *epoch);
const char *weather_service_timezone(void);
