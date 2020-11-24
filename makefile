chip8vm: chip8vm.c testingsys.c
	gcc -Wall chip8vm.c testingsys.c -lSDL2 -o chip8vm
