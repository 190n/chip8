#include <SFML/Graphics.hpp>
#include "chip8.hpp"

#define SCALE_FACTOR 16

sf::RenderWindow* window;

sf::RectangleShape pixels[64][32];

void initGfx() {
    window = new sf::RenderWindow(sf::VideoMode(64 * SCALE_FACTOR, 32 * SCALE_FACTOR), "CHIP-8");
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 32; y++) {
            pixels[x][y].setSize(sf::Vector2f(SCALE_FACTOR, SCALE_FACTOR));
            pixels[x][y].setPosition(x * SCALE_FACTOR, y * SCALE_FACTOR);
            pixels[x][y].setFillColor(sf::Color::Black);
        }
    }
}

void drawGfx(chip8 cpu) {
    window->clear(sf::Color::Black);

    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 32; y++) {
            if (cpu.gfx[y * 64 + x]) {
                pixels[x][y].setFillColor(sf::Color::White);
            } else {
                pixels[x][y].setFillColor(sf::Color::Black);
            }

            window->draw(pixels[x][y]);
        }
    }

    window->display();
}
