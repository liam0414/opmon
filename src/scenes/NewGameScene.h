#pragma once
#include "core/Scene.h"
#include "graphics/Button.h"
#include "game/GameStateManager.h"
#include <memory>
#include <vector>

class NewGameScene : public Scene {
private:
    // UI Elements
    std::vector<std::unique_ptr<Button>> difficultyButtons;
    std::unique_ptr<Button> startButton;
    std::unique_ptr<Button> backButton;
    
    sf::Text titleText;
    sf::Text descriptionText;
    sf::Text difficultyDescText;
    
    // Character selection (future expansion)
    sf::Sprite characterPreview;
    
    // Selected options
    GameDifficulty selectedDifficulty;
    
    void setupUI();
    void updateDifficultyDescription();
    void startNewGame();

public:
    NewGameScene();
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};