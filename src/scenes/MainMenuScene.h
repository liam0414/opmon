#pragma once
#include "core/Scene.h"
#include "graphics/Button.h"
#include <memory>
#include <vector>

class MainMenuScene : public Scene {
private:
    // Background
    sf::Sprite backgroundSprite;
    
    // UI Elements
    std::vector<std::unique_ptr<Button>> buttons;
    sf::Text titleText;
    sf::Text versionText;
    
    // Menu state
    int selectedButton;
    bool transitioning;
    
    // Animation
    float logoAnimationTime;
    sf::Vector2f originalLogoPosition;
    
    void setupUI();
    void setupBackground();
    void createButtons();
    void handleButtonClick(const std::string& buttonId);
    void updateAnimations(float deltaTime);

public:
    MainMenuScene();
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};