#include "app.h"
#include "../core/chip8.h"
#include "../platform/platform.h"
#include "stdint.h"
#include "stdlib.h"

static Chip8 chip8;

int app_run() {
    // Initialize emulator
    chip8_init(&chip8);

    if (platform_init() < 0) {
        return -1;
    }

    // Load ROM
    uint8_t* rom = NULL;
    // passes test rom 1-4
    // fails 5
    // strange bug in rom 6 with key 7 & 9 beeing pressed together (a & d on keyboard)
    int rom_size = platform_get_rom(&rom, "./roms/6-keypad.ch8");
    if (rom_size < 0) {
        return -1;
    }

    chip8_load_rom(&chip8, rom, rom_size);

    platform_free_rom(rom);

    // Start Emulator
    while (platform_poll_events(chip8.keys)) {
        bool refresh_requested = chip8_tick(&chip8);
        // TODO: change to only degrement @ 60hz
        chip8_decrement_timers(&chip8, true);
        if (refresh_requested) {
            platform_draw(chip8.frame);
        }
    }

    return 1;
}
