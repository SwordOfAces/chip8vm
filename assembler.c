#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset

void invalid_line(char *line, int line_no);
int conv_hex_str(char *str, int len);
void write_op(unsigned short opcode, FILE *outfile);

int main(int argc, char *argv[]){
    if (argc < 2)
    {
        printf("Usage: assembler <codefile>\n");
        // TODO option to name out file
        exit(1);
    }

    // Open an assembly code file
    FILE *codefile = fopen(argv[1], "r");
    if (codefile == NULL)
    {
        printf("Could not open file: %s\n", argv[1]);
        exit(1);
    }

    // Open a file for our bytecode:
    // If creating name change option, be sure to change print text
    FILE *outfile = fopen("out.ch8", "w");
    if (outfile == NULL)
    {
        printf("Could not create out.ch8\n");
    }
    
    // Line limit 50 is fine, even if we want commenting
    char line[50]; 
    // compare first n chars. has to be at least 5 with current mnemonics
    int line_no = 0; // useful for error reporting

    // some declarations for placeholders (val == nn, addr == nnn)
    int x, y, val, addr;
    unsigned short opcode;

    while (fgets(line, sizeof(line), codefile) != NULL)
    {
        line_no++;
        // Comments start with %, let's head those off right away
        if (line[0] == '%')
            continue;
        // Can't use a switch with strings, so if-else if-else ladder
        // Compare strings by their first few characters
        // Enough to guarantee correct, not enough to get to args
        else if (strncmp(line, "CALLPROG", 5) == 0)
        {
            addr = conv_hex_str(&line[10], 3);
            opcode = addr;
        }
        else if (strncmp(line, "CLRS", 4) == 0)
            opcode = 0x00e0;
        else if (strncmp(line, "RET", 3) == 0)
        {
            opcode = 0x00ee;
        }
        else if (strncmp(line, "GOTO", 4) == 0)
        {
            addr = conv_hex_str(&line[6], 3);
            opcode = 0x1000 + addr;
        }
        else if (strncmp(line, "CALL ", 5) == 0)
        {
            addr = conv_hex_str(&line[6], 3);
            opcode = 0x2000 + addr;
        }
        else if (strncmp(line, "TEQ.I", 5) == 0)
        {
            x = conv_hex_str(&line[7], 1);
            val = conv_hex_str(&line[10], 2);
            opcode = 0x3000 + (x * 0x100) + val;
        }
        else if (strncmp(line, "TNE.I", 5) == 0)
        {
            x = conv_hex_str(&line[7], 1);
            val = conv_hex_str(&line[10], 2);
            opcode = 0x4000 + (x * 0x100) + val;
        }
        else if (strncmp(line, "TEQ ", 4) == 0)
        {
            x = conv_hex_str(&line[5], 1);
            y = conv_hex_str(&line[8], 1);
            opcode = 0x5000 + (x * 0x100) + (y * 0x10);
        }
        else if (strncmp(line, "MOV.I", 5) == 0)
        {
            x = conv_hex_str(&line[7], 1);
            val = conv_hex_str(&line[10], 2);
            opcode = 0x6000 + (x * 0x100) + val;
        }
        else if (strncmp(line, "INC.I", 5) == 0)
        {
            x = conv_hex_str(&line[7], 1);
            val = conv_hex_str(&line[10], 2);
            opcode = 0x7000 + (x * 0x100) + val;
        }
        else if (strncmp(line, "MOV.V", 5) == 0)
        {
            x = conv_hex_str(&line[7], 1);
            y = conv_hex_str(&line[10], 1);
            opcode = 0x8000 + (x * 0x100) + (y * 0x10);
        }
        else if (strncmp(line, "OR", 2) == 0)
        {
            x = conv_hex_str(&line[4], 1);
            y = conv_hex_str(&line[7], 1);
            opcode = 0x8001 + (x * 0x100) + (y * 0x10);
        }
        else if (strncmp(line, "AND", 3) == 0)
        {
            x = conv_hex_str(&line[5], 1);
            y = conv_hex_str(&line[8], 1);
            opcode = 0x8002 + (x * 0x100) + (y * 0x10);
        }
        else if (strncmp(line, "XOR", 3) == 0)
        {
            x = conv_hex_str(&line[5], 1);
            y = conv_hex_str(&line[8], 1);
            opcode = 0x8003 + (x * 0x100) + (y * 0x10);
        }
        else if (strncmp(line, "INC.V", 5) == 0)
        {
            x = conv_hex_str(&line[7], 1);
            y = conv_hex_str(&line[10], 1);
            opcode = 0x8004 + (x * 0x100) + (y * 0x10);
        }
        else if (strncmp(line, "SUB", 3) == 0)
        {
            x = conv_hex_str(&line[5], 1);
            y = conv_hex_str(&line[8], 1);
            opcode = 0x8005 + (x * 0x100) + (y * 0x10);
        }
        else if (strncmp(line, "SHR", 3) == 0)
        {
            x = conv_hex_str(&line[5], 1);
            y = conv_hex_str(&line[8], 1);
            opcode = 0x8006 + (x * 0x100) + (y * 0x10);
        }
        else if (strncmp(line, "LESS", 4) == 0)
        {
            x = conv_hex_str(&line[6], 1);
            y = conv_hex_str(&line[9], 1);
            opcode = 0x8007 + (x * 0x100) + (y * 0x10);
        }
        else if (strncmp(line, "SHL", 3) == 0)
        {
            x = conv_hex_str(&line[5], 1);
            y = conv_hex_str(&line[8], 1);
            opcode = 0x800e + (x * 0x100) + (y * 0x10);
        }
        else if (strncmp(line, "TNE ", 4) == 0)
        {
            x = conv_hex_str(&line[5], 1);
            y = conv_hex_str(&line[8], 1);
            opcode = 0x9000 + (x * 0x100) + (y * 0x10);
        }
        else if (strncmp(line, "INDEX", 5) == 0)
        {
            addr = conv_hex_str(&line[7], 3);
            opcode = 0xa000 + addr;
        }
        else if (strncmp(line, "JMPOFF", 6) == 0)
        {
            addr = conv_hex_str(&line[8], 3);
            opcode = 0xb000 + addr;
        }
        else if (strncmp(line, "RAND", 4) == 0)
        {
            x = conv_hex_str(&line[6], 1);
            val = conv_hex_str(&line[9], 2);
            opcode = 0xc000 + (x * 0x100) + val;
        }
        else if (strncmp(line, "DRAW", 4) == 0)
        {
            x = conv_hex_str(&line[6], 1);
            y = conv_hex_str(&line[9], 1);
            val = conv_hex_str(&line[11], 1);
            opcode = 0xd000 + (x * 0x100) + (y * 0x10) + val;
        }
        else if (strncmp(line, "TKEY", 4) == 0)
        {
            x = conv_hex_str(&line[6], 1);
            opcode = 0xe09e + (x * 0x100);
        }
        else if (strncmp(line, "TNKEY", 5) == 0)
        {
            x = conv_hex_str(&line[7], 1);
            opcode = 0xe0a1 + (x * 0x100);
        }
        else if (strncmp(line, "SET.DT", 6) == 0)
        {
            x = conv_hex_str(&line[8], 1);
            opcode = 0xf007 + (x * 0x100);
        }
        else if (strncmp(line, "GETKEY", 6) == 0)
        {
            x = conv_hex_str(&line[8], 1);
            opcode = 0xf00a + (x * 0x100);
        }
        else if (strncmp(line, "GET.DT", 6) == 0)
        {
            x = conv_hex_str(&line[8], 1);
            opcode = 0xf015 + (x * 0x100);
        }
        else if (strncmp(line, "SET.ST", 6) == 0)
        {
            x = conv_hex_str(&line[8], 1);
            opcode = 0xf018 + (x * 0x100);
        }
        else if (strncmp(line, "IADD", 4) == 0)
        {
            x = conv_hex_str(&line[6], 1);
            opcode = 0xf01e + (x * 0x100);
        }
        else if (strncmp(line, "FONT", 4) == 0)
        {
            x = conv_hex_str(&line[6], 1);
            opcode = 0xf029 + (x * 0x100);
        }
        else if (strncmp(line, "BCD", 3) == 0)
        {
            x = conv_hex_str(&line[5], 1);
            opcode = 0xf033 + (x * 0x100);
        }
        else if (strncmp(line, "STORE", 5) == 0)
        {
            x = conv_hex_str(&line[7], 1);
            opcode = 0xf055 + (x * 0x100);
        }
        else if (strncmp(line, "LOAD", 4) == 0)
        {
            x = conv_hex_str(&line[6], 1);
            opcode = 0xf065 + (x * 0x100);
        }
        // If it doesnt match anything above, we've got a bad line
        else
            invalid_line(line, line_no);

        // Now, we should have an opcode, so write it to the file
        write_op(opcode, outfile);
    }

    // We've reached the end of our code file, so close both files
    fclose(codefile); 
    fclose(outfile);
}

// Handles printing error messages when encountering an invalid mnemonic
void invalid_line(char *line, int line_no)
{
    printf("ERROR at line: %i\n", line_no);
    printf("ERROR: Invalid mnemonic: %s\n", line);
    exit(1);
}

int conv_hex_str(char *str, int len)
{
    char sub[len];
    strncpy(sub, str, len);
    return strtol(str, NULL, 16);
}

void write_op(unsigned short opcode, FILE *outfile)
{
    printf("%04x\n", opcode);
    unsigned char msb = (opcode & 0xff00) >> 8;
    unsigned char lsb = opcode & 0xff;
    fputc(msb, outfile);
    fputc(lsb, outfile);
}
