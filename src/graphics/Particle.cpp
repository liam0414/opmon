#include "Particle.h"
#include <cmath>

Particle::Particle() 
    : position(0, 0), velocity(0, 0), acceleration(0, 0),
      color(sf::Color::White), startColor(sf::Color::White), endColor(sf::Color::Transparent),
      life(0), maxLife(1), size(1), startSize(1), endSize(0), alive(false) {
}

void Particle::update(float deltaTime) {
    if (!alive) return;
    
    // Update physics
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;
    
    // Update life
    life -= deltaTime;
    if (life <= 0) {
        alive = false;
        return;
    }
    
    // Interpolate properties
    float t = 1.0f - (life / maxLife); // 0 to 1 over lifetime
    
    // Color interpolation
    color.r = static_cast<sf::Uint8>(startColor.r + (endColor.r - startColor.r) * t);
    color.g = static_cast<sf::Uint8>(startColor.g + (endColor.g - startColor.g) * t);
    color.b = static_cast<sf::Uint8>(startColor.b + (endColor.b - startColor.b) * t);
    color.a = static_cast<sf::Uint8>(startColor.a + (endColor.a - startColor.a) * t);
    
    // Size interpolation
    size = startSize + (endSize - startSize) * t;
}

void Particle::reset() {
    alive = false;
    life = 0;
    position = {0, 0};
    velocity = {0, 0};
    acceleration = {0, 0};
}