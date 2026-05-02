#pragma once
#include "stdbool.h"
#include "stdint.h"

int platform_init();
int platform_get_rom(uint8_t** rom, const char* path);
void platform_free_rom(uint8_t* rom);
bool platform_poll_events();
void platform_draw(const bool* buff);
