#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset, memcpy

#include "chip8vm.h"
#include "testingsys.h"


int main(int argc, char *argv[]){
    // Ensure that we're being used with what we'll assume is a romfile
    if (argc != 2)
    {
        printf("Usage: chip8vm <romfile>\n");
        exit(1);
    }

    // Create and initialize a state struct
    chip8_state *state = create_state();

    // Run tests with -t
    if (strcmp(argv[1], "-t") == 0)
    {
        int errors = test_suite(state);
        printf("TOTAL ERRORS: %i\n", errors);
        exit(0);
    }


    // Load given romfile into VM memory
    // BYPASSED FOR TESTING
    load_rom(argv[1], state);
    // END BYPASS

    // these ive snagged and will go into advancing afterwards
    // unsigned short opcode = memory[pc] << 8 | memory[pc + 1];
    // pc += 2;

    // we'll peek into memory again later, when we start
    // emulating things to do with it
    // dump_memory(state);


    // Dump useful state variables (rather, ones that aren't
    // huge arrays of memory)
    // dump_state(state);

    // Destroy the state
    free(state);
}


chip8_state * create_state(void)
{
    //  Create a pointer to a state, so we can modify it in functions
    chip8_state *state = malloc(sizeof(chip8_state));
    // Initialize important fields
    // It's not important for say, registers to be cleared
    // and graphics will undoubtably be immediately cleared with 0x00e0 op 
    memset(state->memory, 0, sizeof(state->memory));
    state->pc = 0x200;
    state->sp = 0xf;
    memset(state->key, 0, sizeof(state->key));
    return state;
}


// Load a rom into vm memory
void load_rom(char *romfilename, chip8_state *state)
{
    // Open a romfile
    FILE *romfile = fopen(romfilename, "r");
    if (romfile == NULL)
    {
        printf("Could not open file: %s\n", romfilename);
        exit(1);
    }

    // Fill our memory with program data, starting at 0x200
    // 0x1000 total memory - 0x200 reserved = 0xe00 for rom 
    fread(state->memory + 0x200, 1, 0xe00, romfile);
}

void unimplemented_opcode_err(unsigned short opcode)
{
    printf("ERROR!\nUnimplemented Opcode: %04x\n", opcode);
    exit(1);
}


void invalid_opcode(unsigned short pc, unsigned short opcode)
{
    printf("Invalid opcode at 0x%04x\n", pc);
    printf("Opcode: %04x\n", opcode);
    exit(1);
}



// Decode & emulate opcode. Mainly grouped by first nibble.
void emulate_opcode(chip8_state *state)
{
    // placeholder until actually emulating
    unsigned short pc = 0;
    
    unsigned short opcode = state->opcode;

    // Getting X & Y from opcodes is tricky and having a var
    // is very nice, so we declare them up here.
    // these are the values, vx = 7 means register x is storing 7
    // x & y are the indices. x = 7 means register 7
    unsigned char vx, vy;
    unsigned char x = (opcode & 0xf00) >> 8;
    unsigned char y = (opcode & 0x0f0) >> 4;

    // NN & NNN are just masks w/o shifts so they dont need them

    // Get just first nibble
    // Shifts are 4 * nibbles moved bc 4 bits to a nibble
    // (ie, this first shifts 0xa000 to 0xa by shifting 3*4 = 12)
    switch ((opcode & 0xf000)>> 12)
    {
        case 0x0:
            if (opcode == 0x00e0)
                // Clear screen
                unimplemented_opcode_err(opcode);
            else if (opcode == 0x00ee)
            {
                // 0x00ee: Return from subroutine
                state->pc = state->stack[state->sp];
                state->sp == 0xf ? state->sp = 0x0 : state->sp++;
            }
            else
                // Call RCA 1802 program (probably don't need)
                unimplemented_opcode_err(opcode);
            break;
        case 0x1:
            // 1NNN: GOTO NNN
            state->pc = opcode & 0xfff;
            break;
        case 0x2:
            // 2NNN: Call subroutine
            state->sp == 0 ? state->sp = 0xf : state->sp--;
            state->stack[state->sp] = state->pc;
            state->pc = opcode & 0xfff;
            break;
        case 0x3:
            // Skip next instruction if VX == NN
            vx  = state->v[x];
            if (vx == (opcode & 0xff))
                state->pc += 2;
            break;
        case 0x4:
            // Skip next instruction if VX != NN
            vx  = state->v[x];
            if (vx != (opcode & 0xff))
                state->pc += 2;
            break;
        case 0x5:
            // Skip next instruction if VX == VY
            // opcode must end in 0 or isn't valid
            if ((opcode & 0xf) != 0)
                invalid_opcode(pc, opcode);
            // else
            vx  = state->v[x];
            vy  = state->v[y];
            if (vx == vy)
                state->pc += 2;
            break;
        case 0x6:
            // Sets VX to NN
            state->v[x] = (opcode & 0xff);
            break;
        case 0x7:
            // Increment VX by NN
            vx = state->v[x];
            state->v[x] = (vx + (opcode & 0xff)) & 0xff;
            break;
        case 0x8:
            // 0x8??? is messier than the neat categories so far.
            // operates on VX and VY depending on last hex digit
            switch (opcode & 0xf)
            {
                case 0x0:
                    // Assign VX value in VY
                    vy = state->v[y];
                    state->v[x] = vy;
                    break;
                case 0x1:
                    // VX = VX | VY
                    vx = state->v[x];
                    vy = state->v[y];
                    state->v[x] = (vx | vy);
                    break;
                case 0x2:
                    // VX = VX & VY
                    vx = state->v[x];
                    vy = state->v[y];
                    state->v[x] = (vx & vy);
                    break;
                case 0x3:
                    // VX = VX ^ VY
                    vx = state->v[x];
                    vy = state->v[y];
                    state->v[x] = (vx ^ vy);
                    break;
                case 0x4:
                    // Increment VX by VY
                    vx = state->v[x];
                    vy = state->v[y];
                    state->v[x] = (vx + vy) & 0xff;
                    break;
                case 0x5:
                    // Decrement VX by VY
                    vx = state->v[x];
                    vy = state->v[y];
                    state->v[x] = (vx - vy) & 0xff;
                    break;
                case 0x6:
                    // Shift VY right by one and set into VX
                    // Set VF to VY's pre shift LSB
                    // This behavior changed in 48 and Super
                    state->v[0xf] = state->v[y] & 1;
                    state->v[x] = (state->v[y] >> 1);
                    break;
                case 0x7:
                    // VX = VY - VX
                    vx = state->v[x];
                    vy = state->v[y];
                    state->v[x] = (vy - vx) & 0xff;
                    break;
                // No 0x8XY8 - 0x8XYd, or *f
                case 0xe:
                    // Shifts VY left by one and stores in VX
                    // SET VF to VY's pre shift MSB
                    // Like 0x8XY6, was patched in -48 and Super
                    state->v[0xf] = (state->v[y] & 0x80) >> 7;
                    state->v[x] = (state->v[y] << 1) & 0xff;
                    break;
                default:
                    invalid_opcode(pc, opcode);
            }
            break;
        // (Back to first nibble decoding)
        case 0x9:
            // Skip next instruction if VX != VY
            // opcode must end in 0 or isn't valid
            if ((opcode & 0xf) != 0)
                invalid_opcode(pc, opcode);
            // else
            vx  = state->v[x];
            vy  = state->v[y];
            if (vx != vy)
                state->pc += 2;
            break;
        case 0xa:
            // Set index register (I) to adress NNN
            state->index_reg = opcode & 0xfff;
            break;
        case 0xb:
            // Jump PC to address V0 + NNN
            vx = state->v[0];
            state->pc = (vx + (opcode & 0xfff)) & 0xfff;
            break;
        case 0xc:
            // Set VX to random number between 0 and 255,
            // bitmasked with NN
            unimplemented_opcode_err(opcode);
            break;
        case 0xd:
            // 0xdXYN
            // Draw sprite of N height at address in I
            // to coords VX, VY (all sprites are 8 bits wide)
            unimplemented_opcode_err(opcode);
            break;
        case 0xe:
            // Skip next instruction if key stored in VX is pressed:
            if ((opcode & 0xff) == 0x9e)
                unimplemented_opcode_err(opcode);
            // Skip next instruction if key NOT pressed:
            else if ((opcode & 0xff) == 0xa1)
                unimplemented_opcode_err(opcode);
            else
                invalid_opcode(pc, opcode);
            break;
        case 0xf:
            // Another messy one, depends on last 2 digits
            switch (opcode & 0xff)
            {
                case 0x07:
                    // 0xfX07: Set VX to value of delay timer
                    state->v[x] = state->delay_timer;
                    break;
                case 0x0a:
                    // Wait for keypress, then store it in VX
                    // Blocking operation
                    unimplemented_opcode_err(opcode);
                    break;
                case 0x15:
                    // 0xfX15: Set delay timer to VX
                    state->delay_timer = state->v[x];
                    break;
                case 0x18:
                    // 0xfX18: Set sound timer to VX
                    state->sound_timer = state->v[x];
                    break;
                case 0x1e:
                    // Add VX to I Set VF to if overflowed
                    vx = state->v[x];
                    state->index_reg = state->index_reg + vx;
                    // I > 0xfff iff overflow happened
                    state->v[0xf] = state->index_reg > 0xfff ? 1 : 0;
                    state->index_reg &= 0xfff;
                    break;
                case 0x29:
                    // sets I to the built-in sprite address for the
                    // character stored in VX
                    unimplemented_opcode_err(opcode);
                    break;
                case 0x33:
                    // Stores BCD of VX starting at I
                    unimplemented_opcode_err(opcode);
                    break;
                case 0x55:
                    // Stores registers V0 to & incl. VX into memory
                    unimplemented_opcode_err(opcode);
                    break;
                case 0x65:
                    // Loads registers v0 to & incl. VX from memory
                    unimplemented_opcode_err(opcode);
                    break;
                default:
                    // Invalid opcode starting with 0xf
                    invalid_opcode(pc, opcode);
            }
            break;
    }
}



// Dump memory contents to console.
// Useful now as a display of results, later as debugging tool
void dump_memory(chip8_state *state)
{
    // For each 16 byte "block"
    for (int i = 0; i < 256; i++)
    {
        // Print the address of the first byte
        printf("%03x: ", i * 16);
        // Possibility: check against previous line, if same, then
        // do *** for one line, and pick back up when memory is different
        for (int j = 0; j < 16; j++)
        {
            // Print the jth byte in this ith 16 byte block
            printf("%02x ", state->memory[i * 16 + j]);
        }
        printf("\n");
    }
    printf("Memory dumped\n");
}


// Dump most of the state variables
// excludes: memory, gfx, key
// key could be in this
void dump_state(chip8_state *state)
{
    printf("Curr Opcode: %04x\n", state->opcode);
    printf("Registers:\n");
    for (int i = 0; i < 16; i++)
    {
        printf("    V%i: %02x\n", i, state->v[i]);
    }
    printf("Index register: %04x\n", state->index_reg);
    printf("PC: %04x\n", state->pc);
    printf("Timers: Delay: %02x\n", state->delay_timer);
    printf("        Sound: %02x\n", state->sound_timer);
    printf("Stack:\n");
    for (int i = 0; i < 16; i++)
    {
        printf("    %02i:  %04x\n", i, state->stack[i]);
    }
    printf("SP: %i\n", state->sp);
}
