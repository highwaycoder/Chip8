## Chip8
Is an emulator written in C for the Chip8 device.

## Known Bugs
 - Sound is disabled

## Features
 - Supports all opcodes from the Chip8 instruction set to the best of my knowledge.
 - Display works (hopefully: please report bugs!)
 - Keypad support:
 	Chip8 keypad:
 	[1][2][3][C]
 	[4][5][6][D]
 	[7][8][9][E]
 	[A][0][B][F]
 	Maps to:
 	[W][E][R][T]
 	[S][D][F][G]
 	[X][C][V][B]
 	[H][J][K][L]

 	Really sorry that I chose this bizarre scheme, the numpad on the keypad of the development machine was broken.  I do plan on rectifying the situation at some point.

 	Also!  Press Q to force the CPU into an error state, causing it to quit.  The error will be reported as 08 (EUSRQ)

## CPU Error Code Table

| Error Code | Name  | Description                   |
| :--------: | ----- | ----------------------------- |
|     0      | ENONE | The default state, of course. |
|     1      | EPCOF | Program Counter Overflow.  The ROM file may be corrupt or invalid.  If you get this error a lot, please report it here on github.com/cjbrowne/Chip8. |
|     2      | EPCUF | Program Counter Underflow.  Again, this is probably caused by a corrupt ROM file or a program that's somehow jumping to memory that doesn't exist on the Chip8 system.  It's a rare but possible error condition.  Note to Chip8 programmers: 0x200 is the cut off, if your program jumps to any address before 0x200, this is an error (it's used to store things like sprite data, in case you were wondering) |
|     3      | ESTOF | Stack Overflow.  This is most likely to be a programmer error.  The program has 'pushed' to the stack too many times without 'popping'.  Check your loops! |
|     4      | ESTUF | Stack Underflow.  This is again most likely to be a programmer error but is caused by the inverse - popping too many times without pushing. |
|     5      | EBDOP | An opcode was encountered that is invalid.  This almost always points to a corrupt or invalid ROM file, but it's also possible that your compiler/assembler is bugged. Of course, it could also be that this program is bugged, so please submit a detailed bug report (including the opcode in question which should be present in the debug output) here on github.com/cjbrowne/Chip8. |
|     6      | ENIMP | An opcode that has not yet been implemented was encountered.  This is definitely a bug in Chip8 and needs to be reported immediately.  There are no known opcodes that produce this error condition in the current build (it was used during development, mostly as a way to stop the CPU early during execution of a test program) |
|     7      | ESEGV | Segmentation Fault.  Yeah, no kidding!  The program has tried to read from or write to invalid memory.  Usually caused by opcode 0xF029 (LD sprite) with an invalid argument.  Definitely a bug in your program, although there is an outside chance you've given an invalid program or Chip8 is bugged.  If you're sure your program is good, submit a bug report.  |
|     8      | EUSRQ | User Quit.  The 'q' key was pressed. |