#pragma once
#include "stdint.h"
#include "stdbool.h"

int platform_init();
int platform_get_rom(uint8_t** rom, const char* path);
bool platform_poll_events();
void platform_draw(const bool* buff);