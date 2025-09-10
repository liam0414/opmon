#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct AnimationFrame {
    sf::IntRect textureRect;
    float duration;
    
    AnimationFrame(const sf::IntRect& rect, float dur) 
        : textureRect(rect), duration(dur) {}
};

class Animation {
private:
    std::vector<AnimationFrame> frames;
    float currentTime;
    size_t currentFrame;
    bool looping;
    bool playing;
    bool finished;

public:
    Animation();
    
    // Frame management
    void addFrame(const sf::IntRect& textureRect, float duration);
    void addFrames(const sf::Vector2u& frameSize, int frameCount, float frameDuration, 
                   int startX = 0, int startY = 0, int spacing = 0);
    
    // Playback control
    void play();
    void pause();
    void stop();
    void reset();
    void setLooping(bool loop) { looping = loop; }
    
    // Update and state
    void update(float deltaTime);
    sf::IntRect getCurrentFrame() const;
    
    // Getters
    bool isPlaying() const { return playing; }
    bool isFinished() const { return finished; }
    bool isLooping() const { return looping; }
    size_t getFrameCount() const { return frames.size(); }
    size_t getCurrentFrameIndex() const { return currentFrame; }
    
    // Utility
    float getTotalDuration() const;
    void setSpeed(float multiplier);
};