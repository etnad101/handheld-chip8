#pragma once
#include "memory.h"
#include "stdbool.h"
#include "stdint.h"

#define MEM_SIZE 4096
#define FRAME_SIZE 64 * 32
#define PRGM_START 0x200
#define FONT_SIZE 80
#define FONT_START 0x50

extern const uint8_t FONT[FONT_SIZE];

typedef struct {
    bool frame[FRAME_SIZE];
    bool keys[0x10];
    bool prev_keys_down[0x10];
    uint8_t memory[MEM_SIZE];
    uint16_t pc;
    uint16_t r_i;
    uint16_t sp;
    uint16_t stack[16];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t r_v[16];
} Chip8;

void chip8_init(Chip8* c);
void chip8_load_rom(Chip8* c, uint8_t* rom, uint16_t size);
void chip8_decrement_timers(Chip8* c, bool should_decrement);
bool chip8_tick(Chip8* c);
