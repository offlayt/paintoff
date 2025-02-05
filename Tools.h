//#pragma once
//
//#include <SFML/Graphics.hpp>
//#include <queue>
//
//
//#include "Canvas.h"
//
//
//class Tools {
//private:
//    Canvas& canvas;
//    sf::Color currentColor = sf::Color::Black;
//    int brushSize = 5;
//    bool isErasing = false;
//    bool isFilling = false;
//    bool isSelecting = false;
//    sf::RectangleShape selectionRect;
//    sf::Image selectedPixels;
//    bool hasSelection = false;
//
//public:
//    Tools(Canvas& canv) : canvas(canv) {
//        selectionRect.setFillColor(sf::Color::Transparent);
//        selectionRect.setOutlineColor(sf::Color::Black);
//        selectionRect.setOutlineThickness(1);
//    }
//
//    void setColor(const sf::Color& color) {
//        currentColor = color;
//    }
//
//    void setBrushSize(int size) {
//        brushSize = size;
//    }
//
//    void setErasing(bool erasing) {
//        isErasing = erasing;
//    }
//
//    void setFilling(bool filling) {
//        isFilling = filling;
//    }
//
//    void setSelecting(bool selecting) {
//        isSelecting = selecting;
//    }
//
//    void handleDrawing(const sf::Vector2f& start, const sf::Vector2f& end) {
//        if (isErasing) {
//            canvas.drawLine(start, end, sf::Color::White, brushSize);
//        }
//        else {
//            canvas.drawLine(start, end, currentColor, brushSize);
//        }
//    }
//
//    void handleFill(const sf::Vector2i& pos) {
//        if (isFilling) {
//            canvas.drawFill(pos, isErasing ? sf::Color::White : currentColor);
//        }
//    }
//
//    void handleSelection(const sf::Vector2i& start, const sf::Vector2i& end) {
//        if (isSelecting) {
//            // Логика выделения
//        }
//    }
//
//    void clearSelection() {
//        hasSelection = false;
//    }
//};