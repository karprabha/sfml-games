#include "imgui.h"
#include "imgui-SFML.h"

#include  <SFML/Graphics.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

constexpr float MIN_RADIUS = 0.0f;
constexpr float MAX_RADIUS = 300.f;

constexpr int MIN_SEGMENT = 3;
constexpr int MAX_SEGMENT = 64;

constexpr int MAX_TEXT_SIZE = 255;

class CircleWithText {
    void updateTextPosition() {
        const float updatedTextOriginX = m_text.getLocalBounds().left + m_text.getLocalBounds().width / 2.0f;
        const float updatedTextOriginY = m_text.getLocalBounds().top + m_text.getLocalBounds().height / 2.0f;

        const float circleCenterX = m_circle.getPosition().x + m_circle.getRadius();
        const float circleCenterY = m_circle.getPosition().y + m_circle.getRadius();

        m_text.setOrigin(updatedTextOriginX, updatedTextOriginY);
        m_text.setPosition(circleCenterX, circleCenterY);
    }

    void updateCirclePosition() {
        sf::Color updatedShapeColor(m_shapeColor[0] * 255, m_shapeColor[1] * 255, m_shapeColor[2] * 255);

        float updatedCirclePosX = m_circle.getPosition().x + m_speedX;
        float updatedCirclePosY = m_circle.getPosition().y + m_speedY;

        if (updatedCirclePosX > m_wWidth - 2 * m_circle.getRadius() || updatedCirclePosX < 0.0f) {
            m_speedX *= -1;
            updatedCirclePosX = std::max(updatedCirclePosX, 0.0f);
            updatedCirclePosX = std::min(updatedCirclePosX, m_wWidth - 2 * m_circle.getRadius());
        }

        if (updatedCirclePosY > m_wHeight - 2 * m_circle.getRadius() || updatedCirclePosY < 0.0f) {
            m_speedY *= -1;
            updatedCirclePosY = std::max(updatedCirclePosY, 0.0f);
            updatedCirclePosY = std::min(updatedCirclePosY, m_wHeight - 2 * m_circle.getRadius());
        }

        m_circle.setRadius(m_radius);
        m_circle.setPointCount(m_segment);
        m_circle.setFillColor(updatedShapeColor);
        m_circle.setPosition(updatedCirclePosX, updatedCirclePosY);
    }
public:
    sf::Text m_text;
    sf::CircleShape m_circle;

    bool m_showText;
    bool m_showShape;
    char m_displayString[MAX_TEXT_SIZE];

    int m_wWidth, m_wHeight;

    int m_segment;
    float m_radius, m_posX, m_posY, m_speedX, m_speedY;
    float m_shapeColor[3] = { 0 };

    CircleWithText(const std::string& shapeName, const float posX, const float posY, const float speedX, const float speedY, const sf::Font& myFont, const int fontSize, const int fontColor[],const int wWidth,const int wHeight, const int shapeColor[], const float radius, const int segment = 32)
        : m_radius(radius)
        , m_posX(posX)
        , m_posY(posY)
        , m_speedX(speedX)
        , m_speedY(speedY)
        , m_segment(segment)
        , m_wWidth(wWidth)
        , m_wHeight(wHeight)
        , m_showText(true)
        , m_showShape(true)
        , m_circle(sf::CircleShape(radius, segment))
        , m_text(sf::Text(shapeName, myFont, fontSize)) {

        // Initialize Circle
        m_circle.setPosition(posX, posY);

        m_shapeColor[0] = shapeColor[0] / 255.0f;
        m_shapeColor[1] = shapeColor[1] / 255.0f;
        m_shapeColor[2] = shapeColor[2] / 255.0f;

        // Initialize Text 
        sf::Color textColor(fontColor[0], fontColor[1], fontColor[2]);
        m_text.setFillColor(textColor);

        updateTextPosition();

        strncpy_s(m_displayString, shapeName.c_str(), MAX_TEXT_SIZE);
        m_displayString[MAX_TEXT_SIZE - 1] = '\0';
    }

    void resetPosition() {
        m_circle.setPosition(m_posX, m_posY);
    }

    void update() {
        updateCirclePosition();
        updateTextPosition();
    }
};


int main() {
    int wWidth = 1280;
    int wHeight = 720;
    int fontSize = 12;
    int fontColor[3] = { 255,255,255 };
    std::string configType;
    std::vector<CircleWithText> circles;

    sf::Font myFont;

    std::ifstream gameConfig("src/config/config.txt");
    while (gameConfig >> configType) {
        if (configType == "Window") {
            gameConfig >> wWidth >> wHeight;
        }
        else if (configType == "Font") {
            std::string fontFile;
            gameConfig >> fontFile >> fontSize >> fontColor[0] >> fontColor[1] >> fontColor[2];

            if (!myFont.loadFromFile(fontFile)) {
                std::cerr << "Couldn't load font!!\n";
                return EXIT_FAILURE;
            }

        }
        else if (configType == "Circle") {
            std::string shapeName;
            float posX, posY, speedX, speedY, radius;
            int shapeColor[3] = { 0 };

            gameConfig >> shapeName >> posX >> posY >> speedX >> speedY >> shapeColor[0] >> shapeColor[1] >> shapeColor[2] >> radius;

            circles.emplace_back(CircleWithText(shapeName, posX, posY, speedX, speedY, myFont, fontSize, fontColor, wWidth, wHeight, shapeColor, radius));
        }
    }

    sf::RenderWindow window(sf::VideoMode(wWidth, wHeight), "Shape Renderer");
    window.setFramerateLimit(60);

    ImGui::SFML::Init(window);
    sf::Clock deltaClock;


    ImGui::GetStyle().ScaleAllSizes(1.0f);

    std::string selectedCircleName = "Select";
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                std::cout << "Key pressed with code = " << event.key.code << "\n";

                if (event.key.code == sf::Keyboard::X) {
                    std::cout << "Pressed X\n";
                }
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Shape Editor");
        ImGui::Text("Edit Shape");

        if (ImGui::BeginCombo("Select Circle", selectedCircleName.c_str())) {
            for (auto& circle : circles) {
                bool is_selected = (selectedCircleName == circle.m_text.getString());
                if (ImGui::Selectable(circle.m_text.getString().toAnsiString().c_str(), is_selected)) {
                    selectedCircleName = circle.m_text.getString();
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        CircleWithText* selectedCircle = nullptr;
        for (auto& circle : circles) {
            if (circle.m_text.getString() == selectedCircleName) {
                selectedCircle = &circle;
                break;
            }
        }

        if (selectedCircle) {
            ImGui::Checkbox("Show Circle", &selectedCircle->m_showShape);
            ImGui::SameLine();
            ImGui::Checkbox("Show Text", &selectedCircle->m_showText);
            ImGui::SliderFloat("Radius", &selectedCircle->m_radius, MIN_RADIUS, MAX_RADIUS);
            ImGui::SliderInt("Sides", &selectedCircle->m_segment, MIN_SEGMENT, MAX_SEGMENT);
            ImGui::ColorEdit3("Color Circle", selectedCircle->m_shapeColor);
            ImGui::InputText("Text", selectedCircle->m_displayString, MAX_TEXT_SIZE);

            if (ImGui::Button("Set Text")) {
                selectedCircle->m_text.setString(selectedCircle->m_displayString);
            }

            ImGui::SameLine();
            if (ImGui::Button("Reset Circle")) {
                selectedCircle->resetPosition();
            }
        }


        ImGui::End();


        for (auto& circle : circles) {
            circle.update();
        }


        window.clear();

        for (const auto& circle : circles) {
            if (circle.m_showShape) {
                window.draw(circle.m_circle);
            }

            if (circle.m_showText) {
                window.draw(circle.m_text);
            }
        }

        ImGui::SFML::Render(window);
        window.display();
    }


    return 0;
}