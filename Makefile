CC=gcc
CFLAGS=-Wall -Wextra -std=gnu99 -pedantic -g
LDFLAGS=

chip8: main.c cpu.o opcodes.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o chip8 main.c cpu.o opcodes.o

cpu.o: cpu.c cpu.h opcodes.h
	$(CC) $(CFLAGS) -o cpu.o -c cpu.c

opcodes.o: opcodes.c opcodes.h
	$(CC) $(CFLAGS) -o opcodes.o -c opcodes.c
