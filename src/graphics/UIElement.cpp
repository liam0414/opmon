#include "UIElement.h"

UIElement::UIElement() 
    : bounds(0, 0, 100, 30), state(UIState::Normal), visible(true), interactive(true) {
}

bool UIElement::handleMouseMove(const sf::Vector2f& mousePos) {
    if (!visible || !interactive) return false;
    
    bool wasHovered = (state == UIState::Hovered);
    bool isHovered = contains(mousePos);
    
    if (isHovered && !wasHovered) {
        setState(UIState::Hovered);
        if (onHover) onHover();
        return true;
    } else if (!isHovered && wasHovered) {
        setState(UIState::Normal);
        if (onUnhover) onUnhover();
    }
    
    return isHovered;
}

bool UIElement::handleMouseClick(const sf::Vector2f& mousePos, sf::Mouse::Button button) {
    if (!visible || !interactive) return false;
    
    if (contains(mousePos) && button == sf::Mouse::Left) {
        setState(UIState::Pressed);
        return true;
    }
    
    return false;
}

void UIElement::handleMouseRelease(const sf::Vector2f& mousePos, sf::Mouse::Button button) {
    if (!visible || !interactive) return;
    
    if (state == UIState::Pressed && button == sf::Mouse::Left) {
        if (contains(mousePos)) {
            setState(UIState::Hovered);
            if (onClick) onClick();
        } else {
            setState(UIState::Normal);
        }
    }
}

bool UIElement::contains(const sf::Vector2f& point) const {
    return bounds.contains(point);
}