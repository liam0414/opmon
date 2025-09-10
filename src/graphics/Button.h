#pragma once
#include "UIElement.h"
#include <SFML/Graphics.hpp>
#include <string>

class Button : public UIElement {
private:
    sf::RectangleShape background;
    sf::Text text;
    sf::Font* font;
    
    sf::Color normalColor;
    sf::Color hoverColor;
    sf::Color pressedColor;
    sf::Color disabledColor;
    
    sf::Color textColor;
    unsigned int fontSize;
    
    void updateAppearance();

public:
    Button();
    Button(const std::string& text, const sf::Font& font);
    
    // Text
    void setText(const std::string& str);
    std::string getText() const;
    void setFont(const sf::Font& f);
    void setFontSize(unsigned int size);
    void setTextColor(const sf::Color& color);
    
    // Colors
    void setColors(const sf::Color& normal, const sf::Color& hover, 
                   const sf::Color& pressed, const sf::Color& disabled);
    
    // Overrides
    void setBounds(const sf::FloatRect& newBounds) override;
    void update(float deltaTime) override;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};