#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    bool available,fresh,imu_ready,shake_enabled,auto_rotate,flipped,pending,save_failed;
    float celsius,humidity;
    int acceleration[3];
    int64_t updated_ms;
    unsigned samples,gestures;
} sensor_snapshot_t;
void sensor_service_init(void);
void sensor_service_snapshot(sensor_snapshot_t *out);
bool sensor_service_configure(bool shake,bool rotate);
void sensor_service_diagnostics(void);
