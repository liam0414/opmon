#include "Button.h"
#include "FontManager.h"

Button::Button() : Button("Button", FontManager::getInstance().getDefaultFont()) {}

Button::Button(const std::string& buttonText, const sf::Font& buttonFont) 
    : normalColor(sf::Color(100, 100, 100)), 
      hoverColor(sf::Color(120, 120, 120)),
      pressedColor(sf::Color(80, 80, 80)),
      disabledColor(sf::Color(60, 60, 60)),
      textColor(sf::Color::White),
      fontSize(16) {
    
    font = const_cast<sf::Font*>(&buttonFont);
    text.setFont(buttonFont);
    text.setString(buttonText);
    text.setCharacterSize(fontSize);
    text.setFillColor(textColor);
    
    background.setSize(sf::Vector2f(bounds.width, bounds.height));
    background.setPosition(bounds.left, bounds.top);
    
    updateAppearance();
}

void Button::setText(const std::string& str) {
    text.setString(str);
    updateAppearance();
}

std::string Button::getText() const {
    return text.getString();
}

void Button::setFont(const sf::Font& f) {
    font = const_cast<sf::Font*>(&f);
    text.setFont(f);
    updateAppearance();
}

void Button::setFontSize(unsigned int size) {
    fontSize = size;
    text.setCharacterSize(size);
    updateAppearance();
}

void Button::setTextColor(const sf::Color& color) {
    textColor = color;
    text.setFillColor(color);
}

void Button::setColors(const sf::Color& normal, const sf::Color& hover, 
                      const sf::Color& pressed, const sf::Color& disabled) {
    normalColor = normal;
    hoverColor = hover;
    pressedColor = pressed;
    disabledColor = disabled;
    updateAppearance();
}

void Button::setBounds(const sf::FloatRect& newBounds) {
    UIElement::setBounds(newBounds);
    background.setSize(sf::Vector2f(bounds.width, bounds.height));
    background.setPosition(bounds.left, bounds.top);
    updateAppearance();
}

void Button::updateAppearance() {
    // Set background color based on state
    sf::Color bgColor;
    switch (state) {
        case UIState::Normal:
            bgColor = normalColor;
            break;
        case UIState::Hovered:
            bgColor = hoverColor;
            break;
        case UIState::Pressed:
            bgColor = pressedColor;
            break;
        case UIState::Disabled:
            bgColor = disabledColor;
            break;
    }
    
    background.setFillColor(bgColor);
    
    // Center text
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setPosition(
        bounds.left + (bounds.width - textBounds.width) / 2 - textBounds.left,
        bounds.top + (bounds.height - textBounds.height) / 2 - textBounds.top
    );
}

void Button::update(float deltaTime) {
    (void)deltaTime; // Suppress unused parameter warning
    updateAppearance();
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!visible) return;
    
    states.transform *= getTransform();
    target.draw(background, states);
    target.draw(text, states);
}