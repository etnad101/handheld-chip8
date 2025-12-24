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

    uint16_t opcode = ((uint16_t)hi << 8) | (uint16_t)lo;

    bool frame_updated = false;
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0:
                    memset(c->frame, 0, sizeof(c->frame));
                    break;
                case 0x00EE: {
                    c->pc = c->stack[--c->sp];
                    break;
                }
            }
            break;
        case 0x1000:
            c->pc = opcode & 0x0FFF;
            break;
        case 0x2000: {
            c->stack[c->sp++] = c->pc;
            c->pc = opcode & 0x0FFF;
            break;
        }
        case 0x3000:
            if (c->r_v[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                c->pc += 2;
            break;
        case 0x4000:
            if (c->r_v[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                c->pc += 2;
            break;
        case 0x5000:
            if (c->r_v[(opcode & 0x0F00) >> 8] == c->r_v[(opcode & 0x00F0) >> 4])
                c->pc += 2;
            break;
        case 0x6000:
            c->r_v[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;
        case 0x7000:
            c->r_v[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;
        case 0x8000: {
            uint8_t vx = c->r_v[(opcode & 0x0F00) >> 8];
            uint8_t vy = c->r_v[(opcode & 0x00F0) >> 4];
            switch (opcode & 0x000F) {
                case 0x0:
                    c->r_v[(opcode & 0x0F00) >> 8] = c->r_v[(opcode & 0x00F0) >> 4];
                    break;
                case 0x1:
                    c->r_v[(opcode & 0x0F00) >> 8] = vx | vy;
                    break;
                case 0x2:
                    c->r_v[(opcode & 0x0F00) >> 8] = vx & vy;
                    break;
                case 0x3:
                    c->r_v[(opcode & 0x0F00) >> 8] = vx ^ vy;
                    break;
                case 0x4: {
                    uint16_t sum = (uint16_t)vx + (uint16_t)vy;
                    if (sum > 255) {
                        c->r_v[0xF] = 1;
                    } else {
                        c->r_v[0xF] = 0;
                    }
                    c->r_v[(opcode & 0x0F00) >> 8] = (uint8_t)sum;
                    break;
                }
                case 0x5:
                    if (vx >= vy) {
                        c->r_v[0xF] = 1;
                    } else {
                        c->r_v[0xF] = 0;
                    }
                    c->r_v[(opcode & 0x0F00) >> 8] = vx - vy;
                    break;
                case 0x6:
                    c->r_v[(opcode & 0x0F00) >> 8] = vy >> 1;
                    c->r_v[0xF] = vy & 1;
                    break;
                case 0x7:
                    if (vy >= vx) {
                        c->r_v[0xF] = 1;
                    } else {
                        c->r_v[0xF] = 0;
                    }
                    c->r_v[(opcode & 0x0F00) >> 8] = vy - vx;
                    break;
                case 0xE:
                    c->r_v[(opcode & 0x0F00) >> 8] = vy << 1;
                    c->r_v[0xF] = (vy & 0x1F) > 0;
                    break;
                }
            }
            break;
        case 0x9000:
            if (c->r_v[(opcode & 0x0F00) >> 8] != c->r_v[(opcode & 0x00F0) >> 4])
                c->pc += 2;
            break;
        case 0xA000:
            c->r_i = opcode & 0x0FFF;
            break;
        case 0xB000:
            c->pc = (opcode & 0x0FFF) + c->r_v[0];
            break;
        case 0xC000:
            // TODO: random
            break;
        case 0xD000: {
            uint8_t vx = (opcode & 0x0F00) >> 8;
            uint8_t vy = (opcode & 0x00F0) >> 4;
            uint8_t n = opcode & 0x000F;

            chip8_draw(c, c->r_v[vx], c->r_v[vy], n);
            frame_updated = true; 
            break;
        }
        case 0xE000: {
            uint8_t key = c->r_v[(opcode & 0x0F00) >> 8];
            switch (opcode & 0x00FF) {
                case 0x009E:
                    if (c->keys[key])
                        c->pc += 2;
                    break;
                case 0x00A1: 
                    if (!c->keys[key])
                        c->pc += 2;
                    break;
            }
            break;
        }
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x07:
                    c->r_v[(opcode & 0x0F00) >> 8] = c->delay_timer;
                    break; 
                case 0x0A:
                    // TODO: Get Key
                    break;
                case 0x15:
                    c->delay_timer = c->r_v[(opcode & 0x0F00) >> 8];
                    break; 
                case 0x18:
                    c->sound_timer = c->r_v[(opcode & 0x0F00) >> 8];
                    break; 
                case 0x1E:
                    c->r_i += c->r_v[(opcode & 0x0F00) >> 8];
                    break;
                case 0x29:
                    c->r_i = FONT_START + (c->r_v[(opcode & 0x0F00) >> 8] & 0x0F) * 5;
                    break;
                case 0x33:
                    // TODO: BCD
                    break;
                case 0x55: {
                    uint8_t x = (opcode & 0x0F00) >> 8;
                    for (int i = 0; i <= x; i++) {
                        c->memory[c->r_i + i] = c->r_v[i];
                    }
                    break;
                }
                case 0x65: {
                    uint8_t x = (opcode & 0x0F00) >> 8;
                    for (int i = 0; i <= x; i++) {
                        c->r_v[i] = c->memory[c->r_i + i];
                    }
                    break;
                }
            }
            break;
    }

    return frame_updated;
}