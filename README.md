# GraphicEditor

Графический редактор на C++ с использованием SFML 3 и Dear ImGui. Проект создан с целью демонстрации принципов ООП: наследование, полиморфизм, паттерн Command (undo/redo), а также работы с шейдерами (GLSL-фильтры).

---

## Возможности

- Рисование кистью (PenTool)
- Рисование фигур — прямоугольник, круг (ShapeTool)
- Выбор цвета через палитру
- Фильтры:
  - Яркость / Контраст
  - Чёрно-белый
  - Виньетка
  - Размытие
  - Сепия
- Очистка холста
- Сохранение в PNG

---

## Структура проекта

```
GraphicEditor/
    CMakeLists.txt
    README.md
    assets/
        shaders/
        brightness.frag     - фильтр яркости
        grayscale.frag      - чёрно-белый
        vignette.frag       - виньетка
        blur.frag           - размытие
        sepia.frag          - сепия
    src/
        main.cpp
        Editor.hpp / Editor.cpp         ← главный класс
        Canvas.hpp / Canvas.cpp         ← холст (RenderTexture)
        Shape.hpp                       ← абстрактный базовый класс фигур
        CircleShape.hpp / .cpp          ← круг
        RectShape.hpp / .cpp            ← прямоугольник
        Tool.hpp                        ← абстрактный инструмент
        PenTool.hpp / .cpp              ← инструмент кисть
        ShapeTool.hpp / .cpp            ← инструмент фигуры
        Command.hpp                     ← интерфейс Command
        AddShapeCommand.hpp             ← команда добавления фигуры
        CommandHistory.hpp / .cpp       ← стек с историей операцией
```

---

## Архитектура и ООП

### Иерархия классов

```
Shape  (абстрактный)
   + draw(RenderTarget&)  — чисто виртуальный
   + move(Vector2f)       — чисто виртуальный
   # m_color              — общий цвет

    RectShape             — прямоугольник
    CircleShapeDrawable   — круг

Tool  (абстрактный)
   + onMousePressed(pos, canvas)
   + onMouseMoved(pos, canvas)
   + onMouseReleased(pos, canvas)

    PenTool               — рисует точки при движении мыши
    ShapeTool             — рисует фигуру между двумя точками

Command  (абстрактный)
   + execute()
   + undo()

    AddShapeCommand       — добавить фигуру / удалить при отмене
```


## Управление

| Действие | Клавиша / кнопка |
|---|---|
| Рисовать | Зажать ЛКМ на холсте |
| Отмена (Undo) | `Ctrl + Z` |
| Очистить холст | Кнопка в панели |
| Сохранить PNG | Кнопка «Сохранить» |
| Выбор инструмента | Панель слева |
| Выбор фильтра | Выпадающий список |
| Закрыть | `Escape` или крестик |

---
