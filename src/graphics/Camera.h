#pragma once
#include <SFML/Graphics.hpp>

class Camera {
private:
    sf::View view;
    sf::Vector2f targetPosition;
    sf::Vector2f currentPosition;
    sf::Vector2f bounds;
    sf::FloatRect worldBounds;
    
    float followSpeed;
    float zoomLevel;
    float minZoom, maxZoom;
    bool smoothFollow;
    bool constrainToBounds;
    
    sf::Vector2f shakeOffset;
    float shakeIntensity;
    float shakeDuration;
    float shakeTimer;

public:
    Camera();
    explicit Camera(const sf::Vector2f& size);
    
    // Position
    void setPosition(const sf::Vector2f& position);
    void setTarget(const sf::Vector2f& target);
    sf::Vector2f getPosition() const;
    
    // Following
    void setFollowSpeed(float speed) { followSpeed = speed; }
    void setSmoothFollow(bool smooth) { smoothFollow = smooth; }
    
    // Zoom
    void setZoom(float zoom);
    void setZoomLimits(float minZoom, float maxZoom);
    float getZoom() const { return zoomLevel; }
    void zoomIn(float factor = 1.1f);
    void zoomOut(float factor = 0.9f);
    
    // Bounds
    void setWorldBounds(const sf::FloatRect& bounds);
    void setConstrainToBounds(bool constrain) { constrainToBounds = constrain; }
    
    // Screen shake
    void shake(float intensity, float duration);
    void stopShake();
    
    // Update
    void update(float deltaTime);
    
    // View access
    const sf::View& getView() const { return view; }
    sf::View& getView() { return view; }
    
    // Utility
    bool isInView(const sf::FloatRect& bounds) const;
    sf::Vector2f screenToWorld(const sf::Vector2i& screenPos, sf::RenderWindow& window) const;
    sf::Vector2i worldToScreen(const sf::Vector2f& worldPos, sf::RenderWindow& window) const;
};