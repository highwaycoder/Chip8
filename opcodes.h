#ifndef OPCODES_H
#define OPCODES_H
#include <stdint.h>
#include "cpu.h"

void sys(cpu_t* cpu,uint16_t current_opcode);
void jmp(cpu_t* cpu,uint16_t current_opcode);
void call(cpu_t* cpu,uint16_t current_opcode);
void se(cpu_t* cpu,uint16_t current_opcode);
void sne(cpu_t* cpu,uint16_t current_opcode);
void ld(cpu_t* cpu,uint16_t current_opcode);
void add(cpu_t* cpu,uint16_t current_opcode);
void or(cpu_t* cpu,uint16_t current_opcode);
void and(cpu_t* cpu,uint16_t current_opcode);
void xor(cpu_t* cpu,uint16_t current_opcode);
void sub(cpu_t* cpu,uint16_t current_opcode);
void shr(cpu_t* cpu,uint16_t current_opcode);
void subn(cpu_t* cpu,uint16_t current_opcode);
void shl(cpu_t* cpu,uint16_t current_opcode);
void rnd(cpu_t* cpu,uint16_t current_opcode);
void drw(cpu_t* cpu,uint16_t current_opcode);
void skp(cpu_t* cpu,uint16_t current_opcode);
void sknp(cpu_t* cpu,uint16_t current_opcode);
uint16_t get_sprite_loc(uint8_t sprite);
uint8_t get_random_byte(void);

#endif
