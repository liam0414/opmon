#pragma once
#include <SFML/Graphics.hpp>

class Scene {
public:
    virtual ~Scene() = default;
    
    // Scene lifecycle
    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void onPause() {}
    virtual void onResume() {}
    
    // Main loop methods
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
};