#ifndef CPU_H
#define CPU_H

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
};

typedef struct {
  uint8_t registers[16];
  uint8_t delay;
  uint8_t sound;
  uint8_t stack_pointer;
  uint16_t pc;
  uint16_t stack[16];
  uint8_t* memory;
  uint8_t errno;
  uint16_t address;
  uint8_t screen[64][32];
  uint8_t draw;
  uint8_t keypad[16];
} cpu_t;

void dump_state(cpu_t cpu);
cpu_t* new_cpu(void);
void free_cpu(cpu_t* cpu);
void heap_dump(cpu_t cpu);
void stack_trace(cpu_t cpu);
void step(cpu_t* cpu);
void cpu_load(FILE* from,cpu_t* cpu);
void cpu_run(cpu_t* cpu);
void flip(uint8_t screen[64][32],unsigned int frameno);

#endif
