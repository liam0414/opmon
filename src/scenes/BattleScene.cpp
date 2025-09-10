#include "BattleScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "graphics/FontManager.h"
#include "GameWorldScene.h"

BattleScene::BattleScene(std::vector<std::unique_ptr<Character>> playerParty,
                        std::vector<std::unique_ptr<Character>> enemies) 
    : uiState(BattleUIState::WaitingForTurn), currentActor(nullptr), selectedAction(ActionType::Attack) {
    
    battleSystem = std::make_unique<BattleSystem>();
    battleEffects = std::make_unique<ParticleSystem>();
    
    // Add characters to battle system
    for (auto& character : playerParty) {
        battleSystem->addPlayerPartyMember(std::move(character));
    }
    
    for (auto& enemy : enemies) {
        battleSystem->addEnemy(std::move(enemy));
    }
    
    setupUI();
    
    // Setup battle system callbacks
    battleSystem->setOnBattleMessage([this](const std::string& message) {
        addBattleMessage(message);
    });
    
    battleSystem->setOnDamageDealt([this](Character* target, int damage) {
        // Create damage particle effect at target position
        battleEffects->setEmissionPosition(target->getPosition());
        battleEffects->setColors(sf::Color::Red, sf::Color::Transparent);
        battleEffects->setEmissionRate(50.0f);
        battleEffects->start();
    });
    
    battleSystem->setOnBattleEnd([this](bool victory) {
        uiState = BattleUIState::BattleEnding;
        if (victory) {
            addBattleMessage("Victory! You won the battle!");
        } else {
            addBattleMessage("Defeat... You lost the battle.");
        }
    });
}

void BattleScene::onEnter() {
    LOG_BATTLE_INFO("Entered dedicated battle scene");
    
    // Start battle
    battleSystem->startBattle();
    uiState = BattleUIState::WaitingForTurn;
    
    // Play battle music
    Application::getInstance()->getAudioManager().playMusic("battle_theme", true);
    
    // Setup battle effects
    battleEffects->setEmissionPosition(sf::Vector2f(512, 300));
    battleEffects->setEmissionShape(EmissionShape::Circle, sf::Vector2f(200, 200));
    battleEffects->setEmissionRate(5.0f);
    battleEffects->setColors(sf::Color(100, 100, 255, 100), sf::Color::Transparent);
    battleEffects->setLifeRange(2.0f, 5.0f);
    battleEffects->start();
}

void BattleScene::onExit() {
    LOG_BATTLE_INFO("Exited battle scene");
}

void BattleScene::setupUI() {
    auto& fontManager = FontManager::getInstance();
    
    // Battle log
    battleLogText.setFont(fontManager.getDefaultFont());
    battleLogText.setCharacterSize(16);
    battleLogText.setFillColor(sf::Color::White);
    battleLogText.setPosition(20, 500);
    
    // Turn indicator
    turnIndicatorText.setFont(fontManager.getDefaultFont());
    turnIndicatorText.setCharacterSize(24);
    turnIndicatorText.setFillColor(sf::Color::Yellow);
    turnIndicatorText.setPosition(20, 20);
    
    setupActionButtons();
    setupTargetButtons();
    
    addBattleMessage("Battle begins!");
}

void BattleScene::setupActionButtons() {
    auto& fontManager = FontManager::getInstance();
    
    actionButtons.clear();
    
    std::vector<std::string> actions = {"Attack", "Defend", "Ability", "Item", "Flee"};
    std::vector<ActionType> actionTypes = {
        ActionType::Attack, ActionType::Defend, ActionType::UseAbility, 
        ActionType::UseItem, ActionType::Flee
    };
    
    float buttonWidth = 120;
    float buttonHeight = 40;
    float startX = 20;
    float startY = 650;
    
    for (size_t i = 0; i < actions.size(); ++i) {
        auto button = std::make_unique<Button>(actions[i], fontManager.getDefaultFont());
        button->setBounds(sf::FloatRect(startX + i * (buttonWidth + 10), startY, buttonWidth, buttonHeight));
        
        sf::Color normalColor(50, 100, 150, 200);
        sf::Color hoverColor(70, 120, 170, 220);
        sf::Color pressColor(30, 80, 130, 240);
        
        button->setColors(normalColor, hoverColor, pressColor, sf::Color::Gray);
        
        ActionType actionType = actionTypes[i];
        button->setOnClick([this, actionType]() {
            handleActionSelection(actionType);
        });
        
        actionButtons.push_back(std::move(button));
    }
}

void BattleScene::setupTargetButtons() {
    targetButtons.clear();
    // Target buttons will be created dynamically based on available targets
}

void BattleScene::updateHealthBars() {
    playerHealthBars.clear();
    enemyHealthBars.clear();
    playerNames.clear();
    enemyNames.clear();
    
    auto& fontManager = FontManager::getInstance();
    
    // Player party health bars (left side)
    const auto& playerParty = battleSystem->getPlayerParty();
    for (size_t i = 0; i < playerParty.size(); ++i) {
        if (!playerParty[i]) continue;
        
        float y = 100 + i * 80;
        
        // Health bar background
        sf::RectangleShape bgBar;
        bgBar.setSize(sf::Vector2f(200, 20));
        bgBar.setPosition(50, y);
        bgBar.setFillColor(sf::Color(100, 0, 0));
        playerHealthBars.push_back(bgBar);
        
        // Health bar
        sf::RectangleShape healthBar;
        float healthPercent = playerParty[i]->getStats().getHealthPercentage();
        healthBar.setSize(sf::Vector2f(200 * healthPercent, 20));
        healthBar.setPosition(50, y);
        healthBar.setFillColor(healthPercent > 0.5f ? sf::Color::Green : 
                              healthPercent > 0.25f ? sf::Color::Yellow : sf::Color::Red);
        playerHealthBars.push_back(healthBar);
        
        // Name
        sf::Text nameText;
        nameText.setFont(fontManager.getDefaultFont());
        nameText.setString(playerParty[i]->getName());
        nameText.setCharacterSize(18);
        nameText.setFillColor(sf::Color::White);
        nameText.setPosition(50, y - 25);
        playerNames.push_back(nameText);
    }
    
    // Enemy health bars (right side)
    const auto& enemies = battleSystem->getEnemies();
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (!enemies[i]) continue;
        
        float y = 100 + i * 80;
        
        // Health bar background
        sf::RectangleShape bgBar;
        bgBar.setSize(sf::Vector2f(200, 20));
        bgBar.setPosition(774, y);
        bgBar.setFillColor(sf::Color(100, 0, 0));
        enemyHealthBars.push_back(bgBar);
        
        // Health bar
        sf::RectangleShape healthBar;
        float healthPercent = enemies[i]->getStats().getHealthPercentage();
        healthBar.setSize(sf::Vector2f(200 * healthPercent, 20));
        healthBar.setPosition(774, y);
        healthBar.setFillColor(healthPercent > 0.5f ? sf::Color::Green : 
                              healthPercent > 0.25f ? sf::Color::Yellow : sf::Color::Red);
        enemyHealthBars.push_back(healthBar);
        
        // Name
        sf::Text nameText;
        nameText.setFont(fontManager.getDefaultFont());
        nameText.setString(enemies[i]->getName());
        nameText.setCharacterSize(18);
        nameText.setFillColor(sf::Color::White);
        nameText.setPosition(774, y - 25);
        enemyNames.push_back(nameText);
    }
}

void BattleScene::updateBattleLog() {
    std::string logString;
    
    // Show last 8 messages
    size_t startIndex = battleLog.size() > 8 ? battleLog.size() - 8 : 0;
    
    for (size_t i = startIndex; i < battleLog.size(); ++i) {
        logString += battleLog[i] + "\n";
    }
    
    battleLogText.setString(logString);
}

void BattleScene::handleActionSelection(ActionType action) {
    if (uiState != BattleUIState::SelectingAction) return;
    
    selectedAction = action;
    
    if (action == ActionType::Attack || action == ActionType::UseAbility) {
        // Need to select target
        uiState = BattleUIState::SelectingTarget;
        
        // Create target buttons
        targetButtons.clear();
        auto targets = battleSystem->getValidTargets(currentActor, true);
        
        for (size_t i = 0; i < targets.size(); ++i) {
            auto button = std::make_unique<Button>(targets[i]->getName(), 
                                                 FontManager::getInstance().getDefaultFont());
            button->setBounds(sf::FloatRect(300 + i * 150, 650, 140, 40));
            button->setColors(sf::Color(150, 50, 50, 200), sf::Color(170, 70, 70, 220),
                             sf::Color(130, 30, 30, 240), sf::Color::Gray);
            
            Character* target = targets[i];
            button->setOnClick([this, target]() {
                handleTargetSelection(target);
            });
            
            targetButtons.push_back(std::move(button));
        }
        
        addBattleMessage("Select a target...");
    } else {
        // No target needed, execute action
        BattleAction battleAction(currentActor, action, nullptr, 5);
        battleSystem->queueAction(battleAction);
        
        uiState = BattleUIState::WaitingForTurn;
        addBattleMessage(currentActor->getName() + " performed " + 
                        (action == ActionType::Defend ? "Defend" : 
                         action == ActionType::Flee ? "Flee" : "an action"));
    }
}

void BattleScene::handleTargetSelection(Character* target) {
    if (uiState != BattleUIState::SelectingTarget || !target) return;
    
    BattleAction battleAction(currentActor, selectedAction, target, 5);
    battleSystem->queueAction(battleAction);
    
    uiState = BattleUIState::WaitingForTurn;
    targetButtons.clear();
    
    addBattleMessage(currentActor->getName() + " targets " + target->getName());
}

void BattleScene::addBattleMessage(const std::string& message) {
    battleLog.push_back(message);
    updateBattleLog();
    LOG_BATTLE_INFO("Battle: {}", message);
}

void BattleScene::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
    
    // Handle action buttons
    if (uiState == BattleUIState::SelectingAction) {
        for (auto& button : actionButtons) {
            if (event.type == sf::Event::MouseMoved) {
                button->handleMouseMove(mousePos);
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                button->handleMouseClick(mousePos, event.mouseButton.button);
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                button->handleMouseRelease(mousePos, event.mouseButton.button);
            }
        }
    }
    
    // Handle target buttons
    if (uiState == BattleUIState::SelectingTarget) {
        for (auto& button : targetButtons) {
            if (event.type == sf::Event::MouseMoved) {
                button->handleMouseMove(mousePos);
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                button->handleMouseClick(mousePos, event.mouseButton.button);
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                button->handleMouseRelease(mousePos, event.mouseButton.button);
            }
        }
    }
    
    // Handle battle end
    if (uiState == BattleUIState::BattleEnding) {
        if (event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonPressed) {
            // Return to game world
            Application::getInstance()->changeScene(std::make_unique<GameWorldScene>());
        }
    }
    
    // Keyboard shortcuts
    if (event.type == sf::Event::KeyPressed && uiState == BattleUIState::SelectingAction) {
        switch (event.key.code) {
            case sf::Keyboard::Num1: handleActionSelection(ActionType::Attack); break;
            case sf::Keyboard::Num2: handleActionSelection(ActionType::Defend); break;
            case sf::Keyboard::Num3: handleActionSelection(ActionType::UseAbility); break;
            case sf::Keyboard::Num4: handleActionSelection(ActionType::UseItem); break;
            case sf::Keyboard::Num5: handleActionSelection(ActionType::Flee); break;
            default: break;
        }
    }
}

void BattleScene::update(float deltaTime) {
    if (!battleSystem) return;
    
    battleSystem->update(deltaTime);
    battleEffects->update(deltaTime);
    
    // Update UI state based on battle state
    currentActor = battleSystem->getCurrentActor();
    
    if (battleSystem->getCurrentPhase() == BattlePhase::PlayerTurn && currentActor) {
        if (currentActor->getType() == CharacterType::Player) {
            if (uiState == BattleUIState::WaitingForTurn) {
                uiState = BattleUIState::SelectingAction;
                turnIndicatorText.setString(currentActor->getName() + "'s Turn");
                addBattleMessage(currentActor->getName() + "'s turn! Choose an action.");
            }
        }
    } else if (uiState == BattleUIState::SelectingAction || uiState == BattleUIState::SelectingTarget) {
        uiState = BattleUIState::WaitingForTurn;
        targetButtons.clear();
    }
    
    // Update health bars
    updateHealthBars();
    
    // Update buttons
// Update buttons
    for (auto& button : actionButtons) {
        button->update(deltaTime);
    }
    
    for (auto& button : targetButtons) {
        button->update(deltaTime);
    }
    
    // Check if battle ended
    if (!battleSystem->isBattleActive() && uiState != BattleUIState::BattleEnding) {
        uiState = BattleUIState::BattleEnding;
        
        if (battleSystem->hasPlayerWon()) {
            addBattleMessage("Victory achieved! Press any key to continue...");
            turnIndicatorText.setString("VICTORY!");
            turnIndicatorText.setFillColor(sf::Color::Green);
        } else {
            addBattleMessage("Defeat... Press any key to continue...");
            turnIndicatorText.setString("DEFEAT");
            turnIndicatorText.setFillColor(sf::Color::Red);
        }
    }
}

void BattleScene::render(sf::RenderWindow& window) {
    // Clear with battle background color
    window.clear(sf::Color(20, 30, 50));
    
    // Draw battle effects
    window.draw(*battleEffects);
    
    // Draw character sprites (simplified - just draw at fixed positions)
    const auto& playerParty = battleSystem->getPlayerParty();
    const auto& enemies = battleSystem->getEnemies();
    
    for (size_t i = 0; i < playerParty.size(); ++i) {
        if (playerParty[i] && playerParty[i]->isAlive()) {
            // Draw player characters on left side (simplified representation)
            sf::CircleShape playerIcon(25);
            playerIcon.setFillColor(sf::Color::Blue);
            playerIcon.setPosition(25, 125 + i * 80);
            window.draw(playerIcon);
        }
    }
    
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (enemies[i] && enemies[i]->isAlive()) {
            // Draw enemies on right side
            sf::CircleShape enemyIcon(25);
            enemyIcon.setFillColor(sf::Color::Red);
            enemyIcon.setPosition(950, 125 + i * 80);
            window.draw(enemyIcon);
        }
    }
    
    // Draw health bars
    for (const auto& bar : playerHealthBars) {
        window.draw(bar);
    }
    for (const auto& bar : enemyHealthBars) {
        window.draw(bar);
    }
    
    // Draw character names
    for (const auto& name : playerNames) {
        window.draw(name);
    }
    for (const auto& name : enemyNames) {
        window.draw(name);
    }
    
    // Draw UI
    window.draw(turnIndicatorText);
    window.draw(battleLogText);
    
    // Draw action buttons
    if (uiState == BattleUIState::SelectingAction) {
        for (auto& button : actionButtons) {
            window.draw(*button);
        }
    }
    
    // Draw target buttons
    if (uiState == BattleUIState::SelectingTarget) {
        for (auto& button : targetButtons) {
            window.draw(*button);
        }
    }
    
    // Draw instructions based on state
    sf::Text instructionText;
    instructionText.setFont(FontManager::getInstance().getDefaultFont());
    instructionText.setCharacterSize(18);
    instructionText.setFillColor(sf::Color::Yellow);
    instructionText.setPosition(400, 600);
    
    switch (uiState) {
        case BattleUIState::WaitingForTurn:
            instructionText.setString("Waiting for turn...");
            break;
        case BattleUIState::SelectingAction:
            instructionText.setString("Choose your action (1-5 for shortcuts)");
            break;
        case BattleUIState::SelectingTarget:
            instructionText.setString("Select a target");
            break;
        case BattleUIState::BattleEnding:
            instructionText.setString("Press any key to continue...");
            break;
    }
    
    window.draw(instructionText);
}