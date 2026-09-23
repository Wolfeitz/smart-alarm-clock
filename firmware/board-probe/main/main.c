#include <stdio.h>
#include "esp_system.h"
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    i2c_master_bus_handle_t bus;
    const i2c_master_bus_config_t cfg = {
        .i2c_port = I2C_NUM_0, .sda_io_num = GPIO_NUM_27,
        .scl_io_num = GPIO_NUM_26, .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7, .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &bus));
    for (unsigned round = 0; ; ++round) {
        printf("BOARD_PROBE idf=%s SDA=27 SCL=26 round=%u\n", esp_get_idf_version(), round);
        unsigned found = 0, errors = 0;
        for (unsigned addr = 8; addr < 0x78; ++addr) {
            esp_err_t err = i2c_master_probe(bus, addr, 30);
            if (err == ESP_OK) { printf("I2C_ACK address=0x%02x\n", addr); ++found; }
            else if (err != ESP_ERR_NOT_FOUND) {
                printf("I2C_ERROR address=0x%02x status=%s\n", addr, esp_err_to_name(err));
                ++errors;
            }
        }
        printf("PROBE_COMPLETE found=%u errors=%u\n", found, errors);
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
