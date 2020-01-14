#include <stdint.h>

#include "chip8.h"

/* Fetch instruction at address referenced by the program counter */
uint16_t fetch_instruction(uint16_t address, uint8_t *memory) {
    return memory[address] << 8 | memory[address + 1];
}

void init_cpu(CPU *cpu) {
    /* CHIP-8 Programs start at memory address 0x200 */
    cpu->pc = 0x200;
}