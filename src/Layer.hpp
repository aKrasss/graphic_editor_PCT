#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class Layer {
private:
    sf::RenderTexture texture;
    sf::Sprite sprite;
    std::string name;
    float opacity;
    bool visible;
    int id;

public:
    Layer(unsigned int width, unsigned int height, int layerId, const std::string& layerName = "Layer")
        : name(layerName), opacity(1.0f), visible(true), id(layerId) {
        texture.create(width, height);
        texture.clear(sf::Color::Transparent);
        texture.display();
        sprite.setTexture(texture.getTexture());
    }

    void clear(sf::Color color = sf::Color::Transparent) {
        texture.clear(color);
        texture.display();
    }

    sf::RenderTexture& getTexture() {
        return texture;
    }

    sf::Sprite& getSprite() {
        return sprite;
    }

    void setOpacity(float op) {
        opacity = op;
        sf::Color color = sprite.getColor();
        color.a = static_cast<sf::Uint8>(opacity * 255);
        sprite.setColor(color);
    }

    float getOpacity() const {
        return opacity;
    }

    void setVisible(bool vis) {
        visible = vis;
    }

    bool isVisible() const {
        return visible;
    }

    std::string getName() const {
        return name;
    }

    void setName(const std::string& newName) {
        name = newName;
    }

    int getId() const {
        return id;
    }

    void display() {
        texture.display();
    }
};
