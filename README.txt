CHIP-8 Interpreter

Read about it on Wikipedia, but the gist is that it was a virtual machine made in the 1970s for a pair of computers for game programming, and has some hobbyist appeal.

It has just 35* opcodes, and a simple structure, so it's a recommended starting point for learning how to create an emulator, which is why I'm creating this emulator (which is technically/really a CHIP-8 interpreter, since it's a virtual machine). 

*35 distinct functions. Opcodes carry their operands, eg, 0x6XNN sets register VX to the byte NN (ie, 0x60FF sets V0 to FF)


Includes emulator/interpreter and disassembler. Only disassemble files with which you are allowed to do so. Disassembles in a rudimentary way, with 1:1 replacement and no separation of subroutines or labeling.


Specifications of CHIP-8:

Opcodes: 35, 2 bytes, no distinct operands, big-endian.

Memory: 4K x 1 byte. 0x000 - 0xFFF. 0x000-0x1FF were for the interpreter itself: ignorable except for 0x050-0x09F, which hold sprites for characters.

Registers: 16 8-bit registers, V0-VF. VF used as a flag for operation results.

16 bit index register (I): special register used to store memory locations.

16 bit program counter (PC): holds address of next instruction.

Stack: 16 levels, used only to store PC for subroutines. Stack pointer stores last position used.

2 Timers: a sound timer and a delay timer. Both can be set to values from registers, and count down at 60Hz. The delay timer is used for game event timing, and the sound timer is used for simple sound (a buzzing)

16 input keys: 0-F, either pressed or not pressed. Keyboard mapping to come.

Graphics: 64x32 px monochrome screen, sprite based graphics.


Files:
chip8vm.c -- CHIP-8 Emulator
disasm.c -- CHIP-8 bytecode disassembler (rudimentary)
