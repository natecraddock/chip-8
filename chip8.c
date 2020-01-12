#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "chip8.h"

int main(int argc, char **argv) {
    uint8_t memory[MEMORY_SIZE];
    
    printf("CHIP-8 Interpreter\n");
    if (argc != 2) {
        fprintf(stderr, "Usage: chip8 [ROM]\n");
        return 1;
    }
    
    /* Load ROM into memory */
    FILE *ROM = fopen(argv[1], "rb");
    if (ROM == NULL) {
        fprintf(stderr, "chip8: cannot read file \"%s\"\n", argv[1]);
        return 2;
    }

    fseek(ROM, 0, SEEK_END);
    long size = ftell(ROM);
    
    printf("ROM size is %ld\n", size);
    fseek(ROM, 0, SEEK_SET);

    uint8_t *program = memory + 0x200;
    fread(program, size, sizeof (uint8_t), ROM);

    printf("Memory at 0x200-201: 0x%02X%02X\n", memory[0x200], memory[0x201]);

    return 0;
}
