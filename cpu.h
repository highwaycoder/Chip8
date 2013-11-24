#ifndef CPU_H
#define CPU_H

#include <SDL/SDL.h>

enum ERRORS {
  ENONE = 0x0,
  // pc overflow
  EPCOF = 0x1,
  // pc underflow
  EPCUF = 0x2,
  // stack overflow
  ESTOF = 0x3,
  // stack underflow
  ESTUF = 0x4,
  // bad opcode
  EBDOP = 0x5,
  // not implemented
  ENIMP = 0x6,
  // segmentation fault
  ESEGV = 0x7,
  // user quit
  EUSRQ = 0x8,
};

 
 enum chip8_key {
	 KEY_1 = 0x0001,
	 KEY_2 = 0x0002,
	 KEY_3 = 0x0004,
	 KEY_C = 0x0008,
	 KEY_4 = 0x0010,
	 KEY_5 = 0x0020,
     KEY_6 = 0x0040,
     KEY_D = 0x0080,
     KEY_7 = 0x0100,
     KEY_8 = 0x0200,
     KEY_9 = 0x0400,
     KEY_E = 0x0800,
     KEY_A = 0x1000,
     KEY_0 = 0x2000,
     KEY_B = 0x4000,
     KEY_F = 0x8000
 };
 
 enum boolean {
     b_FALSE = 0,
     b_TRUE = 1,
 };

typedef struct {
  uint8_t registers[16];
  uint8_t delay;
  uint8_t sound;
  uint8_t stack_pointer;
  uint16_t pc;
  uint16_t stack[16];
  uint8_t errno;
  uint16_t address;
  uint8_t screen[64][32];
  uint8_t draw;
  uint16_t keypad;
  uint8_t* memory;
  enum boolean wait;
  uint16_t breakpoint;
  enum boolean stepping;
} cpu_t;

void dump_state(cpu_t cpu);
cpu_t* new_cpu(void);
void free_cpu(cpu_t* cpu);
void heap_dump(cpu_t cpu);
void stack_trace(cpu_t cpu);
void step(cpu_t* cpu);
void cpu_load(FILE* from,cpu_t* cpu);
void cpu_run(cpu_t* cpu,SDL_Surface* screen);
void flip(uint8_t screen[64][32],unsigned int frameno);

#endif
