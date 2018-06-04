#include <SFML/Window.hpp>
#include "input.hpp"

using sf::Keyboard;
char input[16];

char* getInput() {
    // 1 2 3 C     1 2 3 4
    // 4 5 6 D --\ Q W E R
    // 7 8 9 E --/ A S D F
    // A 0 B F     Z X C V
    input[0x0] = Keyboard::isKeyPressed(Keyboard::X);
    input[0x1] = Keyboard::isKeyPressed(Keyboard::Num1);
    input[0x2] = Keyboard::isKeyPressed(Keyboard::Num2);
    input[0x3] = Keyboard::isKeyPressed(Keyboard::Num3);
    input[0x4] = Keyboard::isKeyPressed(Keyboard::Q);
    input[0x5] = Keyboard::isKeyPressed(Keyboard::W);
    input[0x6] = Keyboard::isKeyPressed(Keyboard::E);
    input[0x7] = Keyboard::isKeyPressed(Keyboard::A);
    input[0x8] = Keyboard::isKeyPressed(Keyboard::S);
    input[0x9] = Keyboard::isKeyPressed(Keyboard::D);
    input[0xA] = Keyboard::isKeyPressed(Keyboard::Z);
    input[0xB] = Keyboard::isKeyPressed(Keyboard::C);
    input[0xC] = Keyboard::isKeyPressed(Keyboard::Num4);
    input[0xD] = Keyboard::isKeyPressed(Keyboard::R);
    input[0xE] = Keyboard::isKeyPressed(Keyboard::F);
    input[0xF] = Keyboard::isKeyPressed(Keyboard::V);
    return input;
}
