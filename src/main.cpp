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

        int x = current.x;
        int y = current.y;

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

int main() {
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Paint Prototype");
    window.setFramerateLimit(144);
    if (!ImGui::SFML::Init(window))
        return -1;

    sf::RenderTexture canvas;
    if (!canvas.create(1200, 800))
        return -1;
    canvas.clear(sf::Color::Black);
    canvas.display();

    sf::Texture penTexture, eraserTexture, garbageTexture;
    penTexture.loadFromFile("Textures/pen.png");
    eraserTexture.loadFromFile("Textures/eraser.png");
    garbageTexture.loadFromFile("Textures/garbage.png");
    sf::Sprite penSprite(penTexture);
    sf::Sprite eraserSprite(eraserTexture);
    sf::Sprite garbageSprite(garbageTexture);

    bool drawMode = true;
    bool fillMode = false; 
    bool isErasing = false;
    float thickness = 10.f;
    ImVec4 drawingColor = ImVec4(1.f, 1.f, 1.f, 1.f);
    ImVec4 oldColor = drawingColor;

    bool mouseLeftPressed = false;
    sf::Vector2f prevMousePos;
    float step = 1.0f;

    sf::Clock deltaClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed)
                window.close();

            if (fillMode && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Image image = canvas.getTexture().copyToImage();
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (mousePos.x >= 0 && mousePos.x < (int)image.getSize().x &&
                    mousePos.y >= 0 && mousePos.y < (int)image.getSize().y) {
                    sf::Color targetColor = image.getPixel(mousePos.x, mousePos.y);
                    sf::Color brushColor(
                        static_cast<sf::Uint8>(drawingColor.x * 255),
                        static_cast<sf::Uint8>(drawingColor.y * 255),
                        static_cast<sf::Uint8>(drawingColor.z * 255),
                        static_cast<sf::Uint8>(drawingColor.w * 255)
                    );
                    floodFill(image, mousePos, targetColor, brushColor);
                    sf::Texture newTex;
                    newTex.loadFromImage(image);
                    canvas.draw(sf::Sprite(newTex));
                    canvas.display();
                }
            }
        }

        sf::Color brushColor;
        if (isErasing) {
            brushColor = sf::Color::Black;
        } else {
            brushColor = sf::Color(
                static_cast<sf::Uint8>(drawingColor.x * 255),
                static_cast<sf::Uint8>(drawingColor.y * 255),
                static_cast<sf::Uint8>(drawingColor.z * 255),
                static_cast<sf::Uint8>(drawingColor.w * 255)
            );
        }

        bool leftMouse = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        if (drawMode && !fillMode && leftMouse) {
            sf::Vector2f currentMouse = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
            if (mouseLeftPressed) {
                sf::Vector2f diff = currentMouse - prevMousePos;
                float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
                for (float i = 0; i < distance; i += step) {
                    sf::CircleShape dot(thickness);
                    dot.setFillColor(brushColor);
                    dot.setOrigin(thickness, thickness);
                    sf::Vector2f intermediate = prevMousePos + diff * (i / distance);
                    dot.setPosition(intermediate);
                    canvas.draw(dot);
                }
                canvas.display();
            }
            prevMousePos = currentMouse;
            mouseLeftPressed = true;
        } else {
            mouseLeftPressed = false;
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Mode:");
        ImGui::SameLine();
        if (ImGui::RadioButton("Draw", drawMode)) {
            drawMode = true;
            fillMode = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Fill", fillMode)) {
            fillMode = true;
            drawMode = false;
        }

        ImGui::Separator();

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

        ImGui::Separator();

        ImGui::SliderFloat("Brush size", &thickness, 1.f, 30.f);

        if (ImGui::ImageButton(penSprite, ImVec2(32, 32))) {
            isErasing = false;
        }
        ImGui::SameLine();
        if (ImGui::ImageButton(eraserSprite, ImVec2(32, 32))) {
            isErasing = true;
        }
        ImGui::SameLine();
        if (ImGui::ImageButton(garbageSprite, ImVec2(32, 32))) {
            canvas.clear(sf::Color::Black);
            canvas.display();
        }

        ImGui::End();

        window.clear();
        sf::Sprite canvasSprite(canvas.getTexture());
        window.draw(canvasSprite);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}