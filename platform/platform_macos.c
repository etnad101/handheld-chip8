#include "platform.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"

int platform_get_rom(uint8_t** rom, char* path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size <= 0) {
        fclose(f);
        return -2;
    }

    // TODO
    *rom = (uint8_t*)malloc(size);
    if (!*rom) {
        fclose(f);
        return -3;
    }

    long read = (long)fread(*rom, 1, size, f);
    fclose(f);

    if (read != size) {
        free(*rom);
        return -4;
    }
    
    return (int)size;
}