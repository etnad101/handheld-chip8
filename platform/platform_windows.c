#include "platform.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include <SDL2/SDL.h>

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Event event;

int platform_init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("SDL Failed to init\n");
        return -1;
    }

    window = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, 0);
    if (window == NULL) {
        printf("Failed to create window\n");
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Failed to create renderer\n");
        return -1;
    }

    return 0;
}

int platform_get_rom(uint8_t** rom, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("Failed to open file\n");
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size <= 0) {
        fclose(f);
        printf("File size is 0 or other error\n");
        return -1;
    }

    *rom = (uint8_t*)malloc(size);
    if (!*rom) {
        fclose(f);
        printf("Failed to allocate space for ROM\n");
        return -1;
    }

    long read = (long)fread(*rom, 1, size, f);
    fclose(f);

    if (read != size) {
        free(*rom);
        printf("Failed to read file\n");
        return -1;
    }

    return (int)size;
}

void platform_free_rom(uint8_t* rom) { free(rom); }

// TODO: pass keys pointer to this function and poll for key events
bool platform_poll_events(bool* keys) {
    SDL_PollEvent(&event);
    switch (event.type) {
    case SDL_QUIT:
        return false;
        break;
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_1:
            keys[0x1] = true;
            break;
        case SDLK_2:
            keys[0x2] = true;
            break;
        case SDLK_3:
            keys[0x3] = true;
            break;
        case SDLK_4:
            keys[0xC] = true;
            break;
        case SDLK_q:
            keys[0x4] = true;
            break;
        case SDLK_w:
            keys[0x5] = true;
            break;
        case SDLK_e:
            keys[0x6] = true;
            break;
        case SDLK_r:
            keys[0xD] = true;
            break;
        case SDLK_a:
            keys[0x7] = true;
            break;
        case SDLK_s:
            keys[0x8] = true;
            break;
        case SDLK_d:
            keys[0x9] = true;
            break;
        case SDLK_f:
            keys[0xE] = true;
            break;
        case SDLK_z:
            keys[0xA] = true;
            break;
        case SDLK_x:
            keys[0x0] = true;
            break;
        case SDLK_c:
            keys[0xB] = true;
            break;
        case SDLK_v:
            keys[0xF] = true;
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event.key.keysym.sym) {
        case SDLK_1:
            keys[0x1] = false;
            break;
        case SDLK_2:
            keys[0x2] = false;
            break;
        case SDLK_3:
            keys[0x3] = false;
            break;
        case SDLK_4:
            keys[0xC] = false;
            break;
        case SDLK_q:
            keys[0x4] = false;
            break;
        case SDLK_w:
            keys[0x5] = false;
            break;
        case SDLK_e:
            keys[0x6] = false;
            break;
        case SDLK_r:
            keys[0xD] = false;
            break;
        case SDLK_a:
            keys[0x7] = false;
            break;
        case SDLK_s:
            keys[0x8] = false;
            break;
        case SDLK_d:
            keys[0x9] = false;
            break;
        case SDLK_f:
            keys[0xE] = false;
            break;
        case SDLK_z:
            keys[0xA] = false;
            break;
        case SDLK_x:
            keys[0x0] = false;
            break;
        case SDLK_c:
            keys[0xB] = false;
            break;
        case SDLK_v:
            keys[0xF] = false;
            break;
        }
    }
    return true;
}

void platform_draw(const bool* buff) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            int idx = (y * 64) + x;
            if (buff[idx]) {
                SDL_Rect rect;
                rect.x = x * 10;
                rect.y = y * 10;
                rect.w = 10;
                rect.h = 10;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}
