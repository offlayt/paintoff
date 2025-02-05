//#pragma once
//
//#include <SFML/Graphics.hpp>
//
//class Canvas {
//private:
//    sf::RenderWindow& window;
//    sf::Texture canvasTexture;
//    sf::Sprite canvasSprite;
//    sf::Image canvasImage;
//    sf::View canvasView;
//    float zoomLevel = 1.0f;
//    sf::Vector2u canvasSize;
//
//public:
//    Canvas(sf::RenderWindow& win, const sf::Vector2u& size)
//        : window(win), canvasSize(size) {
//        canvasImage.create(canvasSize.x, canvasSize.y, sf::Color::White);
//        canvasTexture.loadFromImage(canvasImage);
//        canvasSprite.setTexture(canvasTexture);
//        canvasView = window.getDefaultView();
//    }
//
//    void drawLine(const sf::Vector2f& start, const sf::Vector2f& end, const sf::Color& color, int brushSize) {
//        float dx = end.x - start.x;
//        float dy = end.y - start.y;
//        float length = std::sqrt(dx * dx + dy * dy);
//
//        for (float i = 0; i <= length; i += 1.0f) {
//            float x = start.x + (dx * i / length);
//            float y = start.y + (dy * i / length);
//
//            for (int j = -brushSize; j <= brushSize; ++j) {
//                for (int k = -brushSize; k <= brushSize; ++k) {
//                    if (j * j + k * k <= brushSize * brushSize) {
//                        if (x + j >= 0 && y + k >= 0 && x + j < canvasSize.x && y + k < canvasSize.y) {
//                            canvasImage.setPixel(static_cast<int>(x + j), static_cast<int>(y + k), color);
//                        }
//                    }
//                }
//            }
//        }
//        canvasTexture.loadFromImage(canvasImage);
//    }
//
//    void drawFill(const sf::Vector2i& startPos, const sf::Color& newColor) {
//        sf::Color targetColor = canvasImage.getPixel(startPos.x, startPos.y);
//        if (targetColor == newColor) return;
//
//        std::queue<sf::Vector2i> pixels;
//        pixels.push(startPos);
//
//        while (!pixels.empty()) {
//            sf::Vector2i current = pixels.front();
//            pixels.pop();
//
//            if (current.x < 0 || current.x >= canvasSize.x || current.y < 0 || current.y >= canvasSize.y)
//                continue;
//
//            if (canvasImage.getPixel(current.x, current.y) != targetColor)
//                continue;
//
//            canvasImage.setPixel(current.x, current.y, newColor);
//
//            pixels.push(sf::Vector2i(current.x + 1, current.y));
//            pixels.push(sf::Vector2i(current.x - 1, current.y));
//            pixels.push(sf::Vector2i(current.x, current.y + 1));
//            pixels.push(sf::Vector2i(current.x, current.y - 1));
//        }
//        canvasTexture.loadFromImage(canvasImage);
//    }
//
//    void clear() {
//        canvasImage.create(canvasSize.x, canvasSize.y, sf::Color::White);
//        canvasTexture.loadFromImage(canvasImage);
//    }
//
//    void setZoom(float zoom) {
//        zoomLevel = zoom;
//        canvasView.setSize(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel);
//        canvasView.setCenter(window.getSize().x / 2, window.getSize().y / 2);
//    }
//
//    void draw() {
//        window.setView(canvasView);
//        window.draw(canvasSprite);
//    }
//
//    sf::Image& getImage() {
//        return canvasImage;
//    }
//
//    void setImage(const sf::Image& image) {
//        canvasImage = image;
//        canvasTexture.loadFromImage(canvasImage);
//    }
//};