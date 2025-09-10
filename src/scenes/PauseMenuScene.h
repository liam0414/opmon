#pragma once
#include "core/Scene.h"
#include "graphics/Button.h"
#include <memory>
#include <vector>

class PauseMenuScene : public Scene {
private:
    // UI Elements
    std::vector<std::unique_ptr<Button>> menuButtons;
    sf::RectangleShape backgroundOverlay;
    sf::Text titleText;
    
    // Menu state
    int selectedButton;
    
    void setupUI();
    void handleButtonClick(const std::string& buttonId);

public:
    PauseMenuScene();
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};