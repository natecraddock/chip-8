#include <stdint.h>

#include "chip8.h"

/* Fetch instruction at address referenced by the program counter */
uint16_t read_instruction(CPU *cpu, uint8_t *memory) {
    return memory[cpu->pc] << 8 | memory[cpu->pc + 1];
}

void init_cpu(CPU *cpu) {
    /* CHIP-8 Programs start at memory address 0x200 */
    cpu->pc = 0x200;
}