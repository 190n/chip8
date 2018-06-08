#include <SFML/Graphics.hpp>
#include "chip8.hpp"

#define SCALE_FACTOR 16

sf::RenderWindow* window;

sf::Uint8* pixels;
sf::Texture tex;
sf::Sprite sp;

void initGfx() {
    window = new sf::RenderWindow(sf::VideoMode(64 * SCALE_FACTOR, 32 * SCALE_FACTOR), "CHIP-8");
    pixels = new sf::Uint8[8192];
    for (int i = 0; i < 8192; i++) {
        if (i % 4 == 3) {
            pixels[i] = 255;
        } else {
            pixels[i] = 0;
        }
    }
    tex.create(64, 32);
    sp.setTexture(tex);
    sp.setScale(sf::Vector2f(SCALE_FACTOR, SCALE_FACTOR));
}

void drawGfx(chip8 cpu) {
    window->clear(sf::Color::Black);

    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 32; y++) {
            int i = y * 64 + x;
            if (cpu.gfx[i]) {
                pixels[i * 4] = 255;
                pixels[i * 4 + 1] = 255;
                pixels[i * 4 + 2] = 255;
            } else {
                pixels[i * 4] = 0;
                pixels[i * 4 + 1] = 0;
                pixels[i * 4 + 2] = 0;
            }
        }
    }

    tex.update(pixels);
    window->draw(sp);
    window->display();
}
