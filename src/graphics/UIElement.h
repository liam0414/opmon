#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

enum class UIState {
    Normal,
    Hovered,
    Pressed,
    Disabled
};

class UIElement : public sf::Drawable, public sf::Transformable {
protected:
    sf::FloatRect bounds;
    UIState state;
    bool visible;
    bool interactive;
    
    std::function<void()> onClick;
    std::function<void()> onHover;
    std::function<void()> onUnhover;

public:
    UIElement();
    virtual ~UIElement() = default;
    
    // Properties
    virtual void setBounds(const sf::FloatRect& newBounds) { bounds = newBounds; }
    sf::FloatRect getBounds() const { return bounds; }
    
    void setVisible(bool vis) { visible = vis; }
    bool isVisible() const { return visible; }
    
    void setInteractive(bool inter) { interactive = inter; }
    bool isInteractive() const { return interactive; }
    
    // State
    void setState(UIState newState) { state = newState; }
    UIState getState() const { return state; }
    
    // Events
    void setOnClick(std::function<void()> callback) { onClick = callback; }
    void setOnHover(std::function<void()> callback) { onHover = callback; }
    void setOnUnhover(std::function<void()> callback) { onUnhover = callback; }
    
    // Input handling
    virtual bool handleMouseMove(const sf::Vector2f& mousePos);
    virtual bool handleMouseClick(const sf::Vector2f& mousePos, sf::Mouse::Button button);
    virtual void handleMouseRelease(const sf::Vector2f& mousePos, sf::Mouse::Button button);
    
    // Update and render
    virtual void update(float deltaTime) {}
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override = 0;
    
    // Utility
    bool contains(const sf::Vector2f& point) const;
};