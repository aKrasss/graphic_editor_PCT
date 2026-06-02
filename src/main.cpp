#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <stack>

std::vector<ImVec4> recentColors;

void addRecentColor(const ImVec4& color) {
    const size_t MAX_SIZE = 10;
    auto it = std::find_if(recentColors.begin(), recentColors.end(),
        [&](const ImVec4& c) {
            return c.x == color.x && c.y == color.y &&
                   c.z == color.z && c.w == color.w;
        });
    if (it != recentColors.end())
        recentColors.erase(it);
    recentColors.insert(recentColors.begin(), color);
    if (recentColors.size() > MAX_SIZE)
        recentColors.pop_back();
}

void floodFill(sf::Image& image, sf::Vector2i startPoint, sf::Color targetColor, sf::Color replacementColor) {
    if (targetColor == replacementColor) return;
    std::stack<sf::Vector2i> pixels;
    pixels.push(startPoint);
    sf::Vector2u size = image.getSize();
    while (!pixels.empty()) {
        sf::Vector2i current = pixels.top();
        pixels.pop();
        int x = current.x, y = current.y;
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

void drawBrush(sf::Vector2i from, sf::Vector2i to, sf::RenderTexture& canvas,
               const sf::Color& color, float brushSize) {
    sf::Vector2f f(from.x, from.y);
    sf::Vector2f t(to.x, to.y);
    sf::Vector2f diff = t - f;
    float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    int steps = std::max(1, (int)(len / 2.0f));
    for (int i = 0; i <= steps; ++i) {
        float t = (steps == 0) ? 0.0f : (float)i / steps;
        sf::Vector2f pos = f + diff * t;
        sf::CircleShape dot(brushSize);
        dot.setFillColor(color);
        dot.setOrigin(brushSize, brushSize);
        dot.setPosition(pos);
        canvas.draw(dot);
    }
    canvas.display();
}

void clearCanvas(sf::RenderTexture& canvas, const sf::Color& bgColor) {
    canvas.clear(bgColor);
    canvas.display();
}


void createSelection(sf::RenderWindow& window, sf::RenderTexture& canvas,
                     sf::Vector2i start, sf::IntRect& outRect,
                     sf::RenderTexture& selectionTex, bool& hasSelection,
                     bool& isMoving, sf::Vector2i& movingEnd, const sf::Color& bgColor) {
    sf::Vector2i end = sf::Mouse::getPosition(window);
    outRect = sf::IntRect(std::min(start.x, end.x), std::min(start.y, end.y),
                          std::abs(end.x - start.x), std::abs(end.y - start.y));
    if (outRect.width > 2 && outRect.height > 2) {
        sf::Image canvasImg = canvas.getTexture().copyToImage();
        sf::Image selImg;
        selImg.create(outRect.width, outRect.height, sf::Color::Transparent);
        for (int x = 0; x < outRect.width; ++x) {
            for (int y = 0; y < outRect.height; ++y) {
                selImg.setPixel(x, y, canvasImg.getPixel(outRect.left + x, outRect.top + y));
            }
        }
        sf::Texture tmpTex;
        tmpTex.loadFromImage(selImg);
        selectionTex.create(outRect.width, outRect.height);
        selectionTex.clear(sf::Color::Transparent);
        selectionTex.draw(sf::Sprite(tmpTex));
        selectionTex.display();

        sf::RectangleShape eraser(sf::Vector2f(outRect.width, outRect.height));
        eraser.setPosition(outRect.left, outRect.top);
        eraser.setFillColor(bgColor);
        canvas.draw(eraser);
        canvas.display();

        hasSelection = true;
        isMoving = true;
        movingEnd = end;
    }
}

void moveSelection(sf::RenderTexture& canvas, sf::RenderTexture& selectionTex,
                   const sf::Vector2i& newPos, bool& isMoving, bool& hasSelection) {
    sf::Sprite selSprite(selectionTex.getTexture());
    selSprite.setPosition((float)newPos.x, (float)newPos.y);
    canvas.draw(selSprite);
    canvas.display();
    isMoving = false;
    hasSelection = false;
}

void drawMovingOutline(sf::RenderWindow& window, const sf::RenderTexture& selectionTex,
                       const sf::Vector2i& pos) {
    sf::Sprite selSprite(selectionTex.getTexture());
    selSprite.setPosition((float)pos.x, (float)pos.y);
    window.draw(selSprite);
    sf::RectangleShape outline(sf::Vector2f(selectionTex.getSize().x, selectionTex.getSize().y));
    outline.setPosition((float)pos.x, (float)pos.y);
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(sf::Color::Cyan);
    outline.setOutlineThickness(1.5f);
    window.draw(outline);
}

void drawSelectionRect(sf::RenderWindow& window, sf::Vector2i start, sf::Vector2i end) {
    sf::RectangleShape rect(sf::Vector2f(std::abs(end.x - start.x), std::abs(end.y - start.y)));
    rect.setPosition((float)std::min(start.x, end.x), (float)std::min(start.y, end.y));
    rect.setFillColor(sf::Color(80, 150, 255, 60));
    rect.setOutlineColor(sf::Color(80, 150, 255));
    rect.setOutlineThickness(1.2f);
    window.draw(rect);
}

int main() {
    const int WIN_W = 1200, WIN_H = 800;
    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Paint Prototype");
    window.setFramerateLimit(144);
    if (!ImGui::SFML::Init(window))
        return -1;

    sf::RenderTexture canvas;
    if (!canvas.create(WIN_W, WIN_H))
        return -1;
    sf::Color bgColor = sf::Color::Black;
    clearCanvas(canvas, bgColor);

    sf::Texture penTex, eraserTex, garbageTex;
    penTex.loadFromFile("Textures/pen.png");
    eraserTex.loadFromFile("Textures/eraser.png");
    garbageTex.loadFromFile("Textures/garbage.png");
    sf::Sprite penSprite(penTex), eraserSprite(eraserTex), garbageSprite(garbageTex);

    enum Mode { DRAW, FILL, SELECT };
    Mode currentMode = DRAW;
    bool isErasing = false;

    float brushSize = 10.f;
    ImVec4 drawingColor = ImVec4(1.f, 1.f, 1.f, 1.f);
    ImVec4 oldColor = drawingColor;

    bool mouseLeftPressed = false;
    sf::Vector2i prevMousePos;

    bool selecting = false;
    bool hasSelection = false;
    bool isMoving = false;
    sf::Vector2i selectionStart;
    sf::IntRect selectionRect;
    sf::RenderTexture selectionTexture;
    sf::Vector2i movingPos;

    sf::Clock deltaClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (currentMode == SELECT && !isMoving && !hasSelection) {
                    selectionStart = sf::Mouse::getPosition(window);
                    selecting = true;
                }
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                if (currentMode == SELECT && selecting) {
                    createSelection(window, canvas, selectionStart, selectionRect,
                                    selectionTexture, hasSelection, isMoving, movingPos, bgColor);
                    selecting = false;
                }
                else if (currentMode == SELECT && hasSelection && isMoving) {
                    moveSelection(canvas, selectionTexture, movingPos, isMoving, hasSelection);
                }
                else {
                    mouseLeftPressed = false;
                }
            }
        }

        sf::Color brushColor;
        if (isErasing && currentMode == DRAW) {
            brushColor = bgColor;
        } else {
            brushColor = sf::Color(
                static_cast<sf::Uint8>(drawingColor.x * 255),
                static_cast<sf::Uint8>(drawingColor.y * 255),
                static_cast<sf::Uint8>(drawingColor.z * 255),
                static_cast<sf::Uint8>(drawingColor.w * 255)
            );
        }

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        bool leftMouse = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        if (currentMode == DRAW && leftMouse && !selecting && !ImGui::GetIO().WantCaptureMouse) {
            if (mouseLeftPressed) {
                drawBrush(prevMousePos, mousePos, canvas, brushColor, brushSize);
            }
            prevMousePos = mousePos;
            mouseLeftPressed = true;
        } else {
            if (currentMode != DRAW) mouseLeftPressed = false;
        }

        if (currentMode == FILL && !selecting && event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left && !ImGui::GetIO().WantCaptureMouse) {
            sf::Image img = canvas.getTexture().copyToImage();
            sf::Vector2i mp = sf::Mouse::getPosition(window);
            if (mp.x >= 0 && mp.x < WIN_W && mp.y >= 0 && mp.y < WIN_H) {
                sf::Color target = img.getPixel(mp.x, mp.y);
                floodFill(img, mp, target, brushColor);
                sf::Texture newTex;
                newTex.loadFromImage(img);
                canvas.draw(sf::Sprite(newTex));
                canvas.display();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Mode:");
        ImGui::SameLine();
        if (ImGui::RadioButton("Draw", currentMode == DRAW)) currentMode = DRAW;
        ImGui::SameLine();
        if (ImGui::RadioButton("Fill", currentMode == FILL)) currentMode = FILL;
        ImGui::SameLine();
        if (ImGui::RadioButton("Select", currentMode == SELECT)) currentMode = SELECT;

        ImGui::Separator();

        if (currentMode != SELECT) {
            static bool showPicker = false;
            if (ImGui::Button("Choose color"))
                showPicker = !showPicker;
            ImGui::SameLine();
            ImGui::ColorButton("##preview", drawingColor, 0, ImVec2(20, 20));
            if (showPicker) {
                if (ImGui::ColorPicker4("##picker", (float*)&drawingColor)) {
                    if (oldColor.x != drawingColor.x || oldColor.y != drawingColor.y ||
                        oldColor.z != drawingColor.z || oldColor.w != drawingColor.w) {
                        addRecentColor(drawingColor);
                        oldColor = drawingColor;
                    }
                }
            }

            ImGui::Separator();
            ImGui::Text("Recent colors:");
            if (recentColors.empty()) {
                ImGui::TextDisabled("No recent colors");
            } else {
                int itemsPerRow = 5;
                for (size_t i = 0; i < recentColors.size(); ++i) {
                    ImGui::PushID((int)i);
                    if (ImGui::ColorButton("##recent", recentColors[i],
                                           ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder,
                                           ImVec2(30, 30))) {
                        drawingColor = recentColors[i];
                        oldColor = drawingColor;
                        isErasing = false;
                    }
                    if ((i + 1) % itemsPerRow != 0 && i + 1 < recentColors.size())
                        ImGui::SameLine();
                    ImGui::PopID();
                }
            }
        }

        ImGui::Separator();

        ImGui::SliderFloat("Brush size", &brushSize, 1.f, 30.f);

        if (currentMode == DRAW) {
            if (ImGui::ImageButton(penSprite, ImVec2(32, 32))) {
                isErasing = false;
            }
            ImGui::SameLine();
            if (ImGui::ImageButton(eraserSprite, ImVec2(32, 32))) {
                isErasing = true;
            }
        }

        ImGui::SameLine();
        if (ImGui::ImageButton(garbageSprite, ImVec2(32, 32))) {
            clearCanvas(canvas, bgColor);
        }

        ImGui::End();

        window.clear(bgColor);
        sf::Sprite canvasSprite(canvas.getTexture());
        window.draw(canvasSprite);

        if (currentMode == SELECT && selecting) {
            sf::Vector2i cur = sf::Mouse::getPosition(window);
            drawSelectionRect(window, selectionStart, cur);
        }
        if (currentMode == SELECT && hasSelection && isMoving) {
            sf::Vector2i mouseNow = sf::Mouse::getPosition(window);
            movingPos = mouseNow;
            drawMovingOutline(window, selectionTexture, movingPos);
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}