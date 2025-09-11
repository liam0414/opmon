#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>

class Button {
private:
    sf::RectangleShape shape;
    sf::RectangleShape shadowShape;
    sf::Text text;
    sf::Font* font;
    bool isHovered;
    bool isPressed;
    std::function<void()> onClick;
    
    // Colors
    sf::Color normalColor;
    sf::Color hoverColor;
    sf::Color pressColor;
    sf::Color shadowColor;
    
    // Animation
    float hoverScale;
    float targetScale;
    float animationSpeed;
    
    // Position
    sf::Vector2f originalPosition;
    sf::Vector2f originalSize;

public:
    Button(const std::string& buttonText, sf::Font& buttonFont, float x, float y, float width, float height);
    
    void setColors(sf::Color normal, sf::Color hover, sf::Color press);
    void setOnClick(std::function<void()> callback);
    
    void update(sf::Vector2i mousePos, float deltaTime);
    void handleClick(sf::Vector2i mousePos);
    void draw(sf::RenderWindow& window);
    
    bool contains(sf::Vector2i point);
};