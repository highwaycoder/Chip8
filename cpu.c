#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <SDL/SDL.h>
#include "opcodes.h"
#include "cpu.h"
#include "sdl.h"

const uint16_t interpreter[512] = {
  // zero:
  0x90F0,0x9090,0x00F0,
  // one:
  0x6020,0x2020,0x0070,
  // two:
  0x10F0,0x80F0,0x00F0,
  // three:
  0x10F0,0x10F0,0x00F0,
  // four:
  0x9090,0x10F0,0x0010,
  // five:
  0x80F0,0x10F0,0x00F0,
  // six:
  0x80F0,0x90F0,0x00F0,
  // seven:
  0x10F0,0x4020,0x0040,
  // eight:
  0x90F0,0x90F0,0x00F0,
  // nine:
  0x90F0,0x10F0,0x00F0,
  // A:
  0x90F0,0x90F0,0x0090,
  // B:
  0x90E0,0x90E0,0x00E0,
  // C:
  0x80F0,0x8080,0x00F0,
  // D:
  0x90E0,0x9090,0x00E0,
  // E:
  0x80F0,0x80F0,0x00F0,
  // F:
  0x80F0,0x80F0,0x0080
  };
  
void handle_signal(int signum) {}

void cpu_run(cpu_t* cpu,SDL_Surface* screen)
{
  struct timeval* prev = malloc(sizeof(struct timeval));
  struct timeval* cur = malloc(sizeof(struct timeval));
  unsigned int frameno = 0;
  gettimeofday(prev,NULL);
  srand(prev->tv_usec);
  
  // block the SIGINT signal so we can use it to interrupt the cpu
  struct sigaction handler;
  handler.sa_handler = handle_signal;
  sigaction(SIGINT,&handler,NULL);
  
  // handle the SIGUSR1 signal to dump cpu state
  sigaction(SIGUSR1,&handler,NULL);
  
  sigset_t signals_to_block;
  sigemptyset(&signals_to_block);
  sigaddset(&signals_to_block,SIGINT);
  sigaddset(&signals_to_block,SIGUSR1);
  sigprocmask(SIG_SETMASK,&signals_to_block,NULL);

  #ifdef DEBUG_MODE
  FILE* stacktrc = fopen("stack.trc","w");
  #endif
  do
  {
    // before anything, check if a key has been pressed this loop and disable waiting if it has
    if(cpu->keypad != 0 && cpu->wait == b_TRUE)
    {
      // increment the program counter because it hasn't been incremented yet
      cpu->pc+=2;
      cpu->wait = b_FALSE;
    }
    #ifdef DEBUG_MODE
    // don't dump state during wait
    if(cpu->wait == b_FALSE)
    {
      // rolls over at step 65535 but that's not the end of the world as long as you quit early enough
      fprintf(stacktrc,"------- STEP NUMBER: %.4X -------\n",frameno);
      dump_state(*cpu,stacktrc);
      fputc('\n',stacktrc);
    }
    // pause for input
    if(cpu->stepping == b_TRUE)
    {
      cpu->keypad = key_value(getchar());
    }
    // check if the breakpoint has been reached, if so flag 'stepping'
    if(cpu->breakpoint == cpu->pc)
    {
      cpu->stepping = b_TRUE;
      cpu->keypad = getchar();
    }
    #endif
    // handle events regardless of drawing or not
    sdl_handle_events(cpu);
    if(cpu->draw)
    {
      sdl_flip(screen,cpu->screen,frameno);
      cpu->draw = 0;
      // scale the CPU down to near chip8 levels (TODO: make this configurable)
      usleep(10000);
    }
    if(cpu->wait == b_FALSE)
    {
        step(cpu);
        frameno++;
    }
    gettimeofday(cur,NULL);
    
    if(cpu->delay > 0)
      cpu->delay--;
    
    if(cpu->sound > 0)
      cpu->sound--;
    
    // signal polling stuff
    sigset_t signal_set;
    sigpending(&signal_set);
    if(sigismember(&signal_set,SIGINT)) {
        cpu->errno = EUSRQ;
    } else if(sigismember(&signal_set,SIGUSR1)) {
        dump_state(*cpu,stdout);
    }

  } while(cpu->errno == ENONE);
  dump_state(*cpu,stdout);
  free(cur);
  free(prev);
  #ifdef DEBUG_MODE
  fclose(stacktrc);
  #endif
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
  cpu->delay = 0x00;
  // buzzer off to start with
  cpu->sound = 0x00;
  // set all the registers to 0
  memset(cpu->registers,0,16);
  // set the address register to 0
  cpu->address = 0x0;
  // don't draw until the draw opcode occurs
  cpu->draw = 0;
  // set up the keypad to have no presses registered to begin with
  cpu->keypad = 0x0000;
  // don't start in suspended mode
  cpu->wait = b_FALSE;
  // if no breakpoint is set, set it at the very end of the program to stop the cpu pausing constantly
  cpu->breakpoint = 0xFFFF;
  cpu->stepping = b_FALSE;
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

void dump_state(cpu_t cpu, FILE* logfile)
{
  int i=0;
  for(i=0;i<16;i++)
  {
    char c = ((i%4 == 3) ? '\n' : '\t');
    fprintf(logfile,"Register %.1X: %.2X",i,cpu.registers[i]);
    fputc(c,logfile);
  }
  fprintf(logfile,"Address: %.4X\t",cpu.address);
  fprintf(logfile,"Delay: %.2X\t",cpu.delay);
  fprintf(logfile,"Sound: %.2X\n",cpu.sound);
  fprintf(logfile,"Program Counter: %.4X\t",cpu.pc);
  fprintf(logfile,"Breakpoint: %.4X\n",cpu.breakpoint);
  fprintf(logfile,"Error number: %.2X\n",cpu.errno);
  fprintf(logfile,"Draw next frame: %s\n",cpu.draw ? "yes" : "no");
  fprintf(logfile,"Next opcode: %.4X\n",(cpu.memory[cpu.pc]<<8) | cpu.memory[cpu.pc+1]);
  fprintf(logfile,"Keypad value (hex): %.4X\n",cpu.keypad);
  fprintf(logfile,"%s\n",cpu.wait==b_TRUE?"waiting for input":"running");
  stack_trace(cpu,logfile);
  heap_dump(cpu);
}

void heap_dump(cpu_t cpu)
{
  FILE* logfile = fopen("core.dmp","w");
  if(logfile == NULL)
    return;
  #ifdef BINARY_DUMPS
  fwrite(cpu.memory,1,0x1000,logfile);
  #else
  int i = 0;
  fprintf(logfile,"Memory: \n");
  for(i=0;i<0x1000;i++)
  {
    fprintf(logfile,"{%.4X : %.2X} ",i,cpu.memory[i]);
    if(!(i % 5)) fputc('\n',logfile);
  }
  #endif
  fclose(logfile);
}

void stack_trace(cpu_t cpu, FILE* logfile)
{
  int i = 0;
  fprintf(logfile,"Stack trace: \n");
  if(cpu.stack_pointer > 0)
  {
    for(i=cpu.stack_pointer;i>0;i--)
    {
      fprintf(logfile,"\t0x%.1X : %.2X\n",i,cpu.stack[i]);
    }
  }
  else
  {
    fprintf(logfile,"\tStack empty\n");
  }
}
