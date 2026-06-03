#pragma once
#include "Tool.hpp"
#include "Layer.hpp"
#include <memory>
#include <cmath>
#include <vector>

class BrushTool : public Tool {
private:
    std::shared_ptr<Layer> layer;
public:
    BrushTool(std::shared_ptr<Layer> l) : layer(l) {}
    void setLayer(std::shared_ptr<Layer> l) { layer = l; }
    void onPress(sf::Vector2f pos, sf::Color color, float size) override {
        if (!layer) return;
        sf::CircleShape brush(size);
        brush.setFillColor(color);
        brush.setPosition(pos.x - size, pos.y - size);
        layer->getTexture().draw(brush);
    }
    void onDrag(sf::Vector2f from, sf::Vector2f to, sf::Color color, float size) override {
        if (!layer) return;
        float dist = std::hypot(to.x - from.x, to.y - from.y);
        int steps = std::max(1, static_cast<int>(dist / (size * 0.25f)));
        for (int i = 0; i <= steps; ++i) {
            float t = static_cast<float>(i) / steps;
            sf::Vector2f pos(from.x + (to.x - from.x) * t, from.y + (to.y - from.y) * t);
            sf::CircleShape brush(size);
            brush.setFillColor(color);
            brush.setPosition(pos.x - size, pos.y - size);
            layer->getTexture().draw(brush);
        }
    }
    void onRelease() override { if (layer) layer->display(); }
    std::string getName() const override { return "Brush"; }
};

class FillTool : public Tool {
private:
    std::shared_ptr<Layer> layer;
    void floodFill(sf::Image& image, int x, int y, sf::Color target, sf::Color fill) {
        if (target == fill) return;
        sf::Vector2u size = image.getSize();
        std::vector<sf::Vector2i> stack;
        stack.push_back(sf::Vector2i(x, y));
        while (!stack.empty()) {
            sf::Vector2i cur = stack.back();
            stack.pop_back();
            if (cur.x < 0 || cur.x >= (int)size.x || cur.y < 0 || cur.y >= (int)size.y) continue;
            if (image.getPixel(cur.x, cur.y) != target) continue;
            image.setPixel(cur.x, cur.y, fill);
            stack.push_back(sf::Vector2i(cur.x+1, cur.y));
            stack.push_back(sf::Vector2i(cur.x-1, cur.y));
            stack.push_back(sf::Vector2i(cur.x, cur.y+1));
            stack.push_back(sf::Vector2i(cur.x, cur.y-1));
        }
    }
public:
    FillTool(std::shared_ptr<Layer> l) : layer(l) {}
    void setLayer(std::shared_ptr<Layer> l) { layer = l; }
    void onPress(sf::Vector2f pos, sf::Color color, float) override {
        if (!layer) return;
        sf::Image img = layer->getTexture().getTexture().copyToImage();
        sf::Color target = img.getPixel((int)pos.x, (int)pos.y);
        floodFill(img, (int)pos.x, (int)pos.y, target, color);
        sf::Texture tex; tex.loadFromImage(img);
        layer->getTexture().clear(sf::Color::Transparent);
        layer->getTexture().draw(sf::Sprite(tex));
        layer->display();
    }
    void onDrag(sf::Vector2f, sf::Vector2f, sf::Color, float) override {}
    void onRelease() override {}
    std::string getName() const override { return "Fill"; }
};

class EraserTool : public Tool {
private:
    std::shared_ptr<Layer> layer;
public:
    EraserTool(std::shared_ptr<Layer> l) : layer(l) {}
    void setLayer(std::shared_ptr<Layer> l) { layer = l; }
    void onPress(sf::Vector2f pos, sf::Color, float size) override {
        if (!layer) return;
        sf::CircleShape brush(size);
        brush.setFillColor(sf::Color::White);
        brush.setPosition(pos.x - size, pos.y - size);
        layer->getTexture().draw(brush);
    }
    void onDrag(sf::Vector2f from, sf::Vector2f to, sf::Color, float size) override {
        if (!layer) return;
        float dist = std::hypot(to.x - from.x, to.y - from.y);
        int steps = std::max(1, static_cast<int>(dist / (size * 0.25f)));
        for (int i = 0; i <= steps; ++i) {
            float t = static_cast<float>(i) / steps;
            sf::Vector2f pos(from.x + (to.x - from.x) * t, from.y + (to.y - from.y) * t);
            sf::CircleShape brush(size);
            brush.setFillColor(sf::Color::White);
            brush.setPosition(pos.x - size, pos.y - size);
            layer->getTexture().draw(brush);
        }
    }
    void onRelease() override { if (layer) layer->display(); }
    std::string getName() const override { return "Eraser"; }
};

class PipetteTool : public Tool {
private:
    std::shared_ptr<Layer> layer;
    float* brushColor;
public:
    PipetteTool(std::shared_ptr<Layer> l, float* colorArray) : layer(l), brushColor(colorArray) {}
    void setLayer(std::shared_ptr<Layer> l) { layer = l; }
    void onPress(sf::Vector2f pos, sf::Color, float) override {
        if (!layer || !brushColor) return;
        sf::Image img = layer->getTexture().getTexture().copyToImage();
        sf::Vector2u sz = img.getSize();
        int x = (int)pos.x, y = (int)pos.y;
        if (x >= 0 && x < (int)sz.x && y >= 0 && y < (int)sz.y) {
            sf::Color c = img.getPixel(x, y);
            brushColor[0] = c.r / 255.0f;
            brushColor[1] = c.g / 255.0f;
            brushColor[2] = c.b / 255.0f;
        }
    }
    void onDrag(sf::Vector2f, sf::Vector2f, sf::Color, float) override {}
    void onRelease() override {}
    std::string getName() const override { return "Pipette"; }
};