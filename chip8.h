#include <stdint.h>

/* CPU */
typedef struct {
    /* Registers (variables) */
    uint8_t v[0xF];
    
    /* Program Counter */
    uint16_t pc;

    /* Address Register */
    uint16_t i;
} CPU;

#define MEMORY_SIZE 0x1000
