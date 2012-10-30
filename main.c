#include <stdio.h>
#include <stdint.h>
#include <SDL/SDL.h>
#include "sdl.h"
#include "cpu.h"

#define usage "Usage: chip8 romfile"

int main(int argc, char** argv)
{
  cpu_t* cpu = NULL;
  cpu = new_cpu();
  FILE* infile = NULL;
  int rv = 0;
  SDL_Surface* screen = NULL;
  switch(argc)
  {
    case 0:
      printf("Your system is incompatible with this software.\n");
      rv = !0;
      break;
    case 1:
      printf("%s\n",usage);
      rv = !0;
      break;
    case 2:
      infile = fopen(argv[1],"r");
      rv = (infile == NULL ? !0 : 0);
      break;
    default:
      printf("%s\n",usage);
      rv = !0;
      break;
  }
  if(infile != NULL)
  {
    cpu_load(infile,cpu);
    sdl_init(&screen);
    // make sure the 'screen' could be initialized
    if(screen != NULL)
        cpu_run(cpu,screen);
    else
        fprintf(stderr,"ERROR: could not initialize SDL_Surface* screen.\n");
    fclose(infile);
  }
  free_cpu(cpu);
  SDL_Quit();
  return rv;
}
