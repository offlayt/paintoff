#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <string>
#include <cmath>
#include <iostream>
#include <queue>
#include "tinyfiledialogs/tinyfiledialogs.h"

#include "Figures.h"

class Paint {
private:
    sf::RenderWindow window;
    sf::Texture canvasTexture;
    sf::Sprite canvasSprite;
    sf::Image canvasImage;
    float color[3] = { 0.f, 0.f, 0.f };
    sf::Color currentColor = sf::Color::Black;
    int brushSize = 5;
    bool isErasing = false;
    char filename[128] = "drawing.png";
    float zoomLevel = 1.0f;
    sf::View canvasView;
    sf::Vector2i lastMousePos;
    bool isMousePressed = false;
    sf::Vector2u canvasSize; // Новое поле для хранения размера холста
    sf::Texture PaintButtonTexture; // SFML-текстура иконки
    //ImTextureID iconTextureID; // ImGui-совместимая текстура
    bool isFilling = false; // Флаг инструмента "Заливка"

    bool isSelecting = false;               // Флаг режима выделения
    sf::Vector2f selectionStart;            // Начальная позиция выделения
    sf::Vector2f selectionEnd;              // Конечная позиция выделения
    sf::Image selectionBuffer;              // Буфер для хранения выделенных пикселей
    sf::Vector2f selectionOffset;           // Смещение выделения при перемещении
    bool isMovingSelection = false;         // Флаг перемещения выделения

    FigureType currentFigure = FigureType::Rectangle; // Текущая фигура
    sf::Vector2f figureStartPos;             // Начальная позиция рисования фигуры
    bool isDrawingFigure = false;            // Флаг рисования фигуры

    sf::Texture ColorIconTexture;



public:
    Paint() {
        // Получаем разрешение экрана
        sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
        // Устанавливаем размер окна (80% от разрешения экрана)
        unsigned int windowWidth = desktopMode.width * 0.8;
        unsigned int windowHeight = desktopMode.height * 0.8;

        window.create(sf::VideoMode(windowWidth, windowHeight), "Paint", sf::Style::Default);

        // Загружаем иконку
        sf::Image icon;
        if (!icon.loadFromFile("canva.png")) {
            std::cerr << "Failed to load icon!\n";
        }
        else {
            window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
        }

        window.setFramerateLimit(60);
        ImGui::SFML::Init(window);

        canvasSize = window.getSize();
        canvasImage.create(canvasSize.x, canvasSize.y, sf::Color::White);
        canvasTexture.loadFromImage(canvasImage);
        canvasSprite.setTexture(canvasTexture);

        canvasView = window.getDefaultView();

        /*if (!ColorIconTexture.loadFromFile("ColorIcon.png")) {
            std::cerr << "Failed to load brush icon!\n";
        }*/
    }

    void run() {
        sf::Clock deltaClock;
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                ImGui::SFML::ProcessEvent(event);
                if (event.type == sf::Event::Closed) {
                    window.close();
                }

                // Обработка изменения размеров окна
                if (event.type == sf::Event::Resized) {
                    sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                    window.setView(sf::View(visibleArea));
                    canvasView.setSize(event.size.width * zoomLevel, event.size.height * zoomLevel);
                    canvasView.setCenter(event.size.width / 2, event.size.height / 2);
                }

                // Начало рисования (левая кнопка мыши)
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    isMousePressed = true;
                    lastMousePos = sf::Mouse::getPosition(window);
                }

                // Завершение рисования (левая кнопка мыши)
                if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                    isMousePressed = false;
                }

                // Рисование на холсте (только если курсор не над интерфейсом и не в режиме выделения)
                if (isMousePressed && !ImGui::IsAnyItemHovered() && !isSelecting) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, canvasView); // Преобразуем координаты с учетом масштаба
                    sf::Vector2f lastWorldPos = window.mapPixelToCoords(lastMousePos, canvasView);

                    // Рисуем линию между предыдущей и текущей позициями мыши
                    drawLine(lastWorldPos, worldPos);

                    lastMousePos = mousePos; // Обновляем предыдущую позицию мыши
                }

                // Начало рисования фигуры (левая кнопка мыши)
                if (event.type == sf::Event::MouseButtonPressed &&
                    event.mouseButton.button == sf::Mouse::Right &&
                    !ImGui::IsAnyItemHovered())
                {
                    if (currentFigure == FigureType::Rectangle ||
                        currentFigure == FigureType::Circle ||
                        currentFigure == FigureType::ConnectLine)
                    {
                        isDrawingFigure = true;
                        figureStartPos = window.mapPixelToCoords(
                            sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
                        );
                    }
                }

                // Завершение рисования фигуры
                if (event.type == sf::Event::MouseButtonReleased &&
                    event.mouseButton.button == sf::Mouse::Right &&
                    isDrawingFigure)
                {
                    isDrawingFigure = false;
                    sf::Vector2f endPos = window.mapPixelToCoords(
                        sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
                    );

                    // Рисуем фигуру
                    switch (currentFigure) {
                    case FigureType::Rectangle:
                        drawRectangle(figureStartPos, endPos);
                        break;
                    case FigureType::Circle:
                        drawCircle(figureStartPos, endPos);
                        break;
                    case FigureType::ConnectLine:
                        drawLine(figureStartPos, endPos);
                        break;
                    default:
                        break;
                    }
                }


                // Начало выделения
                if (event.type == sf::Event::MouseButtonPressed &&
                    event.mouseButton.button == sf::Mouse::Left &&
                    !ImGui::IsAnyItemHovered())
                {
                    if (isSelecting) {
                        // Начало выделения области
                        selectionStart = window.mapPixelToCoords(
                            sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
                        );
                    }
                    else if (isMovingSelection) {
                        // Начало перемещения выделения
                        isMousePressed = false;
                        selectionOffset = window.mapPixelToCoords(
                            sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
                        ) - selectionStart;
                    }
                }

                // Завершение выделения (отпускание левой кнопки мыши)
                if (event.type == sf::Event::MouseButtonReleased &&
                    event.mouseButton.button == sf::Mouse::Left)
                {
                    if (isSelecting) {
                        selectionEnd = window.mapPixelToCoords(
                            sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
                        );

                        // Копирование выделенных пикселей в буфер
                        int width = static_cast<int>(std::abs(selectionEnd.x - selectionStart.x));
                        int height = static_cast<int>(std::abs(selectionEnd.y - selectionStart.y));
                        selectionBuffer.create(width, height, sf::Color::Transparent);

                        for (int x = 0; x < width; ++x) {
                            for (int y = 0; y < height; ++y) {
                                int px = static_cast<int>(selectionStart.x) + x;
                                int py = static_cast<int>(selectionStart.y) + y;
                                if (px >= 0 && py >= 0 && px < canvasSize.x && py < canvasSize.y) {
                                    selectionBuffer.setPixel(x, y, canvasImage.getPixel(px, py));
                                }
                            }
                        }
                        isSelecting = false;
                        isMovingSelection = true; // режим перемещения
                    }
                    else if (isMovingSelection) {
                        // Вставка выделенных пикселей на новое место
                        sf::Vector2f newPos = window.mapPixelToCoords(
                            sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
                        ) - selectionOffset;

                        for (int x = 0; x < selectionBuffer.getSize().x; ++x) {
                            for (int y = 0; y < selectionBuffer.getSize().y; ++y) {
                                int px = static_cast<int>(newPos.x) + x;
                                int py = static_cast<int>(newPos.y) + y;
                                if (px >= 0 && py >= 0 && px < canvasSize.x && py < canvasSize.y) {
                                    canvasImage.setPixel(px, py, selectionBuffer.getPixel(x, y));
                                }
                            }
                        }

                        isSelecting = true; // Кнопка активна после выделения
                        isMovingSelection = false; // Завершаем перемещение
                        canvasTexture.loadFromImage(canvasImage);
                    }
                }

                // Перемещение выделения
                if (isMovingSelection && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    selectionStart = mousePos - selectionOffset;
                }


                // Масштабирование колесом мыши
                if (event.type == sf::Event::MouseWheelScrolled) {
                    if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                        // Изменение уровня масштабирования с ограничениями
                        zoomLevel *= (event.mouseWheelScroll.delta > 0) ? 1.1f : 0.9f;
                        zoomLevel = std::max(0.1f, std::min(zoomLevel, 5.0f)); // Ограничение от 0.1x до 5x
                        canvasView.setSize(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel);
                        canvasView.setCenter(window.getSize().x / 2, window.getSize().y / 2);
                    }
                }

                // Начало перемещения холста колесом мыши
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Middle) {
                    lastMousePos = sf::Mouse::getPosition(window);
                }
            }

            // Перемещение холста при зажатой средней кнопке мыши
            if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
                sf::Vector2i mouseDelta = sf::Mouse::getPosition(window) - lastMousePos;
                canvasView.move(-mouseDelta.x * zoomLevel, -mouseDelta.y * zoomLevel);
                lastMousePos = sf::Mouse::getPosition(window);
            }

            ImGui::SFML::Update(window, deltaClock.restart());

            // Отрисовка интерфейса
            drawGUI();

            // заливка (если инструмент активен и курсор не над интерфейсом)
            if (isFilling && sf::Mouse::isButtonPressed(sf::Mouse::Left) && !ImGui::IsAnyItemHovered()) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, canvasView);

                if (worldPos.x >= 0 && worldPos.y >= 0 &&
                    worldPos.x < canvasSize.x && worldPos.y < canvasSize.y)
                {
                    drawFill(sf::Vector2i(worldPos), isErasing ? sf::Color::White : currentColor);
                    canvasTexture.loadFromImage(canvasImage);
                }
            }

            // Отрисовка
            window.clear();
            window.setView(canvasView);
            window.draw(canvasSprite);


            // Отрисовка прямоугольника выделения
            if (isSelecting && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                sf::Vector2f currentMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                sf::RectangleShape selectionRect;
                selectionRect.setPosition(selectionStart);
                selectionRect.setSize(currentMousePos - selectionStart);
                selectionRect.setFillColor(sf::Color::Transparent);
                selectionRect.setOutlineColor(sf::Color::Red);
                selectionRect.setOutlineThickness(1);
                window.draw(selectionRect);
            }

            // Отрисовка перемещаемой области
            if (isMovingSelection) {
                sf::Sprite selectionSprite;
                sf::Texture selectionTexture;
                selectionTexture.loadFromImage(selectionBuffer);
                selectionSprite.setTexture(selectionTexture);
                selectionSprite.setPosition(selectionStart);
                window.draw(selectionSprite);
            }


            window.setView(window.getDefaultView());
            ImGui::SFML::Render(window);
            window.display();
        }
        ImGui::SFML::Shutdown();
    }

    void drawGUI() {
        // позицию и размер окна инструментов (верхняя панель)
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(window.getSize().x, 67)); // Высота панели — 60 пикселей
        ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // размер кнопок
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));

        // Кнопка с иконкой для выпадающего меню
        //ImTextureID textureID = (ImTextureID)(void*)PaintButtonTexture.getNativeHandle();
        //if (ImGui::ImageButton(textureID, ImVec2(30, 30))) { // Кнопка с иконкой
        //    ImGui::OpenPopup("ColorMenu"); // Открываем выпадающее меню
        //}

       /* static sf::Texture ColorIconTexture;
        if (!ColorIconTexture.loadFromFile("ColorIcon.png")) {
            std::cerr << "Failed to load Color icon!\n";
        }*/

        // ImTextureID ColorIcon = (ImTextureID)(void*)(uintptr_t)ColorIconTexture.getNativeHandle();

        // кнопка для выпадающего меню цветов
        if (ImGui::Button("C", ImVec2(30, 30))) {
            ImGui::OpenPopup("ColorMenu"); // Открываем выпадающее меню
        }

        // Выпадающее меню для выбора цвета
        if (ImGui::BeginPopup("ColorMenu", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            if (ImGui::ColorEdit3("Color", color)) {
                currentColor.r = static_cast<sf::Uint8>(color[0] * 255);
                currentColor.g = static_cast<sf::Uint8>(color[1] * 255);
                currentColor.b = static_cast<sf::Uint8>(color[2] * 255);
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine(); // следующий элемент на той же линии

        // кнопка для выпадающего меню фигур
        if (ImGui::Button("F", ImVec2(30, 30))) {
            ImGui::OpenPopup("FigureMenu"); // Открываем выпадающее меню
        }

        // Выпадающее меню для выбора figure
        if (ImGui::BeginPopup("FigureMenu", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            if (ImGui::Button("Circle")) {
                currentFigure = FigureType::Circle;
                ImGui::CloseCurrentPopup(); // Закрываем меню после выбора
            }
            if (ImGui::Button("Rectangle")) {
                currentFigure = FigureType::Rectangle;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Line")) {
                currentFigure = FigureType::ConnectLine;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine(); // следующий элемент на той же линии

        if (ImGui::Button("Fill", ImVec2(80, 30))) {
            isFilling = !isFilling;
            isMousePressed = false;
        }
        ImGui::SameLine(); // следующий элемент на той же линии

        // Кнопка выделения
        if (ImGui::Button("Select", ImVec2(80, 30))) {
            if (isSelecting || isMovingSelection) {
                isSelecting = false;
                isMovingSelection = false;
                isFilling = false;
                isMousePressed = false;
            }
            else {
                isSelecting = true;
                isMovingSelection = false;
            }
        }
        ImGui::SameLine(); // следующий элемент на той же линии

        // Размер кисти
        ImGui::SetNextItemWidth(100); // ширина элемента
        ImGui::SliderInt("Brush", &brushSize, 1, 50);

        ImGui::SameLine(); // следующий элемент на той же линии

        // Ластик
        ImGui::Checkbox("Eraser", &isErasing);

        ImGui::SameLine(); // следующий элемент на той же линии

        // Поле для ввода имени файла
        //ImGui::SetNextItemWidth(200); // Фиксированная ширина элемента
        //ImGui::InputText("Filename", filename, sizeof(filename));

        // Кнопка сохранения
        if (ImGui::Button("Save", ImVec2(80, 30))) {
            const char* filters[] = { "*.png" };
            const char* savePath = tinyfd_saveFileDialog(
                "Save image",
                filename,
                1,
                filters,
                "PNG file (*.png)");
            if (savePath) {
                std::string path = savePath;
                std::replace(path.begin(), path.end(), '\\', '/'); // Заменяем обратные слэши на прямые
                if (path.size() < 4 || path.substr(path.size() - 4) != ".png") {
                    path += ".png";
                }
                if (!canvasImage.saveToFile(path)) {
                    std::cerr << "Error saving!\n";
                }
            }
        }

        ImGui::SameLine(); // следующий элемент на той же линии

        // Кнопка очистки холста
        if (ImGui::Button("Clear", ImVec2(80, 30))) {
            canvasImage.create(canvasSize.x, canvasSize.y, sf::Color::White);
            canvasTexture.loadFromImage(canvasImage);
        }

        ImGui::SameLine(); // следующий элемент на той же линии

        // Кнопка сброса масштаба
        if (ImGui::Button("Scale", ImVec2(80, 30))) {
            zoomLevel = 1.0f;
            canvasView.setSize(window.getSize().x, window.getSize().y);
            canvasView.setCenter(window.getSize().x / 2, window.getSize().y / 2);
        }

        ImGui::SameLine(); // следующий элемент на той же линии


        ImGui::PopStyleVar(); // Возвращаем стандартный размер кнопок
        ImGui::End();
    }

    void drawLine(const sf::Vector2f& start, const sf::Vector2f& end) {
        float dx = end.x - start.x;
        float dy = end.y - start.y;
        float length = std::sqrt(dx * dx + dy * dy);

        for (float i = 0; i <= length; i += 1.0f) {
            float x = start.x + (dx * i / length);
            float y = start.y + (dy * i / length);

            for (int j = -brushSize; j <= brushSize; ++j) {
                for (int k = -brushSize; k <= brushSize; ++k) {
                    if (j * j + k * k <= brushSize * brushSize) {
                        // Проверяем границы холста
                        if (x + j >= 0 && y + k >= 0 &&
                            x + j < canvasSize.x &&
                            y + k < canvasSize.y) {
                            canvasImage.setPixel(
                                static_cast<int>(x + j),
                                static_cast<int>(y + k),
                                isErasing ? sf::Color::White : currentColor
                            );
                        }
                    }
                }
            }
        }
        canvasTexture.loadFromImage(canvasImage);
    }

    // =]
    void drawConnectLine(const sf::Vector2f& position) {
        int size = brushSize * 2;
        for (int x = -size; x <= size; ++x) {
            for (int y = -size; y <= size; ++y) {
                if (std::abs(x) + std::abs(y) <= size) {
                    int px = static_cast<int>(position.x + x);
                    int py = static_cast<int>(position.y + y);
                    if (px >= 0 && py >= 0 && px < canvasSize.x && py < canvasSize.y) {
                        canvasImage.setPixel(px, py, isErasing ? sf::Color::White : currentColor);
                    }
                }
            }
        }
        canvasTexture.loadFromImage(canvasImage);
    }

    void drawRectangle(const sf::Vector2f& start, const sf::Vector2f& end) {
        sf::Vector2f size = end - start;
        for (int x = 0; x <= std::abs(size.x); ++x) {
            for (int y = 0; y <= std::abs(size.y); ++y) {
                int px = static_cast<int>(start.x + (size.x > 0 ? x : -x));
                int py = static_cast<int>(start.y + (size.y > 0 ? y : -y));
                if (px >= 0 && py >= 0 && px < canvasSize.x && py < canvasSize.y) {
                    canvasImage.setPixel(px, py, isErasing ? sf::Color::White : currentColor);
                }
            }
        }
        canvasTexture.loadFromImage(canvasImage);
    }

    void drawCircle(const sf::Vector2f& center, const sf::Vector2f& edge) {
        float radius = std::sqrt(
            (edge.x - center.x) * (edge.x - center.x) +
            (edge.y - center.y) * (edge.y - center.y)
        );

        for (int x = -radius; x <= radius; ++x) {
            for (int y = -radius; y <= radius; ++y) {
                if (x * x + y * y <= radius * radius) {
                    int px = static_cast<int>(center.x + x);
                    int py = static_cast<int>(center.y + y);
                    if (px >= 0 && py >= 0 && px < canvasSize.x && py < canvasSize.y) {
                        canvasImage.setPixel(px, py, isErasing ? sf::Color::White : currentColor);
                    }
                }
            }
        }
        canvasTexture.loadFromImage(canvasImage);
    }

    void drawFill(sf::Vector2i startPos, sf::Color newColor) {
        sf::Color targetColor = canvasImage.getPixel(startPos.x, startPos.y);
        if (targetColor == newColor) return;

        std::queue<sf::Vector2i> pixels;
        pixels.push(startPos);

        while (!pixels.empty()) {
            sf::Vector2i current = pixels.front();
            pixels.pop();

            if (current.x < 0 || current.x >= canvasSize.x ||
                current.y < 0 || current.y >= canvasSize.y)
                continue;

            if (canvasImage.getPixel(current.x, current.y) != targetColor)
                continue;

            canvasImage.setPixel(current.x, current.y, newColor);

            pixels.push(sf::Vector2i(current.x + 1, current.y));
            pixels.push(sf::Vector2i(current.x - 1, current.y));
            pixels.push(sf::Vector2i(current.x, current.y + 1));
            pixels.push(sf::Vector2i(current.x, current.y - 1));
        }
    }

};