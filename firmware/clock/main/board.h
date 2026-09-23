#pragma once
#include <stdbool.h>
#include "driver/i2c_master.h"
void board_init(void);
i2c_master_bus_handle_t board_bus(void);
void board_flush(int x1,int y1,int x2,int y2,void *data);
bool board_touch(int *x,int *y);
void board_brightness(bool dim);
