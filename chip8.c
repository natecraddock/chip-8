#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <unistd.h>

#include "chip8.h"

#include "SDL2/SDL.h"

void emulate_chip8(CPU *cpu, uint8_t *memory, Keyboard *keyboard);

void set_keys(Keyboard *keyboard, int key) {
    for (int i = 0; i <= 0xF; ++i) {
        keyboard->keys[i] = 0;
    }

    switch (key) {
        case SDLK_1:
            keyboard->keys[0x1] = 1;
            break;
        case SDLK_2:
            keyboard->keys[0x2] = 1;
            break;
        case SDLK_3:
            keyboard->keys[0x3] = 1;
            break;
        case SDLK_q:
            keyboard->keys[0x4] = 1;
            break;
        case SDLK_w:
            keyboard->keys[0x5] = 1;
            break;
        case SDLK_e:
            keyboard->keys[0x6] = 1;
            break;
        case SDLK_a:
            keyboard->keys[0x7] = 1;
            break;
        case SDLK_s:
            keyboard->keys[0x8] = 1;
            break;
        case SDLK_d:
            keyboard->keys[0x9] = 1;
            break;
        case SDLK_x:
            keyboard->keys[0x0] = 1;
            break;
        case SDLK_z:
            keyboard->keys[0xA] = 1;
            break;
        case SDLK_c:
            keyboard->keys[0xB] = 1;
            break;
        case SDLK_4:
            keyboard->keys[0xC] = 1;
            break;
        case SDLK_r:
            keyboard->keys[0xD] = 1;
            break;
        case SDLK_f:
            keyboard->keys[0xE] = 1;
            break;
        case SDLK_v:
            keyboard->keys[0xF] = 1;
            break;
        default:
            break;
    }
}

uint8_t wait_for_keypress(Keyboard *keyboard) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            int key = event.key.keysym.sym;
            set_keys(keyboard, key);
        }
    }
    for (int i = 0; i <= 0xF; ++i) {
        if (keyboard->keys[i]) {
            return i;
        }
    }
    return 0;
}

/* Load font data into memory */
void load_font_data(uint8_t *memory) {
    uint8_t chip8_fontset[80] = { 
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i = 0; i < sizeof chip8_fontset / sizeof (uint8_t); ++i) {
        memory[i] = chip8_fontset[i];
    }
}



void draw_square(int x, int y, SDL_Surface *surface) {
    SDL_Rect rect = {x, y, 10, 10};

    SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 255, 255, 255));
}

uint64_t get_time_milis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (1000 * time.tv_sec) + (time.tv_usec / 1000.0);
}

void draw_display(uint8_t *display, SDL_Surface* surface) {
    int width = 8;
    int height = 32;

    /* Draw display from memory */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t row = display[x + (y * width)];
            for (int i = 0; i < 8; ++i) {
                if (row & (0x80 >> i)) {
                    /* This pixel is set, draw a 10x10 square */
                    draw_square(((x * 8) + i) * 10, y * 10, surface);
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    uint8_t *memory = calloc(MEMORY_SIZE, sizeof memory);
    if (!memory) {
        fprintf(stderr, "Failed to allocate memory\n");
    }
    
    printf("CHIP-8 Interpreter\n");
    if (argc != 2) {
        fprintf(stderr, "Usage: chip8 [ROM]\n");
        return 1;
    }

    /* Init SDL 2 */
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);

    if (!window) {
        fprintf(stderr, "Unable to create window\n");
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        fprintf(stderr, "Unable to create renderer\n");
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

    load_font_data(memory);

    /* Initialize CPU */
    CPU cpu;
    init_cpu(&cpu);

    uint64_t hertz = get_time_milis();
    while (1) {
        SDL_Event event;
        Keyboard keyboard;

        // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        // SDL_RenderClear(renderer);
        SDL_Surface *surface = SDL_GetWindowSurface(window);

        /* Clear */
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
        draw_display(memory + 0xF00, surface);
        SDL_UpdateWindowSurface(window);
        // SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                // printf("Key pressed");

                int key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) {
                    SDL_Quit();
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    free(memory);
                    return 0;
                }
                else {
                    /* Handle key presses */
                    set_keys(&keyboard, key);
                }
            }

            if (event.type == SDL_QUIT) {
                SDL_Quit();
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                free(memory);
                return 0;
            }
        }
        emulate_chip8(&cpu, memory, &keyboard);

        if (get_time_milis() - hertz > 16.6667) {
            if (cpu.delay_timer > 0) {
                cpu.delay_timer -= 1;
            }
            if (cpu.sound_timer > 0) {
                cpu.sound_timer -= 1;
            }
        }

        usleep(10000);
    }

    free(memory);

    return 0;
}

/* Stack pointer */
uint16_t sp = 0xEA0;

void unknown_opcode(uint16_t opcode) {
    fprintf(stderr, "Unknown opcode %04x\n", opcode);
    exit(1);
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

void emulate_chip8(CPU *cpu, uint8_t *memory, Keyboard *keyboard) {
    /* Fetch instruction */
    // printf("address: %04x  ", cpu->pc);
    uint16_t opcode = fetch_instruction(cpu, memory);
    // printf("opcode: %04x\n", opcode);

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
                    cpu->v[0xF] = cpu->v[x] & 0x8;
                    cpu->v[x] <<= 1;
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
                // printf("line=%02x\n", line);
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
        case 0xE000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            /* Keyboard input */
            switch (opcode & 0x00FF) {
                case 0x009E:
                    /* [EX9E] skip if key pressed */
                    if (keyboard->keys[x]) {
                        cpu->pc += 2;
                    }
                    break;
                case 0x00A1:
                    /* [EXA1] skip if key not pressed */
                    if (!keyboard->keys[x]) {
                        cpu->pc += 2;
                    }
                    break;
                default:
                    unknown_opcode(opcode);
                    break;
            }
            break;
        }
        case 0xF000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            switch (opcode & 0x00FF) {
                case 0x0007:
                    /* [FX07] VX = delay_timer */
                    cpu->v[x] = cpu->delay_timer;
                    break;
                case 0x000A: {
                    /* [FX0A] VX = get_key (blocking) */
                    uint8_t key = wait_for_keypress(keyboard);
                    cpu->v[x] = key;
                    break;
                }
                case 0x0015:
                    /* [FX15] delay_timer = VX */
                    cpu->delay_timer = cpu->v[x];
                    break;
                case 0x0018:
                    /* [FX18] sound_timer = VX */
                    cpu->sound_timer = cpu->v[x];
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
                    cpu->i = cpu->v[x] * 5;
                    break;
                case 0x0033:
                    /* [FX33] binary coded decimal */
                    memory[cpu->i] = x / 100;
                    memory[cpu->i + 1] = (x % 100) / 10;
                    memory[cpu->i + 2] = (x % 100) % 10;
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
