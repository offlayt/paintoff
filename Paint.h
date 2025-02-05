//#pragma once
//
//#include <SFML/Graphics.hpp>
//#include <imgui.h>
//#include <imgui-SFML.h>
//
//#include "Tools.h"
//#include "Canvas.h"
//#include "GUI.h"
//
//class Paint {
//private:
//    sf::RenderWindow window;
//    Canvas canvas;
//    Tools tools;
//    GUI gui;
//
//public:
//    Paint()
//        : window(sf::VideoMode(800, 600), "Paint"),
//        canvas(window, sf::Vector2u(800, 600)),
//        tools(canvas),
//        gui(window, tools) {
//        window.setFramerateLimit(60);
//        ImGui::SFML::Init(window);
//    }
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
//                // Обработка событий мыши
//                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
//                    // Логика рисования
//                }
//            }
//
//            ImGui::SFML::Update(window, deltaClock.restart());
//            gui.draw();
//
//            window.clear();
//            canvas.draw();
//            ImGui::SFML::Render(window);
//            window.display();
//        }
//        ImGui::SFML::Shutdown();
//    }
//};