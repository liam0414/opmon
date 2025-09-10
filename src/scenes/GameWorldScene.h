#pragma once
#include "core/Scene.h"
#include "game/GameStateManager.h"
#include "game/BattleSystem.h"
#include "graphics/Camera.h"
#include "graphics/ParticleSystem.h"
#include <memory>
#include <vector>

enum class GameWorldState {
    Exploration,
    InBattle,
    InMenu,
    Dialogue,
    Cutscene
};

class GameWorldScene : public Scene {
private:
    GameWorldState currentState;
    
    // Camera and rendering
    std::unique_ptr<Camera> camera;
    sf::Sprite backgroundSprite;
    
    // Game systems
    std::unique_ptr<BattleSystem> battleSystem;
    
    // UI Elements
    sf::Text debugText;
    sf::RectangleShape healthBar;
    sf::RectangleShape healthBarBg;
    sf::Text playerStatsText;
    
    // Effects
    std::unique_ptr<ParticleSystem> environmentEffects;
    
    // Input handling
    void handleExplorationInput();
    void handleMovementInput(float deltaTime);
    void checkForInteractions();
    void checkForRandomEncounters();
    
    // UI updates
    void updateUI();
    void updatePlayerStats();
    
    // State management
    void enterBattleState(const std::vector<std::string>& enemies);
    void exitBattleState(bool victory);
    
    // Rendering by state
    void renderExploration(sf::RenderWindow& window);
    void renderBattle(sf::RenderWindow& window);
    void renderUI(sf::RenderWindow& window);

public:
    GameWorldScene();
    ~GameWorldScene() = default;
    
    void onEnter() override;
    void onExit() override;
    void onPause() override;
    void onResume() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};