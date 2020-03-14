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

chip8_state * create_state();
void load_rom(char *romfilename, chip8_state *state);
void dump_memory(chip8_state *state);

int main(int argc, char *argv[]){
    // Ensure that we're being used with what we'll assume is a romfile
    if (argc != 2)
    {
        printf("Usage: chip8vm <romfile>\n");
        exit(1);
    }

    // Create and initialize a state struct
    chip8_state *state = create_state();

    // Load given romfile into VM memory
    load_rom(argv[1], state);

    // dump memory to console. will get removed once there is "interesting"
    // effects that can be observed
    dump_memory(state);

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


void dump_memory(chip8_state *state)
{
    // Dump our memory contents to the console
    for (int i = 0; i < 256; i++)
    {
        printf("%03x: ", i * 16);
        // Possibility: check against previous line, if same, then
        // do *** for one line, and pick back up when memory is different
        for (int j = 0; j < 16; j++)
        {
            printf("%02x ", state->memory[i * 16 + j]);
        }
        printf("\n");
    }
    printf("Memory dumped\n");
}
