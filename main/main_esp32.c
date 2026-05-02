// #include "../app/app.h"
#include "../app/app.h"
#include "esp_log.h"

static const char* TAG = "chip8";

void app_main(void) {

    ESP_LOGI(TAG, "Starting CHIP-8");
    int status = (int)app_run();
    if (status < 0) {
        ESP_LOGE(TAG, "Emulator returned with code %d\n", status);
    }
    ESP_LOGI(TAG, "Exiting");
}
