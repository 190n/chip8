#include <stdio.h>
#include "chip8.hpp"
#include "input.hpp"
#include "gfx.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("usage: %s <rom file>\n", argv[0]);
        return 1;
    }

    chip8 cpu;
    cpu.init();
    cpu.load(argv[1]);

    initGfx();

    while (true) {
        cpu.cycle();

        if (cpu.drawFlag) {
            drawGfx(cpu);
        }

        cpu.setKeys();
    }

    return 0;
}
