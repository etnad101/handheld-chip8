#include "app.h"
#include "stdlib.h"
#include "stdint.h"
#include "../core/chip8.h"
#include "../platform/platform.h"

int app_run() {
    // Initialize emulator
    Chip8 chip8;
    chip8_init(&chip8);

    // Load ROM
    uint8_t* rom = NULL;
    int rom_size = platform_get_rom(&rom, "./roms/IBM Logo.ch8");
    if ( rom_size < 0) {
        return rom_size;
    }

    chip8_load_rom(&chip8, rom, rom_size);

    free(rom);

    // Start Emulator
    while(true) {
        bool refresh_requested = chip8_tick(&chip8);
        if (refresh_requested) {
            
        }
    }

    return 1;
}