#include "Button.h"
#include <cmath>

Button::Button(const std::string& buttonText, sf::Font& buttonFont, float x, float y, float width, float height) 
    : font(&buttonFont), isHovered(false), isPressed(false), hoverScale(1.0f), targetScale(1.0f), animationSpeed(8.0f) {
    
    originalPosition = sf::Vector2f(x, y);
    originalSize = sf::Vector2f(width, height);
    
    // Setup button shape with rounded corners effect
    shape.setSize(originalSize);
    shape.setPosition(originalPosition);
    shape.setOutlineThickness(3);
    
    // Setup shadow
    shadowShape.setSize(originalSize);
    shadowShape.setPosition(x + 5, y + 5);
    shadowColor = sf::Color(0, 0, 0, 80);
    shadowShape.setFillColor(shadowColor);
    
    // Setup button text
    text.setFont(buttonFont);
    text.setString(buttonText);
    text.setCharacterSize(28);
    text.setFillColor(sf::Color::White);
    text.setStyle(sf::Text::Bold);
    
    // Center text in button
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setPosition(
        x + (width - textBounds.width) / 2 - textBounds.left,
        y + (height - textBounds.height) / 2 - textBounds.top
    );
    
    // Default colors with gradients
    normalColor = sf::Color(60, 90, 140, 220);
    hoverColor = sf::Color(80, 110, 160, 240);
    pressColor = sf::Color(40, 70, 120, 255);
    shape.setFillColor(normalColor);
    shape.setOutlineColor(sf::Color(100, 150, 200, 180));
}

void Button::setColors(sf::Color normal, sf::Color hover, sf::Color press) {
    normalColor = normal;
    hoverColor = hover;
    pressColor = press;
    shape.setFillColor(normalColor);
}

void Button::setOnClick(std::function<void()> callback) {
    onClick = callback;
}

void Button::update(sf::Vector2i mousePos, float deltaTime) {
    bool wasHovered = isHovered;
    isHovered = contains(mousePos);
    
    // Set target scale based on hover state
    targetScale = isHovered ? 1.05f : 1.0f;
    
    // Smooth scale animation
    float scaleDiff = targetScale - hoverScale;
    hoverScale += scaleDiff * animationSpeed * deltaTime;
    
    // Apply scale
    sf::Vector2f scaledSize = originalSize;
    scaledSize.x *= hoverScale;
    scaledSize.y *= hoverScale;
    
    sf::Vector2f scaledPos = originalPosition;
    scaledPos.x -= (scaledSize.x - originalSize.x) / 2;
    scaledPos.y -= (scaledSize.y - originalSize.y) / 2;
    
    shape.setSize(scaledSize);
    shape.setPosition(scaledPos);
    
    // Update shadow position
    shadowShape.setSize(scaledSize);
    shadowShape.setPosition(scaledPos.x + 5, scaledPos.y + 5);
    
    // Update text position to stay centered
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setPosition(
        scaledPos.x + (scaledSize.x - textBounds.width) / 2 - textBounds.left,
        scaledPos.y + (scaledSize.y - textBounds.height) / 2 - textBounds.top
    );
    
    // Update visual state with smooth color transitions
    if (isPressed) {
        shape.setFillColor(pressColor);
        shape.setOutlineColor(sf::Color(150, 200, 255, 255));
    } else if (isHovered) {
        shape.setFillColor(hoverColor);
        shape.setOutlineColor(sf::Color(120, 170, 220, 220));
    } else {
        shape.setFillColor(normalColor);
        shape.setOutlineColor(sf::Color(100, 150, 200, 180));
    }
}

void Button::handleClick(sf::Vector2i mousePos) {
    if (contains(mousePos)) {
        isPressed = true;
        if (onClick) {
            onClick();
        }
    }
}

void Button::draw(sf::RenderWindow& window) {
    // Draw shadow first
    window.draw(shadowShape);
    // Draw button
    window.draw(shape);
    window.draw(text);
}

bool Button::contains(sf::Vector2i point) {
    return shape.getGlobalBounds().contains(static_cast<sf::Vector2f>(point));
}