//#pragma once
//
//#include <imgui.h>
//#include <imgui-SFML.h>
//#include <SFML/Graphics.hpp>
//#include <string>
//
//
//#include "Tools.h"
//#include "Paint.h"
//
//#include "Canvas.h"
//#include "GUI.h"
//
//
//
//class GUI {
//private:
//    sf::RenderWindow& window;
//    Tools& tools;
//    float color[3] = { 0.f, 0.f, 0.f };
//    char filename[128] = "drawing.png";
//
//public:
//    GUI(sf::RenderWindow& win, Tools& t) : window(win), tools(t) {}
//
//    void draw() {
//        ImGui::SetNextWindowPos(ImVec2(0, 0));
//        ImGui::SetNextWindowSize(ImVec2(window.getSize().x, 60));
//        ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
//
//        if (ImGui::Button("C", ImVec2(30, 30))) {
//            ImGui::OpenPopup("ColorMenu");
//        }
//
//        if (ImGui::BeginPopup("ColorMenu")) {
//            if (ImGui::ColorEdit3("Color", color)) {
//                tools.setColor(sf::Color(
//                    static_cast<sf::Uint8>(color[0] * 255),
//                    static_cast<sf::Uint8>(color[1] * 255),
//                    static_cast<sf::Uint8>(color[2] * 255)
//                ));
//            }
//            ImGui::EndPopup();
//        }
//
//        ImGui::SameLine();
//        ImGui::SliderInt("Brush", &brushSize, 1, 50);
//        tools.setBrushSize(brushSize);
//
//        ImGui::SameLine();
//        bool erasing = tools.isErasing();
//        if (ImGui::Checkbox("Eraser", &erasing)) {
//            tools.setErasing(erasing);
//        }
//
//        ImGui::SameLine();
//        if (ImGui::Button("Clear", ImVec2(80, 0))) {
//            canvas.clear();
//        }
//
//        ImGui::End();
//    }
//};