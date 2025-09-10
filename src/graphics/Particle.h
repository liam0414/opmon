#pragma once
#include <SFML/Graphics.hpp>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    sf::Color color;
    sf::Color startColor;
    sf::Color endColor;
    
    float life;
    float maxLife;
    float size;
    float startSize;
    float endSize;
    
    bool alive;
    
    Particle();
    void update(float deltaTime);
    void reset();
};