#pragma once
#include <SFML/Graphics.hpp>
#include "Localization.hpp"
#include "LayerManager.hpp"
#include "EditorUI.hpp"
#include "HistoryManager.hpp"

class Application {
    private:
        void processEvents();
        void update(float dt);
        void render();
        void saveStateForUndo();

        void onLoadImage();
        void onSaveImage();
        void onClearCanvas();
        void onResizeCanvas();

        std::string openFileDialog();
        std::string saveFileDialog();

        sf::RenderWindow window;
        Localization localization;
        LayerManager layerManager;
        EditorUI editorUI;
        HistoryManager history;

        sf::Vector2f canvasOffset;
        float zoomLevel;
        bool showGrid;
        bool showRulers;
        float brushSize;
        float brushColor[3];
        bool isDrawing;
        sf::Vector2f lastMousePos;
        sf::Vector2i mouseCanvasPos;
        int canvasWidth;
        int canvasHeight;
        bool isPanning;
        sf::Vector2f panStart;

        sf::Clock deltaClock;

    public:
        Application();
        void run();

};