#include <stdio.h>

#include "chip8vm.h"
#include "testingsys.h"

// Reporting test results
int test_op(chip8_state *state,
        unsigned short t_val, unsigned short e_val, char dump)
{
    if (t_val == e_val)
    {
        printf(".");
        return 0;
    }
    else
    {
        printf("F\n");
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
// Set important values & opcode, emulate, test result
// dump: whether to dump memory & state on failed test, 0 or 1
int test_suite(chip8_state *state, unsigned char dump)
{
    unsigned short tested;
    // if i need more than this i am a) testing way too much
    // and b) and yet doing a horrible job
    unsigned char errors = 0;

    // Notes:
    // remember to set everything related to the test,
    //   you can't rely that v0 is still 0 eg, and it's
    //   good anyway
    // if i add the PC advancing to emulate_opcode that will
    //   cause lots of failures for any flow ops, except:
    unsigned short jumped = 0x202;


    // 0x0NNN: TODO
    
    // 0x00e0: TODO
    
    // 0x00ee: Return from subroutine
    printf("\n0x00ee: ");
    // Basic test
    state->opcode = 0x00ee;
    state->sp = 0x4;
    state->stack[state->sp] = 0x391;
    state->pc = 0x500;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x391, dump);
    tested = state->sp;
    errors += test_op(state, tested, 0x5, dump);
    // sp overflow
    state->opcode = 0x00ee;
    state->sp = 0xf;
    state->stack[state->sp] = 0x393;
    state->pc = 0x503;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x393, dump);
    tested = state->sp;
    errors += test_op(state, tested, 0x0, dump);
    
    // 0x1NNN: GOTO NNN
    // needs to set PC to NNN 
    printf("\n0x1NNN: ");
    state->opcode = 0x1234;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x234, dump);
    // test 2
    state->opcode = 0x1fff;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0xfff, dump);


    // 0x2NNN: TODO
    printf("\n0x2NNN: ");
    // Most normal:
    state->opcode = 0x2145;
    state->pc = 0x200;
    state->sp = 0xf;
    state->stack[0xe] = 0;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x145, dump);
    tested = state->sp;
    errors += test_op(state, tested, 0xe, dump);
    tested = state->stack[0xe];
    errors += test_op(state, tested, 0x200, dump);
    // decrementing sp == 0
    state->opcode = 0x2245;
    state->pc = 0x201;
    state->sp = 0x0;
    state->stack[0xf] = 0;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x245, dump);
    tested = state->sp;
    errors += test_op(state, tested, 0xf, dump);
    tested = state->stack[0xf];
    errors += test_op(state, tested, 0x201, dump);
    // overwriting stack
    state->opcode = 0x2354;
    state->pc = 0x300;
    state->sp = 0x0;
    state->stack[0xf] = 0x202;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x354, dump);
    tested = state->sp;
    errors += test_op(state, tested, 0xf, dump);
    tested = state->stack[0xf];
    errors += test_op(state, tested, 0x300, dump);

    // integrating 0x2NNN with 0x00ee (return)
    // And with the 0xf/0x0 boundary
    printf("\n0x2NNN & 0x00ee: ");
    state->opcode = 0x2356;
    state->pc = 0x302;
    state->sp = 0x0;
    state->stack[0xf] = 0x202;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x356, dump);
    tested = state->sp;
    errors += test_op(state, tested, 0xf, dump);
    tested = state->stack[0xf];
    errors += test_op(state, tested, 0x302, dump);
    state->opcode = 0x00ee;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x302, dump);
    tested = state->sp;
    errors += test_op(state, tested, 0x0, dump);
    

    // 0x3XNN:
    // Skip next instruction if value in VX == NN
    // (ie, increment PC by 2 so that with the usual advance
    //  it will skip over the next)
    printf("\n0x3XNN: ");
    // test positive
    state->opcode = 0x321a;
    state->v[2] = 0x1a;
    state->pc = 0x200;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, jumped, dump);
    // test negative
    state->opcode = 0x321b;
    state->v[2] = 0x1a;
    state->pc = 0x200;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x200, dump);


    // 0x4XNN: skip next instruction if VX != NN
    // (opposite of 0x3XNN)
    printf("\n0x4XNN: ");
    // test positive
    state->opcode = 0x441a;
    state->v[4] = 0x1b;
    state->pc = 0x200;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, jumped, dump);
    // test negative
    state->opcode = 0x441b;
    state->v[4] = 0x1b;
    state->pc = 0x200;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x200, dump);


    // 0x5XY0: Skip next instruction if VX == VY
    printf("\n0x5XY0: ");
    // test positive
    state->opcode = 0x5ab0;
    state->pc = 0x200;
    state->v[0xa] = 0x24;
    state->v[0xb] = 0x24;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, jumped, dump);
    // test neg
    state->opcode = 0x5fa0;
    state->pc = 0x200;
    state->v[0xa] = 0x6;
    state->v[0xf] = 0x9;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x200, dump);


    // 0x6XNN: set VX to NN
    // test 1
    printf("\n0x6XNN: ");
    state->opcode = 0x60ef;
    emulate_opcode(state);
    tested = state->v[0];
    errors += test_op(state, tested, 0xef, dump);
    // test 2
    state->opcode = 0x6fff;
    emulate_opcode(state);
    tested = state->v[0xf];
    errors += test_op(state, tested, 0xff, dump);


    // 0x7XNN: increment  VX by NN
    printf("\n0x7XNN: ");
    // standard
    state->opcode = 0x7301;
    state->v[3] = 0xcf;
    emulate_opcode(state);
    tested = state->v[3];
    errors += test_op(state, tested, 0xd0, dump);
    // overflow
    state->opcode = 0x7301;
    state->v[3] = 0xff;
    emulate_opcode(state);
    tested = state->v[3];
    errors += test_op(state, tested, 0x00, dump);
    // add 0
    state->opcode = 0x7300;
    state->v[3] = 0xff;
    emulate_opcode(state);
    tested = state->v[3];
    errors += test_op(state, tested, 0xff, dump);
    // add ff
    state->opcode = 0x73ff;
    state->v[3] = 0xff;
    emulate_opcode(state);
    tested = state->v[3];
    errors += test_op(state, tested, 0xfe, dump);


    // 0x8XY0: set VX to value of VY
    printf("\n0x8XY0: ");
    // test 1
    state->opcode = 0x8690;
    state->v[9] = 0xbb;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, tested, 0xbb, dump);


    // 0x8XY1: VX stores result of bitwise or with VY
    printf("\n0x8XY1: ");
    // 0x29 | 0xff = 0xff
    state->opcode = 0x8691;
    state->v[6] = 0x29;
    state->v[9] = 0xff;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, tested, 0xff, dump);
    // 0x29 | 0x00 = 0x29
    state->opcode = 0x8691;
    state->v[6] = 0x29;
    state->v[9] = 0x00;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, tested, 0x29, dump);
    // 0x29 | 0x57 = 0x7f
    state->opcode = 0x8691;
    state->v[6] = 0x29;
    state->v[9] = 0x57;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, tested, 0x7f, dump);


    // 0x8XY2: VX stores result of bitwise and with VY
    printf("\n0x8XY2: ");
    // 0x29 & 0xff = 0x29
    state->opcode = 0x8692;
    state->v[6] = 0x29;
    state->v[9] = 0xff;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, tested, 0x29, dump);
    // 0x29 & 0x00 = 0x00
    state->opcode = 0x8692;
    state->v[6] = 0x29;
    state->v[9] = 0x00;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, tested, 0x00, dump);
    // 0x29 & 0x57 = 0x01
    state->opcode = 0x8692;
    state->v[6] = 0x29;
    state->v[9] = 0x57;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, tested, 0x01, dump);



    // 0x8XY3: VX stores result of bitwise xor with VY
    printf("\n0x8XY3: ");
    // 0x29 ^ 0xff = 0xd6
    state->opcode = 0x8693;
    state->v[6] = 0x29;
    state->v[9] = 0xff;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, tested, 0xd6, dump);
    // 0x29 ^ 0x00 = 0x29
    state->opcode = 0x8693;
    state->v[6] = 0x29;
    state->v[9] = 0x00;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, tested, 0x29, dump);
    // 0x29 ^ 0x57 = 0x7e
    state->opcode = 0x8693;
    state->v[6] = 0x29;
    state->v[9] = 0x57;
    emulate_opcode(state);
    tested = state->v[6];
    errors += test_op(state, tested, 0x7e, dump);


    // 0x8XY4: Increment VX by VY
    printf("\n0x8XY4: ");
    // standard
    state->opcode = 0x8104;
    state->v[1] = 0x32;
    state->v[0] = 0x10;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, tested, 0x42, dump);
    // with overflow:
    state->opcode = 0x8104;
    state->v[1] = 0x43;
    state->v[0] = 0xff;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, tested, 0x42, dump);


    // 0x8XY5: Decrement VX by VY
    printf("\n0x8XY5: ");
    // standard
    state->opcode = 0x8105;
    state->v[1] = 0x32;
    state->v[0] = 0x10;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, tested, 0x22, dump);
    // with underflow:
    state->opcode = 0x8105;
    state->v[1] = 0x43;
    state->v[0] = 0xff;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, tested, 0x44, dump);


    // 0x8XY6: Bitshift VY right by 1 and store in VX
    // (I'm using original CHIP-8 rules)
    // Store VY's LSB pre-shift in VF
    printf("\n0x8XY6: ");
    // LSB odd
    state->opcode = 0x8106;
    state->v[0] = 0xda;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, tested, 0x6d, dump);
    tested = state->v[0xf];
    errors += test_op(state, tested, 0x00, dump);
    // LSB even
    state->opcode = 0x8106;
    state->v[0] = 0xdb;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, tested, 0x6d, dump);
    tested = state->v[0xf];
    errors += test_op(state, tested, 0x01, dump);
    // VX 0xff
    state->opcode = 0x8106;
    state->v[0] = 0xff;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, tested, 0x7f, dump);
    tested = state->v[0xf];
    errors += test_op(state, tested, 0x01, dump);


    // 0x8XY7: Set VX to VY - VX
    printf("\n0x8XY7: ");
    // standard
    state->opcode = 0x8017;
    state->v[1] = 0x32;
    state->v[0] = 0x10;
    emulate_opcode(state);
    tested = state->v[0];
    errors += test_op(state, tested, 0x22, dump);
    // with underflow:
    state->opcode = 0x8017;
    state->v[1] = 0x43;
    state->v[0] = 0xff;
    emulate_opcode(state);
    tested = state->v[0];
    errors += test_op(state, tested, 0x44, dump);


    // 0x8XYe: Bitshift VY left by 1 and store in VX
    // (I'm using original CHIP-8 rules)
    // Store VY's MSB pre-shift in VF
    printf("\n0x8XYe: ");
    // MSB odd, overflows
    state->opcode = 0x810e;
    state->v[0] = 0xda;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, tested, 0xb4, dump);
    tested = state->v[0xf];
    errors += test_op(state, tested, 0x01, dump);
    // MSB even
    state->opcode = 0x810e;
    state->v[0] = 0x2b;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, tested, 0x56, dump);
    tested = state->v[0xf];
    errors += test_op(state, tested, 0x00, dump);
    // VX 0xff
    state->opcode = 0x810e;
    state->v[0] = 0xff;
    emulate_opcode(state);
    tested = state->v[1];
    errors += test_op(state, tested, 0xfe, dump);
    tested = state->v[0xf];
    errors += test_op(state, tested, 0x01, dump);


    // 0x9XY0: Skip next instruction if VX != VY
    printf("\n0x9XY0: ");
    // test positive
    state->opcode = 0x9ab0;
    state->pc = 0x200;
    state->v[0xa] = 0x44;
    state->v[0xb] = 0x24;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, jumped, dump);
    // test neg
    state->opcode = 0x9fa0;
    state->pc = 0x200;
    state->v[0xa] = 0x9;
    state->v[0xf] = 0x9;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x200, dump);
    

    // 0xANNN: Sets I (index register) to the address NNN
    printf("\n0xaNNN: ");
    state->opcode = 0xaf34;
    state->index_reg = 0x0;
    emulate_opcode(state);
    tested = state->index_reg;
    errors += test_op(state, tested, 0xf34, dump);


    // 0xBNNN: Jump (set PC) to address NNN + V0
    printf("\n0xbNNN: ");
    // Straightforward:
    state->opcode = 0xb100;
    state->v[0] = 0x11;
    state->pc = 0x0;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x111, dump);
    // large NNN
    state->opcode = 0xbfff;
    state->v[0] = 0x11;
    state->pc = 0x0;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x010, dump);
    // large V0 and NNN
    state->opcode = 0xbfff;
    state->v[0] = 0xff;
    state->pc = 0x0;
    emulate_opcode(state);
    tested = state->pc;
    errors += test_op(state, tested, 0x0fe, dump);


    // 0xcXNN: Set VX to bitwise and of NN and random value 0-255
    // TODO
    //
    // 0xdXYN: Display TODO
    //
    // 0xeX9e: keyop TODO
    // 0xeXa1: keyop TODO


    // 0xfX07: Set VX to value of delay timer
    printf("\n0xfX07: ");
    state->opcode = 0xf207;
    state->v[2] = 0xff;
    state->delay_timer = 0x40;
    emulate_opcode(state);
    tested = state->v[2];
    errors += test_op(state, tested, 0x40, dump);


    // fx0a keyop TODO


    // 0xfX15: Set delay timer to value in VX
    printf("\n0xfX15: ");
    state->opcode = 0xf215;
    state->v[2] = 0x40;
    state->delay_timer = 0xff;
    emulate_opcode(state);
    tested = state->delay_timer;
    errors += test_op(state, tested, 0x40, dump);


    // 0xfX18: Set sound timer to VX
    printf("\n0xfX18: ");
    state->opcode = 0xf218;
    state->v[2] = 0x40;
    state->sound_timer = 0xff;
    emulate_opcode(state);
    tested = state->sound_timer;
    errors += test_op(state, tested, 0x40, dump);


    // 0xfX1e: Increment I by VX, set VF to 1 if overflow, else 0
    printf("\n0xfX1e: ");
    // No overflow:
    state->opcode = 0xf21e;
    state->v[2] = 0x40;
    state->index_reg = 0x002;
    emulate_opcode(state);
    tested = state->index_reg;
    errors += test_op(state, tested, 0x42, dump);
    tested = state->v[0xf];
    errors += test_op(state, tested, 0x0, dump);
    // Overflow large I:
    state->opcode = 0xf21e;
    state->v[2] = 0x40;
    state->index_reg = 0xfff;
    emulate_opcode(state);
    tested = state->index_reg;
    errors += test_op(state, tested, 0x03f, dump);
    tested = state->v[0xf];
    errors += test_op(state, tested, 0x1, dump);



    // fX29: set I to address for sprite of value in VX TODO
    // fX33: store BCD of VX starting at I TODO
    // fx55: register dump TODO
    // fx65: register load TODO




    printf("\n");
    return errors;
}
