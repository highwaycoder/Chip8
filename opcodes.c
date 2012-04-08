#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "opcodes.h"

void sys(cpu_t* cpu,uint16_t current_opcode)
{
  switch(current_opcode & 0xFF)
  {
    case 0xE0:
      memset(cpu->screen,0,(64 * 32));
      break;
    case 0xEE:
      if(cpu->stack_pointer == 0)
      {
        cpu->errno = ESTUF;
        break;
      }
      cpu->pc = cpu->stack[cpu->stack_pointer];
      cpu->stack_pointer--;
      break;
    default:
      // default behaviour is to ignore these opcodes
      break;
  }
  cpu->pc += 2;
}

void jmp(cpu_t* cpu,uint16_t current_opcode)
{
  if((current_opcode & 0xF000) == 0x1000)
    cpu->pc = (current_opcode & 0x0FFF);
  else if((current_opcode & 0xF000) == 0xB000)
    cpu->pc = (current_opcode & 0x00FF) + cpu->registers[(current_opcode & 0x0F00) >> 8];
}

void call(cpu_t* cpu,uint16_t current_opcode)
{
  cpu->stack_pointer++;
  cpu->stack[cpu->stack_pointer] = cpu->pc;
  cpu->pc = current_opcode & 0x0FFF;
}

void se(cpu_t* cpu,uint16_t current_opcode)
{
  switch((current_opcode & 0xF000) >> 12)
  {
    case 0x3:
      if(cpu->registers[(current_opcode & 0x0F00) >> 8] == (current_opcode & 0x00FF))
        cpu->pc += 4;
      break;
    case 0x5:
      if(cpu->registers[(current_opcode & 0x0F00) >> 8] == cpu->registers[(current_opcode & 0x00F0) >> 4])
        cpu->pc += 4;
      break;
    default:
      cpu->errno = EBDOP;
      break;
  }
  cpu->pc += 2;
}

void sne(cpu_t* cpu,uint16_t current_opcode)
{
  switch((current_opcode & 0xF000) >> 12)
  {
    case 0x4:
      if(cpu->registers[(current_opcode & 0x0F00) >> 8] != (current_opcode & 0x00FF))
        cpu->pc += 4;
      break;
    case 0x9:
      if(cpu->registers[(current_opcode & 0x0F00) >> 8] != cpu->registers[(current_opcode & 0x00F0) >> 4])
        cpu->pc += 4;
      break;
    default:
      cpu->errno = EBDOP;
      break;
  }
  cpu->pc += 2;
}

void ld(cpu_t* cpu,uint16_t current_opcode)
{
  int i = 0;
  if((current_opcode & 0xF000) == 0xF000)
  {
    switch(current_opcode & 0xFF)
    {
      case 0x07:
        cpu->registers[(current_opcode & 0x0F00) >> 8] = cpu->delay;
        break;
      case 0x0A:
        // TODO: not implemented
        cpu->errno = ENIMP;
        printf("Warning: unimplemented opcode used");
        break;
      case 0x15:
        cpu->delay = cpu->registers[(current_opcode & 0x0F00) >> 8];
        break;
      case 0x18:
        cpu->sound = cpu->registers[(current_opcode & 0x0F00) >> 8];
        break;
      case 0x29:
        cpu->address = get_sprite_loc(cpu->registers[(current_opcode & 0x0F00) >> 8]);
        break;
      case 0x33:
        cpu->memory[cpu->address] = ((current_opcode & 0x0F00) >> 8) / 100;
        cpu->memory[cpu->address+1] = (((current_opcode & 0x0F00) >> 8)/10)%10;
        cpu->memory[cpu->address+2] = (((current_opcode & 0x0F00) >> 8)%100)%10;
        break;
      case 0x55:
        for(i=0;i<((current_opcode & 0x0F00)>>8);i++)
        {
          cpu->memory[cpu->address+i] = cpu->registers[i];
        }
        break;
      case 0x65:
        for(i=0;i<((current_opcode & 0x0F00)>>8);i++)
        {
          cpu->registers[i] = cpu->memory[cpu->address+i];
        }
        break;
      default:
        cpu->errno = EBDOP;
        break;
    }
  }
  else if((current_opcode & 0xF00F) == 0x8000)
  {
    cpu->registers[(current_opcode & 0x0F00) >> 8] = cpu->registers[(current_opcode & 0x00F0) >> 4];
  }
  else if((current_opcode & 0xF000) == 0x6000)
  {
    cpu->registers[(current_opcode & 0x0F00) >> 8] = (current_opcode & 0x00FF);
  }
  else if((current_opcode & 0xF000) == 0xA000)
  {
    cpu->address = (current_opcode & 0x0FFF);
  }
  else
  {
    cpu->errno = EBDOP;
  }
  cpu->pc += 2;
}

void add(cpu_t* cpu,uint16_t current_opcode)
{
  uint8_t cache;
  if((current_opcode & 0xF000) == 0x7000)
  {
    cpu->registers[(current_opcode & 0x0F00) >> 8] += (current_opcode & 0x00FF);
  }
  else if((current_opcode & 0xF00F) == 0x8004)
  {
    cache = cpu->registers[(current_opcode & 0x0F00) >> 8];
    cpu->registers[(current_opcode & 0x0F00) >> 8] += cpu->registers[(current_opcode & 0x00F0) >> 4];
    if(cpu->registers[(current_opcode & 0x0F00) >> 8] > cpu->registers[(current_opcode & 0x00F0)] ||
       cpu->registers[(current_opcode & 0x0F00) >> 8] > cache)
       cpu->registers[0xF] = 1;
    else
       cpu->registers[0xF] = 0;
  }
  else if((current_opcode & 0xF0FF) == 0xF01E)
  {
    cpu->address += cpu->registers[(current_opcode & 0x0F00) >> 8];
  }
  else
  {
    cpu->errno = EBDOP;
  }
  cpu->pc += 2;
}

void or(cpu_t* cpu,uint16_t current_opcode)
{
  cpu->registers[(current_opcode & 0x0F00) >> 8] |= cpu->registers[(current_opcode & 0x00F0) >> 4];
  cpu->pc += 2;
}

void and(cpu_t* cpu,uint16_t current_opcode)
{
  cpu->registers[(current_opcode & 0x0F00) >> 8] &= cpu->registers[(current_opcode & 0x00F0) >> 4];
  cpu->pc += 2;
}

void xor(cpu_t* cpu,uint16_t current_opcode)
{
  cpu->registers[(current_opcode & 0x0F00) >> 8] ^= cpu->registers[(current_opcode & 0x00F0) >> 4];
  cpu->pc += 2;
}

void sub(cpu_t* cpu,uint16_t current_opcode)
{
  if(cpu->registers[(current_opcode & 0x0F00) >> 8] > cpu->registers[(current_opcode & 0x00F0) >> 4])
    cpu->registers[0xF] = 1;
  else
    cpu->registers[0xF] = 0;
  cpu->registers[(current_opcode & 0x0F00) >> 8] -= cpu->registers[(current_opcode & 0x00F0) >> 4];
  cpu->pc += 2;
}

void shr(cpu_t* cpu,uint16_t current_opcode)
{
  cpu->registers[0xF] = cpu->registers[(current_opcode & 0x0F00) >> 8] & 0x1;
  cpu->registers[(current_opcode & 0x0F00) >> 8] >> 1;
  cpu->pc += 2;
}

void subn(cpu_t* cpu,uint16_t current_opcode)
{
  if(cpu->registers[(current_opcode & 0x0F00) >> 8] > cpu->registers[(current_opcode & 0x00F0) >> 4])
    cpu->registers[0xF] = 1;
  else
    cpu->registers[0xF] = 0;
  cpu->registers[(current_opcode & 0x0F00) >> 8] = cpu->registers[(current_opcode & 0x00F0) >> 4] - cpu->registers[(current_opcode & 0x0F00) >> 8];
  cpu->pc += 2;
}

void shl(cpu_t* cpu,uint16_t current_opcode)
{
  cpu->registers[0xF] = (cpu->registers[(current_opcode & 0x0F00) >> 8] & 0x80) >> 7;
  cpu->registers[(current_opcode & 0x0F00) >> 8] = cpu->registers[(current_opcode & 0x0F00) >> 8] << 1;
  cpu->pc += 2;
}

void rnd(cpu_t* cpu,uint16_t current_opcode)
{
  cpu->registers[(current_opcode & 0x0F00) >> 8] = (get_random_byte() & (current_opcode & 0x00FF));
  cpu->pc += 2;
}

void drw(cpu_t* cpu,uint16_t current_opcode)
{
  uint8_t num_bytes = (uint8_t)(current_opcode);
  uint8_t x_coord = cpu->registers[(current_opcode & 0x0F00) >> 8];
  uint8_t y_coord = cpu->registers[(current_opcode & 0x00F0) >> 4];
  uint8_t sprite_byte;
  uint8_t pixels[8] = {0};
  unsigned int i = 0;
  
  // no collision yet
  cpu->registers[0xF] = 0x0;
  
  for(i=0;i<num_bytes;i++)
  {
    sprite_byte = cpu->memory[cpu->address + i];
    // set up the individual pixels
    pixels[0] = (sprite_byte & 0x80) >> 7;
    pixels[1] = (sprite_byte & 0x40) >> 6;
    pixels[2] = (sprite_byte & 0x20) >> 5;
    pixels[3] = (sprite_byte & 0x10) >> 4;
    pixels[4] = (sprite_byte & 0x08) >> 3;
    pixels[5] = (sprite_byte & 0x04) >> 2;
    pixels[6] = (sprite_byte & 0x02) >> 1;
    pixels[7] = (sprite_byte & 0x01);
    
    // first check if the register is unset - otherwise we waste a lot of time
    // making pointless comparisons (it can't really be set twice!)
    if( cpu->registers[0xF] == 0 && (
    // if the old pixel was set
        cpu->screen[x_coord+0][y_coord+i] & 0x1 ||
        cpu->screen[x_coord+1][y_coord+i] & 0x1 ||
        cpu->screen[x_coord+2][y_coord+i] & 0x1 ||
        cpu->screen[x_coord+3][y_coord+i] & 0x1 ||
        cpu->screen[x_coord+4][y_coord+i] & 0x1 ||
        cpu->screen[x_coord+5][y_coord+i] & 0x1 ||
        cpu->screen[x_coord+6][y_coord+i] & 0x1 ||
        cpu->screen[x_coord+7][y_coord+i] & 0x1))
    {
        cpu->registers[0xF] = 1;
    }
    
    // XOR them into video memory
    cpu->screen[x_coord+0][y_coord+i] ^= pixels[0];
    cpu->screen[x_coord+1][y_coord+i] ^= pixels[1];
    cpu->screen[x_coord+2][y_coord+i] ^= pixels[2];
    cpu->screen[x_coord+3][y_coord+i] ^= pixels[3];
    cpu->screen[x_coord+4][y_coord+i] ^= pixels[4];
    cpu->screen[x_coord+5][y_coord+i] ^= pixels[5];
    cpu->screen[x_coord+6][y_coord+i] ^= pixels[6];
    cpu->screen[x_coord+7][y_coord+i] ^= pixels[7];
    
  }
  cpu->draw = 1;
  cpu->pc += 2;
}
void skp(cpu_t* cpu,uint16_t current_opcode)
{
  if(cpu->keypad[cpu->registers[(current_opcode & 0x0F00) >> 8]] == 1)
    cpu->pc += 4;
  cpu->pc += 2;
}
void sknp(cpu_t* cpu,uint16_t current_opcode)
{
  if(cpu->keypad[cpu->registers[(current_opcode & 0x0F00) >> 8]] == 0)
    cpu->pc += 4;
  cpu->pc += 2;
}

// helper functions
uint16_t get_sprite_loc(uint8_t sprite)
{
  return sprite * 5; // sprites are at 5-byte offsets, handily
}

uint8_t get_random_byte(void)
{
  return (uint8_t)(rand() % 256);
}
