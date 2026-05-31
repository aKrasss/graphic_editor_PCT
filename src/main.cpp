#include <SFML/Graphics.hpp>
#include "ImGUI/imgui.h"
#include "ImGUI/imgui-SFML.h"

static void draw(sf::Vector2i mousePosPrev, sf::Vector2i mousePos, sf::RenderTexture& canvas, float* brushColor, float brushSize) {
    sf::Color color((sf::Uint8)(brushColor[0] * 255), (sf::Uint8)(brushColor[1] * 255), (sf::Uint8)(brushColor[2] * 255));

    sf::Vector2f from(mousePosPrev.x, mousePosPrev.y);
    sf::Vector2f to(mousePos.x, mousePos.y);
    sf::Vector2f diff = to - from;
    float lenDiff = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    int steps = (int)(lenDiff / 2.0f);
    for (int i = 0; i <= steps; i++) {
        float ii = (steps == 0) ? 0.0f : (float)i / steps;
        sf::Vector2f pos = from + diff * ii;
        sf::CircleShape dot(brushSize);
        dot.setFillColor(color);
        dot.setOrigin(brushSize, brushSize);
        dot.setPosition(pos);

        canvas.draw(dot);
    }
    canvas.display();
}

static void clearCanvas(sf::RenderTexture& canvas) {
    canvas.clear(sf::Color::White);
    canvas.display();
}

static void createArea(sf::RenderWindow& window, sf::RenderTexture& canvas, bool& isSelectionMode, bool& selectionNow, bool& hasSelection, bool& isMoving, sf::Vector2i selectionStart, sf::Vector2i& movingEnd, sf::IntRect& selection, sf::RenderTexture& selectionTexture) {
    isSelectionMode = false;
    selectionNow = false;
    sf::Vector2i selectionEnd = sf::Mouse::getPosition(window);
    selection = sf::IntRect(std::min(selectionStart.x, selectionEnd.x), std::min(selectionStart.y, selectionEnd.y), std::abs(selectionEnd.x - selectionStart.x), std::abs(selectionEnd.y - selectionStart.y));
    if (selection.width > 0 && selection.height > 2) {
        sf::Image canvasImg = canvas.getTexture().copyToImage();
        sf::Image selectionImg;
        selectionImg.create(selection.width, selection.height, sf::Color::White);
        for (int x = 0; x < selection.width; x++) {
            for (int y = 0; y < selection.height; y++) {
                selectionImg.setPixel(x, y, canvasImg.getPixel(selection.left + x, selection.top + y));
            }
        }

        sf::Texture tmp;
        tmp.loadFromImage(selectionImg);

        selectionTexture.create(selection.width, selection.height);
        selectionTexture.clear(sf::Color::White);
        selectionTexture.draw(sf::Sprite(tmp));
        selectionTexture.display();

        sf::RectangleShape eraser(sf::Vector2f((float)selection.width, (float)selection.height));
        eraser.setPosition(selection.left, selection.top);
        eraser.setFillColor(sf::Color::White);
        canvas.draw(eraser);
        canvas.display();

        hasSelection = true;
        isMoving = true;
        movingEnd = selectionEnd;
    }
}

void createMoveArea(sf::RenderTexture& canvas, sf::RenderTexture& selectionTexture, sf::Vector2i movingEnd, bool& isMoving, bool& hasSelection) {
    sf::Sprite selectionSprite(selectionTexture.getTexture());
    selectionSprite.setPosition((float)movingEnd.x, (float)movingEnd.y);
    canvas.draw(selectionSprite);
    canvas.display();
    isMoving = false;
    hasSelection = false;
}

void drawMoveArea(sf::RenderWindow& window, sf::Vector2i& movingEnd, sf::Vector2i mousePos, sf::RenderTexture& selectionTexture) {
    movingEnd = mousePos;

    sf::Sprite selectionSprite(selectionTexture.getTexture());
    selectionSprite.setPosition((float)movingEnd.x, (float)movingEnd.y);
    window.draw(selectionSprite);

    sf::RectangleShape shape(sf::Vector2f((float)selectionTexture.getSize().x, (float)selectionTexture.getSize().y));
    shape.setPosition((float)movingEnd.x, (float)movingEnd.y);
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineColor(sf::Color::Black);
    shape.setOutlineThickness(1.0f);
    window.draw(shape);
}

void drawSelectionArea(sf::RenderWindow& window, sf::Vector2i mousePos, sf::Vector2i selectionStart) {
    sf::RectangleShape selRect(sf::Vector2f((float)std::abs(mousePos.x - selectionStart.x), (float)std::abs(mousePos.y - selectionStart.y)));
    selRect.setPosition((float)std::min(selectionStart.x, mousePos.x), (float)std::min(selectionStart.y, mousePos.y));
    selRect.setFillColor(sf::Color(100, 150, 255, 40));
    selRect.setOutlineColor(sf::Color(100, 150, 255));
    selRect.setOutlineThickness(1.0f);
    window.draw(selRect);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test");
    window.setFramerateLimit(60);

    ImGui::SFML::Init(window);
    sf::Clock deltaTime;

    sf::RenderTexture canvas;
    canvas.create(800, 600);
    canvas.clear(sf::Color::White);
    canvas.display();

    sf::Vector2i mousePosPrev = sf::Mouse::getPosition(window);
    bool isDrawing = false;
    bool isSelectionMode = false;
    bool isMoving = false;
    bool hasSelection = false;
    bool selectionNow = false;

    sf::Vector2i selectionStart;
    sf::IntRect selection;
    sf::RenderTexture selectionTexture;
    sf::Vector2i movingEnd;

    float brushColor[3] = { 0.0f, 0.0f, 0.0f };
    float brushSize = 5.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (!isSelectionMode && !isMoving) {
                    isDrawing = true;
                }
                else if (isSelectionMode && !isMoving) {
                    selectionStart = sf::Mouse::getPosition(window);
                    selectionNow = true;
                }
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                if (!isSelectionMode && !isMoving) {
                    isDrawing = false;
                }
                else if (isSelectionMode && !isMoving) {
                    createArea(window, canvas, isSelectionMode, selectionNow, hasSelection, isMoving, selectionStart, movingEnd, selection, selectionTexture);
                }
                else if (hasSelection && isMoving) {
                    createMoveArea(canvas, selectionTexture, movingEnd, isMoving, hasSelection);
                }
            }
        }

        ImGui::SFML::Update(window, deltaTime.restart());
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(200, 200));
        ImGui::Begin("Panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::Separator();
        ImGui::Text("Mode");
        if (ImGui::Button("Draw")) isSelectionMode = false;
        if (ImGui::Button("Work with area")) {
            isSelectionMode = true; selectionNow = false;
        } 

        ImGui::Separator();
        ImGui::Text("Color");
        ImGui::ColorPicker3("color", brushColor);

        ImGui::Separator();
        ImGui::Text("Brush size");
        ImGui::SliderFloat("size", &brushSize, 1.0f, 33.0f);

        ImGui::Separator();
        if (ImGui::Button("Clear canvas")) {
            clearCanvas(canvas);
        }
        ImGui::End();

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        if (isDrawing && !ImGui::GetIO().WantCaptureMouse) {
            draw(mousePosPrev, mousePos, canvas, brushColor, brushSize);
        }
        mousePosPrev = mousePos;

        sf::Sprite canvasSprite(canvas.getTexture());
        window.clear();
        window.draw(canvasSprite);

        if (isMoving && hasSelection) {
            drawMoveArea(window, movingEnd, mousePos, selectionTexture);
        }

        if (isSelectionMode && selectionNow) {
            drawSelectionArea(window, mousePos, selectionStart);
        }

        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
}