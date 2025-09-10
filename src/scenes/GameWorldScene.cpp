#include "GameWorldScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "core/InputManager.h"
#include "graphics/TextureManager.h"
#include "graphics/FontManager.h"
#include "game/EventSystem.h"
#include "BattleScene.h"
#include "PauseMenuScene.h"
#include <random>

GameWorldScene::GameWorldScene() : currentState(GameWorldState::Exploration) {
    camera = std::make_unique<Camera>(sf::Vector2f(1024, 768));
    battleSystem = std::make_unique<BattleSystem>();
    environmentEffects = std::make_unique<ParticleSystem>();
    
    // Setup battle system callbacks
    battleSystem->setOnBattleEnd([this](bool victory) {
        exitBattleState(victory);
    });
    
    battleSystem->setOnBattleMessage([](const std::string& message) {
        LOG_BATTLE_INFO("Battle: {}", message);
    });
}

void GameWorldScene::onEnter() {
    LOG_INFO("Entered Game World");
    
    auto& gameState = GameStateManager::getInstance();
    auto& world = gameState.getWorld();
    
    // Setup camera to follow player
    if (gameState.getPlayer()) {
        camera->setTarget(gameState.getPlayer()->getPosition());
        camera->setFollowSpeed(5.0f);
        camera->setSmoothFollow(true);
    }
    
    // Load current location background
    auto* currentLocation = world.getCurrentLocation();
    if (currentLocation) {
        auto& textureManager = TextureManager::getInstance();
        const std::string& bgTexture = currentLocation->getBackgroundTexture();
        
        if (!bgTexture.empty() && textureManager.loadTexture("location_bg", bgTexture)) {
            backgroundSprite.setTexture(textureManager.getTexture("location_bg"));
        }
        
        // Play location music
        const std::string& music = currentLocation->getMusicTrack();
        if (!music.empty()) {
            Application::getInstance()->getAudioManager().playMusic(music, true);
        }
    }
    
    // Setup UI
    auto& fontManager = FontManager::getInstance();
    debugText.setFont(fontManager.getDefaultFont());
    debugText.setCharacterSize(16);
    debugText.setFillColor(sf::Color::White);
    debugText.setPosition(10, 10);
    
    playerStatsText.setFont(fontManager.getDefaultFont());
    playerStatsText.setCharacterSize(18);
    playerStatsText.setFillColor(sf::Color::White);
    playerStatsText.setPosition(10, 680);
    
    // Health bar setup
    healthBarBg.setSize(sf::Vector2f(300, 20));
    healthBarBg.setFillColor(sf::Color(100, 0, 0));
    healthBarBg.setPosition(10, 720);
    
    healthBar.setSize(sf::Vector2f(300, 20));
    healthBar.setFillColor(sf::Color(0, 255, 0));
    healthBar.setPosition(10, 720);
    
    // Setup environment effects
    environmentEffects->setEmissionPosition(sf::Vector2f(512, 100));
    environmentEffects->setEmissionShape(EmissionShape::Rectangle, sf::Vector2f(1024, 50));
    environmentEffects->setEmissionRate(10.0f);
    environmentEffects->setColors(sf::Color(200, 200, 255, 100), sf::Color(100, 100, 200, 0));
    environmentEffects->setLifeRange(3.0f, 8.0f);
    environmentEffects->setSizeRange(2.0f, 6.0f);
    environmentEffects->setVelocityRange(sf::Vector2f(-20, 20), sf::Vector2f(20, 80));
    environmentEffects->start();
}

void GameWorldScene::onExit() {
    LOG_INFO("Exited Game World");
}

void GameWorldScene::onPause() {
    LOG_INFO("Game World paused");
}

void GameWorldScene::onResume() {
    LOG_INFO("Game World resumed");
}

void GameWorldScene::handleEvent(const sf::Event& event) {
    switch (currentState) {
        case GameWorldState::Exploration:
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    // Open pause menu
                    Application::getInstance()->pushScene(std::make_unique<PauseMenuScene>());
                }
                else if (event.key.code == sf::Keyboard::B) {
                    // Debug: Start test battle
                    enterBattleState({"marine_soldier", "marine_officer"});
                }
                else if (event.key.code == sf::Keyboard::E) {
                    // Interaction key
                    checkForInteractions();
                }
            }
            break;
            
        case GameWorldState::InBattle:
            // Battle input will be handled by battle system
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Num1) {
                    // Attack action
                    auto* player = GameStateManager::getInstance().getPlayer();
                    if (player && battleSystem->canCharacterAct(player)) {
                        auto targets = battleSystem->getValidTargets(player, true);
                        if (!targets.empty()) {
                            BattleAction action(player, ActionType::Attack, targets[0], 5);
                            battleSystem->queueAction(action);
                        }
                    }
                }
                else if (event.key.code == sf::Keyboard::Num2) {
                    // Defend action
                    auto* player = GameStateManager::getInstance().getPlayer();
                    if (player && battleSystem->canCharacterAct(player)) {
                        BattleAction action(player, ActionType::Defend, nullptr, 3);
                        battleSystem->queueAction(action);
                    }
                }
            }
            break;
            
        default:
            break;
    }
}

void GameWorldScene::update(float deltaTime) {
    auto& gameState = GameStateManager::getInstance();
    auto& eventSystem = EventSystem::getInstance();
    
    // Update game state
    gameState.update(deltaTime);
    eventSystem.update(deltaTime);
    
    switch (currentState) {
        case GameWorldState::Exploration:
            handleExplorationInput();
            handleMovementInput(deltaTime);
            checkForRandomEncounters();
            break;
            
        case GameWorldState::InBattle:
            if (battleSystem) {
                battleSystem->update(deltaTime);
                
                // Check if battle ended
                if (!battleSystem->isBattleActive()) {
                    exitBattleState(battleSystem->hasPlayerWon());
                }
            }
            break;
            
        default:
            break;
    }
    
    // Update camera
    camera->update(deltaTime);
    
    // Update effects
    environmentEffects->update(deltaTime);
    
    // Update UI
    updateUI();
    updatePlayerStats();
}

void GameWorldScene::handleExplorationInput() {
    auto& inputManager = Application::getInstance()->getInputManager();
    
    // Quick menu access
    if (inputManager.isActionJustPressed(InputAction::Menu)) {
        Application::getInstance()->pushScene(std::make_unique<PauseMenuScene>());
    }
    
    // Interaction
    if (inputManager.isActionJustPressed(InputAction::Interact)) {
        checkForInteractions();
    }
}

void GameWorldScene::handleMovementInput(float deltaTime) {
    auto& inputManager = Application::getInstance()->getInputManager();
    auto& gameState = GameStateManager::getInstance();
    auto* player = gameState.getPlayer();
    
    if (!player || !player->getCanMove()) return;
    
    sf::Vector2f movement(0, 0);
    float moveSpeed = player->getMoveSpeed();
    
    // Get movement input
    if (inputManager.isActionPressed(InputAction::MoveUp)) {
        movement.y -= 1.0f;
    }
    if (inputManager.isActionPressed(InputAction::MoveDown)) {
        movement.y += 1.0f;
    }
    if (inputManager.isActionPressed(InputAction::MoveLeft)) {
        movement.x -= 1.0f;
    }
    if (inputManager.isActionPressed(InputAction::MoveRight)) {
        movement.x += 1.0f;
    }
    
    // Normalize diagonal movement
    float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
    if (length > 0) {
        movement /= length;
        movement *= moveSpeed * deltaTime;
        
        // Apply movement
        player->move(movement);
        player->setVelocity(movement / deltaTime);
        
        // Update camera target
        camera->setTarget(player->getPosition());
    } else {
        player->setVelocity(sf::Vector2f(0, 0));
    }
}

void GameWorldScene::checkForInteractions() {
    auto& gameState = GameStateManager::getInstance();
    auto* player = gameState.getPlayer();
    if (!player) return;
    
    sf::Vector2f playerPos = player->getPosition();
    float interactionRange = 50.0f;
    
    // Check for NPCs
    auto& world = gameState.getWorld();
    auto* currentLocation = world.getCurrentLocation();
    if (currentLocation) {
        auto npcs = world.getNPCsAtLocation(currentLocation->getId());
        
        for (auto* npc : npcs) {
            if (npc && player->getDistanceTo(*npc) <= interactionRange) {
                LOG_INFO("Talking to {}", npc->getName());
                // TODO: Start dialogue system
                return;
            }
        }
    }
    
    // Check for items or interactive objects
    // TODO: Implement item pickup system
    
    LOG_DEBUG("No interactions available");
}

void GameWorldScene::checkForRandomEncounters() {
    auto& gameState = GameStateManager::getInstance();
    auto* player = gameState.getPlayer();
    auto& world = gameState.getWorld();
    auto* currentLocation = world.getCurrentLocation();
    
    if (!player || !currentLocation || currentLocation->isSafeZone()) {
        return;
    }
    
    // Only check for encounters if player is moving
    sf::Vector2f velocity = player->getVelocity();
    if (std::abs(velocity.x) < 1.0f && std::abs(velocity.y) < 1.0f) {
        return;
    }
    
    // Random encounter chance (very low for demo purposes)
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    
    static float encounterTimer = 0;
    encounterTimer += 0.016f; // Approximate frame time
    
    if (encounterTimer >= 1.0f) { // Check once per second
        encounterTimer = 0;
        
        float encounterChance = 0.02f; // 2% chance per second while moving
        if (dis(gen) < encounterChance) {
            const auto& enemyTypes = currentLocation->getEnemyTypes();
            if (!enemyTypes.empty()) {
                // Pick random enemy types
                std::vector<std::string> encounterEnemies;
                int numEnemies = 1 + (dis(gen) < 0.3 ? 1 : 0); // 1-2 enemies
                
                for (int i = 0; i < numEnemies; ++i) {
                    int randomIndex = static_cast<int>(dis(gen) * enemyTypes.size());
                    encounterEnemies.push_back(enemyTypes[randomIndex]);
                }
                
                LOG_INFO("Random encounter! {} enemies", encounterEnemies.size());
                enterBattleState(encounterEnemies);
            }
        }
    }
}

void GameWorldScene::updateUI() {
    auto& gameState = GameStateManager::getInstance();
    auto* player = gameState.getPlayer();
    
    if (player) {
        // Update health bar
        float healthPercent = player->getStats().getHealthPercentage();
        healthBar.setSize(sf::Vector2f(300 * healthPercent, 20));
        
        // Color based on health
        if (healthPercent > 0.6f) {
            healthBar.setFillColor(sf::Color::Green);
        } else if (healthPercent > 0.3f) {
            healthBar.setFillColor(sf::Color::Yellow);
        } else {
            healthBar.setFillColor(sf::Color::Red);
        }
    }
    
    // Debug information
    std::string debugInfo = "FPS: 60\n"; // TODO: Calculate actual FPS
    debugInfo += "State: " + std::to_string(static_cast<int>(currentState)) + "\n";
    if (player) {
        sf::Vector2f pos = player->getPosition();
        debugInfo += "Position: (" + std::to_string(static_cast<int>(pos.x)) + 
                    ", " + std::to_string(static_cast<int>(pos.y)) + ")\n";
    }
    debugText.setString(debugInfo);
}

void GameWorldScene::updatePlayerStats() {
    auto& gameState = GameStateManager::getInstance();
    auto* player = gameState.getPlayer();
    
    if (player) {
        std::string stats = player->getName() + " - Level " + 
                           std::to_string(player->getStats().getFinalStat(StatType::Level));
        stats += "\nHP: " + std::to_string(player->getStats().getCurrentHealth()) + 
                "/" + std::to_string(player->getStats().getMaxHealth());
        stats += "\nBerry: " + std::to_string(gameState.getInventory().getBerry());
        
        if (player->hasDevilFruit()) {
            stats += "\nDF: " + player->getDevilFruit().getName();
        }
        
        playerStatsText.setString(stats);
    }
}

void GameWorldScene::enterBattleState(const std::vector<std::string>& enemies) {
    currentState = GameWorldState::InBattle;
    
    // Clear battle system
    battleSystem->clear();
    
    // Add player party
    auto& gameState = GameStateManager::getInstance();
    auto activeParty = gameState.getActiveParty();
    
    for (auto* member : activeParty) {
        if (member) {
            // Create copy for battle (to avoid ownership issues)
            auto battleMember = std::make_unique<Character>(member->getName(), member->getType());
            battleMember->fromJson(member->toJson()); // Copy all data
            battleSystem->addPlayerPartyMember(std::move(battleMember));
        }
    }
    
    // Add enemies
    for (const std::string& enemyType : enemies) {
        auto enemy = std::make_unique<Character>("Enemy " + enemyType, CharacterType::Enemy);
        
        // Set enemy stats based on type
        if (enemyType == "marine_soldier") {
            enemy->getStats().setBaseStat(StatType::Level, 1);
            enemy->getStats().setBaseStat(StatType::MaxHealth, 60);
            enemy->getStats().setBaseStat(StatType::Health, 60);
            enemy->getStats().setBaseStat(StatType::Attack, 8);
            enemy->getStats().setBaseStat(StatType::Defense, 4);
        } else if (enemyType == "marine_officer") {
            enemy->getStats().setBaseStat(StatType::Level, 3);
            enemy->getStats().setBaseStat(StatType::MaxHealth, 120);
            enemy->getStats().setBaseStat(StatType::Health, 120);
            enemy->getStats().setBaseStat(StatType::Attack, 15);
            enemy->getStats().setBaseStat(StatType::Defense, 8);
        }
        
        battleSystem->addEnemy(std::move(enemy));
    }
    
    // Setup battle rewards
    int totalExp = enemies.size() * 50;
    long totalBerry = enemies.size() * 100;
    battleSystem->setRewards(totalExp, totalBerry);
    
    // Start battle
    battleSystem->startBattle();
    
    // Camera shake for battle start
    camera->shake(10.0f, 1.0f);
    
    // Play battle music
    Application::getInstance()->getAudioManager().playMusic("battle_theme", true);
    
    LOG_BATTLE_INFO("Entered battle with {} enemies", enemies.size());
}

void GameWorldScene::exitBattleState(bool victory) {
    currentState = GameWorldState::Exploration;
    
    auto& gameState = GameStateManager::getInstance();
    auto& eventSystem = EventSystem::getInstance();
    
    if (victory) {
        // Apply rewards to actual party members
        int expReward = battleSystem->getExpReward();
        long berryReward = battleSystem->getBerryReward();
        
        // Distribute experience
        auto activeParty = gameState.getActiveParty();
        if (!activeParty.empty()) {
            int expPerMember = expReward / activeParty.size();
            for (auto* member : activeParty) {
                if (member && member->isAlive()) {
                    member->getStats().addExperience(expPerMember);
                }
            }
        }
        
        // Give berries
        gameState.getInventory().addBerry(berryReward);
        
        // Update statistics
        gameState.getStats().battlesWon++;
        gameState.getStats().berryEarned += berryReward;
        
        // Emit event
        eventSystem.emitBattleEnd(true, expReward);
        
        LOG_INFO("🎉 Battle victory! Gained {} EXP, {} Berry", expReward, berryReward);
    } else {
        // Handle defeat
        gameState.getStats().battlesLost++;
        eventSystem.emitBattleEnd(false, 0);
        
        // TODO: Implement defeat consequences (respawn, lose berries, etc.)
        LOG_INFO("💀 Battle defeat...");
    }
    
    // Clear battle system
    battleSystem->clear();
    
    // Resume exploration music
    auto& world = gameState.getWorld();
    auto* currentLocation = world.getCurrentLocation();
    if (currentLocation) {
        const std::string& music = currentLocation->getMusicTrack();
        if (!music.empty()) {
            Application::getInstance()->getAudioManager().playMusic(music, true);
        }
    }
}

void GameWorldScene::render(sf::RenderWindow& window) {
    // Set camera view
    window.setView(camera->getView());
    
    switch (currentState) {
        case GameWorldState::Exploration:
            renderExploration(window);
            break;
        case GameWorldState::InBattle:
            renderBattle(window);
            break;
        default:
            renderExploration(window);
            break;
    }
    
    // Switch to UI view for UI elements
    sf::View uiView(sf::FloatRect(0, 0, 1024, 768));
    window.setView(uiView);
    renderUI(window);
}

void GameWorldScene::renderExploration(sf::RenderWindow& window) {
    // Draw background
    window.draw(backgroundSprite);
    
    // Draw environment effects
    window.draw(*environmentEffects);
    
    // Draw player and crew
    auto& gameState = GameStateManager::getInstance();
    auto* player = gameState.getPlayer();
    if (player) {
        window.draw(*player);
    }
    
    const auto& crew = gameState.getCrew();
    for (const auto& member : crew) {
        if (member) {
            window.draw(*member);
        }
    }
    
    // Draw NPCs at current location
    auto& world = gameState.getWorld();
    auto* currentLocation = world.getCurrentLocation();
    if (currentLocation) {
        auto npcs = world.getNPCsAtLocation(currentLocation->getId());
        for (auto* npc : npcs) {
            if (npc) {
                window.draw(*npc);
            }
        }
    }
}

void GameWorldScene::renderBattle(sf::RenderWindow& window) {
    // Draw simplified battle background
    window.clear(sf::Color(40, 20, 20));
    
    // Draw battle participants
    if (battleSystem) {
        const auto& playerParty = battleSystem->getPlayerParty();
        const auto& enemies = battleSystem->getEnemies();
        
        // Draw player party on the left
        for (size_t i = 0; i < playerParty.size(); ++i) {
            if (playerParty[i]) {
                window.draw(*playerParty[i]);
            }
        }
        
        // Draw enemies on the right
        for (size_t i = 0; i < enemies.size(); ++i) {
            if (enemies[i]) {
                window.draw(*enemies[i]);
            }
        }
    }
    
    // Draw battle effects
    window.draw(*environmentEffects);
}

void GameWorldScene::renderUI(sf::RenderWindow& window) {
    // Draw player stats
    window.draw(playerStatsText);
    
    // Draw health bar
    window.draw(healthBarBg);
    window.draw(healthBar);
    
    // Draw debug info
    window.draw(debugText);
    
    // Battle-specific UI
    if (currentState == GameWorldState::InBattle) {
        // Draw battle UI
        sf::Text battleInstructions;
        battleInstructions.setFont(FontManager::getInstance().getDefaultFont());
        battleInstructions.setString("1: Attack  2: Defend  ESC: Menu");
        battleInstructions.setCharacterSize(20);
        battleInstructions.setFillColor(sf::Color::White);
        battleInstructions.setPosition(300, 700);
        window.draw(battleInstructions);
        
        // Draw turn order
        if (battleSystem) {
            const auto& turnOrder = battleSystem->getTurnOrder();
            sf::Text turnText;
            turnText.setFont(FontManager::getInstance().getDefaultFont());
            turnText.setCharacterSize(16);
            turnText.setFillColor(sf::Color::Yellow);
            turnText.setPosition(800, 50);
            
            std::string turnInfo = "Turn Order:\n";
            for (size_t i = 0; i < std::min(turnOrder.size(), size_t(5)); ++i) {
                if (turnOrder[i].character) {
                    turnInfo += turnOrder[i].character->getName();
                    if (i == 0) turnInfo += " <--";
                    turnInfo += "\n";
                }
            }
            turnText.setString(turnInfo);
            window.draw(turnText);
        }
    }
}