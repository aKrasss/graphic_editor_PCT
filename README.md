# GraphicEditor

Графический редактор на C++ с использованием SFML и Dear ImGui. Поддерживает рисование кистью, фигуры, слои, фильтры, undo/redo и локализацию RU/ENG.

---

## Функционал

- Инструменты: кисть, ластик, заливка, пипетка, фигуры, текст, выделение
- Фигуры: прямоугольник, квадрат, круг, эллипс, линия, звезда
- Слои: создание, удаление, изменение порядка, прозрачность
- Undo / Redo (до 50 состояний)
- Фильтры: grayscale, invert, blur, sharpen, sepia, brightness
- Масштабирование и панорамирование холста
- Загрузка и сохранение изображений (PNG, JPG)
- Локализация: RU / ENG

---

## Зависимости

| Библиотека | Версия | Назначение |
|---|---|---|
| [SFML](https://github.com/SFML/SFML) | 2.6.1 | Окно, графика, события |
| [Dear ImGui](https://github.com/ocornut/imgui) | 1.87 | UI-интерфейс |
| [ImGui-SFML](https://github.com/SFML/imgui-sfml) | 2.6 | Связка ImGui + SFML |
| CMake | ≥ 3.16 | Система сборки |
| C++ | 17 | Стандарт языка |

Все библиотеки подтягиваются автоматически через CMake `FetchContent` — вручную ничего устанавливать не нужно.

---

## Сборка

### Linux / macOS

```bash
# Установить системные зависимости SFML
sudo apt install g++ cmake git \
    libxrandr-dev libxcursor-dev libudev-dev \
    libfreetype-dev libflac-dev libvorbis-dev \
    libgl1-mesa-dev libegl1-mesa-dev    # Ubuntu/Debian

# Клонировать и собрать
git clone https://github.com/username/GraphicEditor.git
cd GraphicEditor
mkdir build && cd build
cmake ..
cmake --build .
./GraphicEditor
```

### Windows (Visual Studio)

```bash
git clone https://github.com/username/GraphicEditor.git
cd GraphicEditor
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

---

## Структура проекта

```
GraphicEditor/
├── CMakeLists.txt
└── src/
    ├── main.cpp
    ├── Application.hpp / .cpp   - главный класс, главный цикл
    ├── Layer.hpp                - один слой холста
    ├── LayerManager.hpp         - управление слоями
    ├── HistoryManager.hpp       - undo / redo
    ├── Tool.hpp                 - абстрактный инструмент
    ├── Tools.hpp                - Brush, Eraser, Fill, Pipette
    ├── SelectionTool.hpp        - выделение и перемещение
    ├── ShapeTool.hpp            - рисование фигур
    ├── TextTool.hpp             - текст на холсте
    ├── Filter.hpp               - абстрактный фильтр
    ├── Filters.hpp              - все фильтры
    ├── EditorUI.hpp             - весь ImGui-интерфейс
    ├── Localization.hpp         - переводы RU / ENG
    ├── Textures/                - иконки инструментов
    └── fonts/                   - шрифты для TextTool
```

---

## Архитектура

### Иерархия классов

```
Tool  (абстрактный)
├── BrushTool       - рисование кистью
├── EraserTool      - ластик
├── FillTool        - заливка (flood fill)
├── PipetteTool     - пипетка (взять цвет)
├── ShapeTool       - фигуры (rect, circle, line, star...)
├── SelectionTool   - выделение и перемещение области
└── TextTool        - текст на холсте

Filter  (абстрактный)
├── GrayscaleFilter
├── InvertFilter
├── BlurFilter
├── BrightnessFilter
├── SepiaFilter
└── SharpenFilter
```

### Паттерны

- **Полиморфизм** — `Tool` и `Filter` как абстрактные базовые классы
- **Command** — `HistoryManager` сохраняет снимки состояния для undo/redo
- **Shared ownership** — слои хранятся через `shared_ptr<Layer>`

---

## Описание классов

### Application

Главный класс. Содержит главный цикл и связывает все модули.

| Метод | Назначение |
|---|---|
| `run()` | Запустить главный цикл |
| `processEvents()` | Обработать события мыши и клавиатуры |
| `update(float dt)` | Обновить состояние каждый кадр |
| `render()` | Отрисовать холст и UI |
| `saveStateForUndo()` | Сохранить снимок текущего слоя в историю |
| `applyFilterWithUndo(func)` | Применить фильтр с сохранением в историю |
| `centerCanvas()` | Выровнять холст по центру окна |
| `toggleFullscreen()` | Переключить полноэкранный режим |
| `onLoadImage()` | Загрузить изображение с диска |
| `onSaveImage()` | Сохранить изображение на диск |
| `onClearCanvas()` | Очистить холст белым цветом |
| `onResizeCanvas()` | Изменить размер холста |

---

### Layer

Один слой холста на основе `sf::RenderTexture`.

| Метод | Назначение |
|---|---|
| `Layer(w, h, id, name)` | Создать слой нужного размера |
| `clear(color)` | Залить слой цветом |
| `getTexture()` | Получить `RenderTexture` для рисования |
| `getSprite()` | Получить спрайт для отображения |
| `setOpacity(float)` | Задать прозрачность 0.0–1.0 |
| `setVisible(bool)` | Показать / скрыть слой |
| `setName(string)` | Переименовать слой |
| `display()` | Применить изменения текстуры |

---

### LayerManager

Управляет списком слоёв. При создании автоматически добавляет слой «Background».

| Метод | Назначение |
|---|---|
| `addLayer(name)` | Добавить новый слой |
| `deleteLayer(index)` | Удалить слой (нельзя удалить последний) |
| `moveLayer(from, to)` | Изменить порядок слоёв |
| `setCurrentLayer(index)` | Выбрать активный слой |
| `getCurrentLayer()` | Получить активный слой |
| `resizeLayers(w, h)` | Изменить размер всех слоёв |

---

### HistoryManager

Undo/Redo через два стека. Максимум 50 состояний.

| Метод | Назначение |
|---|---|
| `saveState(image, layerId)` | Сохранить состояние слоя |
| `undo()` | Отменить последнее действие |
| `redo()` | Повторить отменённое действие |
| `canUndo()` | Есть ли что отменять |
| `canRedo()` | Есть ли что повторять |
| `clear()` | Очистить всю историю |

---

### Инструменты

| Класс | Назначение |
|---|---|
| `BrushTool` | Кисть — рисует кругами с интерполяцией между кадрами |
| `EraserTool` | Ластик — то же что кисть, но белым цветом |
| `FillTool` | Заливка — flood fill через стек (без рекурсии) |
| `PipetteTool` | Пипетка — берёт цвет пикселя под курсором |
| `ShapeTool` | Фигуры — Rectangle, Square, Circle, Ellipse, Line, Star |
| `SelectionTool` | Выделение области и перемещение |
| `TextTool` | Печать текста на холсте с выбором шрифта |

---

### Фильтры

| Класс | Алгоритм |
|---|---|
| `GrayscaleFilter` | `gray = 0.299r + 0.587g + 0.114b` |
| `InvertFilter` | `255 - r, 255 - g, 255 - b` |
| `BlurFilter` | Усреднение пикселей в ядре 3×3 |
| `BrightnessFilter` | `clamp(r + adj, 0, 255)` |
| `SepiaFilter` | Матричное умножение RGB-каналов |
| `SharpenFilter` | Свёртка с ядром {0,-1,0},{-1,5,-1},{0,-1,0} |

---

### Localization

Переводы интерфейса. Переключение между RU и ENG через `setLanguage()`.

| Метод | Назначение |
|---|---|
| `setLanguage(Language)` | Сменить язык (RU / ENG) |
| `get(key)` | Получить строку по ключу на текущем языке |

---

## Управление

| Действие | Клавиша |
|---|---|
| Рисовать | Зажать ЛКМ на холсте |
| Панорамирование | Зажать ПКМ + тянуть |
| Undo | Ctrl + Z |
| Redo | Ctrl + Y |
| Сохранить | Ctrl + S |

---
