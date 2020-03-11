#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset

typedef struct {
    unsigned short opcode;
    unsigned char memory[4096];
    unsigned char v[16];
    unsigned short index_reg;
    unsigned short pc;
    unsigned char gfx[64 * 32];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[16];
    unsigned char sp;
    unsigned char key[16];
}
chip8_state;


int main(int argc, char *argv[]){
    if (argc != 2)
    {
        printf("Usage: chip8vm <romfile>\n");
        exit(1);
    }

    chip8_state state;
    // Initialize important fields
    // It's not important for say, registers to be cleared
    // and graphics will undoubtably be immediately cleared with 0x00e0 op 
    memset(state.memory, 0, sizeof(state.memory));
    state.pc = 0x200;
    state.sp = 0x0;
    memset(state.key, 0, sizeof(state.key));

    // Open a romfile
    FILE *romfile = fopen(argv[1], "r");
    if (romfile == NULL)
    {
        printf("Could not open file: %s\n", argv[1]);
        exit(1);
    }

    // Fill our memory with program data, starting at 0x200
    for (int i = 0x200; i < 0x1000; i++)
    {
        char c = fgetc(romfile);
        if (c != EOF)
        {
            state.memory[i] = c;
        }
        // If we've reached the EOF, this for loop takes over to fill the
        // remainder with 0s, and then breaks from the for loop
        else
        {
            for (int j = i; j < 0xFFF; j++)
            {
                state.memory[j] = 0x000;
            }
            break;
        }
    }
    fclose(romfile);

    // Dump our memory contents to the console (TEMP)
    for (int i = 0; i < 256; i++)
    {
        printf("%03x: ", i * 16);
        for (int j = 0; j < 16; j++)
        {
            printf("%02x ", state.memory[i * 16 + j]);
        }
        printf("\n");
    }
    printf("Memory dumped\n");
}
