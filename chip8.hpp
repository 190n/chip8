#ifndef CHIP8_HPP
#define CHIP8_HPP

class chip8 {
    unsigned char memory[4096], V[16];
    unsigned short I, pc, opcode;

    unsigned char delay_timer;
    unsigned char sound_timer;

    unsigned short stack[16], sp;

    unsigned char key[16];

    // for 60Hz timers
    long last_ms;

public:
    bool drawFlag = false;
    unsigned char gfx[64 * 32];

    void init();
    void load(const char* file);
    void cycle();
    void setKeys();

    // * = SUPER CHIP-8 opcode
    // https://raw.githubusercontent.com/JonRodtang/super-chip-8/master/REFERENCE.md

    void op0(); // jump to proper 00XX opcode
    void op00BN(); // * scroll display N lines up (N/2 in low res mode)
    void op00CN(); // * scroll display N lines down (N/2 in low res mode)
    void op00E0(); // clear screen
    void op00EE(); // return from subroutine
    void op00FB(); // * scroll display 4 pixels right (2 in low res mode)
    void op00FC(); // * scroll display 4 pixels left (2 in low res mode)
    void op00FD(); // * exit
    void op00FE(); // * disable extended screen mode
    void op00FF(); // * enable extended screen mode
    void op0000(); // debug
    
    void op1NNN(); // jump
    void op2NNN(); // call subroutine
    void op3XNN(); // skip next instruction if VX == NN
    void op4XNN(); // skip next instruction if VX != NN
    void op5XY0(); // skip next instruction if VX == VY
    void op6XNN(); // set VX to constant NN
    void op7XNN(); // VX += NN, no carry

    void opArithmetic(); // execute proper 800X opcode
    void op8XY0(); // VX = VY
    void op8XY1(); // VX |= VY
    void op8XY2(); // VX &= VY
    void op8XY3(); // VX ^= VY
    void op8XY4(); // VX += VY, VF = 1 if carry
    void op8XY5(); // VX -= VY, VF = 0 if borrow
    void op8XY6(); // VX = VX >> 1, VF = least significant bit of VY
    void op8XY7(); // VX = VY - VX, VF = 0 if borrow
    void op8XYE(); // VX = VX << 1, VF = most significant bit of VY

    void op9XY0(); // skip next instruction if VX != VY
    void opANNN(); // I = constant NNN
    void opBNNN(); // jump to NNN + V0
    void opCXNN(); // VX = rand() & NN
    void opDXYN(); // draw N-row sprite from I at (VX, VY)
    void opDXY0(); // draw 16x16 sprite from I at (VX, VY) (8x16 in low res mode)
    
    void opInput(); // execute proper E0XX opcode
    void opEX9E(); // skip next instruction if key in VX is pressed
    void opEXA1(); // skip next instruction if key in VX isn't pressed

    void opF(); // execute proper F0XX opcode
    void opFX07(); // VX = delay timer
    void opFX0A(); // wait for key press, store in VX
    void opFX15(); // delay timer = VX
    void opFX18(); // sound timer = VX
    void opFX1E(); // I += VX
    void opFX29(); // I = sprite for char in VX
    void opFX30(); // I = 10-row sprite for char in VX
    void opFX33(); // decimal representation of VX into I, I+1, I+2
    void opFX55(); // dump registers into memory
    void opFX65(); // load memory into registers
    void opFX75(); // dump registers into RPL user flags (X < 8)
    void opFX85(); // load RPL user flags into registers (X < 8)

    void debug();
    void opInvalid();

    static bool opcodeTablesBuilt;
    static void buildOpcodeTables();
};

#endif
