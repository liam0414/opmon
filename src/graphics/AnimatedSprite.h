#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include "Animation.h"

class AnimatedSprite : public sf::Drawable, public sf::Transformable {
private:
    sf::Sprite sprite;
    std::unordered_map<std::string, std::unique_ptr<Animation>> animations;
    std::string currentAnimationName;
    Animation* currentAnimation;
    
    sf::Color color;
    bool flipX, flipY;

public:
    AnimatedSprite();
    explicit AnimatedSprite(const sf::Texture& texture);
    
    // Animation management
    void addAnimation(const std::string& name, std::unique_ptr<Animation> animation);
    void playAnimation(const std::string& name, bool restart = false);
    void stopAnimation();
    void pauseAnimation();
    
    // Update
    void update(float deltaTime);
    
    // Texture and appearance
    void setTexture(const sf::Texture& texture);
    void setColor(const sf::Color& newColor);
    void setFlip(bool horizontal, bool vertical = false);
    
    // Getters
    const std::string& getCurrentAnimationName() const { return currentAnimationName; }
    bool isAnimationPlaying() const;
    bool isAnimationFinished() const;
    sf::FloatRect getLocalBounds() const;
    sf::FloatRect getGlobalBounds() const;
    
    // sf::Drawable interface
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};