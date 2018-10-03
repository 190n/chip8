#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <chrono>
#include "chip8.hpp"
#include "input.hpp"

using namespace std::chrono;

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

// black magic syntax
// http://www.multigesture.net/wp-content/uploads/mirror/zenogais/FunctionPointers.htm
// https://stackoverflow.com/a/3565888
typedef void (chip8::*opcodeFunc)();

opcodeFunc opcodes[16] = {
    &chip8::op0, &chip8::op1NNN, &chip8::op2NNN, &chip8::op3XNN, &chip8::op4XNN,
    &chip8::op5XY0, &chip8::op6XNN, &chip8::op7XNN, &chip8::opArithmetic,
    &chip8::op9XY0, &chip8::opANNN, &chip8::opBNNN, &chip8::opCXNN,
    &chip8::opDXYN, &chip8::opInput, &chip8::opF
};

opcodeFunc opcodes0[256], inputOpcodes[256], opcodesF[256];

opcodeFunc arithmeticOpcodes[16] = {
    &chip8::op8XY0, &chip8::op8XY1, &chip8::op8XY2, &chip8::op8XY3,
    &chip8::op8XY4, &chip8::op8XY5, &chip8::op8XY6, &chip8::op8XY7,
    &chip8::opInvalid, &chip8::opInvalid, &chip8::opInvalid, &chip8::opInvalid,
    &chip8::opInvalid, &chip8::opInvalid, &chip8::op8XYE, &chip8::opInvalid
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

bool chip8::opcodeTablesBuilt = false;
void chip8::buildOpcodeTables() {
    for (int i = 0; i < 256; i++) {
        if (i == 0) {
            opcodes0[i] = &chip8::op0000;
        } else if (i == 0xB0) {
            opcodes0[i] = &chip8::op00BN;
        } else if (i == 0xC0) {
            opcodes0[i] = &chip8::op00CN;
        } else if (i == 0xE0) {
            opcodes0[i] = &chip8::op00E0;
        } else if (i == 0xEE) {
            opcodes0[i] = &chip8::op00EE;
        } else if (i == 0xFB) {
            opcodes0[i] = &chip8::op00FB;
        } else if (i == 0xFC) {
            opcodes0[i] = &chip8::op00FC;
        } else if (i == 0xFD) {
            opcodes0[i] = &chip8::op00FD;
        } else if (i == 0xFE) {
            opcodes0[i] = &chip8::op00FE;
        } else if (i == 0xFF) {
            opcodes0[i] = &chip8::op00FF;
        } else {
            opcodes0[i] = &chip8::opInvalid;
        }

        if (i == 0x9E) {
            inputOpcodes[i] = &chip8::opEX9E;
        } else if (i == 0xA1) {
            inputOpcodes[i] = &chip8::opEXA1;
        } else {
            inputOpcodes[i] = &chip8::opInvalid;
        }

        if (i == 0x07) {
            opcodesF[i] = &chip8::opFX07;
        } else if (i == 0x0A) {
            opcodesF[i] = &chip8::opFX0A;
        } else if (i == 0x15) {
            opcodesF[i] = &chip8::opFX15;
        } else if (i == 0x18) {
            opcodesF[i] = &chip8::opFX18;
        } else if (i == 0x1E) {
            opcodesF[i] = &chip8::opFX1E;
        } else if (i == 0x29) {
            opcodesF[i] = &chip8::opFX29;
        } else if (i == 0x30) {
            opcodesF[i] = &chip8::opFX30;
        } else if (i == 0x33) {
            opcodesF[i] = &chip8::opFX33;
        } else if (i == 0x55) {
            opcodesF[i] = &chip8::opFX55;
        } else if (i == 0x65) {
            opcodesF[i] = &chip8::opFX65;
        } else if (i == 0x75) {
            opcodesF[i] = &chip8::opFX75;
        } else if (i == 0x85) {
            opcodesF[i] = &chip8::opFX85;
        } else {
            opcodesF[i] = &chip8::opInvalid;
        }
    }

    opcodeTablesBuilt = true;
}

void chip8::init() {
    if (!opcodeTablesBuilt) {
        buildOpcodeTables();
    }

    srand(time(NULL));

    // https://stackoverflow.com/a/32874098
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) - duration_cast<seconds>(now.time_since_epoch());
    last_ms = ms.count();

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
    
    // execute opcode
    opcodeFunc f = opcodes[n0(opcode)];
    (*this.*f)();

    if (delay_timer > 0 || sound_timer > 0) {
        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) - duration_cast<seconds>(now.time_since_epoch());

        if (fmod(ms.count(), 16.5f) < fmod(last_ms, 16.5f)) {
            // decrement the timers
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


        last_ms = ms.count();
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

void chip8::opInvalid() {
    printf("error: at 0x%03X: invalid opcode 0x%04X\n", pc, opcode);
    pc += 2;
}

// OPCODES
void chip8::op0() {
    unsigned char which = opcode & 0x00FF;
    opcodeFunc f = opcodes0[which];
    (*this.*f)();
}

void chip8::op00BN() {
    printf("stub 00BN\n");
    pc += 2;
}

void chip8::op00CN() {
    printf("stub 00CN\n");
    pc += 2;
}

void chip8::op00E0() {
    for (int i = 0; i < 2048; i++) {
        gfx[i] = 0;
    }
    drawFlag = true;
    pc += 2;
}

void chip8::op00EE() {
    pc = stack[--sp] + 2;
    stack[sp] = 0;
}

void chip8::op00FB() {
    printf("stub 00FB\n");
    pc += 2;
}

void chip8::op00FC() {
    printf("stub 00FC\n");
    pc += 2;
}

void chip8::op00FD() {
    printf("exiting per application request\n");
    exit(0);
}

void chip8::op00FE() {
    printf("stub 00FE\n");
    pc += 2;
}

void chip8::op00FF() {
    printf("stub 00FF\n");
    pc += 2;
}

void chip8::op0000() {
    debug();
    pc += 2;
}

void chip8::op1NNN() {
    pc = opcode & 0x0FFF;
}

void chip8::op2NNN() {
    stack[sp] = pc;
    sp++;
    pc = opcode & 0x0FFF;
}

void chip8::op3XNN() {
    if (V[n1(opcode)] == (unsigned char)(opcode & 0x00FF)) {
        pc += 4;
    } else {
        pc += 2;
    }
}

void chip8::op4XNN() {
    if (V[n1(opcode)] != (unsigned char)(opcode & 0x00FF)) {
        pc += 4;
    } else {
        pc += 2;
    }
}

void chip8::op5XY0() {
    if (V[n1(opcode)] == V[n2(opcode)]) {
        pc += 4;
    } else {
        pc += 2;
    }
}

void chip8::op6XNN() {
    V[n1(opcode)] = opcode & 0x00FF;
    pc += 2;
}

void chip8::op7XNN() {
    V[n1(opcode)] += opcode & 0x00FF;
    pc += 2;
}

void chip8::opArithmetic() {
    unsigned char which = n3(opcode);
    opcodeFunc f = arithmeticOpcodes[which];
    (*this.*f)();
}

void chip8::op8XY0() {
    V[n1(opcode)] = V[n2(opcode)];
    pc += 2;
}

void chip8::op8XY1() {
    V[n1(opcode)] |= V[n2(opcode)];
    pc += 2;
}

void chip8::op8XY2() {
    V[n1(opcode)] &= V[n2(opcode)];
    pc += 2;
}

void chip8::op8XY3() {
    V[n1(opcode)] ^= V[n2(opcode)];
    pc += 2;
}

void chip8::op8XY4() {
    if (V[n2(opcode)] > (0xFF - V[n1(opcode)])) {
        V[0xF] = 1;
    } else {
        V[0xF] = 0;
    }
    V[n1(opcode)] += V[n2(opcode)];
    pc += 2;
}

void chip8::op8XY5() {
    if (V[n1(opcode)] < V[n2(opcode)]) {
        V[0xF] = 0;
    } else {
        V[0xF] = 1;
    }
    V[n1(opcode)] -= V[n2(opcode)];
    pc += 2;
}

void chip8::op8XY6() {
    V[0xF] = V[n1(opcode)] & 0x01;
    V[n1(opcode)] = V[n1(opcode)] >> 1;
    pc += 2;
}

void chip8::op8XY7() {
    if (V[n2(opcode)] < V[n1(opcode)]) {
        V[0xF] = 0;
    } else {
        V[0xF] = 1;
    }
    V[n1(opcode)] = V[n2(opcode)] - V[n1(opcode)];
    pc += 2;
}

void chip8::op8XYE() {
    V[0xF] = (V[n1(opcode)] & 0xF0) >> 7;
    V[n1(opcode)] = V[n1(opcode)] << 1;
    pc += 2;
}

void chip8::op9XY0() {
    if (V[n1(opcode)] != V[n2(opcode)]) {
        pc += 4;
    } else {
        pc += 2;
    }
}

void chip8::opANNN() {
    I = opcode & 0x0FFF;
    pc += 2;
}

void chip8::opBNNN() {
    pc = V[0] + (opcode & 0x0FFF);
}

void chip8::opCXNN() {
    V[n1(opcode)] = (rand() % 256) & (opcode & 0x00FF);
    pc += 2;
}

void chip8::opDXYN() {
    if (n3(opcode) == 0) {
        return opDXY0();
    }

    unsigned char x = V[n1(opcode)];
    unsigned char y = V[n2(opcode)];
    unsigned char height = n3(opcode);

    V[0xF] = 0;

    for (int yl = 0; yl < height; yl++) {
        unsigned char row = memory[I + yl];
        for (int xl = 0; xl < 8; xl++) {
            unsigned char pixel = ((row << xl) & (unsigned char)(0x80)) >> 7;
            if (pixel) {
                if (gfx[(y + yl) * 64 + x + xl]) {
                    V[0xF] = 1;
                }
                gfx[(y + yl) * 64 + x + xl] ^= pixel;
            }
        }
    }

    drawFlag = true;
    pc += 2;
}

void chip8::opDXY0() {
    printf("stub DXY0\n");
    pc += 2;
}

void chip8::opInput() {
    unsigned char which = opcode & 0x00FF;
    opcodeFunc f = inputOpcodes[which];
    (*this.*f)();
}

void chip8::opEX9E() {
    if (key[V[n1(opcode)]]) {
        pc += 4;
    } else {
        pc += 2;
    }
}

void chip8::opEXA1() {
    if (!key[V[n1(opcode)]]) {
        pc += 4;
    } else {
        pc += 2;
    }
}

void chip8::opF() {
    unsigned char which = opcode & 0x00FF;
    opcodeFunc f = opcodesF[which];
    (*this.*f)();
}

void chip8::opFX07() {
    V[n1(opcode)] = delay_timer;
    pc += 2;
}

void chip8::opFX0A() {
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
}

void chip8::opFX15() {
    delay_timer = V[n1(opcode)];
    pc += 2;
}

void chip8::opFX18() {
    sound_timer = V[n1(opcode)];
    pc += 2;
}

void chip8::opFX1E() {
    I += V[n1(opcode)];
    pc += 2;
}

void chip8::opFX29() {
    I = V[n1(opcode)] * 5 + 80;
    pc += 2;
}

void chip8::opFX30() {
    printf("stub FX30\n");
    pc += 2;
}

void chip8::opFX75() {
    printf("stub FX75\n");
    pc += 2;
}

void chip8::opFX85() {
    printf("stub FX85\n");
    pc += 2;
}

void chip8::opFX33() {
    memory[I]     = V[n1(opcode)] / 100;
    memory[I + 1] = (V[n1(opcode)] / 10) % 10;
    memory[I + 2] = (V[n1(opcode)] % 100) % 10;
    pc += 2;
}

void chip8::opFX55() {
    unsigned short old = I;
    for (int n = 0; n <= n1(opcode); n++) {
        memory[I] = V[n];
        I++;
    }

    I = old;
    pc += 2;
}

void chip8::opFX65() {
    unsigned short old = I;
    for (int n = 0; n <= n1(opcode); n++) {
        V[n] = memory[I];
        I++;
    }

    I = old;
    pc += 2;
}
