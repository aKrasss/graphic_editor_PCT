#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <queue>
#include <stack>

namespace {

    //const sf::Sprite pen(sf::Texture("Textures/pen.png"));
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



int main()
{

    sf::Texture penTextures;
    sf::Texture eraserTextures;
    sf::Texture garbageTextures;

    penTextures.loadFromFile("Textures/pen.png");
    eraserTextures.loadFromFile("Textures/eraser.png");
    garbageTextures.loadFromFile("Textures/garbage.png");

    sf::Sprite writerSprite(penTextures);
    sf::Sprite deleteSprite(garbageTextures);

    sf::RenderWindow window(sf::VideoMode(1000, 800), "SFML Works!");
    window.setFramerateLimit(144);

    if (!ImGui::SFML::Init(window)) return -1;

    sf::RenderTexture canvas;
    if (!canvas.create(1000, 800)) return -1;

    canvas.clear(sf::Color::Black);
    canvas.display();

    sf::Vector2f previousMousePosition;
    bool mouseLeftButtonWasPressed = false;
    bool isMouseMousePressed = false;
    bool isDraw = false;
    bool isErase = false;
    bool isFillMode = false;

    float thickness = 10.f;
    sf::Color brushColor = sf::Color::Blue;
    float step = 1.0f;
    sf::Clock deltaClock;

    float colorArray[3] = {
        (float)0 / 255,
        (float)0 / 255,
        (float)0 / 255 };

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed)
                window.close();


            if (isFillMode && event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {

                    sf::Image tempImage = canvas.getTexture().copyToImage();
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                    if (mousePos.x >= 0 && mousePos.x < (int)tempImage.getSize().x &&
                        mousePos.y >= 0 && mousePos.y < (int)tempImage.getSize().y) {

                        sf::Color oldColor = tempImage.getPixel(mousePos.x, mousePos.y);

                        floodFill(tempImage, mousePos, oldColor, brushColor);

                        sf::Texture tempTex;
                        tempTex.loadFromImage(tempImage);
                        sf::Sprite tempSpr(tempTex);

                        canvas.draw(tempSpr);
                        canvas.display();
                    }
                }
            }
        }

        if (!isErase) {
            brushColor = sf::Color(
                (float)colorArray[0] * 255,
                (float)colorArray[1] * 255,
                (float)colorArray[2] * 255
            );
            std::cout << "1" << std::endl;
        }
        else {
            brushColor = sf::Color::Black;
            std::cout << "2" << std::endl;
        }//TODO!!!!

        isMouseMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);

        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
        ImGui::Begin("ImGui is working!");
        ImGui::Text("Bruuuh");
        ImGui::Checkbox("Draw", &isDraw);
        ImGui::Checkbox("Fill Mode", &isFillMode);


        auto buttonEraser = ImGui::ImageButton(writerSprite, -1, sf::Color::Transparent, sf::Color::White);
        auto buttonDelete = ImGui::ImageButton(deleteSprite, -1, sf::Color::Transparent, sf::Color::White);

        if (buttonDelete) {
            canvas.clear();
        }
        if (buttonEraser && isErase != true) {
            isErase = true;
            writerSprite.setTexture(eraserTextures);
        }
        else if(buttonEraser && isErase == true) {
            isErase = false;
            writerSprite.setTexture(penTextures);
        }


        ImGui::SliderFloat("Slider", &thickness, 1.f, 20.f);
        ImGui::ColorEdit3("Color Edit", colorArray);
        ImGui::End();

        isMouseMousePressed = isDraw ? sf::Mouse::isButtonPressed(sf::Mouse::Left) : false;

        if (isMouseMousePressed)
        {
            sf::Vector2f currentMousePosition = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

            if (mouseLeftButtonWasPressed)
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
            mouseLeftButtonWasPressed = true;
        }
        else
        {
            mouseLeftButtonWasPressed = false;
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
