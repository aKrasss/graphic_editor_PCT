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

    std::shared_ptr<Layer> getCurrentLayer() {
        if (currentLayerIndex >= 0 && currentLayerIndex < static_cast<int>(layers.size()))
            return layers[currentLayerIndex];
        return nullptr;
    }

    std::vector<std::shared_ptr<Layer>>& getLayers() {
        return layers;
    }
};