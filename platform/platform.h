#pragma once
#include "stdbool.h"
#include "stdint.h"

#define BUTTON_UP 0x2
#define BUTTON_DOWN 0x8
#define BUTTON_LEFT 0x4
#define BUTTON_RIGHT 0x6
#define BUTTON_ACTION_1 0x5
#define BUTTON_ACTION_2 0x0
#define BUTTON_ACTION_3 0x1
#define BUTTON_ACTION_4 0x3
#define BUTTON_ACTION_5 0xE
#define BUTTON_ACTION_6 0xF

int platform_init();
int platform_get_rom(uint8_t** rom, const char* path);
void platform_free_rom(uint8_t* rom);
bool platform_poll_events(bool* keys);
void platform_draw(const bool* buff);
