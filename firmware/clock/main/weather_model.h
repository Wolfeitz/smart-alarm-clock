#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define WEATHER_JSON_LIMIT 12288
/* Provider-neutral values: Fahrenheit, UTC seconds, WMO condition codes. */
typedef struct {
    char zip[6], name[64], region[64], timezone[64];
    double latitude, longitude;
} weather_location_t;
typedef struct {
    int64_t observed_at, fetched_at, day_start;
    double temperature, feels_like, high, low;
    int code, day_code, rain_percent;
} weather_data_t;
bool weather_zip_valid(const char *zip);
bool weather_parse_location(const char *json,size_t size,const char *zip,weather_location_t *out);
bool weather_parse_forecast(const char *json,size_t size,int64_t now,const char *timezone,weather_data_t *out);
bool weather_fresh(const weather_data_t *data,int64_t now);
const char *weather_condition(int code);
/* Network inference is permitted only for an unset, non-manual location. */
bool weather_should_locate(bool manual,const char *zip);
bool weather_parse_ip_zip(const char *json,size_t size,char zip[6]);
