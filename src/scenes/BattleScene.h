#pragma once
#include "core/Scene.h"
#include "game/BattleSystem.h"
#include "graphics/Button.h"
#include "graphics/ParticleSystem.h"
#include <memory>
#include <vector>

class BattleScene : public Scene {
private:
    std::unique_ptr<BattleSystem> battleSystem;
    
    // UI Elements
    std::vector<std::unique_ptr<Button>> actionButtons;
    std::vector<std::unique_ptr<Button>> targetButtons;
    sf::Text battleLogText;
    sf::Text turnIndicatorText;
    
    // Character display
    std::vector<sf::RectangleShape> playerHealthBars;
    std::vector<sf::RectangleShape> enemyHealthBars;
    std::vector<sf::Text> playerNames;
    std::vector<sf::Text> enemyNames;
    
    // Effects
    std::unique_ptr<ParticleSystem> battleEffects;
    
    // Battle state
    enum class BattleUIState {
        WaitingForTurn,
        SelectingAction,
        SelectingTarget,
        ActionExecuting,
        BattleEnding
    } uiState;
    
    Character* currentActor;
    ActionType selectedAction;
    std::vector<std::string> battleLog;
    
    void setupUI();
    void setupActionButtons();
    void setupTargetButtons();
    void updateHealthBars();
    void updateBattleLog();
    void handleActionSelection(ActionType action);
    void handleTargetSelection(Character* target);
    void addBattleMessage(const std::string& message);

public:
    BattleScene(std::vector<std::unique_ptr<Character>> playerParty,
                std::vector<std::unique_ptr<Character>> enemies);
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};