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
  0x90,0x90,0xF0,0x10,0x10,
  // five:
  0xF0,0x80,0xF0,0x10,0xF0,
  // six:
  0xF0,0x80,0xF0,0x90,0xF0,
  // seven:
  0xF0,0x10,0x20,0x40,0x40,
  // eight:
  0xF0,0x90,0xF0,0x90,0xF0,
  // nine:
  0xF0,0x90,0xF0,0x10,0xF0,
  // A:
  0xF0,0x90,0xF0,0x90,0x90,
  // B:
  0xE0,0x90,0xE0,0x90,0xE0,
  // C:
  0xF0,0x80,0x80,0x80,0xF0,
  // D:
  0xE0,0x90,0x90,0x90,0xE0,
  // E:
  0xF0,0x80,0xF0,0x80,0xF0,
  // F:
  0xF0,0x80,0xF0,0x80,0x80
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
    #ifdef DEBUG_MODE
    dump_state(*cpu);
    #else
    if(cpu->draw)
    {
      flip(cpu->screen,frameno);
      cpu->draw = 0;
      sleep(1);
    }
    #endif
    step(cpu);
    frameno++;
    gettimeofday(cur,NULL);
    if((cur->tv_usec - prev->tv_usec) >= 17)
    {
      cpu->delay -= (cur->tv_usec - prev->tv_usec) / 17;
      if(cpu->sound > 0)
        cpu->sound -= (cur->tv_usec - prev->tv_usec) / 17;
      gettimeofday(prev,NULL);
    }
    // pause for input
    #ifdef DEBUG_MODE
    getchar();
    #endif
  }
  dump_state(*cpu);
}

void flip(uint8_t screen[32][64],unsigned int frameno)
{
  int x = 0,y = 0;
  printf("/==============================================================\\\n");
  while(y < 32)
  {
    if(screen[x][y] & 0x1)
      putchar('#');
    else
      putchar(' ');
    x++;
    if(x == 64)
    {
      putchar('|');
      putchar('\n');
      x = 0;
      y++;
    }
  }
  printf("\\==============================================================/\n");
  printf("Frame number: %d\n",frameno);
}

void cpu_load(FILE* from,cpu_t* cpu)
{
  uint8_t* load_into = &(cpu->memory[0x200]);
  fread(load_into,sizeof(uint8_t),(0x1000-0x200),from);
  // set the interpreter memory
  memcpy(cpu->memory,interpreter,512);
  // heap_dump(*cpu);
  cpu->pc = 0x200; // start at the beginning, of course
  // set the delay to 0xFF
  cpu->delay = 0xFF;
  // buzzer off to start with
  cpu->sound = 0x00;
  // set all the registers to 0
  memset(cpu->registers,0,16);
  // set the address register to 0
  cpu->address = 0x0;
  // don't draw until the draw opcode occurs
  cpu->draw = 0;
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
    printf("Register %.1X: %.2X\n",i,cpu.registers[i]);
  printf("Address Register (I): %.4X\n",cpu.address);
  printf("Delay register: %.2X\n",cpu.delay);
  printf("Sound register: %.2X\n",cpu.sound);
  printf("Program Counter: %.4X\n",cpu.pc);
  printf("Error number: %.2X\n",cpu.errno);
  printf("Draw next frame: %s\n",cpu.draw ? "yes" : "no");
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
