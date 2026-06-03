#pragma once
#include "imgui.h"
#include "Localization.hpp"
#include "LayerManager.hpp"
#include "Tools.hpp"
#include "Filters.hpp"
#include "SelectionTool.hpp"
#include "TextTool.hpp"
#include "ShapeTool.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <algorithm>

class EditorUI {
private:
    Localization& localization;
    LayerManager& layerManager;

    float* brushSize;
    float* brushColor;
    float* zoomLevel;
    bool* showGrid;
    bool* showRulers;
    int* canvasWidth;
    int* canvasHeight;
    sf::Vector2i* mouseCanvasPos;

    std::string currentToolName;
    std::vector<std::unique_ptr<Tool>> tools;
    Tool* currentTool = nullptr;
    std::shared_ptr<Layer> currentLayerForTools;

    SelectionTool* selectionTool = nullptr;
    TextTool* textTool = nullptr;
    ShapeTool* shapeTool = nullptr;

    std::string getToolKey(const std::string& toolName) {
        if (toolName == "Brush") return "brush";
        if (toolName == "Fill") return "fill";
        if (toolName == "Eraser") return "eraser";
        if (toolName == "Pipette") return "pipette";
        if (toolName == "Shape") return "shape";
        if (toolName == "Selection") return "selection";
        if (toolName == "Text") return "text";
        return toolName;
    }

    void updateLayerNames() {
        auto& layers = layerManager.getLayers();
        if (layers.empty()) return;
        layers[0]->setName(localization.get("background"));
        for (size_t i = 1; i < layers.size(); ++i) {
            if (layers[i]->getName() == "New Layer") {
                layers[i]->setName(localization.get("new_layer"));
            }
        }
    }

public:
    EditorUI(Localization& loc, LayerManager& lm)
        : localization(loc), layerManager(lm),
          brushSize(nullptr), brushColor(nullptr), zoomLevel(nullptr),
          showGrid(nullptr), showRulers(nullptr),
          canvasWidth(nullptr), canvasHeight(nullptr),
          mouseCanvasPos(nullptr),
          currentToolName("Brush") {}

    void setParameters(float* bSize, float* bColor, float* zoom,
                      bool* grid, bool* rulers,
                      int* cWidth, int* cHeight,
                      sf::Vector2i* mousePos) {
        brushSize = bSize;
        brushColor = bColor;
        zoomLevel = zoom;
        showGrid = grid;
        showRulers = rulers;
        canvasWidth = cWidth;
        canvasHeight = cHeight;
        mouseCanvasPos = mousePos;
    }

    void initTools(std::shared_ptr<Layer> layer, float* brushColorArray) {
        currentLayerForTools = layer;
        tools.clear();

        tools.push_back(std::make_unique<BrushTool>(layer));                     // 0
        tools.push_back(std::make_unique<FillTool>(layer));                     // 1
        tools.push_back(std::make_unique<EraserTool>(layer));                   // 2
        tools.push_back(std::make_unique<PipetteTool>(layer, brushColorArray)); // 3

        auto shape = std::make_unique<ShapeTool>(layer, ShapeType::Rectangle);
        shapeTool = shape.get();
        tools.push_back(std::move(shape));                                      // 4

        tools.push_back(std::make_unique<SelectionTool>(layer));                // 5
        tools.push_back(std::make_unique<TextTool>(layer, &localization));                     // 6

        currentTool = tools[0].get();
        setCurrentToolName(currentTool->getName());

        selectionTool = dynamic_cast<SelectionTool*>(tools[5].get());
        textTool = dynamic_cast<TextTool*>(tools[6].get());

        updateLayerNames();
    }

    void setCurrentTool(int index) {
        if (index >= 0 && index < (int)tools.size()) {
            currentTool = tools[index].get();
            setCurrentToolName(currentTool->getName());
        }
    }

    void setCurrentShapeType(ShapeType type) {
        if (shapeTool) shapeTool->setShapeType(type);
    }

    Tool* getCurrentTool() { return currentTool; }
    SelectionTool* getSelectionTool() { return selectionTool; }
    TextTool* getTextTool() { return textTool; }
    std::string getCurrentToolName() const { return currentToolName; }

    void updateToolLayer(std::shared_ptr<Layer> layer) {
        currentLayerForTools = layer;
        for (auto& t : tools) {
            if (auto brush = dynamic_cast<BrushTool*>(t.get())) brush->setLayer(layer);
            else if (auto fill = dynamic_cast<FillTool*>(t.get())) fill->setLayer(layer);
            else if (auto eraser = dynamic_cast<EraserTool*>(t.get())) eraser->setLayer(layer);
            else if (auto pipette = dynamic_cast<PipetteTool*>(t.get())) pipette->setLayer(layer);
            else if (auto shape = dynamic_cast<ShapeTool*>(t.get())) shape->setLayer(layer);
            else if (auto sel = dynamic_cast<SelectionTool*>(t.get())) sel->setLayer(layer);
            else if (auto txt = dynamic_cast<TextTool*>(t.get())) txt->setLayer(layer);
        }
    }

    template<typename FilterT, typename... Args>
    void applyFilter(Args&&... args) {
        auto layer = layerManager.getCurrentLayer();
        if (!layer) return;
        sf::Image img = layer->getTexture().getTexture().copyToImage();
        FilterT filter(std::forward<Args>(args)...);
        filter.apply(img);
        sf::Texture tex;
        tex.loadFromImage(img);
        layer->getTexture().clear(sf::Color::Transparent);
        layer->getTexture().draw(sf::Sprite(tex));
        layer->display();
    }

    void setCurrentToolName(const std::string& name) {
        currentToolName = name;
    }

    void renderToolPanel(sf::RenderWindow& window,
                        std::function<void()> onLoadImage,
                        std::function<void()> onSaveImage,
                        std::function<void()> onClearCanvas,
                        std::function<void()> onResizeCanvas) {

        float rulerHeight = 30.0f;
        float rulerSide = 30.0f;
        float menuWidth = 300;
        ImGui::SetNextWindowPos(ImVec2(window.getSize().x - menuWidth, rulerHeight));
        ImGui::SetNextWindowSize(ImVec2(menuWidth, window.getSize().y - rulerHeight));
        ImGui::Begin(localization.get("tools").c_str(), nullptr,
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // Выбор языка
        ImGui::Text("%s:", localization.get("language").c_str());
        if (ImGui::Button("English", ImVec2(140,25))) {
            localization.setLanguage(Language::ENG);
            updateLayerNames();
        }
        ImGui::SameLine();
        if (ImGui::Button("Русский", ImVec2(140,25))) {
            localization.setLanguage(Language::RU);
            updateLayerNames();
        }
        ImGui::Separator();

        if (mouseCanvasPos) {
            ImGui::Text("%s: %d, %d", localization.get("mouse_pos").c_str(),
                       mouseCanvasPos->x, mouseCanvasPos->y);
        }
        ImGui::Separator();

        ImGui::Text("%s: %s", localization.get("tool").c_str(),
                   localization.get(getToolKey(currentToolName)).c_str());
        ImGui::Text("%s:", localization.get("tools").c_str());

        const char* toolKeys[] = {"brush", "fill", "eraser", "pipette", "shape", "selection", "text"};
        for (int i = 0; i < 7; ++i) {
            if (ImGui::Button(localization.get(toolKeys[i]).c_str(), ImVec2(90,25))) {
                setCurrentTool(i);
            }
            if (i % 3 != 2) ImGui::SameLine();
        }
        ImGui::Separator();

        if (currentToolName == "Shape") {
            ImGui::Text("%s:", localization.get("shape_type").c_str());
            const char* shapeKeys[] = {"rectangle", "square", "circle", "ellipse", "line", "star"};
            static int shapeIdx = 0;
            std::string shapeNamesStr[6];
            const char* shapeNames[6];
            for (int i = 0; i < 6; ++i) {
                shapeNamesStr[i] = localization.get(shapeKeys[i]);
                shapeNames[i] = shapeNamesStr[i].c_str();
            }
            if (ImGui::Combo("##shape", &shapeIdx, shapeNames, 6)) {
                setCurrentShapeType((ShapeType)shapeIdx);
            }
        }
        ImGui::Separator();

        if (zoomLevel) {
            ImGui::Text("%s: %.1f%%", localization.get("zoom").c_str(), *zoomLevel * 100.0f);
            if (ImGui::SliderFloat("##zoom", zoomLevel, 0.1f, 5.0f, "%.1f")) {
                *zoomLevel = std::max(0.1f, std::min(*zoomLevel, 5.0f));
            }
        }
        ImGui::Separator();

        if (canvasWidth && canvasHeight) {
            ImGui::Text("%s:", localization.get("canvas_size").c_str());
            ImGui::InputInt("Width##canvas", canvasWidth);
            *canvasWidth = std::max(100, std::min(*canvasWidth, 5000));
            ImGui::InputInt("Height##canvas", canvasHeight);
            *canvasHeight = std::max(100, std::min(*canvasHeight, 5000));
            if (ImGui::Button(localization.get("apply_size").c_str(), ImVec2(280,25))) {
                if (onResizeCanvas) onResizeCanvas();
            }
        }
        ImGui::Separator();

        if (brushSize && brushColor) {
            ImGui::Text("%s:", localization.get("brush").c_str());
            ImGui::SliderFloat("Size##brush", brushSize, 1.0f, 50.0f);
            ImGui::ColorEdit3("Color##brush", brushColor);
        }
        ImGui::Separator();

        if (showGrid && showRulers) {
            ImGui::Checkbox(localization.get("grid").c_str(), showGrid);
            ImGui::Checkbox(localization.get("rulers").c_str(), showRulers);
        }
        ImGui::Separator();

        renderLayersPanel();
        ImGui::Separator();

        renderFileOperations(onLoadImage, onSaveImage, onClearCanvas);
        ImGui::Separator();

        ImGui::Text("%s:", localization.get("filters").c_str());
        if (ImGui::Button(localization.get("grayscale").c_str(), ImVec2(135,25))) { applyFilter<GrayscaleFilter>(); }
        ImGui::SameLine();
        if (ImGui::Button(localization.get("invert").c_str(), ImVec2(135,25))) { applyFilter<InvertFilter>(); }
        if (ImGui::Button(localization.get("blur").c_str(), ImVec2(135,25))) { applyFilter<BlurFilter>(); }
        ImGui::SameLine();
        if (ImGui::Button(localization.get("sharpen").c_str(), ImVec2(135,25))) { applyFilter<SharpenFilter>(); }
        if (ImGui::Button(localization.get("sepia").c_str(), ImVec2(135,25))) { applyFilter<SepiaFilter>(); }
        ImGui::SameLine();
        if (ImGui::Button(localization.get("bright_plus").c_str(), ImVec2(67,25))) { applyFilter<BrightnessFilter>(30); }
        ImGui::SameLine();
        if (ImGui::Button(localization.get("bright_minus").c_str(), ImVec2(67,25))) { applyFilter<BrightnessFilter>(-30); }

        ImGui::End();

        if (textTool) textTool->renderTextPopup();
    }

    void renderLayersPanel() {
        ImGui::Text("%s:", localization.get("layers").c_str());
        if (ImGui::Button(localization.get("new_layer").c_str(), ImVec2(280,25))) {
            layerManager.addLayer();
            updateToolLayer(layerManager.getCurrentLayer());
            auto& layers = layerManager.getLayers();
            if (!layers.empty()) {
                layers.back()->setName(localization.get("new_layer"));
            }
        }
        auto& layers = layerManager.getLayers();
        for (int i = layers.size()-1; i >= 0; --i) {
            ImGui::PushID(i);
            bool isCurrent = (i == layerManager.getCurrentLayerIndex());
            if (ImGui::Selectable(layers[i]->getName().c_str(), isCurrent)) {
                layerManager.setCurrentLayer(i);
                updateToolLayer(layerManager.getCurrentLayer());
            }
            float opacity = layers[i]->getOpacity();
            ImGui::Text("%s:", localization.get("opacity").c_str());
            if (ImGui::SliderFloat("##opacity", &opacity, 0.0f, 1.0f, "%.2f")) {
                layers[i]->setOpacity(opacity);
            }
            ImGui::PopID();
        }
        if (layers.size() > 1 && ImGui::Button(localization.get("delete_layer").c_str(), ImVec2(280,25))) {
            layerManager.deleteLayer(layerManager.getCurrentLayerIndex());
            updateToolLayer(layerManager.getCurrentLayer());
        }
    }

    void renderFileOperations(std::function<void()> onLoadImage,
                              std::function<void()> onSaveImage,
                              std::function<void()> onClearCanvas) {
        ImGui::Text("%s:", localization.get("file").c_str());
        if (ImGui::Button(localization.get("load").c_str(), ImVec2(135,30))) {
            if (onLoadImage) onLoadImage();
            updateToolLayer(layerManager.getCurrentLayer());
        }
        ImGui::SameLine();
        if (ImGui::Button(localization.get("save").c_str(), ImVec2(135,30))) {
            if (onSaveImage) onSaveImage();
        }
        if (ImGui::Button(localization.get("clear").c_str(), ImVec2(280,30))) {
            if (onClearCanvas) onClearCanvas();
        }
    }

    void renderRulers(sf::RenderWindow& window,
                    sf::Vector2f canvasOffset,
                    float zoomLevel,
                    int canvasWidth,
                    int canvasHeight) {
        sf::Vector2u windowSize = window.getSize();
        float rulerHeight = 30.0f;
        float rulerSide = 30.0f;

        sf::RectangleShape hRuler(sf::Vector2f(windowSize.x - rulerSide, rulerHeight));
        hRuler.setPosition(rulerSide, 0);
        hRuler.setFillColor(sf::Color(220, 220, 220));
        window.draw(hRuler);

        sf::RectangleShape vRuler(sf::Vector2f(rulerSide, windowSize.y - rulerHeight));
        vRuler.setPosition(0, rulerHeight);
        vRuler.setFillColor(sf::Color(220, 220, 220));
        window.draw(vRuler);

        sf::RectangleShape corner(sf::Vector2f(rulerSide, rulerHeight));
        corner.setPosition(0, 0);
        corner.setFillColor(sf::Color(220, 220, 220));
        window.draw(corner);

        int targetScreenStep = 80;
        float stepPixels = targetScreenStep / zoomLevel;
        int step = 1;
        if (stepPixels >= 500) step = 500;
        else if (stepPixels >= 200) step = 200;
        else if (stepPixels >= 100) step = 100;
        else if (stepPixels >= 50) step = 50;
        else if (stepPixels >= 20) step = 20;
        else if (stepPixels >= 10) step = 10;
        else if (stepPixels >= 5) step = 5;
        else if (stepPixels >= 2) step = 2;
        else step = 1;

        sf::Font font;
        bool hasFont = font.loadFromFile("arialmt.ttf");

        float visibleLeft = -canvasOffset.x / zoomLevel;
        float visibleRight = visibleLeft + windowSize.x / zoomLevel;
        int extendedRange = static_cast<int>(windowSize.x / zoomLevel) * 2;
        int startX = static_cast<int>(visibleLeft) - extendedRange;
        int endX = static_cast<int>(visibleRight) + extendedRange;
        int firstTick = (startX / step) * step;

        float minX = rulerSide + 5.0f;

        for (int x = firstTick; x <= endX; x += step) {
            float screenX = canvasOffset.x + x * zoomLevel;
            if (screenX >= minX && screenX <= windowSize.x) {
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(screenX, 0), sf::Color::Black),
                    sf::Vertex(sf::Vector2f(screenX, rulerHeight - 5), sf::Color::Black)
                };
                window.draw(line, 2, sf::Lines);
                if (hasFont && step >= 5) {
                    sf::Text text(std::to_string(x), font, 10);
                    text.setFillColor(sf::Color::Black);
                    text.setPosition(screenX - 10, rulerHeight - 20);
                    window.draw(text);
                }
            }
        }

        float visibleTop = -canvasOffset.y / zoomLevel;
        float visibleBottom = visibleTop + windowSize.y / zoomLevel;
        int startY = static_cast<int>(visibleTop) - extendedRange;
        int endY = static_cast<int>(visibleBottom) + extendedRange;
        int firstTickY = (startY / step) * step;

        float minY = rulerHeight + 5.0f;

        for (int y = firstTickY; y <= endY; y += step) {
            float screenY = canvasOffset.y + y * zoomLevel;
            if (screenY >= minY && screenY <= windowSize.y) {
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(0, screenY), sf::Color::Black),
                    sf::Vertex(sf::Vector2f(rulerSide - 5, screenY), sf::Color::Black)
                };
                window.draw(line, 2, sf::Lines);
                if (hasFont && step >= 5) {
                    sf::Text text(std::to_string(y), font, 10);
                    text.setFillColor(sf::Color::Black);
                    text.setPosition(rulerSide - 20, screenY - 6);
                    window.draw(text);
                }
            }
        }
    }

    void renderGrid(sf::RenderWindow& window,
                    sf::Vector2f canvasOffset,
                    float zoomLevel,
                    int canvasWidth,
                    int canvasHeight) {
        int gridSize = 10;
        sf::Color gridColor(180, 180, 180, 200);
        for (int x = 0; x <= canvasWidth; x += gridSize) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(canvasOffset.x + x * zoomLevel, canvasOffset.y), gridColor),
                sf::Vertex(sf::Vector2f(canvasOffset.x + x * zoomLevel,
                                       canvasOffset.y + canvasHeight * zoomLevel), gridColor)
            };
            window.draw(line, 2, sf::Lines);
        }
        for (int y = 0; y <= canvasHeight; y += gridSize) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(canvasOffset.x, canvasOffset.y + y * zoomLevel), gridColor),
                sf::Vertex(sf::Vector2f(canvasOffset.x + canvasWidth * zoomLevel,
                                       canvasOffset.y + y * zoomLevel), gridColor)
            };
            window.draw(line, 2, sf::Lines);
        }
    }

    void renderSelectionOverlay(sf::RenderWindow& window, sf::Vector2f canvasOffset, float zoomLevel) {
        if (selectionTool && selectionTool->isActive()) {
            sf::FloatRect rect = selectionTool->getSelectionRect();
            if (rect.width > 0 && rect.height > 0) {
                sf::RectangleShape overlay(sf::Vector2f(rect.width * zoomLevel, rect.height * zoomLevel));
                overlay.setPosition(canvasOffset.x + rect.left * zoomLevel, canvasOffset.y + rect.top * zoomLevel);
                overlay.setFillColor(selectionTool->getOverlayColor());
                window.draw(overlay);
            }
        }
    }
};
