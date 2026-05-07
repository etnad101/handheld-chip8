#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "platform.h"
#include "portmacro.h"
#include "stdbool.h"
#include "stdint.h"

#define PIN_MOSI 23
#define PIN_CLK 18
#define PIN_CS 5
#define PIN_DC 4
#define PIN_RST 22

// TODO: find what pins actually work input
#define PIN_UP 8
#define PIN_DOWN 9
#define PIN_LEFT 10
#define PIN_RIGHT 11
#define PIN_ACTION_1 12
#define PIN_ACTION_2 13
#define PIN_ACTION_3 14
#define PIN_ACTION_4 15
#define PIN_ACTION_5 16
#define PIN_ACTION_6 17

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define PIXEL_SIZE 5
#define FB_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

static spi_device_handle_t spi;
static uint16_t framebuffer[FB_SIZE];
static uint8_t rom_buffer[3584];
// Index = key array index, value = pin
static uint8_t button_map[0x10] = {PIN_ACTION_2, PIN_ACTION_3, PIN_UP,       PIN_ACTION_4, PIN_LEFT, PIN_ACTION_1,
                                   PIN_RIGHT,    255,          PIN_DOWN,     255,          255,      255,
                                   255,          255,          PIN_ACTION_5, PIN_ACTION_6};

static void send_cmd(uint8_t cmd) {
    gpio_set_level(PIN_DC, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    spi_device_transmit(spi, &t);
}

static void send_data(uint8_t data) {
    gpio_set_level(PIN_DC, 1);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data,
    };
    spi_device_transmit(spi, &t);
}

static esp_err_t init_pins() {
    // Configure RST and DC as outputs
    gpio_reset_pin(PIN_RST);
    gpio_reset_pin(PIN_DC);
    gpio_set_direction(PIN_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_DC, GPIO_MODE_OUTPUT);

    // Configure buttons as inputs
    for (int i = 0; i < 0x10; i++) {
        uint8_t pin = button_map[i];
        if (pin > 0x10) {
            continue;
        }
        ESP_ERROR_CHECK(gpio_reset_pin(pin));
        ESP_ERROR_CHECK(gpio_set_direction(pin, GPIO_MODE_INPUT));
    }
}

static void st7789_init() {
    // Hardware reset
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(120 / portTICK_PERIOD_MS);

    send_cmd(0x11); // Sleep out
    vTaskDelay(120 / portTICK_PERIOD_MS);

    send_cmd(0x36); // Memory access control
    send_data(0x70);

    send_cmd(0x3A);  // Colour format: 16bit RGB565
    send_data(0x05); // 0x05 instead of 0x55

    send_cmd(0xB2); // Porch control
    send_data(0x0C);
    send_data(0x0C);
    send_data(0x00);
    send_data(0x33);
    send_data(0x33);

    send_cmd(0xB7); // Gate control
    send_data(0x35);

    send_cmd(0xBB); // VCOMS setting
    send_data(0x19);

    send_cmd(0xC0); // LCM control
    send_data(0x2C);

    send_cmd(0xC2); // VDV and VRH command enable
    send_data(0x01);

    send_cmd(0xC3); // VRH set
    send_data(0x12);

    send_cmd(0xC4); // VDV set
    send_data(0x20);

    send_cmd(0xC6); // Frame rate (60hz)
    send_data(0x0F);

    send_cmd(0xD0); // Power control
    send_data(0xA4);
    send_data(0xA1);

    send_cmd(0x21); // Display inversion on (needed for Waveshare!)

    send_cmd(0x29); // Display on
    vTaskDelay(20 / portTICK_PERIOD_MS);
}

static void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    send_cmd(0x2A);
    send_data(x0 >> 8);
    send_data(x0 & 0xFF);
    send_data(x1 >> 8);
    send_data(x1 & 0xFF);

    send_cmd(0x2B);
    send_data(y0 >> 8);
    send_data(y0 & 0xFF);
    send_data(y1 >> 8);
    send_data(y1 & 0xFF);

    send_cmd(0x2C);
}

static void fb_flush() {
    set_window(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
    gpio_set_level(PIN_DC, 1);

    spi_transaction_t t = {
        .length = FB_SIZE * 16,
        .tx_buffer = framebuffer,
    };

    spi_device_transmit(spi, &t);
}

void fb_fill(uint16_t colour) {
    for (int i = 0; i < FB_SIZE; i++) {
        framebuffer[i] = colour;
    }
}

void fb_set_pixel(uint16_t x, uint16_t y, uint16_t colour) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
        return;
    framebuffer[y * SCREEN_WIDTH + x] = colour;
}

void fb_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour) {
    for (int i_y = y; i_y < y + h; i_y++) {
        for (int i_x = x; i_x < x + w; i_x++) {
            fb_set_pixel(i_x, i_y, colour);
        }
    }
}

int platform_init() {
    init_pins();

    // SPI bus config
    spi_bus_config_t bus = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = FB_SIZE * 2,
    };
    spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);

    // SPI device config
    spi_device_interface_config_t dev = {
        .clock_speed_hz = 40 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_CS,
        .queue_size = 7,
        .flags = SPI_DEVICE_NO_DUMMY,
    };

    spi_bus_add_device(SPI2_HOST, &dev, &spi);

    st7789_init();

    return 1;
}

int platform_get_rom(uint8_t** rom, const char* path) {
    uint8_t ibm_logo_rom[] = {0x00, 0xE0, 0xA2, 0x2A, 0x60, 0x0C, 0x61, 0x08, 0xD0, 0x1F, 0x70, 0x09, 0xA2, 0x39, 0xD0,
                              0x1F, 0xA2, 0x48, 0x70, 0x08, 0xD0, 0x1F, 0x70, 0x04, 0xA2, 0x57, 0xD0, 0x1F, 0x70, 0x08,
                              0xA2, 0x66, 0xD0, 0x1F, 0x70, 0x08, 0xA2, 0x75, 0xD0, 0x1F, 0x12, 0x28, 0xFF, 0x00, 0xFF,
                              0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF,
                              0x00, 0x38, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x38, 0x00, 0xFF, 0x00, 0xFF, 0x80, 0x00, 0xE0,
                              0x00, 0xE0, 0x00, 0x80, 0x00, 0x80, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0x80, 0xF8, 0x00, 0xFC,
                              0x00, 0x3E, 0x00, 0x3F, 0x00, 0x3B, 0x00, 0x39, 0x00, 0xF8, 0x00, 0xF8, 0x03, 0x00, 0x07,
                              0x00, 0x0F, 0x00, 0xBF, 0x00, 0xFB, 0x00, 0xF3, 0x00, 0xE3, 0x00, 0x43, 0xE0, 0x00, 0xE0,
                              0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xE0, 0x00, 0xE0};
    int rom_size = 132;
    for (int i = 0; i < rom_size; i++) {
        rom_buffer[i] = ibm_logo_rom[i];
    }
    *rom = rom_buffer;
    return rom_size;
}

void platform_free_rom(uint8_t* rom) { (void)rom; }

bool platform_poll_events(bool* keys) {
    for (int i = 0; i < 0x10; i++) {
        int pin = button_map[i];
        if (pin > 0x10) {
            continue;
        }

        int level = gpio_get_level(pin);
        keys[i] = level > 0;
    }
    return true;
};

void platform_draw(const bool* buff) {
    int y_offset = 40; // Offset to be centered
    fb_fill(0xF800);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            int idx = (y * 64) + x;
            uint16_t x_pos = x * PIXEL_SIZE;
            uint16_t y_pos = y * PIXEL_SIZE + y_offset;
            if (buff[idx]) {
                fb_draw_rect(x_pos, y_pos, PIXEL_SIZE, PIXEL_SIZE, 0xFFFF);
            } else {
                fb_draw_rect(x_pos, y_pos, PIXEL_SIZE, PIXEL_SIZE, 0x0000);
            }
        }
    }
    fb_flush();
}
