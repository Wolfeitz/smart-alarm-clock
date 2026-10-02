#pragma once
#include <stdbool.h>
#include <stdint.h>
uint8_t sensor_crc(const uint8_t *data,unsigned size);
bool sensor_environment(const uint8_t raw[6],float *celsius,float *humidity);
void sensor_acceleration(const uint8_t raw[6],int mg[3]);
typedef struct {
    bool started,high;
    unsigned pulses;
    int gravity[3];
    int64_t last,armed,first,pulse,cooldown;
} shake_detector_t;
/* 50 Hz samples in milligravity. Disabled/non-ringing resets the gesture. */
bool sensor_shake(shake_detector_t *s,int64_t now_ms,bool enabled,bool ringing,const int mg[3]);

typedef struct {
    bool started,flipped,candidate;
    int previous[3];
    int64_t last,since;
} orientation_detector_t;
/* QMI +Y is the verified current upright mounting; Z is board-normal. */
bool sensor_orientation(orientation_detector_t *s,int64_t now_ms,bool enabled,const int mg[3]);
void sensor_rotate_touch(bool flipped,int *x,int *y);
