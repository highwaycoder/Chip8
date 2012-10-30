#include <SDL/SDL.h>
#include "cpu.h"
#include "sdl.h"

void sdl_init(SDL_Surface** screen)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	*screen = SDL_SetVideoMode(SDL_RESX,SDL_RESY,SDL_BPP,SDL_HWSURFACE);
}

void sdl_flip(SDL_Surface* target,uint8_t source[64][32],unsigned int frameno)
{
	SDL_Rect current_pixel;
	current_pixel.w = SDL_PIXEL_WIDTH;
	current_pixel.h = SDL_PIXEL_HEIGHT;
	Uint32 colour = 0xFFFFFF;
	unsigned int x=0,y=0;
	
	if(target == NULL) {
		fprintf(stderr,"WARNING: attempted to draw on a NULL SDL_Surface.\n");
		return;
	}
	
	while(y < 32)
	{
		current_pixel.x = x*(SDL_PIXEL_WIDTH);
		current_pixel.y = y*(SDL_PIXEL_HEIGHT);
		
		if(source[x][y] & 0x1)
			colour = 0xFFFFFF;
		else 
			colour = 0x000000;
		x++;
		if(x == 64)
		{
			x = 0;
			y++;
		}

		SDL_FillRect(target,&current_pixel,colour);
	}
	
	
	SDL_Flip(target);
}

uint8_t key_value(enum chip8_key key)
{
	switch(key)
	{
		case KEY_0:
			return 0x0;
		case KEY_1:
			return 0x1;
		case KEY_2:
			return 0x2;
		case KEY_3:
			return 0x3;
		case KEY_4:
			return 0x4;
		case KEY_5:
			return 0x5;
		case KEY_6:
			return 0x6;
		case KEY_7:
			return 0x7;
		case KEY_8:
			return 0x8;
		case KEY_9:
			return 0x9;
		case KEY_A:
			return 0xA;
		case KEY_B:
			return 0xB;
		case KEY_C:
			return 0xC;
		case KEY_D:
			return 0xD;
		case KEY_E:
			return 0xE;
		case KEY_F:
			return 0xF;
	}
	return 0x0;
}

void keypad_helper(cpu_t* cpu,SDL_KeyboardEvent ke)
{
	enum chip8_key affected_key;
	switch(ke.keysym.sym)
	{
		// 2
		case SDLK_UP:
		case SDLK_w:
		case SDLK_KP8:
			affected_key = KEY_2;
			break;
		// 8
		case SDLK_DOWN:
		case SDLK_s:
		case SDLK_KP2:
			affected_key = KEY_8;
			break;
		// 4
		case SDLK_LEFT:
		case SDLK_a:
		case SDLK_KP4:
			affected_key = KEY_4;
			break;
		// 6
		case SDLK_RIGHT:
		case SDLK_d:
		case SDLK_KP6:
			affected_key = KEY_6;
			break;
		case SDLK_q:
			// use the Q key to quit
			cpu->errno = EUSRQ;
			break;
		default:
			break;
	}
	if(ke.type == SDL_KEYDOWN)
	{
		cpu->keypad |= affected_key;
	}
	else
	{
		cpu->keypad &= ~affected_key;
	}
}

void sdl_handle_events(cpu_t* cpu)
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				keypad_helper(cpu,event.key);
				break;
			case SDL_QUIT:
				cpu->errno = EUSRQ;
				break;
			default:
				break;
		}
	}
}
