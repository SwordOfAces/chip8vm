#ifndef TEST_H_INC
#define TEST_H_INC

#include "chip8vm.h"

int test_op(chip8_state *state, unsigned short t_val, unsigned short e_val, char dump);
int test_suite(chip8_state *state, unsigned char dump);
void test_graphics(chip8_state *state, int t);

#endif
