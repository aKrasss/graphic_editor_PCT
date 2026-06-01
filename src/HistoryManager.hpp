#pragma once
#include <SFML/Graphics.hpp>
#include <stack>
#include <memory>

struct CanvasState {
    sf::Image image;
    int layerId;
};

class HistoryManager {
private:
    std::stack<CanvasState> undoStack;
    std::stack<CanvasState> redoStack;
    const size_t MAX_HISTORY = 50;

public:
    void saveState(const sf::Image& image, int layerId) {
        CanvasState state;
        state.image = image;
        state.layerId = layerId;

        undoStack.push(state);

        while (!redoStack.empty()) {
            redoStack.pop();
        }

        if (undoStack.size() > MAX_HISTORY) {
            std::stack<CanvasState> tempStack;
            for (size_t i = 0; i < MAX_HISTORY; ++i) {
                tempStack.push(undoStack.top());
                undoStack.pop();
            }
            undoStack = std::stack<CanvasState>();
            while (!tempStack.empty()) {
                undoStack.push(tempStack.top());
                tempStack.pop();
            }
        }
    }

    bool canUndo() const {
        return !undoStack.empty();
    }

    bool canRedo() const {
        return !redoStack.empty();
    }

    CanvasState undo() {
        if (!undoStack.empty()) {
            CanvasState state = undoStack.top();
            undoStack.pop();
            redoStack.push(state);
            return state;
        }
        return CanvasState();
    }

    CanvasState redo() {
        if (!redoStack.empty()) {
            CanvasState state = redoStack.top();
            redoStack.pop();
            undoStack.push(state);
            return state;
        }
        return CanvasState();
    }

    size_t getUndoStackSize() const {
        return undoStack.size();
    }

    size_t getRedoStackSize() const {
        return redoStack.size();
    }

    void clear() {
        while (!undoStack.empty()) undoStack.pop();
        while (!redoStack.empty()) redoStack.pop();
    }
};
