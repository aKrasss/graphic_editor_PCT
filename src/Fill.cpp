#include <iostream>
#include "Fill.hpp"

void floodFill(sf::Image& image, sf::Vector2i startPoint, sf::Color targetColor, sf::Color replacementColor) {
    if (targetColor == replacementColor) return;

    std::stack<sf::Vector2i> pixels;
    pixels.push(startPoint);

    sf::Vector2u size = image.getSize();

    while (!pixels.empty()) {
        sf::Vector2i current = pixels.top();
        pixels.pop();

        int x = current.x;
        int y = current.y;

        if (x >= 0 && x < (int)size.x && y >= 0 && y < (int)size.y) {
            if (image.getPixel(x, y) == targetColor) {
                image.setPixel(x, y, replacementColor);

                pixels.push({ x + 1, y });
                pixels.push({ x - 1, y });
                pixels.push({ x, y + 1 });
                pixels.push({ x, y - 1 });
            }
        }
    }
}