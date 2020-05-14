#ifndef CHIP8VM_H_INC
#define CHIP8VM_H_INC

typedef struct {
    unsigned short opcode;
    unsigned char memory[4096];
    unsigned char v[16];        // registers
    unsigned short index_reg;
    unsigned short pc;          // program counter
    unsigned char gfx[64 * 32]; // VRAM
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[16];
    unsigned char sp;           // stack pointer
    unsigned char key[16];      // keypad key states

    // flags go here?
    unsigned char draw_flag;
    unsigned char key_flag;
}
chip8_state;

chip8_state * create_state();
void load_rom(char *romfilename, chip8_state *state);
void unimplemented_opcode_err(unsigned short opcode);
void invalid_opcode(unsigned short pc, unsigned short opcode);
void emulate_opcode(chip8_state *state);
void dump_memory(chip8_state *state);
void dump_state(chip8_state *state);

#endif
