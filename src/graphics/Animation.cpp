#include "Animation.h"

Animation::Animation() 
    : currentTime(0.0f), currentFrame(0), looping(false), playing(false), finished(false) {
}

void Animation::addFrame(const sf::IntRect& textureRect, float duration) {
    frames.emplace_back(textureRect, duration);
}

void Animation::addFrames(const sf::Vector2u& frameSize, int frameCount, float frameDuration, 
                         int startX, int startY, int spacing) {
    for (int i = 0; i < frameCount; ++i) {
        int x = startX + i * (frameSize.x + spacing);
        int y = startY;
        
        sf::IntRect rect(x, y, frameSize.x, frameSize.y);
        addFrame(rect, frameDuration);
    }
}

void Animation::play() {
    playing = true;
    finished = false;
}

void Animation::pause() {
    playing = false;
}

void Animation::stop() {
    playing = false;
    reset();
}

void Animation::reset() {
    currentTime = 0.0f;
    currentFrame = 0;
    finished = false;
}

void Animation::update(float deltaTime) {
    if (!playing || frames.empty()) {
        return;
    }
    
    currentTime += deltaTime;
    
    if (currentTime >= frames[currentFrame].duration) {
        currentTime = 0.0f;
        currentFrame++;
        
        if (currentFrame >= frames.size()) {
            if (looping) {
                currentFrame = 0;
            } else {
                currentFrame = frames.size() - 1;
                playing = false;
                finished = true;
            }
        }
    }
}

sf::IntRect Animation::getCurrentFrame() const {
    if (frames.empty()) {
        return sf::IntRect();
    }
    return frames[currentFrame].textureRect;
}

float Animation::getTotalDuration() const {
    float total = 0.0f;
    for (const auto& frame : frames) {
        total += frame.duration;
    }
    return total;
}

void Animation::setSpeed(float multiplier) {
    // Adjust all frame durations
    for (auto& frame : frames) {
        frame.duration /= multiplier;
    }
}