#include <stdio.h>
#include <stdint.h>
#include "cpu.h"

#define usage "Usage: chip8 romfile"

int main(int argc, char** argv)
{
  cpu_t* cpu = NULL;
  cpu = new_cpu();
  FILE* infile = NULL;
  int rv = 0;
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
    cpu_run(cpu);
  }
  free_cpu(cpu);
  return rv;
}
