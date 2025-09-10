#include "Camera.h"
#include <cmath>
#include <random>
#include <algorithm>

Camera::Camera() : Camera({1024, 768}) {}

Camera::Camera(const sf::Vector2f& size) 
    : followSpeed(5.0f), zoomLevel(1.0f), minZoom(0.25f), maxZoom(4.0f),
      smoothFollow(true), constrainToBounds(false),
      shakeIntensity(0), shakeDuration(0), shakeTimer(0) {
    
    view.setSize(size);
    view.setCenter(size.x / 2, size.y / 2);
    
    currentPosition = view.getCenter();
    targetPosition = currentPosition;
    
    bounds = size;
    worldBounds = sf::FloatRect(0, 0, size.x * 2, size.y * 2);
}

void Camera::setPosition(const sf::Vector2f& position) {
    currentPosition = position;
    targetPosition = position;
    
    if (constrainToBounds) {
        sf::Vector2f halfSize = bounds * 0.5f / zoomLevel;
        
        currentPosition.x = std::max(worldBounds.left + halfSize.x, 
                                   std::min(worldBounds.left + worldBounds.width - halfSize.x, 
                                          currentPosition.x));
        currentPosition.y = std::max(worldBounds.top + halfSize.y,
                                   std::min(worldBounds.top + worldBounds.height - halfSize.y,
                                          currentPosition.y));
    }
    
    view.setCenter(currentPosition + shakeOffset);
}

void Camera::setTarget(const sf::Vector2f& target) {
    targetPosition = target;
    
    if (!smoothFollow) {
        setPosition(target);
    }
}

sf::Vector2f Camera::getPosition() const {
    return currentPosition;
}

void Camera::setZoom(float zoom) {
    zoomLevel = std::max(minZoom, std::min(maxZoom, zoom));
    view.setSize(bounds / zoomLevel);
}

void Camera::setZoomLimits(float minZ, float maxZ) {
    minZoom = minZ;
    maxZoom = maxZ;
    setZoom(zoomLevel); // Re-apply current zoom with new limits
}

void Camera::zoomIn(float factor) {
    setZoom(zoomLevel * factor);
}

void Camera::zoomOut(float factor) {
    setZoom(zoomLevel * factor);
}

void Camera::setWorldBounds(const sf::FloatRect& bounds) {
    worldBounds = bounds;
}

void Camera::shake(float intensity, float duration) {
    shakeIntensity = intensity;
    shakeDuration = duration;
    shakeTimer = duration;
}

void Camera::stopShake() {
    shakeIntensity = 0;
    shakeDuration = 0;
    shakeTimer = 0;
    shakeOffset = {0, 0};
}

void Camera::update(float deltaTime) {
    // Smooth following
    if (smoothFollow) {
        sf::Vector2f diff = targetPosition - currentPosition;
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        
        if (distance > 1.0f) {
            currentPosition += diff * followSpeed * deltaTime;
        } else {
            currentPosition = targetPosition;
        }
    }
    
    // Screen shake
    if (shakeTimer > 0) {
        shakeTimer -= deltaTime;
        
        if (shakeTimer <= 0) {
            stopShake();
        } else {
            // Generate random shake offset
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
            
            float currentIntensity = shakeIntensity * (shakeTimer / shakeDuration);
            shakeOffset.x = dis(gen) * currentIntensity;
            shakeOffset.y = dis(gen) * currentIntensity;
        }
    }
    
    // Apply position with constraints
    setPosition(currentPosition);
}

bool Camera::isInView(const sf::FloatRect& bounds) const {
    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();
    
    sf::FloatRect viewBounds(center.x - size.x/2, center.y - size.y/2, size.x, size.y);
    
    return viewBounds.intersects(bounds);
}

sf::Vector2f Camera::screenToWorld(const sf::Vector2i& screenPos, sf::RenderWindow& window) const {
    return window.mapPixelToCoords(screenPos, view);
}

sf::Vector2i Camera::worldToScreen(const sf::Vector2f& worldPos, sf::RenderWindow& window) const {
    return window.mapCoordsToPixel(worldPos, view);
}