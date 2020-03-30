#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset, memcpy

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
}
chip8_state;


chip8_state * create_state();
void load_rom(char *romfilename, chip8_state *state);
void unimplemented_opcode_err(unsigned short opcode);
void invalid_opcode(unsigned short pc, unsigned short opcode);
void emulate_opcode(chip8_state *state);
void dump_memory(chip8_state *state);
void dump_state(chip8_state *state);
int test_op(chip8_state *state, unsigned char test_no, unsigned short t_val, unsigned short e_val, char dump);
int test_suite(chip8_state *state);


int main(int argc, char *argv[]){
    // Ensure that we're being used with what we'll assume is a romfile
    if (argc != 2)
    {
        printf("Usage: chip8vm <romfile>\n");
        exit(1);
    }

    // Create and initialize a state struct
    chip8_state *state = create_state();

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
    state->sp = 0x0;
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
                // Return from subroutine
                unimplemented_opcode_err(opcode);
            else
                // Call RCA 1802 program (probably don't need)
                unimplemented_opcode_err(opcode);
            break;
        case 0x1:
            // 1NNN: GOTO NNN
            state->pc = opcode & 0xfff;
            break;
        case 0x2:
            // Call subroutine
            unimplemented_opcode_err(opcode);
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
            if ((opcode & 0xf) != 0)
                invalid_opcode(pc, opcode);
            // Otherwise, 0x9XY0 is skip next instruction if VX == VY
            unimplemented_opcode_err(opcode);
            break;
        case 0xa:
            // Set index register (I) to adress NNN
            unimplemented_opcode_err(opcode);
            break;
        case 0xb:
            // Jump PC to address V0 + NNN
            unimplemented_opcode_err(opcode);
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
                    // Set VX to value of delay timer
                    unimplemented_opcode_err(opcode);
                    break;
                case 0x0a:
                    // Wait for keypress, then store it in VX
                    // Blocking operation
                    unimplemented_opcode_err(opcode);
                    break;
                case 0x15:
                    // Set delay timer to VX
                    unimplemented_opcode_err(opcode);
                    break;
                case 0x18:
                    // Set sound timer to VX
                    unimplemented_opcode_err(opcode);
                    break;
                case 0x1e:
                    // Add VX to I Set VF to if overflowed
                    unimplemented_opcode_err(opcode);
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


int test_op(chip8_state *state, unsigned char test_no,
        unsigned short t_val, unsigned short e_val, char dump)
{
    printf("test %03i: ", test_no);
    if (t_val == e_val)
    {
        printf("success\n");
        return 0;
    }
    else
    {
        printf("FAILURE:\n");
        printf("Opcode: %04x\n", state->opcode);
        printf("Expected: 0x%04x\n", e_val);
        printf("Actual:   0x%04x\n", t_val);
        if (dump == 1)
        {
            dump_memory(state);
            dump_state(state);
        }
        return 1;
    }
}



// Just a big battery of tests in sequence. 
int test_suite(chip8_state *state)
{
    unsigned short tested;
    // if i need more than this i am a) testing way too much
    // and b) and yet doing a horrible job
    unsigned char test_no = 0;
    unsigned char errors = 0;
    unsigned char dump = 0;

    // Notes:
    // remember to set everything related to the test,
    //   you can't rely that v0 is still 0 eg, and it's
    //   good anyway
    // if i add the PC advancing to emulate_opcode that will
    //   cause lots of failures for any flow ops, except:
    unsigned short jumped = 0x202;


    // 0x0NNN: NO TESTS
    
    // 0x00e0: NO TESTS
    
    // 0x00ee: NO TESTS
    
    
    // 0x1NNN: GOTO NNN
    // needs to set PC to NNN 
    printf("0x1NNN\n");

    state->opcode = 0x1234;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, test_no++, tested, 0x234, dump);

    state->opcode = 0x1fff;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, test_no++, tested, 0xfff, dump);


    // 0x2NNN: NO TESTS
    

    // 0x3XNN:
    // Skip next instruction if value in VX == NN
    // (ie, increment PC by 2 so that with the usual advance
    //  it will skip over the next)
    printf("0x3XNN\n");
    
    // test positive
    state->opcode = 0x321a;
    state->v[2] = 0x1a;
    state->pc = 0x200;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, test_no++, tested, jumped, dump);

    // test negative
    state->opcode = 0x321b;
    state->v[2] = 0x1a;
    state->pc = 0x200;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, test_no++, tested, 0x200, dump);


    // 0x4XNN: skip next instruction if VX != NN
    // (opposite of 0x3XNN)
    printf("0x4XNN\n");
    
    // test positive
    state->opcode = 0x441a;
    state->v[4] = 0x1b;
    state->pc = 0x200;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, test_no++, tested, jumped, dump);

    // test negative
    state->opcode = 0x441b;
    state->v[4] = 0x1b;
    state->pc = 0x200;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, test_no++, tested, 0x200, dump);


    // 0x5XY0: Skip next instruction if VX == VY
    printf("0x5XY0\n");
    
    // test positive
    state->opcode = 0x5ab0;
    state->pc = 0x200;
    state->v[0xa] = 0x24;
    state->v[0xb] = 0x24;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, test_no++, tested, jumped, dump);

    // test neg
    state->opcode = 0x5fa0;
    state->pc = 0x200;
    state->v[0xa] = 0x6;
    state->v[0xf] = 0x9;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, test_no++, tested, 0x200, dump);


    // 0x6XNN: set VX to NN
    printf("0x6XNN\n");
    
    state->opcode = 0x60ef;
    emulate_opcode(state);
    tested = state->v[0];
    errors += test_op(state, test_no++, tested, 0xef, dump);

    state->opcode = 0x6fff;
    emulate_opcode(state);
    tested = state->v[0xf];
    errors += test_op(state, test_no++, tested, 0xff, dump);


    // 0x7XNN: increment  VX by NN
    printf("0x7XNN\n");
    
    state->opcode = 0x7301;
    state->v[3] = 0xcf;
    emulate_opcode(state);
    tested = state->v[3];
    errors += test_op(state, test_no++, tested, 0xd0, dump);

    // overflow
    state->opcode = 0x7301;
    state->v[3] = 0xff;
    emulate_opcode(state);
    tested = state->v[3];
    errors += test_op(state, test_no++, tested, 0x00, dump);

    // add 0
    state->opcode = 0x7300;
    state->v[3] = 0xff;
    emulate_opcode(state);
    tested = state->v[3];
    errors += test_op(state, test_no++, tested, 0xff, dump);
    
    // add ff
    state->opcode = 0x73ff;
    state->v[3] = 0xff;
    emulate_opcode(state);
    tested = state->v[3];
    errors += test_op(state, test_no++, tested, 0xfe, dump);


    // 0x8XY0: set VX to value of VY
    printf("0x8XY0\n");

    state->opcode = 0x8690;
    state->v[9] = 0xbb;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, test_no++, tested, 0xbb, dump);


    // 0x8XY1: VX stores result of bitwise or with VY
    printf("0x8XY1\n");

    // 0x29 | 0xff = 0xff
    state->opcode = 0x8691;
    state->v[6] = 0x29;
    state->v[9] = 0xff;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, test_no++, tested, 0xff, dump);

    // 0x29 | 0x00 = 0x29
    state->opcode = 0x8691;
    state->v[6] = 0x29;
    state->v[9] = 0x00;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, test_no++, tested, 0x29, dump);

    // 0x29 | 0x57 = 0x7f
    state->opcode = 0x8691;
    state->v[6] = 0x29;
    state->v[9] = 0x57;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, test_no++, tested, 0x7f, dump);


    // 0x8XY2: VX stores result of bitwise and with VY
    printf("0x8XY2\n");

    // 0x29 & 0xff = 0x29
    state->opcode = 0x8692;
    state->v[6] = 0x29;
    state->v[9] = 0xff;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, test_no++, tested, 0x29, dump);

    // 0x29 & 0x00 = 0x00
    state->opcode = 0x8692;
    state->v[6] = 0x29;
    state->v[9] = 0x00;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, test_no++, tested, 0x00, dump);

    // 0x29 & 0x57 = 0x01
    state->opcode = 0x8692;
    state->v[6] = 0x29;
    state->v[9] = 0x57;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, test_no++, tested, 0x01, dump);



    // 0x8XY3: VX stores result of bitwise xor with VY
    printf("0x8XY3\n");

    // 0x29 ^ 0xff = 0xd6
    state->opcode = 0x8693;
    state->v[6] = 0x29;
    state->v[9] = 0xff;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, test_no++, tested, 0xd6, dump);

    // 0x29 ^ 0x00 = 0x29
    state->opcode = 0x8693;
    state->v[6] = 0x29;
    state->v[9] = 0x00;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, test_no++, tested, 0x29, dump);

    // 0x29 ^ 0x57 = 0x7e
    state->opcode = 0x8693;
    state->v[6] = 0x29;
    state->v[9] = 0x57;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, test_no++, tested, 0x7e, dump);


    // 0x8XY4: Increment VX by VY
    printf("0x8XY4\n");

    state->opcode = 0x8104;
    state->v[1] = 0x32;
    state->v[0] = 0x10;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, test_no++, tested, 0x42, dump);

    // with overflow:
    state->opcode = 0x8104;
    state->v[1] = 0x43;
    state->v[0] = 0xff;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, test_no++, tested, 0x42, dump);


    // 0x8XY5: Decrement VX by VY
    printf("0x8XY5\n");

    state->opcode = 0x8105;
    state->v[1] = 0x32;
    state->v[0] = 0x10;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, test_no++, tested, 0x22, dump);

    // with underflow:
    state->opcode = 0x8105;
    state->v[1] = 0x43;
    state->v[0] = 0xff;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, test_no++, tested, 0x44, dump);


    // 0x8XY6: Bitshift VY right by 1 and store in VX
    // (I'm using original CHIP-8 rules)
    // Store VY's LSB pre-shift in VF
    printf("0x8XY6\n");

    // LSB odd
    state->opcode = 0x8106;
    state->v[0] = 0xda;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, test_no++, tested, 0x6d, dump);
    tested = state->v[0xf];
    errors += test_op(state, test_no++, tested, 0x00, dump);

    // LSB even
    state->opcode = 0x8106;
    state->v[0] = 0xdb;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, test_no++, tested, 0x6d, dump);
    tested = state->v[0xf];
    errors += test_op(state, test_no++, tested, 0x01, dump);

    // VX 0xff
    state->opcode = 0x8106;
    state->v[0] = 0xff;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, test_no++, tested, 0x7f, dump);
    tested = state->v[0xf];
    errors += test_op(state, test_no++, tested, 0x01, dump);


    // 0x8XY7: Set VX to VY - VX
    printf("0x8XY7\n");

    state->opcode = 0x8017;
    state->v[1] = 0x32;
    state->v[0] = 0x10;
    emulate_opcode(state);
    tested = state->v[0];
    errors += test_op(state, test_no++, tested, 0x22, dump);

    // with underflow:
    state->opcode = 0x8017;
    state->v[1] = 0x43;
    state->v[0] = 0xff;
    emulate_opcode(state);
    tested = state->v[0];
    errors += test_op(state, test_no++, tested, 0x44, dump);


    // 0x8XYe: Bitshift VY left by 1 and store in VX
    // (I'm using original CHIP-8 rules)
    // Store VY's MSB pre-shift in VF
    printf("0x8XYe\n");

    // MSB odd, overflows
    state->opcode = 0x810e;
    state->v[0] = 0xda;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, test_no++, tested, 0xb4, dump);
    tested = state->v[0xf];
    errors += test_op(state, test_no++, tested, 0x01, dump);

    // MSB even
    state->opcode = 0x810e;
    state->v[0] = 0x2b;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, test_no++, tested, 0x56, dump);
    tested = state->v[0xf];
    errors += test_op(state, test_no++, tested, 0x00, dump);

    // VX 0xff
    state->opcode = 0x810e;
    state->v[0] = 0xff;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, test_no++, tested, 0xfe, dump);
    tested = state->v[0xf];
    errors += test_op(state, test_no++, tested, 0x01, dump);
    

    return errors;
}

