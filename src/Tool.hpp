#pragma once
#include <SFML/Graphics.hpp>

// Базовый класс для всех инструментов
class Tool {
public:
    virtual ~Tool() = default;
    virtual void onPress(sf::Vector2f position, sf::Color color, float size) = 0;
    virtual void onDrag(sf::Vector2f from, sf::Vector2f to, sf::Color color, float size) = 0;
    virtual void onRelease() = 0;
    virtual std::string getName() const = 0;
};
