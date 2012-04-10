## Chip8
Is an emulator written in C for the Chip8 device.

## Known Bugs
 - 4k of memory will leak if you interrupt a running program.
   The memory should be recoverable and buffered by your OS, but just
   watch out for this.  I plan to implement a signal handler that cleans up
   carefully on interrupt, as otherwise the program doesn't really ever clean
   up after itself since interrupting is pretty much necessary.
 - Keyboard input is disabled
 - Sound is disabled

## Features
 - Supports all opcodes from the Chip8 instruction set to the best of my knowledge.
 - Display works (hopefully: please report bugs!)