#include "ParticleSystem.h"
#include <cmath>

ParticleSystem::ParticleSystem(size_t maxParticles) 
    : maxParticles(maxParticles), texture(nullptr), emissionRate(50.0f), 
      emissionTimer(0), emitting(false), emissionShape(EmissionShape::Point),
      emissionSize(10, 10), velocityRange(-50, 50), accelerationRange(0, 0),
      startColor(sf::Color::White), endColor(sf::Color::Transparent),
      rng(std::random_device{}()), dist(0.0f, 1.0f) {
    
    particles.resize(maxParticles);
    vertices.setPrimitiveType(sf::Quads);
    vertices.resize(maxParticles * 4);
    
    // Default ranges
    lifeRange[0] = 1.0f;
    lifeRange[1] = 3.0f;
    sizeRange[0] = 2.0f;
    sizeRange[1] = 5.0f;
}

void ParticleSystem::setEmissionShape(EmissionShape shape, const sf::Vector2f& size) {
    emissionShape = shape;
    emissionSize = size;
}

void ParticleSystem::setVelocityRange(const sf::Vector2f& min, const sf::Vector2f& max) {
    velocityRange = {min.x, max.x};
}

void ParticleSystem::setAccelerationRange(const sf::Vector2f& min, const sf::Vector2f& max) {
    accelerationRange = {min.x, max.x};
}

void ParticleSystem::setLifeRange(float min, float max) {
    lifeRange[0] = min;
    lifeRange[1] = max;
}

void ParticleSystem::setSizeRange(float min, float max) {
    sizeRange[0] = min;
    sizeRange[1] = max;
}

void ParticleSystem::setColors(const sf::Color& start, const sf::Color& end) {
    startColor = start;
    endColor = end;
}

void ParticleSystem::update(float deltaTime) {
    // Emit new particles
    if (emitting && emissionRate > 0) {
        emissionTimer += deltaTime;
        float emissionInterval = 1.0f / emissionRate;
        
        while (emissionTimer >= emissionInterval) {
            emitParticle();
            emissionTimer -= emissionInterval;
        }
    }
    
    // Update existing particles
    size_t activeCount = 0;
    for (auto& particle : particles) {
        if (particle.alive) {
            particle.update(deltaTime);
            if (particle.alive) {
                activeCount++;
            }
        }
    }
    
    // Update vertex array
    for (size_t i = 0; i < particles.size(); ++i) {
        sf::Vertex* quad = &vertices[i * 4];
        
        if (particles[i].alive) {
            sf::Vector2f pos = particles[i].position;
            float size = particles[i].size;
            sf::Color color = particles[i].color;
            
            // Define quad
            quad[0].position = sf::Vector2f(pos.x - size/2, pos.y - size/2);
            quad[1].position = sf::Vector2f(pos.x + size/2, pos.y - size/2);
            quad[2].position = sf::Vector2f(pos.x + size/2, pos.y + size/2);
            quad[3].position = sf::Vector2f(pos.x - size/2, pos.y + size/2);
            
            // Set color
            for (int j = 0; j < 4; ++j) {
                quad[j].color = color;
            }
            
            // Set texture coordinates if texture is available
            if (texture) {
                quad[0].texCoords = sf::Vector2f(0, 0);
                quad[1].texCoords = sf::Vector2f(texture->getSize().x, 0);
                quad[2].texCoords = sf::Vector2f(texture->getSize().x, texture->getSize().y);
                quad[3].texCoords = sf::Vector2f(0, texture->getSize().y);
            }
        } else {
            // Hide dead particles
            for (int j = 0; j < 4; ++j) {
                quad[j].position = sf::Vector2f(0, 0);
                quad[j].color = sf::Color::Transparent;
            }
        }
    }
}

void ParticleSystem::emitParticle() {
    // Find a dead particle to reuse
    for (auto& particle : particles) {
        if (!particle.alive) {
            // Reset particle
            particle.alive = true;
            particle.position = emissionPosition + getRandomEmissionPosition();
            
            // Random velocity
            particle.velocity = sf::Vector2f(
                randomFloat(velocityRange.x, velocityRange.y),
                randomFloat(velocityRange.x, velocityRange.y)
            );
            
            // Random acceleration
            particle.acceleration = sf::Vector2f(
                randomFloat(accelerationRange.x, accelerationRange.y),
                randomFloat(accelerationRange.x, accelerationRange.y)
            );
            
            // Random life
            particle.maxLife = randomFloat(lifeRange[0], lifeRange[1]);
            particle.life = particle.maxLife;
            
            // Random size
            particle.startSize = randomFloat(sizeRange[0], sizeRange[1]);
            particle.endSize = particle.startSize * 0.1f; // Shrink over time
            particle.size = particle.startSize;
            
            // Colors
            particle.startColor = startColor;
            particle.endColor = endColor;
            particle.color = startColor;
            
            break;
        }
    }
}

sf::Vector2f ParticleSystem::getRandomEmissionPosition() {
    switch (emissionShape) {
        case EmissionShape::Point:
            return {0, 0};
            
        case EmissionShape::Circle: {
            float angle = randomFloat(0, 2 * M_PI);
            float radius = randomFloat(0, emissionSize.x);
            return {radius * std::cos(angle), radius * std::sin(angle)};
        }
        
        case EmissionShape::Rectangle:
            return {
                randomFloat(-emissionSize.x/2, emissionSize.x/2),
                randomFloat(-emissionSize.y/2, emissionSize.y/2)
            };
            
        case EmissionShape::Line:
            return {randomFloat(-emissionSize.x/2, emissionSize.x/2), 0};
    }
    
    return {0, 0};
}

float ParticleSystem::randomFloat(float min, float max) {
    return min + dist(rng) * (max - min);
}

void ParticleSystem::clear() {
    for (auto& particle : particles) {
        particle.reset();
    }
}

size_t ParticleSystem::getParticleCount() const {
    return particles.size();
}

size_t ParticleSystem::getActiveParticleCount() const {
    size_t count = 0;
    for (const auto& particle : particles) {
        if (particle.alive) count++;
    }
    return count;
}

void ParticleSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    states.texture = texture;
    target.draw(vertices, states);
}