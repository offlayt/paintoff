#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <string>
#include <cmath>
#include <iostream>
#include <queue>
#include "tinyfiledialogs/tinyfiledialogs.h"

//#include "Paint.h"
//#include "Tools.h"
//#include "Canvas.h"
//#include "GUI.h"



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

    bool isSelecting = false;



public:
    Paint() {
        // �������� ���������� ������
        sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
        // ������������� ������ ���� (80% �� ���������� ������)
        unsigned int windowWidth = desktopMode.width * 0.8;
        unsigned int windowHeight = desktopMode.height * 0.8;

        window.create(sf::VideoMode(windowWidth, windowHeight), "Paint", sf::Style::Default);

        window.setFramerateLimit(60);
        ImGui::SFML::Init(window);

        canvasSize = window.getSize();
        canvasImage.create(canvasSize.x, canvasSize.y, sf::Color::White);
        canvasTexture.loadFromImage(canvasImage);
        canvasSprite.setTexture(canvasTexture);

        canvasView = window.getDefaultView();
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


                // ������ ����������� ������ (������� ������ ����)
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Middle) {
                    lastMousePos = sf::Mouse::getPosition(window);
                }
                // ��������������� ������� ���� /notwork
                if (event.type == sf::Event::MouseWheelScrolled) {
                    if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                        // ��������� ������ ��������������� � �������������
                        zoomLevel *= (event.mouseWheelScroll.delta > 0) ? 1.1f : 0.9f;
                        zoomLevel = std::max(0.1f, std::min(zoomLevel, 5.0f)); // ����������� �� 0.1x �� 5x
                        canvasView.setSize(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel);
                        canvasView.setCenter(window.getSize().x / 2, window.getSize().y / 2);
                    }
                }
            }





            ImGui::SFML::Update(window, deltaClock.restart());
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

            window.setView(window.getDefaultView());
            ImGui::SFML::Render(window);
            window.display();
        }
        ImGui::SFML::Shutdown();
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

    void drawGUI() {
        // ������� � ������ ���� ������������ (������� ������)
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(window.getSize().x, 60)); // ������ ������ � 60 ��������
        ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // ������ ������
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));



        // ������ � ������� ��� ����������� ����
        //ImTextureID textureID = (ImTextureID)(void*)PaintButtonTexture.getNativeHandle();
        //if (ImGui::ImageButton(textureID, ImVec2(30, 30))) { // ������ � �������
        //    ImGui::OpenPopup("ColorMenu"); // ��������� ���������� ����
        //}


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

        ImGui::SameLine(); // ��������� ������� �� ��� �� �����

        // ������ ����������
        if (ImGui::Button("Save", ImVec2(80, 0))) {
            const char* filters[] = { "*.png" };
            const char* savePath = tinyfd_saveFileDialog(
                "Save image",
                filename,
                1,
                filters,
                "PNG file (*.png)"
            );

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
        if (ImGui::Button("Clear", ImVec2(80, 0))) {
            canvasImage.create(canvasSize.x, canvasSize.y, sf::Color::White);
            canvasTexture.loadFromImage(canvasImage);
        }

        ImGui::SameLine(); // ��������� ������� �� ��� �� �����

        // ������ ������ ��������
        if (ImGui::Button("Scale", ImVec2(80, 0))) {
            zoomLevel = 1.0f;
            canvasView.setSize(window.getSize().x, window.getSize().y);
            canvasView.setCenter(window.getSize().x / 2, window.getSize().y / 2);
        }

        ImGui::SameLine();
        if (ImGui::Button("Fill", ImVec2(80, 0))) {
            isFilling = !isFilling;
            isMousePressed = false;
        }

        ImGui::SameLine(); // ��������� ������� �� ��� �� �����

        // ������ ���������
        if (ImGui::Button("Select", ImVec2(80, 0))) {
            isSelecting = !isSelecting; // �����
            isMousePressed = false;
            isFilling = false;
        }

        ImGui::SameLine(); // ��������� ������� �� ��� �� �����


        ImGui::PopStyleVar(); // ���������� ����������� ������ ������
        ImGui::End();
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


int main() {
    setlocale(LC_ALL, "Russian");

    Paint app;
    app.run();
    return 0;
}