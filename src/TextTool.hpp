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
#include "imgui.h"

class TextTool : public Tool {
private:
    std::shared_ptr<Layer> layer;
    Localization* localization;
    bool waitingForText;
    sf::Vector2f pressPos;
    sf::Color textColor;
    int fontSize;
    char inputBuffer[256];
    sf::Font cachedFont;
    bool fontLoaded;

    static bool fileExists(const std::string& path) {
        std::ifstream f(path.c_str());
        return f.good();
    }

    void loadFont() {
        if (fontLoaded) return;

        // Приоритет: папка fonts/ рядом с программой, затем локальная папка, затем системные
        std::vector<std::string> fontPaths = {
            "fonts/arialmt.ttf",
            "fonts/arial.ttf",
            "arialmt.ttf",
            "arial.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/ubuntu/Ubuntu-Regular.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/segoeui.ttf"
        };

        for (const auto& path : fontPaths) {
            if (fileExists(path) && cachedFont.loadFromFile(path)) {
                fontLoaded = true;
                return;
            }
        }
        fontLoaded = false;
    }

public:
    TextTool(std::shared_ptr<Layer> l, Localization* loc)
        : layer(l), localization(loc), waitingForText(false), fontSize(20), fontLoaded(false)
    {
        memset(inputBuffer, 0, sizeof(inputBuffer));
        loadFont();
    }

    void setLayer(std::shared_ptr<Layer> l) { layer = l; }

    void onPress(sf::Vector2f pos, sf::Color color, float) override {
        if (!layer) return;
        pressPos = pos;
        textColor = color;
        waitingForText = true;
        memset(inputBuffer, 0, sizeof(inputBuffer));
    }

    void onDrag(sf::Vector2f, sf::Vector2f, sf::Color, float) override {}
    void onRelease() override {}

    void renderTextPopup() {
        if (!waitingForText) return;

        if (!fontLoaded) {
            ImGui::OpenPopup("FontError");
            if (ImGui::BeginPopupModal("FontError", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Font not loaded. Cannot add text.");
                ImGui::Text("Expected file: fonts/arialmt.ttf or arial.ttf");
                ImGui::Text("in the program folder or system fonts.");
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
            ImGui::Text("%s:", localization->get("font_size_label").c_str());
            ImGui::InputInt("##font_size_input", &fontSize);
            if (fontSize < 6) fontSize = 6;

            if (ImGui::Button(localization->get("ok").c_str())) {
                if (strlen(inputBuffer) > 0 && layer) {
                    sf::Text text;
                    text.setFont(cachedFont);
                    sf::String utf8String = sf::String::fromUtf8(inputBuffer, inputBuffer + strlen(inputBuffer));
                    text.setString(utf8String);
                    text.setCharacterSize(fontSize);
                    text.setFillColor(textColor);
                    text.setPosition(pressPos);
                    layer->getTexture().draw(text);
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