#include "Application.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

Application::Application()
    : window(sf::VideoMode(1600, 1000), "Graphic Editor"),
      layerManager(1000, 800),
      editorUI(localization, layerManager),
      canvasOffset(50.0f, 50.0f),
      zoomLevel(1.0f),
      showGrid(false),
      showRulers(true),
      brushSize(5.0f),
      isDrawing(false),
      canvasWidth(1000),
      canvasHeight(800),
      isPanning(false)
{
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    brushColor[0] = 0.0f;
    brushColor[1] = 0.0f;
    brushColor[2] = 0.0f;

    editorUI.setParameters(&brushSize, brushColor, &zoomLevel,
                           &showGrid, &showRulers,
                           &canvasWidth, &canvasHeight,
                           &mouseCanvasPos);
    editorUI.initTools(layerManager.getCurrentLayer(), brushColor);
}

void Application::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
        if (event.type == sf::Event::Closed)
            window.close();
    }
}

void Application::update(float dt) {
    (void)dt;
}

void Application::render() {
    window.clear(sf::Color(100, 100, 100));
    ImGui::SFML::Render(window);
    window.display();
}

void Application::run() {
    sf::Clock frameClock;
    while (window.isOpen()) {
        processEvents();
        sf::Time dt = frameClock.restart();
        update(dt.asSeconds());
        ImGui::SFML::Update(window, dt);
        render();
    }
    ImGui::SFML::Shutdown();
}