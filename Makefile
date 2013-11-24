CC=gcc
override CFLAGS+=-Wall -Wextra -std=gnu99 -pedantic -g -fbounds-check
LDFLAGS=
PROGNAME=chip8

$(PROGNAME): main.c cpu.o opcodes.o sdl.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROGNAME)  main.c cpu.o opcodes.o sdl.o `sdl-config --libs`
	$(MAKE) clean-objs

cpu.o: cpu.c cpu.h opcodes.h
	$(CC) $(CFLAGS) -o cpu.o -c cpu.c

opcodes.o: opcodes.c opcodes.h
	$(CC) $(CFLAGS) -o opcodes.o -c opcodes.c
	
sdl.o: sdl.c sdl.h
	$(CC) $(CFLAGS) `sdl-config --cflags` -o sdl.o -c sdl.c

debug:
	$(MAKE) chip8debug CFLAGS="-DDEBUG_MODE -DBINARY_DUMPS" PROGNAME=chip8debug
	$(MAKE) clean-objs

clean-objs:
	rm -f opcodes.o cpu.o

clean: clean-objs
	rm -f $(PROGNAME) chip8debug