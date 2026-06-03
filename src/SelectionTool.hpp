#pragma once
#include "Tool.hpp"
#include "Layer.hpp"
#include <memory>
#include <SFML/Graphics.hpp>

class SelectionTool : public Tool {
private:
    std::shared_ptr<Layer> layer;
    sf::Vector2f selectionStart;
    sf::Vector2f selectionEnd;
    bool isSelecting;
    bool hasSelection;
    sf::Image selectedArea;
    sf::Vector2f selectedAreaPos;
    bool isDraggingSelection;
    sf::Vector2f dragStartPos;
    sf::Vector2f dragStartSelectionPos;
    sf::Color overlayColor;

public:
    SelectionTool(std::shared_ptr<Layer> l)
        : layer(l), isSelecting(false), hasSelection(false), isDraggingSelection(false),
          overlayColor(sf::Color(100, 150, 255, 100)) {}

    void setLayer(std::shared_ptr<Layer> l) { layer = l; }

    void onPress(sf::Vector2f pos, sf::Color, float) override {
        if (!layer) return;
        if (hasSelection) {
            sf::FloatRect rect(selectedAreaPos.x, selectedAreaPos.y,
                               selectedArea.getSize().x, selectedArea.getSize().y);
            if (rect.contains(pos)) {
                isDraggingSelection = true;
                dragStartPos = pos;
                dragStartSelectionPos = selectedAreaPos;
                return;
            } else {
                hasSelection = false;
                selectedArea = sf::Image();
                isSelecting = true;
                selectionStart = pos;
                selectionEnd = pos;
                return;
            }
        }
        isSelecting = true;
        selectionStart = pos;
        selectionEnd = pos;
    }

    void onDrag(sf::Vector2f /*from*/, sf::Vector2f to, sf::Color, float) override {
        if (isDraggingSelection) {
            sf::Vector2f delta = to - dragStartPos;
            sf::Vector2f newPos = dragStartSelectionPos + delta;

            sf::Vector2u canvasSize = layer->getTexture().getSize();
            float maxX = static_cast<float>(canvasSize.x - selectedArea.getSize().x);
            float maxY = static_cast<float>(canvasSize.y - selectedArea.getSize().y);
            if (maxX < 0) maxX = 0;
            if (maxY < 0) maxY = 0;
            newPos.x = std::max(0.0f, std::min(newPos.x, maxX));
            newPos.y = std::max(0.0f, std::min(newPos.y, maxY));

            selectedAreaPos = newPos;
        } else if (isSelecting) {
            selectionEnd = to;
        }
    }

    void onRelease() override {
        if (isDraggingSelection) {
            if (selectedArea.getSize().x == 0 || selectedArea.getSize().y == 0) {
                isDraggingSelection = false;
                return;
            }
            sf::Image layerImage = layer->getTexture().getTexture().copyToImage();
            int x1 = (int)dragStartSelectionPos.x;
            int y1 = (int)dragStartSelectionPos.y;
            int w = selectedArea.getSize().x;
            int h = selectedArea.getSize().y;
            for (int y = y1; y < y1 + h; ++y) {
                for (int x = x1; x < x1 + w; ++x) {
                    if (x >= 0 && x < (int)layerImage.getSize().x && y >= 0 && y < (int)layerImage.getSize().y) {
                        layerImage.setPixel(x, y, sf::Color::White);
                    }
                }
            }
            sf::Texture texClear; texClear.loadFromImage(layerImage);
            layer->getTexture().clear(sf::Color::Transparent);
            layer->getTexture().draw(sf::Sprite(texClear));

            sf::Texture texArea; texArea.loadFromImage(selectedArea);
            sf::Sprite sprite(texArea);
            sprite.setPosition(selectedAreaPos);
            layer->getTexture().draw(sprite);
            layer->display();

            sf::Image newSelectedArea;
            newSelectedArea.create(w, h);
            sf::Image updatedLayerImage = layer->getTexture().getTexture().copyToImage();
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    int sx = (int)selectedAreaPos.x + x;
                    int sy = (int)selectedAreaPos.y + y;
                    if (sx >= 0 && sx < (int)updatedLayerImage.getSize().x && sy >= 0 && sy < (int)updatedLayerImage.getSize().y) {
                        newSelectedArea.setPixel(x, y, updatedLayerImage.getPixel(sx, sy));
                    } else {
                        newSelectedArea.setPixel(x, y, sf::Color::White);
                    }
                }
            }
            selectedArea = std::move(newSelectedArea);
            isDraggingSelection = false;
        }
        else if (isSelecting) {
            sf::Image layerImage = layer->getTexture().getTexture().copyToImage();
            int x1 = (int)std::min(selectionStart.x, selectionEnd.x);
            int y1 = (int)std::min(selectionStart.y, selectionEnd.y);
            int x2 = (int)std::max(selectionStart.x, selectionEnd.x);
            int y2 = (int)std::max(selectionStart.y, selectionEnd.y);
            int w = x2 - x1;
            int h = y2 - y1;
            if (w > 0 && h > 0) {
                selectedArea.create(w, h);
                for (int y = 0; y < h; ++y)
                    for (int x = 0; x < w; ++x)
                        selectedArea.setPixel(x, y, layerImage.getPixel(x1 + x, y1 + y));
                selectedAreaPos = sf::Vector2f(x1, y1);
                hasSelection = true;
            }
            isSelecting = false;
        }
    }

    sf::FloatRect getSelectionRect() const {
        if (isSelecting) {
            return sf::FloatRect(std::min(selectionStart.x, selectionEnd.x),
                                 std::min(selectionStart.y, selectionEnd.y),
                                 std::abs(selectionEnd.x - selectionStart.x),
                                 std::abs(selectionEnd.y - selectionStart.y));
        }
        if (hasSelection) {
            return sf::FloatRect(selectedAreaPos.x, selectedAreaPos.y,
                                 selectedArea.getSize().x, selectedArea.getSize().y);
        }
        return sf::FloatRect();
    }

    bool isActive() const { return isSelecting || hasSelection; }
    sf::Color getOverlayColor() const { return overlayColor; }

    std::string getName() const override { return "Selection"; }
};