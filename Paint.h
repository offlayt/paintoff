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
    sf::Vector2u canvasSize; // ����� ���� ��� �������� ������� ������
    sf::Texture PaintButtonTexture; // SFML-�������� ������
    //ImTextureID iconTextureID; // ImGui-����������� ��������
    bool isFilling = false; // ���� ����������� "�������"

    bool isSelecting = false;               // ���� ������ ���������
    sf::Vector2f selectionStart;            // ��������� ������� ���������
    sf::Vector2f selectionEnd;              // �������� ������� ���������
    sf::Image selectionBuffer;              // ����� ��� �������� ���������� ��������
    sf::Vector2f selectionOffset;           // �������� ��������� ��� �����������
    bool isMovingSelection = false;         // ���� ����������� ���������

    FigureType currentFigure = FigureType::Rectangle; // ������� ������
    sf::Vector2f figureStartPos;             // ��������� ������� ��������� ������
    bool isDrawingFigure = false;            // ���� ��������� ������

    sf::Texture ColorIconTexture;



public:
    Paint() {
        // �������� ���������� ������
        sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
        // ������������� ������ ���� (80% �� ���������� ������)
        unsigned int windowWidth = desktopMode.width * 0.8;
        unsigned int windowHeight = desktopMode.height * 0.8;

        window.create(sf::VideoMode(windowWidth, windowHeight), "Paint", sf::Style::Default);

        // ��������� ������
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

                // ��������� ��������� �������� ����
                if (event.type == sf::Event::Resized) {
                    sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                    window.setView(sf::View(visibleArea));
                    canvasView.setSize(event.size.width * zoomLevel, event.size.height * zoomLevel);
                    canvasView.setCenter(event.size.width / 2, event.size.height / 2);
                }

                // ������ ��������� (����� ������ ����)
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    isMousePressed = true;
                    lastMousePos = sf::Mouse::getPosition(window);
                }

                // ���������� ��������� (����� ������ ����)
                if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                    isMousePressed = false;
                }

                // ��������� �� ������ (������ ���� ������ �� ��� ����������� � �� � ������ ���������)
                if (isMousePressed && !ImGui::IsAnyItemHovered() && !isSelecting) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, canvasView); // ����������� ���������� � ������ ��������
                    sf::Vector2f lastWorldPos = window.mapPixelToCoords(lastMousePos, canvasView);

                    // ������ ����� ����� ���������� � ������� ��������� ����
                    drawLine(lastWorldPos, worldPos);

                    lastMousePos = mousePos; // ��������� ���������� ������� ����
                }

                // ������ ��������� ������ (����� ������ ����)
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

                // ���������� ��������� ������
                if (event.type == sf::Event::MouseButtonReleased &&
                    event.mouseButton.button == sf::Mouse::Right &&
                    isDrawingFigure)
                {
                    isDrawingFigure = false;
                    sf::Vector2f endPos = window.mapPixelToCoords(
                        sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
                    );

                    // ������ ������
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


                // ������ ���������
                if (event.type == sf::Event::MouseButtonPressed &&
                    event.mouseButton.button == sf::Mouse::Left &&
                    !ImGui::IsAnyItemHovered())
                {
                    if (isSelecting) {
                        // ������ ��������� �������
                        selectionStart = window.mapPixelToCoords(
                            sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
                        );
                    }
                    else if (isMovingSelection) {
                        // ������ ����������� ���������
                        isMousePressed = false;
                        selectionOffset = window.mapPixelToCoords(
                            sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
                        ) - selectionStart;
                    }
                }

                // ���������� ��������� (���������� ����� ������ ����)
                if (event.type == sf::Event::MouseButtonReleased &&
                    event.mouseButton.button == sf::Mouse::Left)
                {
                    if (isSelecting) {
                        selectionEnd = window.mapPixelToCoords(
                            sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
                        );

                        // ����������� ���������� �������� � �����
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
                        isMovingSelection = true; // ����� �����������
                    }
                    else if (isMovingSelection) {
                        // ������� ���������� �������� �� ����� �����
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

                        isSelecting = true; // ������ ������� ����� ���������
                        isMovingSelection = false; // ��������� �����������
                        canvasTexture.loadFromImage(canvasImage);
                    }
                }

                // ����������� ���������
                if (isMovingSelection && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    selectionStart = mousePos - selectionOffset;
                }


                // ��������������� ������� ����
                if (event.type == sf::Event::MouseWheelScrolled) {
                    if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                        // ��������� ������ ��������������� � �������������
                        zoomLevel *= (event.mouseWheelScroll.delta > 0) ? 1.1f : 0.9f;
                        zoomLevel = std::max(0.1f, std::min(zoomLevel, 5.0f)); // ����������� �� 0.1x �� 5x
                        canvasView.setSize(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel);
                        canvasView.setCenter(window.getSize().x / 2, window.getSize().y / 2);
                    }
                }

                // ������ ����������� ������ ������� ����
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Middle) {
                    lastMousePos = sf::Mouse::getPosition(window);
                }
            }

            // ����������� ������ ��� ������� ������� ������ ����
            if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
                sf::Vector2i mouseDelta = sf::Mouse::getPosition(window) - lastMousePos;
                canvasView.move(-mouseDelta.x * zoomLevel, -mouseDelta.y * zoomLevel);
                lastMousePos = sf::Mouse::getPosition(window);
            }

            ImGui::SFML::Update(window, deltaClock.restart());

            // ��������� ����������
            drawGUI();

            // ������� (���� ���������� ������� � ������ �� ��� �����������)
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

            // ���������
            window.clear();
            window.setView(canvasView);
            window.draw(canvasSprite);


            // ��������� �������������� ���������
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

            // ��������� ������������ �������
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
        // ������� � ������ ���� ������������ (������� ������)
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(window.getSize().x, 67)); // ������ ������ � 60 ��������
        ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // ������ ������
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));

        // ������ � ������� ��� ����������� ����
        //ImTextureID textureID = (ImTextureID)(void*)PaintButtonTexture.getNativeHandle();
        //if (ImGui::ImageButton(textureID, ImVec2(30, 30))) { // ������ � �������
        //    ImGui::OpenPopup("ColorMenu"); // ��������� ���������� ����
        //}

       /* static sf::Texture ColorIconTexture;
        if (!ColorIconTexture.loadFromFile("ColorIcon.png")) {
            std::cerr << "Failed to load Color icon!\n";
        }*/

        // ImTextureID ColorIcon = (ImTextureID)(void*)(uintptr_t)ColorIconTexture.getNativeHandle();

        // ������ ��� ����������� ���� ������
        if (ImGui::Button("C", ImVec2(30, 30))) {
            ImGui::OpenPopup("ColorMenu"); // ��������� ���������� ����
        }

        // ���������� ���� ��� ������ �����
        if (ImGui::BeginPopup("ColorMenu", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            if (ImGui::ColorEdit3("Color", color)) {
                currentColor.r = static_cast<sf::Uint8>(color[0] * 255);
                currentColor.g = static_cast<sf::Uint8>(color[1] * 255);
                currentColor.b = static_cast<sf::Uint8>(color[2] * 255);
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine(); // ��������� ������� �� ��� �� �����

        // ������ ��� ����������� ���� �����
        if (ImGui::Button("F", ImVec2(30, 30))) {
            ImGui::OpenPopup("FigureMenu"); // ��������� ���������� ����
        }

        // ���������� ���� ��� ������ figure
        if (ImGui::BeginPopup("FigureMenu", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            if (ImGui::Button("Circle")) {
                currentFigure = FigureType::Circle;
                ImGui::CloseCurrentPopup(); // ��������� ���� ����� ������
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

        ImGui::SameLine(); // ��������� ������� �� ��� �� �����

        if (ImGui::Button("Fill", ImVec2(80, 30))) {
            isFilling = !isFilling;
            isMousePressed = false;
        }
        ImGui::SameLine(); // ��������� ������� �� ��� �� �����

        // ������ ���������
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
        ImGui::SameLine(); // ��������� ������� �� ��� �� �����

        // ������ �����
        ImGui::SetNextItemWidth(100); // ������ ��������
        ImGui::SliderInt("Brush", &brushSize, 1, 50);

        ImGui::SameLine(); // ��������� ������� �� ��� �� �����

        // ������
        ImGui::Checkbox("Eraser", &isErasing);

        ImGui::SameLine(); // ��������� ������� �� ��� �� �����

        // ���� ��� ����� ����� �����
        //ImGui::SetNextItemWidth(200); // ������������� ������ ��������
        //ImGui::InputText("Filename", filename, sizeof(filename));

        // ������ ����������
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
                std::replace(path.begin(), path.end(), '\\', '/'); // �������� �������� ����� �� ������
                if (path.size() < 4 || path.substr(path.size() - 4) != ".png") {
                    path += ".png";
                }
                if (!canvasImage.saveToFile(path)) {
                    std::cerr << "Error saving!\n";
                }
            }
        }

        ImGui::SameLine(); // ��������� ������� �� ��� �� �����

        // ������ ������� ������
        if (ImGui::Button("Clear", ImVec2(80, 30))) {
            canvasImage.create(canvasSize.x, canvasSize.y, sf::Color::White);
            canvasTexture.loadFromImage(canvasImage);
        }

        ImGui::SameLine(); // ��������� ������� �� ��� �� �����

        // ������ ������ ��������
        if (ImGui::Button("Scale", ImVec2(80, 30))) {
            zoomLevel = 1.0f;
            canvasView.setSize(window.getSize().x, window.getSize().y);
            canvasView.setCenter(window.getSize().x / 2, window.getSize().y / 2);
        }

        ImGui::SameLine(); // ��������� ������� �� ��� �� �����


        ImGui::PopStyleVar(); // ���������� ����������� ������ ������
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
                        // ��������� ������� ������
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