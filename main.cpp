#include <SFML/Graphics.hpp>
#include <vector>
#include <imgui.h>
#include <iostream>
#include "imgui-SFML.h"

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

int main()
{
    sf::RenderWindow window(sf::VideoMode(600, 800), "SFML works!");

    if (!ImGui::SFML::Init(window)) //
    {
        return -1;
    }

    std::vector<sf::Vertex> points; //вершины
    bool isDrawing = false;

    static ImVec4 drawingColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    static ImVec4 oldColor = drawingColor;
    sf::Color sfColor = sf::Color::White; //значение далее через ImGUI перезаписывается


    sf::Clock deltaClock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (!ImGui::GetIO().WantCaptureMouse)
                        isDrawing = true;
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDrawing = false;
                }
            }

            if (event.type == sf::Event::MouseMoved && isDrawing) {
                sf::Color sfColor(
                    static_cast<sf::Uint8>(drawingColor.x * 255.0f),
                    static_cast<sf::Uint8>(drawingColor.y * 255.0f),
                    static_cast<sf::Uint8>(drawingColor.z * 255.0f),
                    static_cast<sf::Uint8>(drawingColor.w * 255.0f)
                );
                points.push_back(sf::Vertex(sf::Vector2f(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y)), sfColor));
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::Begin("Settings of drawing");

        static bool showPicker = false;
        if (ImGui::Button("Choose a color"))
            showPicker = !showPicker;

        ImGui::SameLine();
        ImGui::ColorButton("##colorpreview", drawingColor, 0, ImVec2(20, 20));

        if (showPicker)
        {

            ImGui::ColorPicker4("##picker", (float*)&drawingColor);

            if (oldColor.x != drawingColor.x || oldColor.y != drawingColor.y ||
                oldColor.z != drawingColor.z || oldColor.w != drawingColor.w) { //если старый цвет не совпадает с новым, тогда вызывается ф-я добавления цвета в массив
                addRecentColor(drawingColor);
                oldColor = drawingColor;
            }
        }

        if (ImGui::Button("Clear"))
            points.clear();

        ImGui::Separator();
        ImGui::Text("Recent colors:");
        if (recentColors.empty()) {
            ImGui::TextDisabled("Not yet");
        }
        else {
            int itemsPerRow = 5;
            for (size_t i = 0; i < recentColors.size(); ++i) {
                ImGui::PushID((int)i);
                if (ImGui::ColorButton("##recent", recentColors[i],
                    ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder,
                    ImVec2(30, 30))) {
                    drawingColor = recentColors[i];
                    oldColor = drawingColor;
                }
                if ((i + 1) % itemsPerRow != 0 && i + 1 < recentColors.size()) // если кнопка не последняя в строке и не последняя в принципе в массиве
                    ImGui::SameLine();
                ImGui::PopID();
            }
        }


        ImGui::End();

        window.clear(sf::Color::Black);

        if (!points.empty()) {
            window.draw(&points[0], points.size(), sf::Points);
        }

        ImGui::SFML::Render(window);

        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
