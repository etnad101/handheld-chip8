CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -O2
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS   := $(shell sdl2-config --libs)

SRC := \
    main/main_macos.c \
    core/chip8.c \
    app/app.c \
    platform/platform_macos.c

OUT := build/chip8

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $(SRC) -o $(OUT) $(SDL_LIBS)

clean:
	rm -rf build
