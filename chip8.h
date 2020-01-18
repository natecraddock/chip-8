#include <stdint.h>

/* CPU */
typedef struct {
    /* Registers (variables) */
    uint8_t v[0x10];
    
    /* Program Counter */
    uint16_t pc;

    /* Address Register */
    uint16_t i;
} CPU;

typedef struct {
    /* Keyboard keys are indexed by their value (0x0 - 0xF) */
    uint8_t keys[0x10];
} Keyboard;

#define MEMORY_SIZE 0x1000

uint16_t fetch_instruction(CPU *cpu, uint8_t *memory);

void init_cpu(CPU *cpu);
