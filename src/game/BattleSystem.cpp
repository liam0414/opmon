#include "BattleSystem.h"
#include "core/Logger.h"
#include <algorithm>
#include <random>

BattleSystem::BattleSystem() 
    : currentPhase(BattlePhase::Setup), currentTurn(0), allowFlee(true), 
      maxTurns(50), currentTurnCount(0), turnTimer(0), maxTurnTime(30.0f),
      battleEnded(false), playerWon(false), expReward(0), berryReward(0) {
}

void BattleSystem::addPlayerPartyMember(std::unique_ptr<Character> character) {
    if (character) {
        character->setInBattle(true);
        allCombatants.push_back(character.get());
        playerParty.push_back(std::move(character));
        LOG_BATTLE_INFO("Added {} to player party", playerParty.back()->getName());
    }
}

void BattleSystem::addEnemy(std::unique_ptr<Character> enemy) {
    if (enemy) {
        enemy->setInBattle(true);
        allCombatants.push_back(enemy.get());
        enemies.push_back(std::move(enemy));
        LOG_BATTLE_INFO("Added enemy {} to battle", enemies.back()->getName());
    }
}

void BattleSystem::setBattleSettings(bool canFlee, int maxT, float turnTime) {
    allowFlee = canFlee;
    maxTurns = maxT;
    maxTurnTime = turnTime;
}

void BattleSystem::startBattle() {
    if (playerParty.empty() || enemies.empty()) {
        LOG_BATTLE_ERROR("Cannot start battle: missing participants");
        return;
    }
    
    LOG_BATTLE_INFO("🔥 Battle started! {} vs {}", 
                   playerParty.size(), enemies.size());
    
    battleEnded = false;
    playerWon = false;
    currentTurnCount = 0;
    
    setupBattle();
    calculateTurnOrder();
    
    currentPhase = BattlePhase::TurnOrder;
    
    if (onBattleMessage) {
        onBattleMessage("Battle begins!");
    }
}

void BattleSystem::setupBattle() {
    // Set initial positions (could be enhanced for tactical positioning)
    float playerX = 100.0f;
    float enemyX = 700.0f;
    float ySpacing = 80.0f;
    
    for (size_t i = 0; i < playerParty.size(); ++i) {
        playerParty[i]->setPosition(sf::Vector2f(playerX, 200.0f + i * ySpacing));
    }
    
    for (size_t i = 0; i < enemies.size(); ++i) {
        enemies[i]->setPosition(sf::Vector2f(enemyX, 200.0f + i * ySpacing));
    }
}

void BattleSystem::calculateTurnOrder() {
    turnOrder.clear();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 20);
    
    // Calculate initiative for all combatants
    for (auto* character : allCombatants) {
        if (character && character->isAlive()) {
            int initiative = character->getStats().getFinalStat(StatType::Speed) + dis(gen);
            turnOrder.emplace_back(character, initiative);
        }
    }
    
    // Sort by initiative (highest first)
    std::sort(turnOrder.begin(), turnOrder.end(), 
        [](const TurnOrder& a, const TurnOrder& b) {
            return a.initiative > b.initiative;
        });
    
    currentTurn = 0;
    
    // Log turn order
    LOG_BATTLE_INFO("Turn order calculated:");
    for (size_t i = 0; i < turnOrder.size(); ++i) {
        LOG_BATTLE_DEBUG("  {}: {} (Initiative: {})", 
                        i + 1, turnOrder[i].character->getName(), turnOrder[i].initiative);
    }
}

bool BattleSystem::queueAction(const BattleAction& action) {
    if (!action.actor || !canCharacterAct(action.actor)) {
        return false;
    }
    
    actionQueue.push(action);
    LOG_BATTLE_DEBUG("Queued action for {}: {}", 
                    action.actor->getName(), static_cast<int>(action.type));
    return true;
}

bool BattleSystem::canCharacterAct(Character* character) const {
    return character && character->isAlive() && character->canAct();
}

std::vector<Character*> BattleSystem::getValidTargets(Character* actor, bool targetEnemies) const {
    std::vector<Character*> targets;
    
    if (!actor) return targets;
    
    // Determine if actor is player or enemy
    bool actorIsPlayer = false;
    for (const auto& player : playerParty) {
        if (player.get() == actor) {
            actorIsPlayer = true;
            break;
        }
    }
    
    // Get appropriate target list
    if ((actorIsPlayer && targetEnemies) || (!actorIsPlayer && !targetEnemies)) {
        // Target enemies
        for (const auto& enemy : enemies) {
            if (enemy && enemy->isAlive()) {
                targets.push_back(enemy.get());
            }
        }
    } else {
        // Target allies
        for (const auto& ally : playerParty) {
            if (ally && ally->isAlive()) {
                targets.push_back(ally.get());
            }
        }
    }
    
    return targets;
}

void BattleSystem::update(float deltaTime) {
    if (battleEnded) return;
    
    // Update all characters
    for (auto* character : allCombatants) {
        if (character) {
            character->update(deltaTime);
        }
    }
    
    // Update turn timer
    turnTimer += deltaTime;
    
    switch (currentPhase) {
        case BattlePhase::TurnOrder:
            if (currentTurn < turnOrder.size()) {
                Character* currentActor = turnOrder[currentTurn].character;
                
                if (!currentActor || !currentActor->isAlive()) {
                    // Skip dead characters
                    currentTurn++;
                    turnTimer = 0;
                } else {
                    // Determine phase based on character type
                    bool isPlayer = false;
                    for (const auto& player : playerParty) {
                        if (player.get() == currentActor) {
                            isPlayer = true;
                            break;
                        }
                    }
                    
                    if (isPlayer) {
                        currentPhase = (currentActor->getType() == CharacterType::Player) ? 
                                     BattlePhase::PlayerTurn : BattlePhase::CrewTurn;
                    } else {
                        currentPhase = BattlePhase::EnemyTurn;
                        processAI(); // AI acts immediately
                    }
                    
                    turnTimer = 0;
                }
            } else {
                // End of round
                currentTurnCount++;
                processActions();
                checkBattleEnd();
                
                if (!battleEnded) {
                    calculateTurnOrder(); // New round
                    currentPhase = BattlePhase::TurnOrder;
                }
            }
            break;
            
        case BattlePhase::PlayerTurn:
        case BattlePhase::CrewTurn:
            // Wait for player input or AI decision
            if (turnTimer >= maxTurnTime) {
                // Auto-skip turn if time limit exceeded
                LOG_BATTLE_WARN("Turn skipped for {} (timeout)", getCurrentActor()->getName());
                currentTurn++;
                currentPhase = BattlePhase::TurnOrder;
                turnTimer = 0;
            }
            break;
            
        case BattlePhase::EnemyTurn:
            // AI should have acted already, move to next turn
            currentTurn++;
            currentPhase = BattlePhase::TurnOrder;
            break;
            
        default:
            break;
    }
}

void BattleSystem::processAI() {
    Character* currentActor = getCurrentActor();
    if (!currentActor) return;
    
    // Simple AI: attack random valid target
    auto targets = getValidTargets(currentActor, true);
    if (!targets.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, targets.size() - 1);
        
        Character* target = targets[dis(gen)];
        BattleAction action(currentActor, ActionType::Attack, target, 5);
        queueAction(action);
        
        LOG_BATTLE_INFO("{} attacks {}!", currentActor->getName(), target->getName());
    }
}

void BattleSystem::processActions() {
    std::vector<BattleAction> actionsToProcess;
    
    // Extract all actions from queue
    while (!actionQueue.empty()) {
        actionsToProcess.push_back(actionQueue.front());
        actionQueue.pop();
    }
    
    // Sort by priority (higher priority first)
    std::sort(actionsToProcess.begin(), actionsToProcess.end(),
        [](const BattleAction& a, const BattleAction& b) {
            return a.priority > b.priority;
        });
    
    // Execute all actions
    for (const auto& action : actionsToProcess) {
        executeAction(action);
    }
}

void BattleSystem::executeAction(const BattleAction& action) {
    if (!action.actor || !action.actor->isAlive()) return;
    
    switch (action.type) {
        case ActionType::Attack: {
            if (action.target && action.target->isAlive()) {
                int damage = action.actor->getStats().getFinalStat(StatType::Attack);
                
                // Add some randomness
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(80, 120);
                damage = (damage * dis(gen)) / 100;
                
                action.target->takeDamage(damage, action.actor);
                
                if (onBattleMessage) {
                    onBattleMessage(action.actor->getName() + " attacks " + 
                                  action.target->getName() + " for " + std::to_string(damage) + " damage!");
                }
                
                if (onDamageDealt) {
                    onDamageDealt(action.target, damage);
                }
            }
            break;
        }
        
        case ActionType::Defend: {
            // Increase defense temporarily
            action.actor->getStats().addModifier(StatType::Defense, 5);
            
            if (onBattleMessage) {
                onBattleMessage(action.actor->getName() + " takes a defensive stance!");
            }
            break;
        }
        
        case ActionType::UseAbility: {
            if (action.actor->useAbility(action.abilityName, action.target)) {
                if (onBattleMessage) {
                    onBattleMessage(action.actor->getName() + " uses " + action.abilityName + "!");
                }
                
                if (onAbilityUsed) {
                    onAbilityUsed(action.actor, action.abilityName);
                }
            }
            break;
        }
        
        case ActionType::Flee: {
            if (allowFlee) {
                endBattle(false);
                
                if (onBattleMessage) {
                    onBattleMessage("Fled from battle!");
                }
            }
            break;
        }
        
        default:
            break;
    }
}

void BattleSystem::checkBattleEnd() {
    // Check if all players are defeated
    bool playersAlive = false;
    for (const auto& player : playerParty) {
        if (player && player->isAlive()) {
            playersAlive = true;
            break;
        }
    }
    
    // Check if all enemies are defeated
    bool enemiesAlive = false;
    for (const auto& enemy : enemies) {
        if (enemy && enemy->isAlive()) {
            enemiesAlive = true;
            break;
        }
    }
    
    if (!playersAlive) {
        endBattle(false); // Player defeat
    } else if (!enemiesAlive) {
        endBattle(true); // Player victory
    } else if (currentTurnCount >= maxTurns) {
        endBattle(false); // Turn limit reached - considered defeat
        
        if (onBattleMessage) {
            onBattleMessage("Battle reached turn limit!");
        }
    }
}

void BattleSystem::endBattle(bool playerVictory) {
    battleEnded = true;
    playerWon = playerVictory;
    currentPhase = playerVictory ? BattlePhase::Victory : BattlePhase::Defeat;
    
    // Clear battle state from characters
    for (auto* character : allCombatants) {
        if (character) {
            character->setInBattle(false);
            character->getStats().clearModifiers(); // Remove temporary battle modifiers
        }
    }
    
    if (playerVictory) {
        applyRewards();
        LOG_BATTLE_INFO("🎉 Victory! Players win the battle!");
    } else {
        LOG_BATTLE_INFO("💀 Defeat! Players lost the battle.");
    }
    
    if (onBattleEnd) {
        onBattleEnd(playerVictory);
    }
}

void BattleSystem::applyRewards() {
    if (!playerWon) return;
    
    // Calculate experience based on enemy levels
    int totalExp = 0;
    for (const auto& enemy : enemies) {
        if (enemy) {
            totalExp += enemy->getStats().getFinalStat(StatType::Level) * 20;
        }
    }
    
    // Calculate berry reward
    long totalBerry = 0;
    for (const auto& enemy : enemies) {
        if (enemy) {
            totalBerry += enemy->getBounty() / 10; // 10% of bounty as reward
        }
    }
    
    expReward = totalExp;
    berryReward = totalBerry;
    
    // Distribute experience to living party members
    std::vector<Character*> livingMembers;
    for (const auto& member : playerParty) {
        if (member && member->isAlive()) {
            livingMembers.push_back(member.get());
        }
    }
    
    if (!livingMembers.empty()) {
        int expPerMember = totalExp / livingMembers.size();
        for (auto* member : livingMembers) {
            member->getStats().addExperience(expPerMember);
        }
    }
    
    LOG_BATTLE_INFO("Battle rewards: {} EXP, {} Berry", totalExp, totalBerry);
}

Character* BattleSystem::getCurrentActor() const {
    if (currentTurn < turnOrder.size()) {
        return turnOrder[currentTurn].character;
    }
    return nullptr;
}

void BattleSystem::setRewards(int exp, long berry, const std::vector<std::string>& items) {
    expReward = exp;
    berryReward = berry;
    itemRewards = items;
}

void BattleSystem::clear() {
    playerParty.clear();
    enemies.clear();
    allCombatants.clear();
    turnOrder.clear();
    
    while (!actionQueue.empty()) {
        actionQueue.pop();
    }
    
    battleEnded = false;
    playerWon = false;
    currentPhase = BattlePhase::Setup;
    currentTurn = 0;
    currentTurnCount = 0;
    turnTimer = 0;
    
    LOG_BATTLE_INFO("Battle system cleared");
}

nlohmann::json BattleSystem::getBattleStats() const {
    return {
        {"phase", static_cast<int>(currentPhase)},
        {"turn", currentTurn},
        {"turnCount", currentTurnCount},
        {"battleEnded", battleEnded},
        {"playerWon", playerWon},
        {"expReward", expReward},
        {"berryReward", berryReward},
        {"playerPartySize", playerParty.size()},
        {"enemyCount", enemies.size()}
    };
}