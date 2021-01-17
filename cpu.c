#include <stdio.h>
#include <stdint.h>

#include "chip8.h"

/* Fetch instruction at address referenced by the program counter
 * Also increment the pc
 */
uint16_t fetch_instruction(CPU *cpu, uint8_t *memory) {
    uint16_t opcode = memory[cpu->pc] << 8 | memory[cpu->pc + 1];
    cpu->pc += 2;

    return opcode;
}

void init_cpu(CPU *cpu) {
    /* CHIP-8 Programs start at memory address 0x200 */
    cpu->pc = 0x200;
    cpu->delay_timer = 0;
    cpu->sound_timer = 0;

    for (int i = 0; i < 0xF; ++i) {
        cpu->v[i] = 0;
    }
}
