//#include <SFML/Graphics.hpp>
//#include <imgui.h>
//#include <imgui-SFML.h>
//#include <string>
//#include <cmath>
//#include <iostream>
//#include <queue>
//#include "tinyfiledialogs/tinyfiledialogs.h"
//#include <windows.h> // Для Windows API
//
//#include "Paint.h"
//#include "Tools.h"
//#include "Canvas.h"
//#include "GUI.h"
//
//
//
//// Структура для хранения текстового блока
//struct TextBlock {
//    sf::Text text;
//    sf::Vector2f position;
//    std::string content;
//    bool isActive = false;
//    bool isMoving = false;
//    sf::Vector2f moveOffset;
//};
//
//
//class Paint {
//private:
//    sf::RenderWindow window;
//    sf::Texture canvasTexture;
//    sf::Sprite canvasSprite;
//    sf::Image canvasImage;
//    float color[3] = { 0.f, 0.f, 0.f };
//    sf::Color currentColor = sf::Color::Black;
//    int brushSize = 5;
//    bool isErasing = false;
//    char filename[128] = "drawing.png";
//    float zoomLevel = 1.0f;
//    sf::View canvasView;
//    sf::Vector2i lastMousePos;
//    bool isMousePressed = false;
//    sf::Vector2u canvasSize; // Новое поле для хранения размера холста
//    sf::Texture PaintButtonTexture; // SFML-текстура иконки
//    //ImTextureID iconTextureID; // ImGui-совместимая текстура
//
//    bool isFilling = false; // Флаг активации инструмента "Заливка"
//
//    bool isSelecting = false; // Флаг режима выделения
//    sf::Vector2i selectionStart; // Начальная точка выделения
//    sf::Vector2i selectionEnd; // Конечная точка выделения
//    sf::RectangleShape selectionRect; // Прямоугольник выделения
//    sf::Image selectedPixels; // Выделенные пиксели
//    bool hasSelection = false; // Флаг наличия выделенной области
//
//    bool isMoving = false; // Флаг перемещения выделенной области
//    bool isResizing = false; // Флаг изменения размера выделенной области
//    sf::Vector2i moveOffset; // Смещение при перемещении выделенной области
//    sf::Cursor cursor; // Курсор для изменения вида указателя
//
//
//    sf::Font font;
//    std::vector<TextBlock> textBlocks;
//    bool textMode = false;
//
//public:
//    Paint() {
//        // Получаем разрешение экрана
//        sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
//        // Устанавливаем размер окна (80% от разрешения экрана)
//        unsigned int windowWidth = desktopMode.width * 0.8;
//        unsigned int windowHeight = desktopMode.height * 0.8;
//
//        window.create(sf::VideoMode(windowWidth, windowHeight), "Paint", sf::Style::Default);
//
//        window.setFramerateLimit(60);
//        ImGui::SFML::Init(window);
//
//
//
//        // Инициализация прямоугольника выделения
//        selectionRect.setFillColor(sf::Color::Transparent);
//        selectionRect.setOutlineColor(sf::Color::Black);
//        selectionRect.setOutlineThickness(1);
//
//        // Загрузка текстуры для пунктирной границы
//        sf::Image dashedTexture;
//        dashedTexture.create(16, 16, sf::Color::Transparent);
//        for (int i = 0; i < 16; i += 2) {
//            dashedTexture.setPixel(i, 0, sf::Color::Black);
//            dashedTexture.setPixel(i, 15, sf::Color::Black);
//            dashedTexture.setPixel(0, i, sf::Color::Black);
//            dashedTexture.setPixel(15, i, sf::Color::Black);
//        }
//        sf::Texture texture;
//        texture.loadFromImage(dashedTexture);
//        selectionRect.setTexture(&texture);
//
//
//        // Загрузка шрифта
//        if (!font.loadFromFile("arial.ttf")) {
//            std::cerr << "Error loading font!\n";
//        }
//
//        // Загрузка текстуры иконки
//        /*if (!PaintButtonTexture.loadFromFile("painticon.png")) {
//            std::cerr << "Ошибка загрузки иконки!\n";
//        }*/
//
//        //// Инициализация прямоугольника выделения
//        //selectionRect.setFillColor(sf::Color::Transparent);
//        //selectionRect.setOutlineColor(sf::Color::Green);
//        //selectionRect.setOutlineThickness(1);
//
//
//        // Преобразование SFML-текстуры в ImGui-совместимый формат
//        //iconTextureID = ImGui::SFML::UpdateTexture(iconTexture);
//
//        // Инициализация холста размером с окно
//        canvasSize = window.getSize();
//        canvasImage.create(canvasSize.x, canvasSize.y, sf::Color::White);
//        canvasTexture.loadFromImage(canvasImage);
//        canvasSprite.setTexture(canvasTexture);
//
//        canvasView = window.getDefaultView();
//    }
//
//
//
//    void run() {
//        sf::Clock deltaClock;
//        while (window.isOpen()) {
//            sf::Event event;
//            while (window.pollEvent(event)) {
//                ImGui::SFML::ProcessEvent(event);
//
//                if (event.type == sf::Event::Closed) {
//                    window.close();
//                }
//
//                // Обработка изменения размеров окна
//                if (event.type == sf::Event::Resized) {
//                    sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
//                    window.setView(sf::View(visibleArea));
//                    canvasView.setSize(event.size.width * zoomLevel, event.size.height * zoomLevel);
//                    canvasView.setCenter(event.size.width / 2, event.size.height / 2);
//                }
//
//                // Начало рисования (левая кнопка мыши)
//                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
//                    isMousePressed = true;
//                    lastMousePos = sf::Mouse::getPosition(window);
//                }
//
//                // Завершение рисования (левая кнопка мыши)
//                if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
//                    isMousePressed = false;
//                }
//
//                // Рисование на холсте (только если курсор не над интерфейсом и не в режиме выделения)
//                if (isMousePressed && !ImGui::IsAnyItemHovered() && !isSelecting) {
//                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
//                    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, canvasView); // Преобразуем координаты с учетом масштаба
//                    sf::Vector2f lastWorldPos = window.mapPixelToCoords(lastMousePos, canvasView);
//
//                    // Рисуем линию между предыдущей и текущей позициями мыши
//                    drawLine(lastWorldPos, worldPos);
//
//                    lastMousePos = mousePos; // Обновляем предыдущую позицию мыши
//                }
//
//
//                // Начало перемещения холста (средняя кнопка мыши)
//                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Middle) {
//                    lastMousePos = sf::Mouse::getPosition(window);
//                }
//                // Масштабирование колесом мыши /notwork
//                if (event.type == sf::Event::MouseWheelScrolled) {
//                    if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
//                        // Изменение уровня масштабирования с ограничениями
//                        zoomLevel *= (event.mouseWheelScroll.delta > 0) ? 1.1f : 0.9f;
//                        zoomLevel = std::max(0.1f, std::min(zoomLevel, 5.0f)); // Ограничение от 0.1x до 5x
//                        canvasView.setSize(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel);
//                        canvasView.setCenter(window.getSize().x / 2, window.getSize().y / 2);
//                    }
//                }
//            }
//
//
//
//
//
//
//            // Начало выделения (левая кнопка мыши)
//            if (isSelecting && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
//                selectionStart = sf::Mouse::getPosition(window);
//                hasSelection = false; // Сбрасываем флаг выделения
//            }
//
//            // Завершение выделения (левая кнопка мыши)
//            if (isSelecting && event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
//                selectionEnd = sf::Mouse::getPosition(window);
//                hasSelection = true;
//
//                // Копируем выделенные пиксели
//                sf::IntRect selectionArea(
//                    std::min(selectionStart.x, selectionEnd.x),
//                    std::min(selectionStart.y, selectionEnd.y),
//                    std::abs(selectionEnd.x - selectionStart.x),
//                    std::abs(selectionEnd.y - selectionStart.y)
//                );
//                selectedPixels.create(selectionArea.width, selectionArea.height, sf::Color::Transparent);
//                for (int x = 0; x < selectionArea.width; ++x) {
//                    for (int y = 0; y < selectionArea.height; ++y) {
//                        selectedPixels.setPixel(x, y, canvasImage.getPixel(selectionArea.left + x, selectionArea.top + y));
//                    }
//                }
//            }
//
//            // Перемещение выделенной области (левая кнопка мыши)
//            if (hasSelection && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
//                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
//                if (selectionRect.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
//                    isMoving = true;
//                    moveOffset = mousePos - sf::Vector2i(selectionRect.getPosition());
//                }
//            }
//
//            // Завершение перемещения (левая кнопка мыши)
//            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
//                isMoving = false;
//            }
//
//            // Удаление выделенной области (клавиша Delete)
//            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Delete) {
//                if (hasSelection) {
//                    // Удаляем выделенную область
//                    for (int x = 0; x < selectedPixels.getSize().x; ++x) {
//                        for (int y = 0; y < selectedPixels.getSize().y; ++y) {
//                            sf::Vector2i destPos(selectionStart.x + x, selectionStart.y + y);
//                            if (destPos.x >= 0 && destPos.y >= 0 && destPos.x < canvasSize.x && destPos.y < canvasSize.y) {
//                                canvasImage.setPixel(destPos.x, destPos.y, sf::Color::Transparent); // Удаляем пиксели
//                            }
//                        }
//                    }
//                    canvasTexture.loadFromImage(canvasImage);
//                    hasSelection = false; // Сбрасываем флаг выделения
//                }
//            }
//
//
//
//            ImGui::SFML::Update(window, deltaClock.restart());
//            drawGUI();
//
//
//            // Отрисовка выделения 1
//            /*if (isSelecting && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
//                selectionEnd = sf::Mouse::getPosition(window);
//                selectionRect.setPosition(
//                    std::min(selectionStart.x, selectionEnd.x),
//                    std::min(selectionStart.y, selectionEnd.y)
//                );
//                selectionRect.setSize(sf::Vector2f(
//                    std::abs(selectionEnd.x - selectionStart.x),
//                    std::abs(selectionEnd.y - selectionStart.y)
//                ));
//            }*/
//
//            // Отрисовка выделения 2
//            if (isSelecting) {
//                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
//                    selectionEnd = sf::Mouse::getPosition(window);
//                    selectionRect.setPosition(
//                        std::min(selectionStart.x, selectionEnd.x),
//                        std::min(selectionStart.y, selectionEnd.y)
//                    );
//                    selectionRect.setSize(sf::Vector2f(
//                        std::abs(selectionEnd.x - selectionStart.x),
//                        std::abs(selectionEnd.y - selectionStart.y)
//                    ));
//                }
//                window.draw(selectionRect); // Рисуем прямоугольник выделения всегда в режиме выделения
//            }
//
//            // Перемещение выделенной области 1
//            /*if (isMoving) {
//                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
//                selectionRect.setPosition(mousePos.x - moveOffset.x, mousePos.y - moveOffset.y);
//            }*/
//
//            // Перемещение выделенной области 2
//            if (isMoving && event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
//                isMoving = false;
//
//                // Обновляем canvasImage с учетом нового положения выделенной области
//                for (int x = 0; x < selectedPixels.getSize().x; ++x) {
//                    for (int y = 0; y < selectedPixels.getSize().y; ++y) {
//                        sf::Vector2i destPos(selectionRect.getPosition().x + x, selectionRect.getPosition().y + y);
//                        if (destPos.x >= 0 && destPos.y >= 0 && destPos.x < canvasSize.x && destPos.y < canvasSize.y) {
//                            canvasImage.setPixel(destPos.x, destPos.y, selectedPixels.getPixel(x, y));
//                        }
//                    }
//                }
//                canvasTexture.loadFromImage(canvasImage);
//            }
//
//
//            // заливка (если инструмент активен и курсор не над интерфейсом)
//            if (isFilling && sf::Mouse::isButtonPressed(sf::Mouse::Left) && !ImGui::IsAnyItemHovered()) {
//                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
//                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, canvasView);
//
//                if (worldPos.x >= 0 && worldPos.y >= 0 &&
//                    worldPos.x < canvasSize.x && worldPos.y < canvasSize.y)
//                {
//                    drawFill(sf::Vector2i(worldPos), isErasing ? sf::Color::White : currentColor);
//                    canvasTexture.loadFromImage(canvasImage);
//                }
//            }
//
//            // Отрисовка
//            window.clear();
//            window.setView(canvasView);
//            window.draw(canvasSprite);
//
//            if (isSelecting && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
//                window.draw(selectionRect); // Рисуем прямоугольник выделения
//            }
//
//            window.setView(window.getDefaultView());
//            ImGui::SFML::Render(window);
//            window.display();
//        }
//        ImGui::SFML::Shutdown();
//    }
//
//    void drawLine(const sf::Vector2f& start, const sf::Vector2f& end) {
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
//                        // Проверяем границы холста
//                        if (x + j >= 0 && y + k >= 0 &&
//                            x + j < canvasSize.x &&
//                            y + k < canvasSize.y) {
//                            canvasImage.setPixel(
//                                static_cast<int>(x + j),
//                                static_cast<int>(y + k),
//                                isErasing ? sf::Color::White : currentColor
//                            );
//                        }
//                    }
//                }
//            }
//        }
//        canvasTexture.loadFromImage(canvasImage);
//    }
//
//    void handleTextInput(const sf::Event& event) {
//        if (textMode && event.type == sf::Event::TextEntered) {
//            for (auto& block : textBlocks) {
//                if (block.isActive) {
//                    if (event.text.unicode == 8 && !block.content.empty()) { // Backspace
//                        block.content.pop_back();
//                    }
//                    else if (event.text.unicode < 128) {
//                        block.content += static_cast<char>(event.text.unicode);
//                    }
//                    updateText(block);
//                }
//            }
//        }
//    }
//
//    void updateText(TextBlock& block) {
//        block.text.setString(block.content);
//        block.text.setFont(font);
//        block.text.setCharacterSize(24);
//        block.text.setFillColor(sf::Color::Black);
//        block.text.setPosition(block.position);
//    }
//
//    void drawGUI() {
//        // позицию и размер окна инструментов (верхняя панель)
//        ImGui::SetNextWindowPos(ImVec2(0, 0));
//        ImGui::SetNextWindowSize(ImVec2(window.getSize().x, 60)); // Высота панели — 60 пикселей
//        ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
//
//        // размер кнопок
//        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
//
//
//
//        ImGui::SameLine();
//        if (ImGui::Button("Text", ImVec2(80, 0))) {
//            textMode = !textMode;
//            if (textMode) {
//                // Создаем новый текстовый блок
//                TextBlock newBlock;
//                newBlock.text.setFont(font);
//                newBlock.text.setCharacterSize(24);
//                newBlock.text.setFillColor(sf::Color::Black);
//                newBlock.position = sf::Vector2f(100, 100); // Начальная позиция
//                newBlock.isActive = true;
//                textBlocks.push_back(newBlock);
//            }
//            else {
//                // Деактивируем все блоки
//                for (auto& block : textBlocks) block.isActive = false;
//            }
//        }
//
//        // Кнопка с иконкой для выпадающего меню
//        //ImTextureID textureID = (ImTextureID)(void*)PaintButtonTexture.getNativeHandle();
//        //if (ImGui::ImageButton(textureID, ImVec2(30, 30))) { // Кнопка с иконкой
//        //    ImGui::OpenPopup("ColorMenu"); // Открываем выпадающее меню
//        //}
//
//
//        // кнопка для выпадающего меню цветов
//        if (ImGui::Button("C", ImVec2(30, 30))) {
//            ImGui::OpenPopup("ColorMenu"); // Открываем выпадающее меню
//        }
//
//        // Выпадающее меню для выбора цвета
//        if (ImGui::BeginPopup("ColorMenu", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
//            if (ImGui::ColorEdit3("Color", color)) {
//                currentColor.r = static_cast<sf::Uint8>(color[0] * 255);
//                currentColor.g = static_cast<sf::Uint8>(color[1] * 255);
//                currentColor.b = static_cast<sf::Uint8>(color[2] * 255);
//            }
//            ImGui::EndPopup();
//        }
//
//        ImGui::SameLine(); // следующий элемент на той же линии
//
//        // Размер кисти
//        ImGui::SetNextItemWidth(100); // ширина элемента
//        ImGui::SliderInt("Brush", &brushSize, 1, 50);
//
//        ImGui::SameLine(); // следующий элемент на той же линии
//
//        // Ластик
//        ImGui::Checkbox("Eraser", &isErasing);
//
//        ImGui::SameLine(); // следующий элемент на той же линии
//
//        // Поле для ввода имени файла
//        //ImGui::SetNextItemWidth(200); // Фиксированная ширина элемента
//        //ImGui::InputText("Filename", filename, sizeof(filename));
//
//        ImGui::SameLine(); // следующий элемент на той же линии
//
//        // Кнопка сохранения
//        if (ImGui::Button("Save", ImVec2(80, 0))) {
//            const char* filters[] = { "*.png" };
//            const char* savePath = tinyfd_saveFileDialog(
//                "Save image",
//                filename,
//                1,
//                filters,
//                "PNG file (*.png)"
//            );
//
//            if (savePath) {
//                std::string path = savePath;
//                std::replace(path.begin(), path.end(), '\\', '/'); // Заменяем обратные слэши на прямые
//                if (path.size() < 4 || path.substr(path.size() - 4) != ".png") {
//                    path += ".png";
//                }
//                if (!canvasImage.saveToFile(path)) {
//                    std::cerr << "Error saving!\n";
//                }
//            }
//        }
//
//        ImGui::SameLine(); // следующий элемент на той же линии
//
//        // Кнопка очистки холста
//        if (ImGui::Button("Clear", ImVec2(80, 0))) {
//            canvasImage.create(canvasSize.x, canvasSize.y, sf::Color::White);
//            canvasTexture.loadFromImage(canvasImage);
//        }
//
//        ImGui::SameLine(); // следующий элемент на той же линии
//
//        // Кнопка сброса масштаба
//        if (ImGui::Button("Scale", ImVec2(80, 0))) {
//            zoomLevel = 1.0f;
//            canvasView.setSize(window.getSize().x, window.getSize().y);
//            canvasView.setCenter(window.getSize().x / 2, window.getSize().y / 2);
//        }
//
//        ImGui::SameLine();
//        if (ImGui::Button("Fill", ImVec2(80, 0))) {
//            isFilling = !isFilling;
//            isMousePressed = false;
//        }
//
//        ImGui::SameLine(); // следующий элемент на той же линии
//
//        // Кнопка выделения
//        if (ImGui::Button("Select", ImVec2(80, 0))) {
//            isSelecting = !isSelecting; // режим
//            isMousePressed = false;
//            isFilling = false;
//        }
//
//        ImGui::SameLine(); // следующий элемент на той же линии
//
//        //// Копируем выделенную область в буфер обмена
//        //if (ImGui::Button("Копировать", ImVec2(80, 0))) {
//        //    if (hasSelection) {
//        //        copyToClipboard(selectedPixels);
//        //    }
//        //}
//
//        ImGui::PopStyleVar(); // Возвращаем стандартный размер кнопок
//        ImGui::End();
//    }
//
//    void drawFill(sf::Vector2i startPos, sf::Color newColor) {
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
//            if (current.x < 0 || current.x >= canvasSize.x ||
//                current.y < 0 || current.y >= canvasSize.y)
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
//    }
//
//    void handleTextMovement(const sf::Event& event) {
//        if (textMode && event.type == sf::Event::MouseButtonPressed) {
//            if (event.mouseButton.button == sf::Mouse::Left) {
//                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
//                for (auto& block : textBlocks) {
//                    if (block.text.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
//                        block.isMoving = true;
//                        block.moveOffset = block.text.getPosition() - sf::Vector2f(mousePos);
//                        block.isActive = true;
//                    }
//                    else {
//                        block.isActive = false;
//                    }
//                }
//            }
//        }
//
//        if (textMode && event.type == sf::Event::MouseMoved) {
//            for (auto& block : textBlocks) {
//                if (block.isMoving) {
//                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
//                    block.position = sf::Vector2f(mousePos) + block.moveOffset;
//                    block.text.setPosition(block.position);
//                }
//            }
//        }
//
//        if (event.type == sf::Event::MouseButtonReleased) {
//            for (auto& block : textBlocks) block.isMoving = false;
//        }
//    }
//
//    void drawTextBlocks() {
//        for (const auto& block : textBlocks) {
//            window.draw(block.text);
//            if (block.isActive) {
//                // Рисуем рамку вокруг активного блока
//                sf::RectangleShape border;
//                border.setSize(sf::Vector2f(block.text.getGlobalBounds().width + 10,
//                    block.text.getGlobalBounds().height + 10));
//                border.setPosition(block.position - sf::Vector2f(5, 5));
//                border.setFillColor(sf::Color::Transparent);
//                border.setOutlineColor(sf::Color::Blue);
//                border.setOutlineThickness(1);
//                window.draw(border);
//            }
//        }
//    }
//
//
//};
//
//void copyToClipboard(const sf::Image& image) {
//    int width = image.getSize().x;
//    int height = image.getSize().y;
//    std::vector<sf::Uint8> pixels(width * height * 4); // RGBA данные
//
//    for (int y = 0; y < height; ++y) {
//        for (int x = 0; x < width; ++x) {
//            sf::Color color = image.getPixel(x, y);
//            pixels[(y * width + x) * 4 + 0] = color.r; // R
//            pixels[(y * width + x) * 4 + 1] = color.g; // G
//            pixels[(y * width + x) * 4 + 2] = color.b; // B
//            pixels[(y * width + x) * 4 + 3] = color.a; // A
//        }
//    }
//
//    // Открываем буфер обмена
//    if (OpenClipboard(nullptr)) {
//        EmptyClipboard();
//
//        // Создаем DIB в буфере обмена
//        HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + width * height * 4);
//        if (hMemory) {
//            BITMAPINFOHEADER* pHeader = static_cast<BITMAPINFOHEADER*>(GlobalLock(hMemory));
//            pHeader->biSize = sizeof(BITMAPINFOHEADER);
//            pHeader->biWidth = width;
//            pHeader->biHeight = -height; // Отрицательная высота для верхнего левого угла
//            pHeader->biPlanes = 1;
//            pHeader->biBitCount = 32;
//            pHeader->biCompression = BI_RGB;
//
//            // Копируем пиксели
//            sf::Uint8* pPixels = reinterpret_cast<sf::Uint8*>(pHeader + 1);
//            std::memcpy(pPixels, pixels.data(), width * height * 4);
//
//            GlobalUnlock(hMemory);
//            SetClipboardData(CF_DIB, hMemory);
//        }
//
//        CloseClipboard();
//    }
//}
//
//
//int main() {
//    setlocale(LC_ALL, "Russian");
//
//    Paint app;
//    app.run();
//    return 0;
//}