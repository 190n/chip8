#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.hpp"
#include "input.hpp"

unsigned char chip8_fontset[80] = {
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

inline unsigned char n0(unsigned short n) {
    return (n & 0xF000) >> 12;
}

inline unsigned char n1(unsigned short n) {
    return (n & 0x0F00) >> 8;
}

inline unsigned char n2(unsigned short n) {
    return (n & 0x00F0) >> 4;
}

inline unsigned char n3(unsigned short n) {
    return n & 0x000F;
}

void chip8::init() {
    srand(time(NULL));

    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    for (int i = 0; i < 2048; i++) {
        gfx[i] = 0;
    }


    for (int i = 0; i < 16; i++) {
        V[i] = 0;
        stack[i] = 0;
    }

    for (int i = 0; i < 4096; i++) {
        memory[i] = 0;
    }

    for (int i = 0; i < 80; i++) {
        memory[i + 80] = chip8_fontset[i];
    }
}

void chip8::load(const char* file) {
    FILE* fp = fopen(file, "rb");
    if (fp == NULL) {
        printf("couldn't load rom: %s\n", file);
        exit(2);
    }

    // get size of file
    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);
    rewind(fp);

    if (sz > 3584) {
        printf("warning: rom %s too large; %d bytes will be ignored\n", file, sz - 3584);
        fread(memory + 512, 1, 3584, fp);
    } else {
        fread(memory + 512, 1, sz, fp);
    }

    fclose(fp);
}

void chip8::cycle() {
    // fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];
    
    // decode opcode
    switch (opcode & 0xF000) {
        case 0x0000:
            if (opcode == 0x00E0) { // 00E0: clear screen
                for (int i = 0; i < 2048; i++) {
                    gfx[i] = 0;
                }
                drawFlag = true;
                pc += 2;
            } else if (opcode == 0x00EE) { // 00EE: return
                pc = stack[--sp] + 2;
                stack[sp] = 0;
            } else if (opcode == 0x0000) { // 0000: debug
                debug();
                pc += 2;
            } else {
                printf("error: at 0x%X: unknown opcode 0x%X\n", pc, opcode);
            }
        break;

        case 0x1000: // 1NNN: jump to NNN
            pc = opcode & 0x0FFF;
        break;

        case 0x2000: // 2NNN: call subroutine at address NNN
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFF;
        break;

        case 0x3000: // 3XNN: skip next instruction if VX == NN
            if (V[n1(opcode)] == (char)(opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
        break;

        case 0x4000: // 4XNN: skip next instruction if VX != NN
            if (V[n1(opcode)] != (char)(opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
        break;

        case 0x5000: // 5XY0: skip next instruction if VX == VY
            if (V[n1(opcode)] == V[n2(opcode)]) {
                pc += 4;
            } else {
                pc += 2;
            }
        break;

        case 0x6000: // 6XNN: set VX to NN
            V[n1(opcode)] = opcode & 0x00FF;
            pc += 2;
        break;

        case 0x7000: // 7XNN: VX += NN w/o carry
            V[n1(opcode)] += opcode & 0x00FF;
            pc += 2;
        break;

        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000: // 8XY0: set VX to VY
                    V[n1(opcode)] = V[n2(opcode)];
                    pc += 2;
                break;

                case 0x0001: // 8XY1: VX |= VY
                    V[n1(opcode)] |= V[n2(opcode)];
                    pc += 2;
                break;

                case 0x0002: // 8XY2: VX &= VY
                    V[n1(opcode)] &= V[n2(opcode)];
                    pc += 2;
                break;

                case 0x0003: // 8XY3: VX ^= VY
                    V[n1(opcode)] ^= V[n2(opcode)];
                    pc += 2;
                break;

                case 0x0004: // 8XY4: add VY to VX, VF = 1 if carry
                    if (V[n2(opcode)] > (0xFF - V[n1(opcode)])) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[n1(opcode)] += V[n2(opcode)];
                    pc += 2;
                break;

                case 0x0005: // 8XY5: VX -= VY, VF = 0 if borrow else 1
                    if (V[n1(opcode)] < V[n2(opcode)]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[n1(opcode)] -= V[n2(opcode)];
                    pc += 2;
                break;

                case 0x0006: // 8XY6: VX = VY >> 1, set VF to least significant bit of VY
                    V[0xF] = V[n2(opcode)] & 0x01;
                    V[n1(opcode)] = V[n2(opcode)] >> 1;
                    pc += 2;
                break;

                case 0x0007: // 8XY7: VX = VY - VX, VF = 0 if borrow else 1
                    if (V[n2(opcode)] < V[n1(opcode)]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[n1(opcode)] = V[n2(opcode)] - V[n1(opcode)];
                    pc += 2;
                break;

                case 0x000E: // 8XYE: VX = VY << 1, VF = most significant bit of VY
                    V[0xF] = (V[n2(opcode)] & 0xF0) >> 7;
                    V[n1(opcode)] = V[n2(opcode)] << 1;
                    pc += 2;
                break;

                default:
                    printf("error: at 0x%X: unknown opcode 0x%X\n", pc, opcode);
            }
        break;

        case 0x9000: // 9XY0: skip next instruction if VX != VY
            if (V[n1(opcode)] != V[n2(opcode)]) {
                pc += 4;
            } else {
                pc += 2;
            }
        break;

        case 0xA000: // ANNN: set I to address NNN
            I = opcode & 0x0FFF;
            pc += 2;
        break;

        case 0xB000: // BNNN: jump to address NNN + V0
            pc = V[0] + (opcode & 0x0FFF);
        break;

        case 0xC000: // CXNN: set VX to random number & NN
            V[n1(opcode)] = (rand() % 256) & (opcode & 0x00FF);
            pc += 2;
        break;

        case 0xD000: { // DXYN: draw N-row sprite at I on screen at (VX, VY)
            unsigned short x = V[n1(opcode)];
            unsigned short y = V[n2(opcode)];
            unsigned short height = n3(opcode);
            unsigned short pixel;

            V[0xF] = 0;

            for (int yline = 0; yline < height; yline++) {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        if (gfx[(x + xline + ((y + yline) * 64))] == 1) V[0xF] == 1;
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            drawFlag = true;
            pc += 2;
        } break;

        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E: // EX9E: skip next instruction if key stored in VX is pressed
                    if (key[V[n1(opcode)]]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                break;

                case 0x00A1: // EXA1: skip next instruction if key in VX isn't pressed
                    if (!key[V[n1(opcode)]]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                break;

                default:
                    printf("error: at 0x%X: unknown opcode 0x%X\n", pc, opcode);
            }
        break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: // FX07: set VX to delay timer
                    V[n1(opcode)] = delay_timer;
                    pc += 2;
                break;

                case 0x000A: { // FX0A: wait for key press then store it in VX
                    bool keyPressed = false;
                    char whichKey;
                    while (!keyPressed) {
                        setKeys();
                        for (int i = 0; i < 16; i++) {
                            if (key[i]) {
                                keyPressed = true;
                                whichKey = i;
                            }
                        }
                    }

                    V[n1(opcode)] = whichKey;
                    pc += 2;
                } break;

                case 0x0015: // FX15: set delay timer to VX
                    delay_timer = V[n1(opcode)];
                    pc += 2;
                break;

                case 0x0018: // FX18: set sound timer to VX
                    sound_timer = V[n1(opcode)];
                    pc += 2;
                break;

                case 0x001E: // FX1E: I += VX
                    I += V[n1(opcode)];
                    pc += 2;
                break;

                case 0x0029: // FX29: I = location of sprite for character in VX
                    I = V[n1(opcode)] * 5 + 80;
                    pc += 2;
                break;

                case 0x0033: // FX33: store decimal representation of VX at I, I+1, I+2
                    memory[I]     = V[n1(opcode)] / 100;
                    memory[I + 1] = (V[n1(opcode)] / 10) % 10;
                    memory[I + 2] = (V[n1(opcode)] % 100) % 10;
                    pc += 2;
                break;

                case 0x0055: { // FX55: store all values VN { 0 <= N <= X} in memory starting at I, incrementing I
                    for (int n = 0; n <= n1(opcode); n++) {
                        memory[I] = V[n];
                        I++;
                    }
                    pc += 2;
                } break;

                case 0x0065: {// FX65: fill V0 to VX (incl. VX) with values from memory starting at I, incrementing I
                    for (int n = 0; n <= n1(opcode); n++) {
                        V[n] = memory[I];
                        I++;
                    }
                    pc += 2;
                } break;

                default:
                    printf("error: at 0x%X: unknown opcode 0x%X\n", pc, opcode);
            }
        break;

        default:
            printf("error: at 0x%X: unknown opcode 0x%X\n", pc, opcode);
    }

    if (delay_timer > 0) {
        delay_timer--;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {
            printf("beep!\n");
        }
        sound_timer--;
    }
}

void chip8::setKeys() {
    char* input = getInput();
    for (int i = 0; i < 16; i++) {
        key[i] = input[i];
    }
}

void chip8::debug() {
    printf("================\ndebug at pc = 0x%X\n\n", pc);

    printf("V0");
    for (int i = 1; i < 16; i++) {
        printf(" V%X", i);
    }
    printf("\n%02X", V[0]);
    for (int i = 1; i < 16; i++) {
        printf(" %02X", V[i]);
    }

    printf("\n\nStack:\n[ ");
    for (int i = 0; i < sp; i++) {
        printf("%03X ", stack[i]);
    }
    printf("]\n\n");

    printf("I = 0x%04X\n", I);
    printf("Some memory around I:\n");

    printf("%02X", memory[I - 16]);
    for (int i = 1; i < 16; i++) {
        printf(" %02X", memory[i + I - 16]);
    }

    printf(" \x1b[47;1m\x1b[30m%02X\x1b[0m", memory[I]);

    for (int i = 1; i <= 16; i++) {
        printf(" %02X", memory[i + I]);
    }

    printf("\n================\n\n");
}

// OPCODES
void chip8::op0() {
    
}

void chip8::op0000() {
    debug();
    pc += 2;
}
