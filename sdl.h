#ifndef SDL_H
#define SDL_H

#define SDL_RESX 640
#define SDL_RESY 320
#define SDL_BPP 32

#define SDL_PIXEL_WIDTH (SDL_RESX / 64)
#define SDL_PIXEL_HEIGHT (SDL_RESY / 32)

#include "cpu.h"

void sdl_handle_events(cpu_t* cpu);
void sdl_init(SDL_Surface** screen);
void sdl_flip(SDL_Surface* target,uint8_t source[64][32],unsigned int frameno);
uint8_t key_value(enum chip8_key key);

#endif
