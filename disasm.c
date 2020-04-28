#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset

void invalid_opcode(int address, unsigned short opcode);

int main(int argc, char *argv[]){
    if (argc < 2)
    {
        printf("Usage: chip8vm <romfile> [offset]\n");
        exit(1);
    }

    // start disassembling from different offset
    // helpful to begin disassembly again after finding data
    int offset = 0;
    if (argc == 3)
    {
        sscanf(argv[2], "%x", &offset);
    }


    // create ram
    unsigned char memory[4096];
    memset(memory, 0, sizeof(memory));

    // Open a romfile
    FILE *romfile = fopen(argv[1], "r");
    if (romfile == NULL)
    {
        printf("Could not open file: %s\n", argv[1]);
        exit(1);
    }
    // determine length of program data:
    // add 0x200 for reserved memory
    fseek(romfile, 0L, SEEK_END);
    int end_byte = ftell(romfile) + 0x200;
    fseek(romfile, 0L, SEEK_SET);

    // load into ram.
    fread(memory + 0x200, 1, 0xe00, romfile);

    fclose(romfile);

    // Dump our memory contents to the console (TEMP)
    printf("0x000 - 0x1ff: intended to be reserved\n");
    printf("Offset from 0x200 is: %3x\n", offset);
    int pc = 0x200 + offset;
    int end_of_rom = (end_byte < 4096) ? end_byte : 4096;
    while (pc < end_of_rom)
    {
        // Print address line
        printf("%03x: ", pc);

        // Decode opcode. Mainly grouped by first nibble.
        unsigned short opcode = memory[pc] << 8 | memory[pc + 1];
        pc += 2;

        // Get just first nibble
        // Shifts are 4 * nibbles moved bc 4 bits to a nibble
        // (ie, this first shifts 0xa000 to 0xa by shifting 3*4 = 12)
        switch ((opcode & 0xf000)>> 12)
        {
            case 0x0:
                if (opcode == 0x00e0)
                    // Clear screen
                    printf("CLRS\n");
                else if (opcode == 0x00ee)
                    // Return from subroutine
                    printf("RET\n");
                else
                    printf("CALLPROG $%03x\n", opcode & 0xfff);
                break;
            case 0x1:
                // calls RCA 1802 program at address. Unlikely to see in ROM
                printf("GOTO $%03x\n", opcode & 0xfff);
                break;
            case 0x2:
                // Call subroutine
                printf("CALL $%03x\n", opcode & 0xfff);
                break;
            case 0x3:
                // Skip next instruction if VX == NN
                printf("TEQ.I V%x $%02x\n",
                        (opcode & 0xf00) >> 8,
                        opcode & 0xff);
                break;
            case 0x4:
                // Skip next instruction if VX != NN
                printf("TNE.I V%x $%02x\n",
                        (opcode & 0xf00) >> 8,
                        opcode & 0xff);
                break;
            case 0x5:
                // Skip next instruction if VX == VY
                // opcode must end in 0 or isn't valid
                if ((opcode & 0xf) != 0)
                    invalid_opcode(pc, opcode);
                printf("TEQ V%x V%x\n",
                        (opcode & 0xf00) >> 8,
                        (opcode & 0xf0) >> 4);
                break;
            case 0x6:
                // Sets VX to NN
                printf("MOV.I V%x $%02x\n",
                        (opcode & 0xf00) >> 8,
                        opcode & 0xff);
                break;
            case 0x7:
                // Increment VX by NN
                printf("INC.I V%x $%02x\n",
                        (opcode & 0xf00) >> 8,
                        opcode & 0xff);
                break;
            case 0x8:
                // 0x8??? gets messier than the very neat categories so far.
                // and operate on VX and VY depending on last hex digit
                switch (opcode & 0xf)
                {
                    case 0x0:
                        // Assign VX value in VY
                        printf("MOV.V V%x V%x\n",
                                (opcode & 0xf00) >> 8,
                                (opcode & 0xf0) >> 4);
                        break;
                    case 0x1:
                        // VX = VX | VY
                        printf("OR V%x V%x\n",
                                (opcode & 0xf00) >> 8,
                                (opcode & 0xf0) >> 4);
                        break;
                    case 0x2:
                        // VX = VX & VY
                        printf("AND V%x V%x\n",
                                (opcode & 0xf00) >> 8,
                                (opcode & 0xf0) >> 4);
                        break;
                    case 0x3:
                        // VX = VX ^ VY
                        printf("XOR V%x V%x\n",
                                (opcode & 0xf00) >> 8,
                                (opcode & 0xf0) >> 4);
                        break;
                    case 0x4:
                        // Increment VX by VY
                        printf("INC.V V%x V%x\n",
                                (opcode & 0xf00) >> 8,
                                (opcode & 0xf0) >> 4);
                        break;
                    case 0x5:
                        // Decrement VX by VY
                        printf("SUB V%x V%x\n",
                                (opcode & 0xf00) >> 8,
                                (opcode & 0xf0) >> 4);
                        break;
                    case 0x6:
                        // Shift VY right by one and set into VX
                        // This behavior changed in 48 and Super
                        printf("SHR V%x V%x\n",
                                (opcode & 0xf00) >> 8,
                                (opcode & 0xf0) >> 4);
                        break;
                    case 0x7:
                        // VX = VY - VX
                        printf("LESS V%x V%x\n",
                                (opcode & 0xf00) >> 8,
                                (opcode & 0xf0) >> 4);
                        break;
                    // No 0x8XY8 - 0x8XYd, or *f
                    case 0xe:
                        // Shifts VY left by one and stores in VX
                        // Like 0x8XY6, was patched in -48 and Super
                        printf("SHL V%x V%x\n",
                                (opcode & 0xf00) >> 8,
                                (opcode & 0xf0) >> 4);
                        break;
                    default:
                        invalid_opcode(pc, opcode);
                }
            // (Back to first nibble decoding)
            case 0x9:
                if ((opcode & 0xf) != 0)
                {
                    invalid_opcode(pc, opcode);
                }
                // Otherwise, 0x9XY0 is skip next instruction if VX != VY
                printf("TNE V%x V%x\n",
                        (opcode & 0xf00) >> 8,
                        (opcode & 0xf0) >> 4);
                break;
            case 0xa:
                // Set index register (I) to adress NNN
                printf("INDEX $%03x\n", opcode & 0xfff);
                break;
            case 0xb:
                // Jump PC to address V0 + NNN
                printf("JMPOFF $%03x\n", opcode & 0xfff);
                break;
            case 0xc:
                // Set VX to random number between 0 and 255,
                // bitmasked with NN
                printf("RAND V%x $%02x\n",
                        (opcode & 0xf00) >> 8,
                        opcode & 0xff);
                break;
            case 0xd:
                // 0xdXYN
                // Draw sprite of N height at address in I
                // to coords VX, VY (all sprites are 8 bits wide)
                printf("DRAW V%x V%x %x\n",
                        (opcode & 0xf00) >> 8,
                        (opcode & 0xf0) >> 4,
                        opcode & 0xf);
                break;
            case 0xe:
                // Skip next instruction if key stored in VX is pressed:
                if ((opcode & 0xff) == 0x9e)
                    printf("TKEY V%x\n", (opcode & 0xf00) >> 8);
                // Skip next instruction if key NOT pressed:
                else if ((opcode & 0xff) == 0xa1)
                    printf("TNKEY V%x\n", (opcode & 0xf00) >> 8);
                else
                    invalid_opcode(pc, opcode);
                break;
            case 0xf:
                // Another messy one, depends on last 2 digits
                switch (opcode & 0xff)
                {
                    case 0x07:
                        // Set VX to value of delay timer
                        printf("SET.DT V%x\n", (opcode & 0xf00) >> 8);
                        break;
                    case 0x0a:
                        // Wait for keypress, then store it in VX
                        // Blocking operation
                        printf("GETKEY V%x\n", (opcode & 0xf00) >> 8);
                        break;
                    case 0x15:
                        // Set delay timer to VX
                        printf("GET.DT V%x\n", (opcode & 0xf00) >> 8);
                        break;
                    case 0x18:
                        // Set sound timer to VX
                        printf("SET.ST V%x\n", (opcode & 0xf00) >> 8);
                        break;
                    case 0x1e:
                        // Add VX to I Set VF to if overflowed
                        printf("IADD V%x\n", (opcode & 0xf00) >> 8);
                        break;
                    case 0x29:
                        // sets I to the built-in sprite address for the
                        // character stored in VX
                        printf("FONT V%x\n", (opcode & 0xf00) >> 8);
                        break;
                    case 0x33:
                        // Stores BCD of VX starting at I
                        printf("BCD V%x\n", (opcode & 0xf00) >> 8);
                        break;
                    case 0x55:
                        // Stores registers V0 to & incl. VX into memory
                        printf("STORE V%x\n", (opcode & 0xf00) >> 8);
                        break;
                    case 0x65:
                        // Loads registers v0 to & incl. VX from memory
                        printf("LOAD V%x\n", (opcode & 0xf00) >> 8);
                        break;
                    default:
                        // Invalid opcode starting with 0xf
                        invalid_opcode(pc, opcode);
                }
        }
    }
}

// Handles printing error messages when encountering an invalid opcode
// If from a working ROM, likely means disassembler ran into data
void invalid_opcode(int address, unsigned short opcode)
{
    printf("ERROR at address 0x%03x\n", address);
    printf("ERROR: Invalid opcode 0x%04x\n", opcode);
    exit(1);
}
