#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include "opcodes.h"
#include "cpu.h"

const uint16_t interpreter[512] = {
  // zero:
  0xF0,0x90,0x90,0x90,0xF0,
  // one:
  0x20,0x60,0x20,0x20,0x70,
  // two:
  0xF0,0x10,0xF0,0x80,0xF0,
  // three:
  0xF0,0x10,0xF0,0x10,0xF0,
  // four:
  
  };

void cpu_run(cpu_t* cpu)
{
  struct timeval* prev = malloc(sizeof(struct timeval));
  struct timeval* cur = malloc(sizeof(struct timeval));
  unsigned int frameno;
  gettimeofday(prev,NULL);
  srand(prev->tv_usec);
  while(cpu->errno == ENONE)
  {
    step(cpu);
    flip(cpu->screen,frameno);
    frameno++;
    gettimeofday(cur,NULL);
    if((cur->tv_usec - prev->tv_usec) >= 17)
    {
      cpu->delay -= (cur->tv_usec - prev->tv_usec) / 17;
      if(cpu->sound > 0)
        cpu->sound -= (cur->tv_usec - prev->tv_usec) / 17;
      gettimeofday(prev,NULL);
    }
    sleep(1);
  }
  dump_state(*cpu);
}

void flip(uint8_t screen[64][32],unsigned int frameno)
{
  int x = 0,y = 0;
  while(y < 32)
  {
    if(screen[x][y] & 0x1)
      putchar('#');
    else
      putchar(' ');
    x++;
    if(x == 32)
    {
      putchar('\n');
      x = 0;
      y++;
    }
  }
  printf("Frame number: %d\n",frameno);
}

void cpu_load(FILE* from,cpu_t* cpu)
{
  uint8_t* load_into = &(cpu->memory[0x200]);
  fread(load_into,sizeof(uint8_t),(0x1000-0x200),from);
  // heap_dump(*cpu);
  cpu->pc = 0x200; // start at the beginning, of course
  // set the delay to 0xFF
  cpu->delay = 0xFF;
  // buzzer off to start with
  cpu->sound = 0x00;
}

cpu_t* new_cpu(void)
{
  cpu_t* cpu = malloc(sizeof(cpu_t));
  memset(cpu,0,sizeof(cpu_t));
  cpu->memory = malloc(0x1000);
  memset(cpu->memory,0,0x1000);
  return cpu;
}

void step(cpu_t* cpu)
{
  uint16_t current_opcode;
  if(cpu->pc >= 0xFFF)
  {
    cpu->errno = EPCOF;
  }
  if(cpu->pc < 0x200)
  {
    cpu->errno = EPCUF;
  }
  current_opcode = ((cpu->memory[cpu->pc]<<8) | cpu->memory[cpu->pc+1]);
  switch((current_opcode & 0xF000) >> 12)
  {
    case 0x0:
      sys(cpu,current_opcode);
      return;
    case 0x1:
      jmp(cpu,current_opcode);
      return;
    case 0x2:
      call(cpu,current_opcode);
      return;
    case 0x3:
      se(cpu,current_opcode);
      return;
    case 0x4:
      sne(cpu,current_opcode);
      return;
    case 0x5:
      if((current_opcode & 0x000F) != 0)
      {
        cpu->errno = EBDOP;
        return;
      }
      se(cpu,current_opcode);
      return;
    case 0x6:
      ld(cpu,current_opcode);
      return;
    case 0x7:
      add(cpu,current_opcode);
      return;
    case 0x8:
      switch(current_opcode & 0x000F)
      {
        case 0x0:
          ld(cpu,current_opcode);
          return;
        case 0x1:
          or(cpu,current_opcode);
          return;
        case 0x2:
          and(cpu,current_opcode);
          return;
        case 0x3:
          xor(cpu,current_opcode);
          return;
        case 0x4:
          add(cpu,current_opcode);
          return;
        case 0x5:
          sub(cpu,current_opcode);
          return;
        case 0x6:
          shr(cpu,current_opcode);
          return;
        case 0x7:
          subn(cpu,current_opcode);
          return;
        case 0xE:
          shl(cpu,current_opcode);
          return;
        default:
          cpu->errno = EBDOP;
          return;
      }
    case 0x9:
      if((current_opcode & 0x000F) != 0)
      {
        cpu->errno = EBDOP;
        return;
      }
      sne(cpu,current_opcode);
      return;
    case 0xA:
      ld(cpu,current_opcode);
      return;
    case 0xB:
      jmp(cpu,current_opcode);
      return;
    case 0xC:
      rnd(cpu,current_opcode);
      return;
    case 0xD:
      drw(cpu,current_opcode);
      return;
    case 0xE:
      if((current_opcode & 0x00FF) == 0x9E)
        skp(cpu,current_opcode);
      else if((current_opcode & 0x00FF) == 0xA1)
        sknp(cpu,current_opcode);
      else
        cpu->errno = EBDOP;
      return;
    case 0xF:
      switch(current_opcode & 0x00FF)
      {
        case 0x07:
        case 0x0A:
        case 0x15:
        case 0x18:
        case 0x29:
        case 0x33:
        case 0x55:
        case 0x65:
          ld(cpu,current_opcode);
          return;
        case 0x1E:
          add(cpu,current_opcode);
          return;
        default:
          cpu->errno = EBDOP;
          return;
      }
    break;
  }
}

void free_cpu(cpu_t* cpu)
{
  free(cpu->memory);
  free(cpu);
}

void dump_state(cpu_t cpu)
{
  int i=0;
  for(i=0;i<16;i++)
    printf("Register %.1X: %.1X\n",i,cpu.registers[i]);
  printf("Address Register (I): %.4X\n",cpu.address);
  printf("Delay register: %.1X\n",cpu.delay);
  printf("Sound register: %.1X\n",cpu.sound);
  printf("Program Counter: %.2X\n",cpu.pc);
  printf("Error number: %.1X\n",cpu.errno);
  printf("Current opcode: %.4X\n",(cpu.memory[cpu.pc]<<8) | cpu.memory[cpu.pc+1]);
  stack_trace(cpu);
  #ifdef MEM_DUMP
  heap_dump(cpu);
  #endif
}

void heap_dump(cpu_t cpu)
{
  int i=0;
  printf("Memory: \n");
  for(i=0;i<0x1000;i++)
  {
    printf("{%.4X : %.2X} ",i,cpu.memory[i]);
    if(!(i % 5)) putchar('\n');
  }
}

void stack_trace(cpu_t cpu)
{
  int i = 0;
  printf("Stack trace: \n");
  if(cpu.stack_pointer > 0)
  {
    for(i=cpu.stack_pointer;i>0;i--)
    {
      printf("\t0x%.1X : %.2X\n",i,cpu.stack[i]);
    }
  }
  else
  {
    printf("\tStack empty\n");
  }
}
