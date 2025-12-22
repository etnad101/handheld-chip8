#include "chip8.h"

const uint8_t FONT[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_init(Chip8 *c) {
    memset(c, 0, sizeof *c);
    c->pc = PRGM_START;
    
    for (int i = 0; i < FONT_SIZE; i++) {
        c->memory[FONT_START + i] = FONT[i];
    }
}

void chip8_load_rom(Chip8 *c, uint8_t* rom, uint16_t size) {
    for (int i = 0; i < size; i++) {
        c->memory[PRGM_START + i] = rom[i];
    }
}

void chip8_draw(Chip8* c, uint8_t x, uint8_t y, uint8_t n) {
    c->r_v[0xF] = 0; // reset collision flag

    for (uint8_t row = 0; row < n; row++) {
        uint8_t sprite_byte = c->memory[c->r_i + row];

        for (uint8_t col = 0; col < 8; col++) {
            uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 1;
            if (sprite_pixel == 0) continue;

            // Wrap around screen (64x32)
            uint8_t px = (x + col) % 64;
            uint8_t py = (y + row) % 32;
            uint16_t index = py * 64 + px;

            // Check for collision
            if (c->frame[index]) {
                c->r_v[0xF] = 1;
            }

            // XOR pixel
            c->frame[index] ^= 1;
        }
    }
}

bool chip8_tick(Chip8 *c) {
    uint8_t hi = c->memory[c->pc];
    c->pc++;
    uint8_t lo = c->memory[c->pc];
    c->pc++;

    uint16_t instr = ((uint16_t)hi << 8) | (uint16_t)lo;

    bool frame_updated = false;
    switch (instr & 0xF000) {
        case 0x0000:
            switch (instr & 0x00FF) {
                case 0x00E0:
                    memset(c->frame, 0, sizeof(c->frame));
                    break;
            }
            break;
        case 0x1000:
            c->pc = instr & 0x0FFF;
            break;
        case 0x6000:
            c->r_v[(instr & 0x0F00) >> 8] = instr & 0x00FF;
            break;
        case 0x7000:
            c->r_v[(instr & 0x0F00) >> 8] += instr & 0x00FF;
            break;
        case 0xA000:
            c->r_i = instr & 0x0FFF;
            break;
        case 0xD000: {
            uint8_t vx = (instr & 0x0F00) >> 8;
            uint8_t vy = (instr & 0x00F0) >> 4;
            uint8_t n = instr & 0x000F;

            chip8_draw(c, c->r_v[vx], c->r_v[vy], n);
            frame_updated = true; 
            break;
        }
    }

    return frame_updated;
}