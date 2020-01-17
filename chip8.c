#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "chip8.h"

void emulate_chip8(CPU *cpu, uint8_t *memory);

void print_display(uint8_t *display) {
    for (int i = 0; i < 256; ++i) {
        if (i % 8 == 0) {
            printf("\n");
        }
        printf("%02x  ", display[i]);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    uint8_t *memory = calloc(MEMORY_SIZE, sizeof memory);
    
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

    fclose(ROM);

    /* Initialize CPU */
    CPU cpu;
    init_cpu(&cpu);

    while (1) {
        emulate_chip8(&cpu, memory);
        print_display(memory + 0xF00);
    }

    free(memory);

    return 0;
}

/* Stack pointer */
uint16_t sp = 0xEA0;

void unknown_opcode(uint16_t opcode) {
    fprintf(stderr, "Unknown opcode %04x\n", opcode);
    // exit(1);
}

void call_stack_push(uint8_t *memory, uint16_t retaddr) {
    memory[sp] = (retaddr & 0xFF00) >> 8;
    memory[sp + 1] = retaddr & 0xFF;

    /* Advance sp */
    sp += 2;
}

uint16_t call_stack_pop(uint8_t *memory) {
    sp -= 2;

    return (memory[sp] << 8) | memory[sp + 1];
}

void emulate_chip8(CPU *cpu, uint8_t *memory) {
    sleep(1);
    /* Fetch instruction */
    printf("address: %04x  ", cpu->pc);
    uint16_t opcode = fetch_instruction(cpu, memory);
    printf("opcode: %04x\n", opcode);

    uint8_t *display = memory + 0xF00;

    /* Do a switch on the leading hex digit */
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode) {
                case 0x00E0:
                    /* [00E0] Clear Screen */
                    for (int i = 0xF00; i < 0x1000; ++i) {
                        memory[i] = 0;
                    }
                    break;
                case 0x00EE:
                    /* [00EE] Return from a subroutine */
                    cpu->pc = call_stack_pop(memory);
                    break;
                default:
                    unknown_opcode(opcode);
                    break;
            }
            break;
        case 0x1000:
            /* [1NNN] Jump to address */
            cpu->pc = opcode & 0x0FFF;
            break;
        case 0x2000:
            /* [2NNN] Call subroutine */
            call_stack_push(memory, cpu->pc);
            cpu->pc = opcode & 0x0FFF;
            break;
        case 0x3000: {
            /* [3XNN] Skip next instruction if VX equals value */
            uint8_t value = opcode & 0x00FF;
            uint8_t x = (opcode & 0x0F00) >> 8;

            if (cpu->v[x] == value) {
                cpu->pc += 2;
            }

            break;
        }
        case 0x4000: {
            /* [4XNN] Skip next instruction if VX does not equal value */
            uint8_t value = opcode & 0x00FF;
            uint8_t x = (opcode & 0x0F00) >> 8;

            if (cpu->v[x] != value) {
                cpu->pc += 2;
            }

            break;
        }
        case 0x5000: {
            /* [5XYN] Skip next instruction if VX == VY */
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;

            if (x == y) {
                cpu->pc += 2;
            }

            break;
        }
        case 0x6000:
            /* [6XNN] Set VX to Value */
            cpu->v[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;
        case 0x7000:
            /* [7XNN] Add value to VX */
            cpu->v[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;
        case 0x8000: {
            /* [8000] Arithmetic Operators */
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            switch (opcode & 0x000F) {
                case 0x0000:
                    /* [8XY0] Assign VX to value of VY */
                    cpu->v[x] = cpu->v[y];
                    break;
                case 0x0001:
                    /* [8XY1] VX = VX | VY */
                    cpu->v[x] = cpu->v[x] | cpu->v[y];
                    break;
                case 0x0002:
                    /* [8XY2] VX = VX & VY */
                    cpu->v[x] = cpu->v[x] & cpu->v[y];
                    break;
                case 0x0003:
                    /* [8XY3] VX = VX ^ VY */
                    cpu->v[x] = cpu->v[x] ^ cpu->v[y];
                    break;
                case 0x0004:
                    /* [8XY4] VX += VY */
                    if ((cpu->v[x] + cpu->v[y]) & 0x0F00) {
                        cpu->v[0xF] = 1;
                    }
                    else {
                        cpu->v[0xF] = 0;
                    }

                    cpu->v[x] += cpu->v[y];
                    break;
                case 0x0005:
                    /* [8XY5] VX -= VY */
                    if (cpu->v[x] < cpu->v[y]) {
                        cpu->v[0xF] = 0;
                    }
                    else {
                        cpu->v[0xF] = 1;
                    }

                    cpu->v[x] -= cpu->v[y];
                    break;
                case 0x0006:
                    /* [8XY6] VX >>= 1 */
                    cpu->v[0xF] = cpu->v[x] & 0x1;
                    cpu->v[x] >>= 1;
                    break;
                case 0x0007:
                    /* [8XY7] VX = VY - VX */
                    if (cpu->v[y] < cpu->v[x]) {
                        cpu->v[0xF] = 0;
                    }
                    else {
                        cpu->v[0xF] = 1;
                    }

                    cpu->v[x] = cpu->v[y] - cpu->v[x];
                    break;
                case 0x000E:
                    /* [8XYE] VX <<= 1 */
                    cpu->v[0xF] = cpu->v[x] & 0xF;
                    cpu->v[0xF] <<= 1;
                    break;
                default:
                    unknown_opcode(opcode);
                    break;
            }
            break;
        }
        case 0x9000: {
            /* [9XY0] Skip next instruction if VX !+ VY */
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            if (cpu->v[x] != cpu->v[y]) {
                cpu->pc += 2;
            }
            break;
        }
        case 0xA000:
            /* [ANNN] Set I to address NNN */
            cpu->i = opcode & 0x0FFF;
            break;
        case 0xB000:
            /* [BNNN] Jump to address NNN + V0 */
            cpu->pc = (opcode & 0x0FFF) + cpu->v[0];
            break;
        case 0xC000: {
            /* [CXNN] VX = rand() & NN */
            uint8_t x = cpu->v[(opcode & 0x0F00) >> 8];
            cpu->v[x] = (rand() % 0x100) & opcode & 0x00FF;
            break;
        }
        case 0xD000: {
            /* [DXYN] Draw sprite on screen */
            uint8_t x = cpu->v[(opcode & 0x0F00) >> 8];
            uint8_t y = cpu->v[(opcode & 0x00F0) >> 4];
            uint8_t height = opcode & 0x000F;

            cpu->v[0xF] = 0;
            for (int j = 0; j < height; ++j) {
                uint8_t line = memory[cpu->i + j];
                printf("line=%02x\n", line);
                for (int i = 0; i < 8; ++i) {

                    if ((line << i) & 0x80) {
                        uint16_t bit = (64 * (j + y)) + i + x;
                        uint8_t index = bit / 8;
                        uint8_t offset = bit % 8;

                        /* Set hit flag */
                        if ((display[index] << offset) & 0x80) {
                            cpu->v[0xF] = 1;
                        }

                        display[index] ^= 0x80 >> offset;
                    }
                }
            }
            break;
        }
        // case 0xE000:
        //     break;
        case 0xF000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            switch (opcode & 0x00FF) {
                case 0x0007:
                    /* [FX07] VX = delay_timer */
                    break;
                case 0x000A:
                    /* [FX0A] VX = get_key (blocking) */
                    break;
                case 0x0015:
                    /* [FX15] delay_timer = VX */
                    break;
                case 0x0018:
                    /* [FX18] sound_timer = VX */
                    break;
                case 0x001E:
                    /* [FX1E] I += VX */
                    if (cpu->i + cpu->v[x] > 0xFFF) {
                        cpu->v[0xF] = 1;
                    }
                    else {
                        cpu->v[0xF] = 1;
                    }

                    cpu->i += cpu->v[x];
                    break;
                case 0x0029:
                    /* [FX29] I = sprite_address[VX] (chars) */
                    break;
                case 0x0033:
                    /* [FX33] binary coded decimal */
                    break;
                case 0x0055:
                    /* [FX55] register dump at I */
                    for (int i = 0; i <= x; ++i) {
                        memory[cpu->i + i] = cpu->v[i];
                    }
                    break;
                case 0x0065:
                    /* [FX65] register load at I */
                    for (int i = 0; i <= x; ++i) {
                        cpu->v[i] = memory[cpu->i + i];
                    }
                    break;
                default:
                    unknown_opcode(opcode);
                    break;
            }
            break;
        }
        default:
            unknown_opcode(opcode);
            break;
    }
}
