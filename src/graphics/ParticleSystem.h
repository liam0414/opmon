#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include "Particle.h"

enum class EmissionShape {
    Point,
    Circle,
    Rectangle,
    Line
};

class ParticleSystem : public sf::Drawable, public sf::Transformable {
private:
    std::vector<Particle> particles;
    sf::VertexArray vertices;
    sf::Texture* texture;
    
    // Emission properties
    sf::Vector2f emissionPosition;
    EmissionShape emissionShape;
    sf::Vector2f emissionSize;
    
    // Particle properties
    float emissionRate;
    float emissionTimer;
    sf::Vector2f velocityRange;
    sf::Vector2f accelerationRange;
    float lifeRange[2];
    float sizeRange[2];
    sf::Color startColor;
    sf::Color endColor;
    
    // Random number generation
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    
    size_t maxParticles;
    bool emitting;
    
    void emitParticle();
    sf::Vector2f getRandomEmissionPosition();
    float randomFloat(float min, float max);

public:
    ParticleSystem(size_t maxParticles = 1000);
    
    // Emission control
    void setEmissionPosition(const sf::Vector2f& position) { emissionPosition = position; }
    void setEmissionShape(EmissionShape shape, const sf::Vector2f& size = {10, 10});
    void setEmissionRate(float particlesPerSecond) { emissionRate = particlesPerSecond; }
    
    // Particle properties
    void setVelocityRange(const sf::Vector2f& min, const sf::Vector2f& max);
    void setAccelerationRange(const sf::Vector2f& min, const sf::Vector2f& max);
    void setLifeRange(float min, float max);
    void setSizeRange(float min, float max);
    void setColors(const sf::Color& start, const sf::Color& end);
    void setTexture(sf::Texture* tex) { texture = tex; }
    
    // Control
    void start() { emitting = true; }
    void stop() { emitting = false; }
    void clear();
    
    // Update
    void update(float deltaTime);
    
    // Info
    size_t getParticleCount() const;
    size_t getActiveParticleCount() const;
    
    // sf::Drawable interface
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};