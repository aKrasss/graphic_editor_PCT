#pragma once
#include "Tool.hpp"
#include "Layer.hpp"
#include "Localization.hpp"
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cstdlib>
#include <sstream>
#include "imgui.h"

class TextTool : public Tool {
private:
    struct FontInfo {
        std::string path;
        std::string name;
    };

    std::shared_ptr<Layer> layer;
    Localization* localization;
    bool waitingForText;
    sf::Vector2f pressPos;
    sf::Color textColor;
    int fontSize;
    char inputBuffer[256];

    std::vector<FontInfo> availableFonts;
    int selectedFontIndex;
    sf::Font currentFont;

    float rotationAngle = 0.0f;
    bool flipX = false, flipY = false;

    static bool fileExists(const std::string& path) {
        std::ifstream f(path.c_str());
        return f.good();
    }

    void scanDirectoryForFonts(const std::string& dir, std::vector<FontInfo>& outFonts) {
        if (!std::filesystem::exists(dir)) return;
        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir, std::filesystem::directory_options::skip_permission_denied)) {
                bool isFile = false;
                if (entry.is_regular_file()) isFile = true;
                else if (entry.is_symlink()) {
                    try {
                        if (std::filesystem::is_regular_file(entry.symlink_status()))
                            isFile = true;
                    } catch (...) {}
                }
                if (isFile) {
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (ext == ".ttf" || ext == ".otf" || ext == ".ttc") {
                        FontInfo info;
                        info.path = entry.path().string();
                        info.name = entry.path().filename().string();
                        outFonts.push_back(info);
                    }
                }
            }
        } catch (const std::exception&) {
        }
    }

    void loadFontsViaFcList() {
        FILE* pipe = popen("fc-list : file", "r");
        if (!pipe) return;
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe)) {
            std::string line(buffer);
            if (!line.empty() && line.back() == '\n') line.pop_back();
            if (line.size() > 4) {
                std::string ext = line.substr(line.size() - 4);
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".ttf" || ext == ".otf" || ext == ".ttc") {
                    FontInfo info;
                    info.path = line;
                    size_t slash = line.find_last_of('/');
                    info.name = (slash != std::string::npos) ? line.substr(slash + 1) : line;
                    availableFonts.push_back(info);
                }
            }
        }
        pclose(pipe);
    }

    void loadAvailableFonts() {
        availableFonts.clear();

        std::vector<std::string> localFolders = {"fonts/", "src/fonts/", "../src/fonts/"};
        for (const auto& folder : localFolders) {
            if (std::filesystem::exists(folder))
                scanDirectoryForFonts(folder, availableFonts);
        }

        #ifdef __linux__
        loadFontsViaFcList();
        #endif

        if (availableFonts.empty()) {
            std::vector<std::string> systemPaths;
            #ifdef _WIN32
                systemPaths = {"C:/Windows/Fonts/"};
            #elif __APPLE__
                systemPaths = {"/System/Library/Fonts/", "/Library/Fonts/"};
                        #else
                systemPaths = {
                    "/usr/share/fonts/",
                    "/usr/local/share/fonts/",
                    std::string(getenv("HOME")) + "/.fonts/",
                    std::string(getenv("HOME")) + "/.local/share/fonts/",
                    "/usr/share/fonts/truetype/msttcorefonts/",
                    "/usr/share/fonts/truetype/",
                    "/usr/share/fonts/opentype/",
                    "/usr/share/fonts/type1/",
                    "/usr/share/fonts/X11/",
                    "/usr/share/fonts/cMap/"
                };
            #endif
            for (const auto& basePath : systemPaths) {
                if (std::filesystem::exists(basePath))
                    scanDirectoryForFonts(basePath, availableFonts);
            }
        }

        for (auto& info : availableFonts) {
            std::error_code ec;
            std::filesystem::path absPath = std::filesystem::absolute(info.path, ec);
            if (!ec) {
                info.path = absPath.string();
            }
        }

        std::sort(availableFonts.begin(), availableFonts.end(),
            [](const FontInfo& a, const FontInfo& b) { return a.path < b.path; });
        availableFonts.erase(std::unique(availableFonts.begin(), availableFonts.end(),
            [](const FontInfo& a, const FontInfo& b) { return a.path == b.path; }), availableFonts.end());

        std::sort(availableFonts.begin(), availableFonts.end(),
            [](const FontInfo& a, const FontInfo& b) { return a.name < b.name; });
        availableFonts.erase(std::unique(availableFonts.begin(), availableFonts.end(),
            [](const FontInfo& a, const FontInfo& b) { return a.name == b.name; }), availableFonts.end());

        selectedFontIndex = -1;
        for (size_t i = 0; i < availableFonts.size(); ++i) {
            if (currentFont.loadFromFile(availableFonts[i].path)) {
                selectedFontIndex = i;
                break;
            }
        }
    }

    bool loadFontFromIndex(int idx) {
        if (idx >= 0 && idx < (int)availableFonts.size()) {
            return currentFont.loadFromFile(availableFonts[idx].path);
        }
        return false;
    }

public:
    TextTool(std::shared_ptr<Layer> l, Localization* loc)
        : layer(l), localization(loc), waitingForText(false), fontSize(20), selectedFontIndex(-1)
    {
        memset(inputBuffer, 0, sizeof(inputBuffer));
        loadAvailableFonts();
    }

    void setLayer(std::shared_ptr<Layer> l) { layer = l; }

    void onPress(sf::Vector2f pos, sf::Color color, float) override {
        if (!layer) return;
        pressPos = pos;
        textColor = color;
        waitingForText = true;
        memset(inputBuffer, 0, sizeof(inputBuffer));
        rotationAngle = 0.0f;
        flipX = flipY = false;
    }

    void onDrag(sf::Vector2f, sf::Vector2f, sf::Color, float) override {}
    void onRelease() override {}
        void renderTextPopup() {
        if (!waitingForText) return;

        if (availableFonts.empty()) {
            ImGui::OpenPopup("FontError");
            if (ImGui::BeginPopupModal("FontError", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("No fonts found on your system.");
                ImGui::Text("Please install fonts or check permissions.");
                ImGui::Text("Try installing 'fontconfig' and running 'fc-list'.");
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                    waitingForText = false;
                }
                ImGui::EndPopup();
            } else {
                waitingForText = false;
            }
            return;
        }

        ImGui::OpenPopup(localization->get("text_dialog_title").c_str());
        if (ImGui::BeginPopupModal(localization->get("text_dialog_title").c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s:", localization->get("text_label").c_str());
            ImGui::InputText("##text_input", inputBuffer, sizeof(inputBuffer));

            ImGui::Text("Font:");
            std::vector<const char*> fontNames;
            for (auto& f : availableFonts) fontNames.push_back(f.name.c_str());
            if (ImGui::Combo("##font_select", &selectedFontIndex, fontNames.data(), fontNames.size())) {
                loadFontFromIndex(selectedFontIndex);
            }

            ImGui::Text("%s:", localization->get("font_size_label").c_str());
            ImGui::InputInt("##font_size_input", &fontSize);
            if (fontSize < 6) fontSize = 6;

            ImGui::Text("Transform:");
            ImGui::SliderFloat("Rotate", &rotationAngle, 0.0f, 360.0f);
            ImGui::Checkbox("Flip X", &flipX); ImGui::SameLine();
            ImGui::Checkbox("Flip Y", &flipY);
            ImGui::Separator();

            if (ImGui::Button(localization->get("ok").c_str())) {
                if (strlen(inputBuffer) > 0 && layer && selectedFontIndex >= 0) {
                    sf::Text text;
                    text.setFont(currentFont);
                    sf::String utf8String = sf::String::fromUtf8(inputBuffer, inputBuffer + strlen(inputBuffer));
                    text.setString(utf8String);
                    text.setCharacterSize(fontSize);
                    text.setFillColor(textColor);
                    text.setPosition(pressPos);

                    sf::Transform transform;
                    transform.rotate(rotationAngle, pressPos.x, pressPos.y);
                    if (flipX) transform.scale(-1, 1, pressPos.x, pressPos.y);
                    if (flipY) transform.scale(1, -1, pressPos.x, pressPos.y);

                    sf::RenderStates states;
                    states.transform = transform;
                    layer->getTexture().draw(text, states);
                    layer->display();
                }
                waitingForText = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button(localization->get("cancel").c_str())) {
                waitingForText = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        } else {
            waitingForText = false;
        }
    }

    std::string getName() const override { return "Text"; }
};
