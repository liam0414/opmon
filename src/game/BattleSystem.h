#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <queue>
#include <nlohmann/json.hpp>
#include "characters/Character.h"

enum class BattlePhase {
    Setup,
    TurnOrder,
    PlayerTurn,
    EnemyTurn,
    CrewTurn,
    Resolution,
    Victory,
    Defeat
};

enum class ActionType {
    Attack,
    Defend,
    UseAbility,
    UseItem,
    Flee
};

struct BattleAction {
    Character* actor;
    ActionType type;
    std::string abilityName;
    std::string itemName;
    Character* target;
    int priority; // Higher values go first
    
    BattleAction(Character* a, ActionType t, Character* tar = nullptr, int prio = 0)
        : actor(a), type(t), target(tar), priority(prio) {}
};

struct TurnOrder {
    Character* character;
    int initiative;
    bool hasActed;
    
    TurnOrder(Character* c, int init) : character(c), initiative(init), hasActed(false) {}
};

class BattleSystem {
private:
    // Battle participants
    std::vector<std::unique_ptr<Character>> playerParty;
    std::vector<std::unique_ptr<Character>> enemies;
    std::vector<Character*> allCombatants;
    
    // Battle state
    BattlePhase currentPhase;
    std::vector<TurnOrder> turnOrder;
    size_t currentTurn;
    std::queue<BattleAction> actionQueue;
    
    // Battle settings
    bool allowFlee;
    int maxTurns;
    int currentTurnCount;
    float turnTimer;
    float maxTurnTime;
    
    // Results
    bool battleEnded;
    bool playerWon;
    int expReward;
    long berryReward;
    std::vector<std::string> itemRewards;
    
    // Battle callbacks
    std::function<void(const std::string&)> onBattleMessage;
    std::function<void(Character*, int)> onDamageDealt;
    std::function<void(Character*, const std::string&)> onAbilityUsed;
    std::function<void(bool)> onBattleEnd;
    
    // Internal methods
    void setupBattle();
    void calculateTurnOrder();
    void processActions();
    void executeAction(const BattleAction& action);
    void checkBattleEnd();
    void processAI();
    void applyRewards();
    
public:
    BattleSystem();
    ~BattleSystem() = default;
    
    // Setup
    void addPlayerPartyMember(std::unique_ptr<Character> character);
    void addEnemy(std::unique_ptr<Character> enemy);
    void setBattleSettings(bool canFlee = true, int maxT = 50, float turnTime = 30.0f);
    
    // Control
    void startBattle();
    void endBattle(bool playerVictory);
    void update(float deltaTime);
    
    // Actions
    bool queueAction(const BattleAction& action);
    bool canCharacterAct(Character* character) const;
    std::vector<Character*> getValidTargets(Character* actor, bool targetEnemies = true) const;
    
    // State
    BattlePhase getCurrentPhase() const { return currentPhase; }
    bool isBattleActive() const { return !battleEnded; }
    bool hasPlayerWon() const { return playerWon; }
    Character* getCurrentActor() const;
    
    // Info
    const std::vector<std::unique_ptr<Character>>& getPlayerParty() const { return playerParty; }
    const std::vector<std::unique_ptr<Character>>& getEnemies() const { return enemies; }
    const std::vector<TurnOrder>& getTurnOrder() const { return turnOrder; }
    
    // Rewards
    void setRewards(int exp, long berry, const std::vector<std::string>& items = {});
    int getExpReward() const { return expReward; }
    long getBerryReward() const { return berryReward; }
    const std::vector<std::string>& getItemRewards() const { return itemRewards; }
    
    // Callbacks
    void setOnBattleMessage(std::function<void(const std::string&)> callback) { onBattleMessage = callback; }
    void setOnDamageDealt(std::function<void(Character*, int)> callback) { onDamageDealt = callback; }
    void setOnAbilityUsed(std::function<void(Character*, const std::string&)> callback) { onAbilityUsed = callback; }
    void setOnBattleEnd(std::function<void(bool)> callback) { onBattleEnd = callback; }
    
    // Utility
    void clear();
    nlohmann::json getBattleStats() const;
};