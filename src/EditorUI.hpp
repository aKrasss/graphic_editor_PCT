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
#include <cstdint>

class EditorUI
{
private:
    Localization &localization;
    LayerManager &layerManager;

    float *brushSize;
    float *brushColor;
    float *zoomLevel;
    bool *showGrid;
    bool *showRulers;
    int *canvasWidth;
    int *canvasHeight;
    sf::Vector2i *mouseCanvasPos;

    std::string currentToolName;
    std::vector<std::unique_ptr<Tool>> tools;
    Tool *currentTool = nullptr;
    std::shared_ptr<Layer> currentLayerForTools;

    SelectionTool *selectionTool = nullptr;
    TextTool *textTool = nullptr;
    ShapeTool *shapeTool = nullptr;

    std::function<void()> saveStateCallback;
    std::function<void()> toggleFullscreenCallback;

    // Иконки
    sf::Texture iconPen, iconEraser, iconFill, iconPipette, iconShape, iconSelection, iconText, iconGarbage;
    ImTextureID penTextureID = nullptr, eraserTextureID = nullptr, fillTextureID = nullptr;
    ImTextureID pipetteTextureID = nullptr, shapeTextureID = nullptr, selectionTextureID = nullptr, textTextureID = nullptr, garbageTextureID = nullptr;
    bool iconsLoaded;

    void loadIcons() {
        iconsLoaded = false;
        auto loadTex = [&](sf::Texture& tex, const std::string& name) -> ImTextureID {
            if (tex.loadFromFile("Textures/" + name) || tex.loadFromFile("src/Textures/" + name)) {
                return reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex.getNativeHandle()));
            }
            return nullptr;
        };
        penTextureID = loadTex(iconPen, "pen.png");
        eraserTextureID = loadTex(iconEraser, "eraser.png");
        fillTextureID = loadTex(iconFill, "fill.png");
        pipetteTextureID = loadTex(iconPipette, "pipette.png");
        shapeTextureID = loadTex(iconShape, "figure.png");
        selectionTextureID = loadTex(iconSelection, "selection.png");
        textTextureID = loadTex(iconText, "text.png");
        garbageTextureID = loadTex(iconGarbage, "garbage.png");
        
        iconsLoaded = (penTextureID || eraserTextureID || fillTextureID || pipetteTextureID ||
                       shapeTextureID || selectionTextureID || textTextureID || garbageTextureID);
    }

    std::string getToolKey(const std::string &toolName)
    {
        if (toolName == "Brush") return "brush";
        if (toolName == "Fill") return "fill";
        if (toolName == "Eraser") return "eraser";
        if (toolName == "Pipette") return "pipette";
        if (toolName == "Shape") return "shape";
        if (toolName == "Selection") return "selection";
        if (toolName == "Text") return "text";
        return toolName;
    }

    void updateLayerNames()
    {
        auto &layers = layerManager.getLayers();
        if (layers.empty()) return;
        layers[0]->setName(localization.get("background"));
        for (size_t i = 1; i < layers.size(); ++i)
        {
            if (layers[i]->getName() == "New Layer")
                layers[i]->setName(localization.get("new_layer"));
        }
    }

public:
    EditorUI(Localization &loc, LayerManager &lm)
        : localization(loc), layerManager(lm),
          brushSize(nullptr), brushColor(nullptr), zoomLevel(nullptr),
          showGrid(nullptr), showRulers(nullptr),
          canvasWidth(nullptr), canvasHeight(nullptr),
          mouseCanvasPos(nullptr),
          currentToolName("Brush")
    {
        loadIcons();
    }

    void setParameters(float *bSize, float *bColor, float *zoom,
                       bool *grid, bool *rulers,
                       int *cWidth, int *cHeight,
                       sf::Vector2i *mousePos)
    {
        brushSize = bSize;
        brushColor = bColor;
        zoomLevel = zoom;
        showGrid = grid;
        showRulers = rulers;
        canvasWidth = cWidth;
        canvasHeight = cHeight;
        mouseCanvasPos = mousePos;
    }

    void setSaveStateCallback(std::function<void()> callback) { saveStateCallback = callback; }
    void setToggleFullscreenCallback(std::function<void()> callback) { toggleFullscreenCallback = callback; }

    void initTools(std::shared_ptr<Layer> layer, float *brushColorArray)
    {
        currentLayerForTools = layer;
        tools.clear();

        tools.push_back(std::make_unique<BrushTool>(layer));                    // 0
        tools.push_back(std::make_unique<FillTool>(layer));                     // 1
        tools.push_back(std::make_unique<EraserTool>(layer));                   // 2
        tools.push_back(std::make_unique<PipetteTool>(layer, brushColorArray)); // 3

        auto shape = std::make_unique<ShapeTool>(layer, ShapeType::Rectangle);
        shapeTool = shape.get();
        tools.push_back(std::move(shape)); // 4

        tools.push_back(std::make_unique<SelectionTool>(layer));           // 5
        tools.push_back(std::make_unique<TextTool>(layer, &localization)); // 6

        currentTool = tools[0].get();
        setCurrentToolName(currentTool->getName());

        selectionTool = dynamic_cast<SelectionTool *>(tools[5].get());
        textTool = dynamic_cast<TextTool *>(tools[6].get());

        updateLayerNames();
    }

    void setCurrentTool(int index)
    {
        if (index >= 0 && index < (int)tools.size())
        {
            currentTool = tools[index].get();
            setCurrentToolName(currentTool->getName());
        }
    }

    void setCurrentShapeType(ShapeType type)
    {
        if (shapeTool) shapeTool->setShapeType(type);
    }

    Tool *getCurrentTool() { return currentTool; }
    SelectionTool *getSelectionTool() { return selectionTool; }
    TextTool *getTextTool() { return textTool; }
    std::string getCurrentToolName() const { return currentToolName; }

    void updateToolLayer(std::shared_ptr<Layer> layer)
    {
        currentLayerForTools = layer;
        for (auto &t : tools)
        {
            if (auto brush = dynamic_cast<BrushTool *>(t.get())) brush->setLayer(layer);
            else if (auto fill = dynamic_cast<FillTool *>(t.get())) fill->setLayer(layer);
            else if (auto eraser = dynamic_cast<EraserTool *>(t.get())) eraser->setLayer(layer);
            else if (auto pipette = dynamic_cast<PipetteTool *>(t.get())) pipette->setLayer(layer);
            else if (auto shape = dynamic_cast<ShapeTool *>(t.get())) shape->setLayer(layer);
            else if (auto sel = dynamic_cast<SelectionTool *>(t.get())) sel->setLayer(layer);
            else if (auto txt = dynamic_cast<TextTool *>(t.get())) txt->setLayer(layer);
        }
    }

    template <typename FilterT, typename... Args>
    void applyFilter(Args &&...args)
    {
        if (saveStateCallback) saveStateCallback();
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

    void setCurrentToolName(const std::string &name) { currentToolName = name; }

    // ------------------------------------------------------------
    // Отрисовка панели инструментов (ширина 360px)
    // ------------------------------------------------------------
    void renderToolPanel(sf::RenderWindow &window,
                         std::function<void()> onLoadImage,
                         std::function<void()> onSaveImage,
                         std::function<void()> onClearCanvas,
                         std::function<void()> onResizeCanvas)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);

        float panelWidth = 360.0f;  // уменьшено на 20% от 450
        float rulerHeight = 30.0f;
        float statusBarHeight = 25.0f;
        ImGui::SetNextWindowPos(ImVec2(window.getSize().x - panelWidth, rulerHeight));
        ImGui::SetNextWindowSize(ImVec2(panelWidth, window.getSize().y - rulerHeight - statusBarHeight));
        ImGui::Begin(localization.get("tools").c_str(), nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // ----- 1. Настройки и помощь -----
        if (ImGui::Button(localization.get("settings").c_str(), ImVec2(170, 30)))
            ImGui::OpenPopup("SettingsPopup");
        ImGui::SameLine();
        if (ImGui::Button(localization.get("help").c_str(), ImVec2(170, 30)))
            ImGui::OpenPopup("HelpPopup");
        renderSettingsPopup();
        renderHelpPopup();
        ImGui::Separator();

        // ----- 2. Файловые операции -----
        if (ImGui::Button(localization.get("load").c_str(), ImVec2(110, 30)))
        {
            if (onLoadImage) onLoadImage();
            updateToolLayer(layerManager.getCurrentLayer());
        }
        ImGui::SameLine();
        if (ImGui::Button(localization.get("save").c_str(), ImVec2(110, 30)))
        {
            if (onSaveImage) onSaveImage();
        }
        ImGui::SameLine();
        if (ImGui::Button(localization.get("clear").c_str(), ImVec2(110, 30)))
        {
            if (onClearCanvas) onClearCanvas();
        }
        ImGui::Separator();

        // ----- 3. Инструменты (в одну строку, 7 штук) -----
        struct ToolInfo { const char* key; char hotkey; ImTextureID texID; };
        ToolInfo infos[] = {
            {"brush", 'B', penTextureID},
            {"fill", 'G', fillTextureID},
            {"eraser", 'E', eraserTextureID},
            {"pipette", 'I', pipetteTextureID},
            {"shape", 'U', shapeTextureID},
            {"selection", 'S', selectionTextureID},
            {"text", 'T', textTextureID}
        };
        for (int i = 0; i < 7; ++i) {
            std::string label = localization.get(infos[i].key);
            std::string tooltip = label + " (" + infos[i].hotkey + ")";
            bool clicked = false;
            if (infos[i].texID) {
                clicked = ImGui::ImageButton(infos[i].texID, ImVec2(30,30));
            } else {
                clicked = ImGui::Button(label.c_str(), ImVec2(48, 30));
            }
            if (clicked) setCurrentTool(i);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", tooltip.c_str());
            if (i < 6) ImGui::SameLine();
        }
        ImGui::Separator();

        // ----- 4. Параметры инструментов -----
        if (currentToolName == "Shape")
        {
            ImGui::Text("%s:", localization.get("shape_type").c_str());
            const char *shapeKeys[] = {"rectangle", "square", "circle", "ellipse", "line", "star"};
            static int shapeIdx = 0;
            std::string shapeNamesStr[6];
            const char *shapeNames[6];
            for (int i = 0; i < 6; ++i) {
                shapeNamesStr[i] = localization.get(shapeKeys[i]);
                shapeNames[i] = shapeNamesStr[i].c_str();
            }
            if (ImGui::Combo("##shape", &shapeIdx, shapeNames, 6))
                setCurrentShapeType((ShapeType)shapeIdx);
            
            ImGui::Text("Transform:");
            static float rotationAngle = 0.0f;
            static bool flipX = false, flipY = false;
            if (ImGui::SliderFloat("Rotate", &rotationAngle, 0.0f, 360.0f))
                if (shapeTool) shapeTool->setRotation(rotationAngle);
            if (ImGui::Checkbox("Flip X", &flipX) || ImGui::Checkbox("Flip Y", &flipY))
                if (shapeTool) shapeTool->setFlip(flipX, flipY);
            ImGui::Separator();
        }

        if (brushSize && brushColor)
        {
            ImGui::Text("%s:", localization.get("brush").c_str());
            ImGui::SliderFloat("Size", brushSize, 1.0f, 50.0f);
            ImGui::ColorEdit3("Color", brushColor);
            ImGui::Separator();
        }

        // ----- 5. Масштаб и размер холста -----
        if (zoomLevel)
        {
            ImGui::Text("%s:", localization.get("zoom").c_str());
            if (ImGui::SliderFloat("##zoom", zoomLevel, 0.1f, 5.0f, "%.1f"))
                *zoomLevel = std::max(0.1f, std::min(*zoomLevel, 5.0f));
            ImGui::Separator();
        }

        if (canvasWidth && canvasHeight)
        {
            ImGui::Text("%s:", localization.get("canvas_size").c_str());
            int minSize = 100, maxSize = 5000;
            static int prevW = *canvasWidth, prevH = *canvasHeight;
            ImGui::SliderInt("Width", canvasWidth, minSize, maxSize);
            ImGui::SliderInt("Height", canvasHeight, minSize, maxSize);
            if ((prevW != *canvasWidth || prevH != *canvasHeight) && onResizeCanvas)
            {
                onResizeCanvas();
                prevW = *canvasWidth; prevH = *canvasHeight;
            }
            ImGui::Separator();
        }

        // ----- 6. Сетка / Линейки -----
        if (showGrid && showRulers)
        {
            ImGui::Checkbox(localization.get("grid").c_str(), showGrid);
            ImGui::Checkbox(localization.get("rulers").c_str(), showRulers);
            ImGui::Separator();
        }

        // ----- 7. Слои -----
        renderLayersPanel();
        ImGui::Separator();

        // ----- 8. Фильтры -----
        if (ImGui::Button(localization.get("filters").c_str(), ImVec2(340, 30)))
            ImGui::OpenPopup("filter_popup");
        if (ImGui::BeginPopup("filter_popup"))
        {
            if (ImGui::MenuItem(localization.get("grayscale").c_str())) applyFilter<GrayscaleFilter>();
            if (ImGui::MenuItem(localization.get("invert").c_str())) applyFilter<InvertFilter>();
            if (ImGui::MenuItem(localization.get("blur").c_str())) applyFilter<BlurFilter>();
            if (ImGui::MenuItem(localization.get("sharpen").c_str())) applyFilter<SharpenFilter>();
            if (ImGui::MenuItem(localization.get("sepia").c_str())) applyFilter<SepiaFilter>();
            if (ImGui::MenuItem(localization.get("bright_plus").c_str())) applyFilter<BrightnessFilter>(30);
            if (ImGui::MenuItem(localization.get("bright_minus").c_str())) applyFilter<BrightnessFilter>(-30);
            ImGui::EndPopup();
        }

        ImGui::End();
        ImGui::PopStyleVar(3);

        if (textTool) textTool->renderTextPopup();
    }

    void renderSettingsPopup()
    {
        if (ImGui::BeginPopupModal("SettingsPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("%s:", localization.get("language").c_str());
            if (ImGui::Button("English", ImVec2(120, 0)))
            {
                localization.setLanguage(Language::ENG);
                updateLayerNames();
            }
            ImGui::SameLine();
            if (ImGui::Button("Русский", ImVec2(120, 0)))
            {
                localization.setLanguage(Language::RU);
                updateLayerNames();
            }
            ImGui::Separator();

            ImGui::Text("Display Mode:");
            if (ImGui::Button("Windowed", ImVec2(120, 0)))
            {
                if (toggleFullscreenCallback) toggleFullscreenCallback();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Fullscreen", ImVec2(120, 0)))
            {
                if (toggleFullscreenCallback) toggleFullscreenCallback();
                ImGui::CloseCurrentPopup();
            }
            ImGui::Separator();

            if (ImGui::Button("Close", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }

    void renderHelpPopup()
    {
        if (ImGui::BeginPopupModal("HelpPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (localization.getLanguage() == Language::RU) {
                ImGui::Text("Графический редактор - Справка");
                ImGui::Separator();
                ImGui::BulletText("Инструменты: Кисть(B), Заливка(G), Ластик(E), Пипетка(I), Фигура(U), Выделение(S), Текст(T)");
                ImGui::BulletText("Горячие клавиши: Ctrl+Z = Отменить, Ctrl+Y = Повторить");
                ImGui::BulletText("Холст: Изменение размера слайдерами на панели инструментов");
                ImGui::BulletText("Слои: Добавить, удалить (кроме фона), изменить прозрачность");
                ImGui::BulletText("Фильтры: Оттенки серого, Инверсия, Размытие, Резкость, Сепия, Яркость");
                ImGui::BulletText("Сетка/Линейки: Включить/выключить на панели");
                ImGui::BulletText("Полноэкранный режим: Настройки -> Fullscreen");
                ImGui::BulletText("Поворот/отражение: Выберите фигуру, настройте параметры трансформации");
            } else {
                ImGui::Text("Graphic Editor - Help");
                ImGui::Separator();
                ImGui::BulletText("Tools: Brush(B), Fill(G), Eraser(E), Pipette(I), Shape(U), Selection(S), Text(T)");
                ImGui::BulletText("Shortcuts: Ctrl+Z = Undo, Ctrl+Y = Redo");
                ImGui::BulletText("Canvas: Resize using sliders in Tools panel");
                ImGui::BulletText("Layers: Add, delete (except background), adjust opacity");
                ImGui::BulletText("Filters: Grayscale, Invert, Blur, Sharpen, Sepia, Brightness");
                ImGui::BulletText("Grid/Rulers: Toggle in Tools panel");
                ImGui::BulletText("Fullscreen: Settings -> Fullscreen");
                ImGui::BulletText("Rotation/Flip: Select Shape tool, adjust transform params");
            }
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }

    void renderLayersPanel()
    {
        ImGui::Text("%s:", localization.get("layers").c_str());
        if (ImGui::Button(localization.get("new_layer").c_str(), ImVec2(340, 25)))
        {
            layerManager.addLayer();
            updateToolLayer(layerManager.getCurrentLayer());
            auto &layers = layerManager.getLayers();
            if (!layers.empty())
                layers.back()->setName(localization.get("new_layer"));
        }
        auto &layers = layerManager.getLayers();
        for (int i = layers.size() - 1; i >= 0; --i)
        {
            ImGui::PushID(i);
            bool isCurrent = (i == layerManager.getCurrentLayerIndex());
            if (ImGui::Selectable(layers[i]->getName().c_str(), isCurrent))
            {
                layerManager.setCurrentLayer(i);
                updateToolLayer(layerManager.getCurrentLayer());
            }
            
            float opacity = layers[i]->getOpacity();
            ImGui::Text("%s:", localization.get("opacity").c_str());
            if (ImGui::SliderFloat("##opacity", &opacity, 0.0f, 1.0f, "%.2f"))
                layers[i]->setOpacity(opacity);
            
            if (i != 0) {
                if (garbageTextureID) {
                    if (ImGui::ImageButton(garbageTextureID, ImVec2(20, 20)))
                    {
                        layerManager.deleteLayer(i);
                        updateToolLayer(layerManager.getCurrentLayer());
                        ImGui::PopID();
                        return;
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Delete layer");
                    ImGui::SameLine();
                    ImGui::Text(" Delete");
                } else {
                    if (ImGui::Button("Delete Layer", ImVec2(100, 20)))
                    {
                        layerManager.deleteLayer(i);
                        updateToolLayer(layerManager.getCurrentLayer());
                        ImGui::PopID();
                        return;
                    }
                }
            }
            ImGui::Separator();
            ImGui::PopID();
        }
    }

    // Строка состояния
    void renderStatusBar(sf::RenderWindow &window)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::SetNextWindowPos(ImVec2(0, window.getSize().y - 25));
        ImGui::SetNextWindowSize(ImVec2(window.getSize().x, 25));
        ImGui::Begin("StatusBar", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
        
        if (mouseCanvasPos)
        {
            ImGui::Text("Mouse: (%d, %d)", mouseCanvasPos->x, mouseCanvasPos->y);
            ImGui::SameLine();
        }
        if (zoomLevel)
        {
            ImGui::Text("Zoom: %.1f%%", *zoomLevel * 100.0f);
            ImGui::SameLine();
        }
        if (canvasWidth && canvasHeight)
        {
            ImGui::Text("Canvas: %dx%d", *canvasWidth, *canvasHeight);
            ImGui::SameLine();
        }
        if (showGrid && showRulers)
        {
            ImGui::Text("Grid: %s", *showGrid ? "ON" : "OFF");
            ImGui::SameLine();
            ImGui::Text("Rulers: %s", *showRulers ? "ON" : "OFF");
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    // ------------------------------------------------------------
    // renderRulers, renderGrid, renderSelectionOverlay (без изменений)
    // ------------------------------------------------------------
    void renderRulers(sf::RenderWindow &window,
                      sf::Vector2f canvasOffset,
                      float zoomLevel,
                      int canvasWidth,
                      int canvasHeight)
    {
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

        sf::Font font;
        bool hasFont = font.loadFromFile("fonts/arialmt.ttf") || font.loadFromFile("arialmt.ttf");

        float visibleLeft = -canvasOffset.x / zoomLevel;
        float visibleRight = visibleLeft + windowSize.x / zoomLevel;
        int extendedRange = static_cast<int>(windowSize.x / zoomLevel) * 2;
        int startX = static_cast<int>(visibleLeft) - extendedRange;
        int endX = static_cast<int>(visibleRight) + extendedRange;
        int firstTick = (startX / step) * step;
        float minX = rulerSide + 5.0f;

        for (int x = firstTick; x <= endX; x += step)
        {
            float screenX = canvasOffset.x + x * zoomLevel;
            if (screenX >= minX && screenX <= windowSize.x)
            {
                sf::Vertex line[] = { sf::Vertex(sf::Vector2f(screenX, 0), sf::Color::Black),
                                      sf::Vertex(sf::Vector2f(screenX, rulerHeight - 5), sf::Color::Black) };
                window.draw(line, 2, sf::Lines);
                if (hasFont && step >= 5)
                {
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

        for (int y = firstTickY; y <= endY; y += step)
        {
            float screenY = canvasOffset.y + y * zoomLevel;
            if (screenY >= minY && screenY <= windowSize.y)
            {
                sf::Vertex line[] = { sf::Vertex(sf::Vector2f(0, screenY), sf::Color::Black),
                                      sf::Vertex(sf::Vector2f(rulerSide - 5, screenY), sf::Color::Black) };
                window.draw(line, 2, sf::Lines);
                if (hasFont && step >= 5)
                {
                    sf::Text text(std::to_string(y), font, 10);
                    text.setFillColor(sf::Color::Black);
                    text.setPosition(rulerSide - 20, screenY - 6);
                    window.draw(text);
                }
            }
        }
    }

    void renderGrid(sf::RenderWindow &window,
                    sf::Vector2f canvasOffset,
                    float zoomLevel,
                    int canvasWidth,
                    int canvasHeight)
    {
        int gridSize = 10;
        sf::Color gridColor(180, 180, 180, 200);
        for (int x = 0; x <= canvasWidth; x += gridSize)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(canvasOffset.x + x * zoomLevel, canvasOffset.y), gridColor),
                sf::Vertex(sf::Vector2f(canvasOffset.x + x * zoomLevel,
                                        canvasOffset.y + canvasHeight * zoomLevel), gridColor)
            };
            window.draw(line, 2, sf::Lines);
        }
        for (int y = 0; y <= canvasHeight; y += gridSize)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(canvasOffset.x, canvasOffset.y + y * zoomLevel), gridColor),
                sf::Vertex(sf::Vector2f(canvasOffset.x + canvasWidth * zoomLevel,
                                        canvasOffset.y + y * zoomLevel), gridColor)
            };
            window.draw(line, 2, sf::Lines);
        }
    }

    void renderSelectionOverlay(sf::RenderWindow& window, sf::Vector2f canvasOffset, float zoomLevel)
    {
        if (selectionTool && selectionTool->isActive())
        {
            sf::FloatRect rect = selectionTool->getSelectionRect();
            if (rect.width > 0 && rect.height > 0)
            {
                sf::RectangleShape overlay(sf::Vector2f(rect.width * zoomLevel, rect.height * zoomLevel));
                overlay.setPosition(canvasOffset.x + rect.left * zoomLevel, canvasOffset.y + rect.top * zoomLevel);
                overlay.setFillColor(selectionTool->getOverlayColor());
                window.draw(overlay);
            }
        }
    }
};