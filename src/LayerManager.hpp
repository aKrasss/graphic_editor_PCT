#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Layer.hpp"

class LayerManager {
private:
    std::vector<std::shared_ptr<Layer>> layers;
    int currentLayerIndex;
    int nextLayerId;
    unsigned int canvasWidth;
    unsigned int canvasHeight;

public:
    LayerManager(unsigned int width, unsigned int height)
        : currentLayerIndex(0), nextLayerId(1), canvasWidth(width), canvasHeight(height) {
        addLayer("Background");
        layers[0]->clear(sf::Color::White);
    }

    void addLayer(const std::string& name = "New Layer") {
        auto layer = std::make_shared<Layer>(canvasWidth, canvasHeight, nextLayerId++, name);
        layers.push_back(layer);
        currentLayerIndex = layers.size() - 1;
    }

    void deleteLayer(int index) {
        if (layers.size() > 1 && index >= 0 && index < static_cast<int>(layers.size())) {
            layers.erase(layers.begin() + index);
            if (currentLayerIndex >= static_cast<int>(layers.size()))
                currentLayerIndex = layers.size() - 1;
        }
    }

    void moveLayer(int from, int to) {
        if (from < 0 || from >= (int)layers.size() || to < 0 || to >= (int)layers.size()) return;
        if (from == to) return;
        auto layer = std::move(layers[from]);
        layers.erase(layers.begin() + from);
        layers.insert(layers.begin() + to, std::move(layer));
        if (currentLayerIndex == from) currentLayerIndex = to;
        else if (currentLayerIndex > from && currentLayerIndex <= to) currentLayerIndex--;
        else if (currentLayerIndex < from && currentLayerIndex >= to) currentLayerIndex++;
    }

    void setCurrentLayer(int index) {
        if (index >= 0 && index < static_cast<int>(layers.size()))
            currentLayerIndex = index;
    }

    std::shared_ptr<Layer> getCurrentLayer() {
        if (currentLayerIndex >= 0 && currentLayerIndex < static_cast<int>(layers.size()))
            return layers[currentLayerIndex];
        return nullptr;
    }

    std::vector<std::shared_ptr<Layer>>& getLayers() { return layers; }
    int getCurrentLayerIndex() const { return currentLayerIndex; }
    size_t getLayerCount() const { return layers.size(); }

    void resizeLayers(unsigned int newWidth, unsigned int newHeight) {
        std::vector<sf::Image> oldImages;
        for (auto& l : layers) oldImages.push_back(l->getTexture().getTexture().copyToImage());
        canvasWidth = newWidth; canvasHeight = newHeight;
        for (size_t i = 0; i < layers.size(); ++i) {
            auto& l = layers[i];
            sf::Image old = oldImages[i];
            l = std::make_shared<Layer>(newWidth, newHeight, l->getId(), l->getName());
            l->setOpacity(l->getOpacity());
            if (i == 0) l->clear(sf::Color::White);
            sf::Texture tex; tex.loadFromImage(old);
            l->getTexture().draw(sf::Sprite(tex));
            l->display();
        }
    }
};