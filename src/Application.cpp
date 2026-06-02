#include "Application.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include <cmath>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #include <commdlg.h>
#else
    #include <cstdlib>
    #include <cstdio>
    #include <array>
#endif

Application::Application()
    : window(sf::VideoMode(1600,1000), "Graphic Editor"),
      layerManager(1000,800),
      editorUI(localization, layerManager),
      canvasOffset(50.0f,50.0f),
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

    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = nullptr;
    if (!font) font = io.Fonts->AddFontFromFileTTF("arialmt.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    if (!font) font = io.Fonts->AddFontFromFileTTF("arial.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    if (!font) font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    if (!font) font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    if (font) {
        io.FontDefault = font;
        ImGui::SFML::UpdateFontTexture();
    }

    brushColor[0] = 0.0f; brushColor[1] = 0.0f; brushColor[2] = 0.0f;

    editorUI.setParameters(&brushSize, brushColor, &zoomLevel,
                           &showGrid, &showRulers,
                           &canvasWidth, &canvasHeight,
                           &mouseCanvasPos);
    editorUI.initTools(layerManager.getCurrentLayer(), brushColor);
}

void Application::saveStateForUndo() {
    auto layer = layerManager.getCurrentLayer();
    if (layer) {
        sf::Image img = layer->getTexture().getTexture().copyToImage();
        history.saveState(img, layerManager.getCurrentLayerIndex());
    }
}

std::string Application::openFileDialog() {
#ifdef _WIN32
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window.getSystemHandle();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) return std::string(szFile);
    return "";
#else
    std::array<char,128> buf;
    std::string res;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("zenity --file-selection --title='Open Image'","r"),pclose);
    if(!pipe) return "";
    while(fgets(buf.data(),buf.size(),pipe.get())) res+=buf.data();
    if(!res.empty() && res.back()=='\n') res.pop_back();
    return res;
#endif
}

std::string Application::saveFileDialog() {
#ifdef _WIN32
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window.getSystemHandle();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "PNG Files\0*.png\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "png";
    if (GetSaveFileNameA(&ofn)) return std::string(szFile);
    return "";
#else
    std::array<char,128> buf;
    std::string res;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("zenity --file-selection --save --confirm-overwrite","r"),pclose);
    if(!pipe) return "";
    while(fgets(buf.data(),buf.size(),pipe.get())) res+=buf.data();
    if(!res.empty() && res.back()=='\n') res.pop_back();
    if(!res.empty() && res.find('.')==std::string::npos) res+=".png";
    return res;
#endif
}

void Application::onLoadImage() {
    std::string path = openFileDialog();
    if(!path.empty()){
        sf::Image img;
        if(img.loadFromFile(path)){
            canvasWidth = img.getSize().x;
            canvasHeight = img.getSize().y;
            layerManager.resizeLayers(canvasWidth, canvasHeight);
            auto layer = layerManager.getCurrentLayer();
            if(layer){
                sf::Texture tex; tex.loadFromImage(img);
                layer->getTexture().clear(sf::Color::Transparent);
                layer->getTexture().draw(sf::Sprite(tex));
                layer->display();
                editorUI.updateToolLayer(layer);
                saveStateForUndo();
            }
        }
    }
}

void Application::onSaveImage() {
    std::string path = saveFileDialog();
    if(!path.empty()){
        sf::RenderTexture finalTex;
        finalTex.create(canvasWidth, canvasHeight);
        finalTex.clear(sf::Color::White);
        for(auto& l : layerManager.getLayers())
            if(l->isVisible()) finalTex.draw(l->getSprite());
        finalTex.display();
        finalTex.getTexture().copyToImage().saveToFile(path);
    }
}

void Application::onClearCanvas() {
    auto layer = layerManager.getCurrentLayer();
    if(layer){
        saveStateForUndo();
        layer->clear(sf::Color::White);
    }
}

void Application::onResizeCanvas() {
    layerManager.resizeLayers(canvasWidth, canvasHeight);
    editorUI.updateToolLayer(layerManager.getCurrentLayer());
    saveStateForUndo();
}

void Application::processEvents() {
    sf::Event event;
    while(window.pollEvent(event)){
        ImGui::SFML::ProcessEvent(event);
        if(event.type == sf::Event::Closed)
            window.close();

        if(event.type == sf::Event::KeyPressed){
            if(event.key.control && event.key.code == sf::Keyboard::Z){
                if(history.canUndo()){
                    CanvasState state = history.undo();
                    auto layer = layerManager.getCurrentLayer();
                    if(layer && state.layerId == layerManager.getCurrentLayerIndex()){
                        sf::Texture tex; tex.loadFromImage(state.image);
                        layer->getTexture().clear(sf::Color::Transparent);
                        layer->getTexture().draw(sf::Sprite(tex));
                        layer->display();
                    }
                }
            }
            else if(event.key.control && event.key.code == sf::Keyboard::Y){
                if(history.canRedo()){
                    CanvasState state = history.redo();
                    auto layer = layerManager.getCurrentLayer();
                    if(layer && state.layerId == layerManager.getCurrentLayerIndex()){
                        sf::Texture tex; tex.loadFromImage(state.image);
                        layer->getTexture().clear(sf::Color::Transparent);
                        layer->getTexture().draw(sf::Sprite(tex));
                        layer->display();
                    }
                }
            }
        }

        if(event.type == sf::Event::MouseWheelScrolled && !ImGui::GetIO().WantCaptureMouse){
            float delta = event.mouseWheelScroll.delta;
            float oldZoom = zoomLevel;
            zoomLevel += delta * 0.1f;
            zoomLevel = std::max(0.1f, std::min(zoomLevel,5.0f));
            sf::Vector2i mp = sf::Mouse::getPosition(window);
            sf::Vector2f wp = sf::Vector2f(mp) - canvasOffset;
            canvasOffset += wp * (1.0f - zoomLevel/oldZoom);
        }

        if(event.type == sf::Event::MouseButtonPressed){
            if(event.mouseButton.button == sf::Mouse::Left && !ImGui::GetIO().WantCaptureMouse && !isPanning){
                sf::Vector2i mp = sf::Mouse::getPosition(window);
                sf::Vector2f cp = (sf::Vector2f(mp) - canvasOffset) / zoomLevel;
                if(cp.x>=0 && cp.x<canvasWidth && cp.y>=0 && cp.y<canvasHeight){
                    isDrawing = true;
                    lastMousePos = cp;
                    saveStateForUndo();
                    sf::Color col(brushColor[0]*255, brushColor[1]*255, brushColor[2]*255);
                    editorUI.getCurrentTool()->onPress(cp, col, brushSize);
                }
            }
            if(event.mouseButton.button == sf::Mouse::Middle && !ImGui::GetIO().WantCaptureMouse){
                isPanning = true;
                panStart = sf::Vector2f(sf::Mouse::getPosition(window));
            }
        }

        if(event.type == sf::Event::MouseButtonReleased){
            if(event.mouseButton.button == sf::Mouse::Left && isDrawing){
                editorUI.getCurrentTool()->onRelease();
                isDrawing = false;
            }
            if(event.mouseButton.button == sf::Mouse::Middle) isPanning = false;
        }

        if(event.type == sf::Event::MouseMoved && isPanning){
            sf::Vector2f mp = sf::Vector2f(sf::Mouse::getPosition(window));
            canvasOffset += mp - panStart;
            panStart = mp;
        }
    }
}

void Application::update(float dt) {
    (void)dt; // пока не используется
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f canvasPos = (sf::Vector2f(mousePos) - canvasOffset) / zoomLevel;
    mouseCanvasPos = sf::Vector2i((int)canvasPos.x, (int)canvasPos.y);

    if(isDrawing && !isPanning && sf::Mouse::isButtonPressed(sf::Mouse::Left)){
        sf::Vector2f curPos = (sf::Vector2f(mousePos) - canvasOffset) / zoomLevel;
        sf::Color col(brushColor[0]*255, brushColor[1]*255, brushColor[2]*255);
        editorUI.getCurrentTool()->onDrag(lastMousePos, curPos, col, brushSize);
        lastMousePos = curPos;
    }
}

void Application::render() {
    window.clear(sf::Color(100,100,100));

    for(auto& layer : layerManager.getLayers()){
        if(layer->isVisible()){
            sf::Sprite sprite = layer->getSprite();
            sprite.setPosition(canvasOffset);
            sprite.setScale(zoomLevel, zoomLevel);
            window.draw(sprite);
        }
    }

    if(showGrid) editorUI.renderGrid(window, canvasOffset, zoomLevel, canvasWidth, canvasHeight);
    if(showRulers) editorUI.renderRulers(window, canvasOffset, zoomLevel, canvasWidth, canvasHeight);

    editorUI.renderSelectionOverlay(window, canvasOffset, zoomLevel);
    if(editorUI.getCurrentToolName() == "Shape"){
        auto* shape = dynamic_cast<ShapeTool*>(editorUI.getCurrentTool());
        if(shape) shape->renderPreview(window, canvasOffset, zoomLevel);
    }

    editorUI.renderToolPanel(
        window,
        [this](){ onLoadImage(); },
        [this](){ onSaveImage(); },
        [this](){ onClearCanvas(); },
        [this](){ onResizeCanvas(); }
    );

    ImGui::SFML::Render(window);
    window.display();
}

void Application::run() {
    sf::Clock frameClock;
    while(window.isOpen()){
        processEvents();
        sf::Time dt = frameClock.restart();
        update(dt.asSeconds());
        ImGui::SFML::Update(window, dt);
        render();
    }
    ImGui::SFML::Shutdown();
}