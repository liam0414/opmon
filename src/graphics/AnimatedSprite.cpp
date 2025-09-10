#include "AnimatedSprite.h"
#include "core/Logger.h"

AnimatedSprite::AnimatedSprite() 
    : currentAnimation(nullptr), color(sf::Color::White), flipX(false), flipY(false) {
}

AnimatedSprite::AnimatedSprite(const sf::Texture& texture) : AnimatedSprite() {
    setTexture(texture);
}

void AnimatedSprite::addAnimation(const std::string& name, std::unique_ptr<Animation> animation) {
    animations[name] = std::move(animation);
    LOG_DEBUG("Added animation '{}' to sprite", name);
}

void AnimatedSprite::playAnimation(const std::string& name, bool restart) {
    auto it = animations.find(name);
    if (it != animations.end()) {
        if (currentAnimationName != name || restart) {
            currentAnimationName = name;
            currentAnimation = it->second.get();
            
            if (restart || currentAnimationName != name) {
                currentAnimation->reset();
            }
            
            currentAnimation->play();
            
            // Update sprite texture rect immediately
            sprite.setTextureRect(currentAnimation->getCurrentFrame());
            
            LOG_DEBUG("Playing animation: {}", name);
        }
    } else {
        LOG_WARN("Animation '{}' not found", name);
    }
}

void AnimatedSprite::stopAnimation() {
    if (currentAnimation) {
        currentAnimation->stop();
    }
}

void AnimatedSprite::pauseAnimation() {
    if (currentAnimation) {
        currentAnimation->pause();
    }
}

void AnimatedSprite::update(float deltaTime) {
    if (currentAnimation) {
        currentAnimation->update(deltaTime);
        sprite.setTextureRect(currentAnimation->getCurrentFrame());
    }
}

void AnimatedSprite::setTexture(const sf::Texture& texture) {
    sprite.setTexture(texture);
}

void AnimatedSprite::setColor(const sf::Color& newColor) {
    color = newColor;
    sprite.setColor(color);
}

void AnimatedSprite::setFlip(bool horizontal, bool vertical) {
    if (flipX != horizontal || flipY != vertical) {
        flipX = horizontal;
        flipY = vertical;
        
        sf::IntRect rect = sprite.getTextureRect();
        
        if (flipX) {
            rect.left += rect.width;
            rect.width = -rect.width;
        }
        
        if (flipY) {
            rect.top += rect.height;
            rect.height = -rect.height;
        }
        
        sprite.setTextureRect(rect);
    }
}

bool AnimatedSprite::isAnimationPlaying() const {
    return currentAnimation && currentAnimation->isPlaying();
}

bool AnimatedSprite::isAnimationFinished() const {
    return currentAnimation && currentAnimation->isFinished();
}

sf::FloatRect AnimatedSprite::getLocalBounds() const {
    return sprite.getLocalBounds();
}

sf::FloatRect AnimatedSprite::getGlobalBounds() const {
    return getTransform().transformRect(sprite.getLocalBounds());
}

void AnimatedSprite::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(sprite, states);
}