#include <SDL/SDL.h>
#include "sdl.h"

void sdl_init(SDL_Surface** screen)
{
	*screen = SDL_SetVideoMode(SDL_RESX,SDL_RESY,SDL_BPP,SDL_HWSURFACE);
}

void sdl_flip(SDL_Surface* target,uint8_t source[64][32],unsigned int frameno)
{
	SDL_Rect* current_pixel = malloc(sizeof(SDL_Rect));
	current_pixel->w = SDL_PIXEL_WIDTH;
	current_pixel->h = SDL_PIXEL_HEIGHT;
	Uint32 colour = 0xFFFFFF;
	unsigned int x=0,y=0;
	
	if(target == NULL) {
		fprintf(stderr,"WARNING: attempted to draw on a NULL SDL_Surface.\n");
		return;
	}
	
	while(y < 32)
	{
		current_pixel->x = x*(SDL_PIXEL_WIDTH);
		current_pixel->y = y*(SDL_PIXEL_HEIGHT);
		
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

		SDL_FillRect(target,current_pixel,colour);
	}
	SDL_Flip(target);
	free(current_pixel);
}
