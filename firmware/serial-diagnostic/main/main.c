#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

void app_main(void)
{
    esp_chip_info_t chip;
    uint32_t flash_bytes = 0;
    esp_chip_info(&chip);
    ESP_ERROR_CHECK(esp_flash_get_size(NULL, &flash_bytes));
    for (unsigned tick = 0; ; ++tick) {
        printf("ESP_LINK_DIAGNOSTIC_OK idf=%s target=%s revision=%u flash_bytes=%" PRIu32 " tick=%u\n",
               esp_get_idf_version(), CONFIG_IDF_TARGET, chip.revision, flash_bytes, tick);
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
