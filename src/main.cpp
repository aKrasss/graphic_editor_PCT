#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <imgui.h>
#include <imgui-SFML.h>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1000, 800), "SFML Works!");
    window.setFramerateLimit(144);

    if (!ImGui::SFML::Init(window)) return -1;

    sf::RenderTexture canvas;
    if (!canvas.create(1000, 800)) return -1;

    canvas.clear(sf::Color::Black);
    canvas.display();

    sf::Vector2f previousMousePosition;
    bool isMouseLeftButtonPressed = false;
    float thickness = 10.f;
    sf::Color brushColor = sf::Color::White;
    float step = 1.0f;

    sf::Clock deltaClock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed)
                window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            sf::Vector2f currentMousePosition = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

            if (isMouseLeftButtonPressed)
            {
                sf::Vector2f diff = currentMousePosition - previousMousePosition;
                float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                for (float i = 0; i < distance; i += step)
                {
                    sf::CircleShape dot(thickness);
                    dot.setFillColor(brushColor);
                    dot.setOrigin(thickness, thickness);

                    sf::Vector2f intermediatePos = previousMousePosition + (diff * (i / distance));
                    dot.setPosition(intermediatePos);

                    canvas.draw(dot);
                }
                canvas.display();
            }
            previousMousePosition = currentMousePosition;
            isMouseLeftButtonPressed = true;
        }
        else
        {
            isMouseLeftButtonPressed = false;
        }

        window.clear();

        sf::Sprite canvasSprite(canvas.getTexture());
        window.draw(canvasSprite);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
