#pragma once
#include <string>
#include <map>

enum class Language {
    RU,
    ENG
};

class Localization {
private:
    Language currentLang;
    std::map<std::string, std::map<Language, std::string>> translations;

public:
    Localization() : currentLang(Language::ENG) {
        // Основные инструменты и разделы
        translations["tools"] = {{Language::ENG, "Tools"}, {Language::RU, "Инструменты"}};
        translations["file"] = {{Language::ENG, "File"}, {Language::RU, "Файл"}};
        translations["brush"] = {{Language::ENG, "Brush"}, {Language::RU, "Кисть"}};
        translations["eraser"] = {{Language::ENG, "Eraser"}, {Language::RU, "Ластик"}};
        translations["fill"] = {{Language::ENG, "Fill"}, {Language::RU, "Заливка"}};
        translations["text"] = {{Language::ENG, "Text"}, {Language::RU, "Текст"}};
        translations["pipette"] = {{Language::ENG, "Pipette"}, {Language::RU, "Пипетка"}};
        translations["shape"] = {{Language::ENG, "Shape"}, {Language::RU, "Фигура"}};
        translations["selection"] = {{Language::ENG, "Selection"}, {Language::RU, "Выделение"}};
        
        // Файловые операции (краткие названия)
        translations["load"] = {{Language::ENG, "Load"}, {Language::RU, "Загрузить"}};
        translations["save"] = {{Language::ENG, "Save"}, {Language::RU, "Сохранить"}};
        translations["clear"] = {{Language::ENG, "Clear"}, {Language::RU, "Очистить"}};
        
        translations["undo"] = {{Language::ENG, "Undo"}, {Language::RU, "Отменить"}};
        translations["redo"] = {{Language::ENG, "Redo"}, {Language::RU, "Повторить"}};
        
        // Фильтры
        translations["filters"] = {{Language::ENG, "Filters"}, {Language::RU, "Фильтры"}};
        translations["grayscale"] = {{Language::ENG, "Grayscale"}, {Language::RU, "Оттенки серого"}};
        translations["invert"] = {{Language::ENG, "Invert"}, {Language::RU, "Инверсия"}};
        translations["blur"] = {{Language::ENG, "Blur"}, {Language::RU, "Размытие"}};
        translations["sharpen"] = {{Language::ENG, "Sharpen"}, {Language::RU, "Резкость"}};
        translations["sepia"] = {{Language::ENG, "Sepia"}, {Language::RU, "Сепия"}};
        translations["bright_plus"] = {{Language::ENG, "Bright+"}, {Language::RU, "Ярче"}};
        translations["bright_minus"] = {{Language::ENG, "Bright-"}, {Language::RU, "Темнее"}};
        
        // Слои
        translations["layers"] = {{Language::ENG, "Layers"}, {Language::RU, "Слои"}};
        translations["new_layer"] = {{Language::ENG, "New Layer"}, {Language::RU, "Новый слой"}};
        translations["delete_layer"] = {{Language::ENG, "Delete Layer"}, {Language::RU, "Удалить слой"}};
        translations["opacity"] = {{Language::ENG, "Opacity"}, {Language::RU, "Прозрачность"}};
        translations["background"] = {{Language::ENG, "Background"}, {Language::RU, "Фон"}};
        
        // Холст
        translations["zoom"] = {{Language::ENG, "Zoom"}, {Language::RU, "Масштаб"}};
        translations["grid"] = {{Language::ENG, "Grid"}, {Language::RU, "Сетка"}};
        translations["rulers"] = {{Language::ENG, "Rulers"}, {Language::RU, "Линейки"}};
        translations["canvas_size"] = {{Language::ENG, "Canvas Size"}, {Language::RU, "Размер холста"}};
        translations["mouse_pos"] = {{Language::ENG, "Mouse Position"}, {Language::RU, "Позиция мыши"}};
        
        // Фигуры
        translations["rectangle"] = {{Language::ENG, "Rectangle"}, {Language::RU, "Прямоугольник"}};
        translations["square"] = {{Language::ENG, "Square"}, {Language::RU, "Квадрат"}};
        translations["circle"] = {{Language::ENG, "Circle"}, {Language::RU, "Круг"}};
        translations["ellipse"] = {{Language::ENG, "Ellipse"}, {Language::RU, "Эллипс"}};
        translations["line"] = {{Language::ENG, "Line"}, {Language::RU, "Линия"}};
        translations["star"] = {{Language::ENG, "Star"}, {Language::RU, "Звезда"}};
        
        translations["language"] = {{Language::ENG, "Language"}, {Language::RU, "Язык"}};
        translations["tool"] = {{Language::ENG, "Tool"}, {Language::RU, "Инструмент"}};
        translations["shape_type"] = {{Language::ENG, "Shape Type"}, {Language::RU, "Тип фигуры"}};
        
        // Диалоги текста
        translations["text_dialog_title"] = {{Language::ENG, "Enter Text"}, {Language::RU, "Ввод текста"}};
        translations["text_label"] = {{Language::ENG, "Text"}, {Language::RU, "Текст"}};
        translations["font_size_label"] = {{Language::ENG, "Font Size"}, {Language::RU, "Размер шрифта"}};
        translations["ok"] = {{Language::ENG, "OK"}, {Language::RU, "ОК"}};
        translations["cancel"] = {{Language::ENG, "Cancel"}, {Language::RU, "Отмена"}};
        
        // Настройки и помощь
        translations["settings"] = {{Language::ENG, "Settings"}, {Language::RU, "Настройки"}};
        translations["help"] = {{Language::ENG, "Help"}, {Language::RU, "Помощь"}};
    }

    void setLanguage(Language lang) { currentLang = lang; }
    Language getLanguage() const { return currentLang; }
    std::string get(const std::string& key) {
        auto it = translations.find(key);
        if (it != translations.end()) return it->second[currentLang];
        return key;
    }
};