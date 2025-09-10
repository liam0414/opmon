#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

enum class InputAction {
    MoveUp,
    MoveDown, 
    MoveLeft,
    MoveRight,
    Confirm,
    Cancel,
    Menu,
    Interact,
    Attack,
    Defend
};

class InputManager {
private:
    std::unordered_map<sf::Keyboard::Key, bool> currentKeys;
    std::unordered_map<sf::Keyboard::Key, bool> previousKeys;
    std::unordered_map<InputAction, sf::Keyboard::Key> keyBindings;
    
    sf::Vector2i mousePosition;
    std::unordered_map<sf::Mouse::Button, bool> currentMouseButtons;
    std::unordered_map<sf::Mouse::Button, bool> previousMouseButtons;

public:
    InputManager();
    
    void handleEvent(const sf::Event& event);
    void update();
    
    // Key states
    bool isKeyPressed(sf::Keyboard::Key key) const;
    bool isKeyJustPressed(sf::Keyboard::Key key) const;
    bool isKeyJustReleased(sf::Keyboard::Key key) const;
    
    // Action states
    bool isActionPressed(InputAction action) const;
    bool isActionJustPressed(InputAction action) const;
    bool isActionJustReleased(InputAction action) const;
    
    // Mouse
    sf::Vector2i getMousePosition() const { return mousePosition; }
    bool isMouseButtonPressed(sf::Mouse::Button button) const;
    bool isMouseButtonJustPressed(sf::Mouse::Button button) const;
    
    // Key binding
    void setKeyBinding(InputAction action, sf::Keyboard::Key key);
    sf::Keyboard::Key getKeyBinding(InputAction action) const;
};