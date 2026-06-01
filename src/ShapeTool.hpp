#pragma once
#include "Tool.hpp"
#include "Layer.hpp"
#include <memory>
#include <cmath>
#include <vector>

enum class ShapeType { Rectangle, Square, Circle, Ellipse, Line, Star };

class ShapeTool : public Tool {
private:
    std::shared_ptr<Layer> layer;
    ShapeType shapeType;
    sf::Vector2f startPos;
    sf::Vector2f endPos;
    bool drawing;
    bool showPreview;
    sf::Vector2f previewEnd;
    sf::Color currentColor;
    float currentThickness;

    void drawThickLine(sf::RenderTexture& target, sf::Vector2f p1, sf::Vector2f p2, sf::Color color, float thickness) {
        sf::Vector2f direction = p2 - p1;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length < 0.001f) return;
        direction /= length;
        sf::Vector2f perpendicular(-direction.y, direction.x);
        sf::Vector2f offset = perpendicular * (thickness / 2.0f);
        
        sf::Vertex vertices[4];
        vertices[0].position = p1 - offset;
        vertices[1].position = p1 + offset;
        vertices[2].position = p2 + offset;
        vertices[3].position = p2 - offset;
        for (int i = 0; i < 4; ++i) vertices[i].color = color;
        target.draw(vertices, 4, sf::Quads);
    }

    void drawStar(sf::RenderTexture& target, sf::Vector2f center, float outerRadius, float innerRadius, int points, sf::Color color, float thickness) {
        std::vector<sf::Vector2f> vertices;
        float angleStep = 360.0f / (2 * points);
        float startAngle = -90.0f;
        for (int i = 0; i < 2 * points; ++i) {
            float radius = (i % 2 == 0) ? outerRadius : innerRadius;
            float angle = (startAngle + i * angleStep) * 3.14159f / 180.0f;
            float x = center.x + radius * std::cos(angle);
            float y = center.y + radius * std::sin(angle);
            vertices.push_back(sf::Vector2f(x, y));
        }
        for (size_t i = 0; i < vertices.size(); ++i) {
            const auto& p1 = vertices[i];
            const auto& p2 = vertices[(i + 1) % vertices.size()];
            drawThickLine(target, p1, p2, color, thickness);
        }
    }

    void drawShape(sf::Vector2f start, sf::Vector2f end, sf::Color color, float thickness) {
        if (!layer) return;
        float w = std::abs(end.x - start.x);
        float h = std::abs(end.y - start.y);
        sf::Vector2f topLeft(std::min(start.x, end.x), std::min(start.y, end.y));

        switch (shapeType) {
            case ShapeType::Rectangle: {
                sf::RectangleShape rect(sf::Vector2f(w, h));
                rect.setPosition(topLeft);
                rect.setFillColor(sf::Color::Transparent);
                rect.setOutlineColor(color);
                rect.setOutlineThickness(thickness);
                layer->getTexture().draw(rect);
                break;
            }
            case ShapeType::Square: {
                float side = std::max(w, h);
                sf::RectangleShape square(sf::Vector2f(side, side));
                square.setPosition(topLeft);
                square.setFillColor(sf::Color::Transparent);
                square.setOutlineColor(color);
                square.setOutlineThickness(thickness);
                layer->getTexture().draw(square);
                break;
            }
            case ShapeType::Circle: {
                float radius = std::max(w, h) / 2;
                sf::CircleShape circle(radius);
                circle.setPosition(topLeft.x + (w - 2*radius)/2, topLeft.y + (h - 2*radius)/2);
                circle.setFillColor(sf::Color::Transparent);
                circle.setOutlineColor(color);
                circle.setOutlineThickness(thickness);
                layer->getTexture().draw(circle);
                break;
            }
            case ShapeType::Ellipse: {
                sf::ConvexShape ellipse;
                ellipse.setPointCount(50);
                float rx = w / 2, ry = h / 2;
                sf::Vector2f center(topLeft.x + rx, topLeft.y + ry);
                for (int i = 0; i < 50; ++i) {
                    float angle = 2 * 3.14159f * i / 50;
                    float x = center.x + rx * std::cos(angle);
                    float y = center.y + ry * std::sin(angle);
                    ellipse.setPoint(i, sf::Vector2f(x, y));
                }
                ellipse.setFillColor(sf::Color::Transparent);
                ellipse.setOutlineColor(color);
                ellipse.setOutlineThickness(thickness);
                layer->getTexture().draw(ellipse);
                break;
            }
            case ShapeType::Line: {
                drawThickLine(layer->getTexture(), start, end, color, thickness);
                break;
            }
            case ShapeType::Star: {
                sf::Vector2f center = start;
                float outerRadius = std::hypot(end.x - start.x, end.y - start.y);
                float innerRadius = outerRadius * 0.4f;
                drawStar(layer->getTexture(), center, outerRadius, innerRadius, 5, color, thickness);
                break;
            }
        }
        layer->display();
    }

    void drawPreview(sf::RenderWindow& window, sf::Vector2f canvasOffset, float zoomLevel) {
        if (!showPreview || !drawing) return;
        sf::Vector2f startScreen = canvasOffset + startPos * zoomLevel;
        sf::Vector2f endScreen = canvasOffset + previewEnd * zoomLevel;
        float w = std::abs(endScreen.x - startScreen.x);
        float h = std::abs(endScreen.y - startScreen.y);
        sf::Vector2f topLeft(std::min(startScreen.x, endScreen.x), std::min(startScreen.y, endScreen.y));
        sf::Color previewColor = currentColor;
        previewColor.a = 180;

        switch (shapeType) {
            case ShapeType::Rectangle: {
                sf::RectangleShape rect(sf::Vector2f(w, h));
                rect.setPosition(topLeft);
                rect.setFillColor(sf::Color::Transparent);
                rect.setOutlineColor(previewColor);
                rect.setOutlineThickness(currentThickness);
                window.draw(rect);
                break;
            }
            case ShapeType::Square: {
                float side = std::max(w, h);
                sf::RectangleShape square(sf::Vector2f(side, side));
                square.setPosition(topLeft);
                square.setFillColor(sf::Color::Transparent);
                square.setOutlineColor(previewColor);
                square.setOutlineThickness(currentThickness);
                window.draw(square);
                break;
            }
            case ShapeType::Circle: {
                float radius = std::max(w, h) / 2;
                sf::CircleShape circle(radius);
                circle.setPosition(topLeft.x + (w - 2*radius)/2, topLeft.y + (h - 2*radius)/2);
                circle.setFillColor(sf::Color::Transparent);
                circle.setOutlineColor(previewColor);
                circle.setOutlineThickness(currentThickness);
                window.draw(circle);
                break;
            }
            case ShapeType::Ellipse: {
                sf::ConvexShape ellipse;
                ellipse.setPointCount(50);
                float rx = w / 2, ry = h / 2;
                sf::Vector2f center(topLeft.x + rx, topLeft.y + ry);
                for (int i = 0; i < 50; ++i) {
                    float angle = 2 * 3.14159f * i / 50;
                    float x = center.x + rx * std::cos(angle);
                    float y = center.y + ry * std::sin(angle);
                    ellipse.setPoint(i, sf::Vector2f(x, y));
                }
                ellipse.setFillColor(sf::Color::Transparent);
                ellipse.setOutlineColor(previewColor);
                ellipse.setOutlineThickness(currentThickness);
                window.draw(ellipse);
                break;
            }
            case ShapeType::Line: {
                drawThickLinePreview(window, startScreen, endScreen, previewColor, currentThickness);
                break;
            }
            case ShapeType::Star: {
                sf::Vector2f centerScreen = startScreen;
                float outerRadius = std::hypot(endScreen.x - startScreen.x, endScreen.y - startScreen.y);
                float innerRadius = outerRadius * 0.4f;
                int points = 5;
                float angleStep = 360.0f / (2 * points);
                float startAngle = -90.0f;
                std::vector<sf::Vector2f> vertices;
                for (int i = 0; i < 2 * points; ++i) {
                    float radius = (i % 2 == 0) ? outerRadius : innerRadius;
                    float angle = (startAngle + i * angleStep) * 3.14159f / 180.0f;
                    float x = centerScreen.x + radius * std::cos(angle);
                    float y = centerScreen.y + radius * std::sin(angle);
                    vertices.push_back(sf::Vector2f(x, y));
                }
                for (size_t i = 0; i < vertices.size(); ++i) {
                    drawThickLinePreview(window, vertices[i], vertices[(i+1)%vertices.size()], previewColor, currentThickness);
                }
                break;
            }
        }
    }

    void drawThickLinePreview(sf::RenderWindow& target, sf::Vector2f p1, sf::Vector2f p2, sf::Color color, float thickness) {
        sf::Vector2f direction = p2 - p1;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length < 0.001f) return;
        direction /= length;
        sf::Vector2f perpendicular(-direction.y, direction.x);
        sf::Vector2f offset = perpendicular * (thickness / 2.0f);
        
        sf::Vertex vertices[4];
        vertices[0].position = p1 - offset;
        vertices[1].position = p1 + offset;
        vertices[2].position = p2 + offset;
        vertices[3].position = p2 - offset;
        for (int i = 0; i < 4; ++i) vertices[i].color = color;
        target.draw(vertices, 4, sf::Quads);
    }

public:
    ShapeTool(std::shared_ptr<Layer> l, ShapeType type = ShapeType::Rectangle)
        : layer(l), shapeType(type), drawing(false), showPreview(false) {}

    void setLayer(std::shared_ptr<Layer> l) { layer = l; }
    void setShapeType(ShapeType type) { shapeType = type; }

    void onPress(sf::Vector2f pos, sf::Color color, float size) override {
        if (!layer) return;
        startPos = pos;
        endPos = pos;
        previewEnd = pos;
        currentColor = color;
        currentThickness = size;
        drawing = true;
        showPreview = true;
    }

    void onDrag(sf::Vector2f from, sf::Vector2f to, sf::Color, float) override {
        if (drawing) {
            endPos = to;
            previewEnd = to;
        }
    }

    void onRelease() override {
        if (!drawing || !layer) return;
        float dist = std::hypot(endPos.x - startPos.x, endPos.y - startPos.y);
        if (dist > 2.0f) {
            drawShape(startPos, endPos, currentColor, currentThickness);
        }
        drawing = false;
        showPreview = false;
    }

    void renderPreview(sf::RenderWindow& window, sf::Vector2f canvasOffset, float zoomLevel) {
        if (showPreview && drawing) {
            drawPreview(window, canvasOffset, zoomLevel);
        }
    }

    std::string getName() const override { return "Shape"; }
};