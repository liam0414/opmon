#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "characters/Character.h"
#include "World.h"
#include "Inventory.h"

enum class GameDifficulty {
    Easy,
    Normal,
    Hard,
    Legendary
};

class GameStateManager {
private:
    static GameStateManager* instance;
    
    // Player data
    std::unique_ptr<Character> player;
    std::vector<std::unique_ptr<Character>> crew;
    std::unique_ptr<Inventory> inventory;
    
    // World state
    std::unique_ptr<World> world;
    
    // Game progress
    std::unordered_map<std::string, bool> gameFlags;
    std::unordered_map<std::string, int> gameCounters;
    std::vector<std::string> completedQuests;
    std::vector<std::string> unlockedLocations;
    
    // Settings
    GameDifficulty difficulty;
    float playtime; // In seconds
    std::string currentSaveFile;
    
    // Statistics
    struct GameStats {
        int battlesWon = 0;
        int battlesLost = 0;
        int enemiesDefeated = 0;
        int crewMembersRecruited = 0;
        int questsCompleted = 0;
        int locationsDiscovered = 0;
        long berryEarned = 0;
        long berrySpent = 0;
        
        nlohmann::json toJson() const;
        void fromJson(const nlohmann::json& data);
    } stats;

public:
    static GameStateManager& getInstance();
    
    // Player management
    void setPlayer(std::unique_ptr<Character> playerCharacter);
    Character* getPlayer() { return player.get(); }
    const Character* getPlayer() const { return player.get(); }
    
    // Crew management
    void addCrewMember(std::unique_ptr<Character> member);
    bool removeCrewMember(const std::string& name);
    Character* getCrewMember(const std::string& name);
    const std::vector<std::unique_ptr<Character>>& getCrew() const { return crew; }
    std::vector<Character*> getActiveParty(); // Get battle-ready crew members
    
    // Inventory
    Inventory& getInventory() { return *inventory; }
    const Inventory& getInventory() const { return *inventory; }
    
    // World
    World& getWorld() { return *world; }
    const World& getWorld() const { return *world; }
    
    // Game flags and progress
    void setFlag(const std::string& flag, bool value) { gameFlags[flag] = value; }
    bool getFlag(const std::string& flag) const;
    void setCounter(const std::string& counter, int value) { gameCounters[counter] = value; }
    int getCounter(const std::string& counter) const;
    void incrementCounter(const std::string& counter, int amount = 1);
    
    // Quest progress
    void markQuestCompleted(const std::string& questId);
    bool isQuestCompleted(const std::string& questId) const;
    const std::vector<std::string>& getCompletedQuests() const { return completedQuests; }
    
    // Location progress
    void unlockLocation(const std::string& locationId);
    bool isLocationUnlocked(const std::string& locationId) const;
    const std::vector<std::string>& getUnlockedLocations() const { return unlockedLocations; }
    
    // Game settings
    void setDifficulty(GameDifficulty diff) { difficulty = diff; }
    GameDifficulty getDifficulty() const { return difficulty; }
    float getPlaytime() const { return playtime; }
    std::string getPlaytimeString() const;
    
    // Statistics
    GameStats& getStats() { return stats; }
    const GameStats& getStats() const { return stats; }
    
    // Update
    void update(float deltaTime);
    
    // Save/Load system
    bool saveGame(const std::string& filename);
    bool loadGame(const std::string& filename);
    bool autoSave();
    std::vector<std::string> getSaveFiles() const;
    
    // New game
    void startNewGame(GameDifficulty difficulty = GameDifficulty::Normal);
    void resetGameState();
    
    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& data);
    
    // Utility
    bool isGameStarted() const { return player != nullptr; }
    void clear();
};