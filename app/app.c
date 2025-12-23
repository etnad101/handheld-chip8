#include "app.h"
#include "stdlib.h"
#include "stdint.h"
#include "../core/chip8.h"
#include "../platform/platform.h"

int app_run() {
    // Initialize emulator
    Chip8 chip8;
    chip8_init(&chip8);

    if (platform_init() < 0) {
        return -1;
    }

    // Load ROM
    uint8_t* rom = NULL;
    int rom_size = platform_get_rom(&rom, "./roms/IBM Logo.ch8");
    if ( rom_size < 0) {
        return -1;
    }

    chip8_load_rom(&chip8, rom, rom_size);

    free(rom);

    // Start Emulator
    while(platform_poll_events()) {
        bool refresh_requested = chip8_tick(&chip8);
        if (refresh_requested) {
            platform_draw(chip8.frame);
        }
    }

    return 1;
}