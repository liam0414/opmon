#pragma once
#include "core/Scene.h"
#include "graphics/ParticleSystem.h"
#include <memory>
#include <vector>
#include <functional>

struct CutsceneFrame {
    std::string backgroundTexture;
    std::string musicTrack;
    std::vector<std::string> characterTextures;
    std::vector<sf::Vector2f> characterPositions;
    std::string dialogue;
    std::string speakerName;
    float duration; // Auto-advance after this time, -1 for manual advance
    
    CutsceneFrame() : duration(-1.0f) {}
};

class CutsceneScene : public Scene {
private:
    // Cutscene data
    std::vector<CutsceneFrame> frames;
    int currentFrame;
    
    // Visual elements
    sf::Sprite backgroundSprite;
    std::vector<sf::Sprite> characterSprites;
    sf::RectangleShape fadeOverlay;
    std::unique_ptr<ParticleSystem> effectSystem;
    
    // Text display
    sf::RectangleShape textBox;
    sf::Text dialogueText;
    sf::Text speakerText;
    
    // Timing and effects
    float frameTimer;
    float fadeAlpha;
    bool fading;
    bool skipRequested;
    
    // Text animation
    std::string fullText;
    std::string displayedText;
    float textTimer;
    float textSpeed;
    bool textComplete;
    
    // Callbacks
    std::function<void()> onCutsceneComplete;
    
    void loadFrame(int frameIndex);
    void updateTextAnimation(float deltaTime);
    void updateFadeEffect(float deltaTime);
    void nextFrame();
    void skipCutscene();

public:
    CutsceneScene(const std::vector<CutsceneFrame>& cutsceneFrames);
    
    void setOnCutsceneComplete(std::function<void()> callback) { onCutsceneComplete = callback; }
    void setTextSpeed(float speed) { textSpeed = speed; }
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    
    // Static factory methods for famous One Piece cutscenes
    static std::unique_ptr<CutsceneScene> createGameIntro();
    static std::unique_ptr<CutsceneScene> createDevilFruitAwakening();
    static std::unique_ptr<CutsceneScene> createCrewReunion();
};