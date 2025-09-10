#include "InputManager.h"
#include "Logger.h"

InputManager::InputManager() {
    // Set default key bindings
    keyBindings = {
        {InputAction::MoveUp, sf::Keyboard::W},
        {InputAction::MoveDown, sf::Keyboard::S},
        {InputAction::MoveLeft, sf::Keyboard::A},
        {InputAction::MoveRight, sf::Keyboard::D},
        {InputAction::Confirm, sf::Keyboard::Enter},
        {InputAction::Cancel, sf::Keyboard::Escape},
        {InputAction::Menu, sf::Keyboard::M},
        {InputAction::Interact, sf::Keyboard::E},
        {InputAction::Attack, sf::Keyboard::Space},
        {InputAction::Defend, sf::Keyboard::LShift}
    };
    
    LOG_SYSTEM_INFO("InputManager initialized with default key bindings");
}

void InputManager::handleEvent(const sf::Event& event) {
    switch (event.type) {
        case sf::Event::KeyPressed:
            currentKeys[event.key.code] = true;
            break;
        case sf::Event::KeyReleased:
            currentKeys[event.key.code] = false;
            break;
        case sf::Event::MouseButtonPressed:
            currentMouseButtons[event.mouseButton.button] = true;
            break;
        case sf::Event::MouseButtonReleased:
            currentMouseButtons[event.mouseButton.button] = false;
            break;
        case sf::Event::MouseMoved:
            mousePosition = {event.mouseMove.x, event.mouseMove.y};
            break;
        default:
            break;
    }
}

void InputManager::update() {
    // Update previous states
    previousKeys = currentKeys;
    previousMouseButtons = currentMouseButtons;
}

bool InputManager::isKeyPressed(sf::Keyboard::Key key) const {
    auto it = currentKeys.find(key);
    return it != currentKeys.end() && it->second;
}

bool InputManager::isKeyJustPressed(sf::Keyboard::Key key) const {
    auto currentIt = currentKeys.find(key);
    auto previousIt = previousKeys.find(key);
    
    bool currentPressed = currentIt != currentKeys.end() && currentIt->second;
    bool previousPressed = previousIt != previousKeys.end() && previousIt->second;
    
    return currentPressed && !previousPressed;
}

bool InputManager::isKeyJustReleased(sf::Keyboard::Key key) const {
    auto currentIt = currentKeys.find(key);
    auto previousIt = previousKeys.find(key);
    
    bool currentPressed = currentIt != currentKeys.end() && currentIt->second;
    bool previousPressed = previousIt != previousKeys.end() && previousIt->second;
    
    return !currentPressed && previousPressed;
}

bool InputManager::isActionPressed(InputAction action) const {
    auto it = keyBindings.find(action);
    if (it != keyBindings.end()) {
        return isKeyPressed(it->second);
    }
    return false;
}

bool InputManager::isActionJustPressed(InputAction action) const {
    auto it = keyBindings.find(action);
    if (it != keyBindings.end()) {
        return isKeyJustPressed(it->second);
    }
    return false;
}

bool InputManager::isActionJustReleased(InputAction action) const {
    auto it = keyBindings.find(action);
    if (it != keyBindings.end()) {
        return isKeyJustReleased(it->second);
    }
    return false;
}

bool InputManager::isMouseButtonPressed(sf::Mouse::Button button) const {
    auto it = currentMouseButtons.find(button);
    return it != currentMouseButtons.end() && it->second;
}

bool InputManager::isMouseButtonJustPressed(sf::Mouse::Button button) const {
    auto currentIt = currentMouseButtons.find(button);
    auto previousIt = previousMouseButtons.find(button);
    
    bool currentPressed = currentIt != currentMouseButtons.end() && currentIt->second;
    bool previousPressed = previousIt != previousMouseButtons.end() && previousIt->second;
    
    return currentPressed && !previousPressed;
}

void InputManager::setKeyBinding(InputAction action, sf::Keyboard::Key key) {
    keyBindings[action] = key;
    LOG_DEBUG("Key binding updated: action {} -> key {}", static_cast<int>(action), static_cast<int>(key));
}

sf::Keyboard::Key InputManager::getKeyBinding(InputAction action) const {
    auto it = keyBindings.find(action);
    if (it != keyBindings.end()) {
        return it->second;
    }
    return sf::Keyboard::Unknown;
}