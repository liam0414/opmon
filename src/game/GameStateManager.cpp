#include "GameStateManager.h"
#include "core/Logger.h"
#include "characters/CharacterFactory.h"
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sstream>

GameStateManager* GameStateManager::instance = nullptr;

GameStateManager& GameStateManager::getInstance() {
    if (!instance) {
        instance = new GameStateManager();
    }
    return *instance;
}

void GameStateManager::setPlayer(std::unique_ptr<Character> playerCharacter) {
    player = std::move(playerCharacter);
    if (player) {
        LOG_INFO("Player character set: {}", player->getName());
    }
}

void GameStateManager::addCrewMember(std::unique_ptr<Character> member) {
    if (member) {
        std::string name = member->getName();
        crew.push_back(std::move(member));
        stats.crewMembersRecruited++;
        LOG_INFO("🎉 {} joined the crew! Total crew members: {}", name, crew.size());
    }
}

bool GameStateManager::removeCrewMember(const std::string& name) {
    auto it = std::find_if(crew.begin(), crew.end(),
        [&name](const std::unique_ptr<Character>& member) {
            return member && member->getName() == name;
        });
    
    if (it != crew.end()) {
        crew.erase(it);
        LOG_INFO("{} left the crew", name);
        return true;
    }
    
    return false;
}

Character* GameStateManager::getCrewMember(const std::string& name) {
    for (auto& member : crew) {
        if (member && member->getName() == name) {
            return member.get();
        }
    }
    return nullptr;
}

std::vector<Character*> GameStateManager::getActiveParty() {
    std::vector<Character*> party;
    
    // Add player if available
    if (player && player->isAlive()) {
        party.push_back(player.get());
    }
    
    // Add crew members (limit to first 4 alive members for balance)
    int count = 0;
    for (auto& member : crew) {
        if (member && member->isAlive() && count < 4) {
            party.push_back(member.get());
            count++;
        }
    }
    
    return party;
}

bool GameStateManager::getFlag(const std::string& flag) const {
    auto it = gameFlags.find(flag);
    return it != gameFlags.end() ? it->second : false;
}

int GameStateManager::getCounter(const std::string& counter) const {
    auto it = gameCounters.find(counter);
    return it != gameCounters.end() ? it->second : 0;
}

void GameStateManager::incrementCounter(const std::string& counter, int amount) {
    gameCounters[counter] += amount;
}

void GameStateManager::markQuestCompleted(const std::string& questId) {
    if (std::find(completedQuests.begin(), completedQuests.end(), questId) == completedQuests.end()) {
        completedQuests.push_back(questId);
        stats.questsCompleted++;
        LOG_INFO("✅ Quest completed: {}", questId);
    }
}

bool GameStateManager::isQuestCompleted(const std::string& questId) const {
    return std::find(completedQuests.begin(), completedQuests.end(), questId) != completedQuests.end();
}

void GameStateManager::unlockLocation(const std::string& locationId) {
    if (std::find(unlockedLocations.begin(), unlockedLocations.end(), locationId) == unlockedLocations.end()) {
        unlockedLocations.push_back(locationId);
        stats.locationsDiscovered++;
        LOG_INFO("🗺️ New location unlocked: {}", locationId);
    }
}

bool GameStateManager::isLocationUnlocked(const std::string& locationId) const {
    return std::find(unlockedLocations.begin(), unlockedLocations.end(), locationId) != unlockedLocations.end();
}

std::string GameStateManager::getPlaytimeString() const {
    int totalSeconds = static_cast<int>(playtime);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setw(2) << minutes << ":" << std::setw(2) << seconds;
    
    return oss.str();
}

void GameStateManager::update(float deltaTime) {
    playtime += deltaTime;
    
    // Update world
    if (world) {
        world->update(deltaTime);
    }
    
    // Update player and crew
    if (player) {
        player->update(deltaTime);
    }
    
    for (auto& member : crew) {
        if (member) {
            member->update(deltaTime);
        }
    }
    
    // Auto-save every 5 minutes
    static float autoSaveTimer = 0;
    autoSaveTimer += deltaTime;
    if (autoSaveTimer >= 300.0f) { // 5 minutes
        autoSave();
        autoSaveTimer = 0;
    }
}

bool GameStateManager::saveGame(const std::string& filename) {
    try {
        std::filesystem::create_directories("saves");
        
        nlohmann::json saveData = toJson();
        
        std::string fullPath = "saves/" + filename + ".json";
        std::ofstream file(fullPath);
        if (file.is_open()) {
            file << saveData.dump(4);
            currentSaveFile = filename;
            
            LOG_INFO("💾 Game saved successfully: {}", fullPath);
            return true;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save game '{}': {}", filename, e.what());
    }
    
    return false;
}

bool GameStateManager::loadGame(const std::string& filename) {
    try {
        std::string fullPath = "saves/" + filename + ".json";
        std::ifstream file(fullPath);
        
        if (file.is_open()) {
            nlohmann::json saveData;
            file >> saveData;
            
            fromJson(saveData);
            currentSaveFile = filename;
            
            LOG_INFO("📁 Game loaded successfully: {}", fullPath);
            return true;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load game '{}': {}", filename, e.what());
    }
    
    return false;
}

bool GameStateManager::autoSave() {
    if (!currentSaveFile.empty()) {
        return saveGame("autosave_" + currentSaveFile);
    } else {
        return saveGame("autosave");
    }
}

std::vector<std::string> GameStateManager::getSaveFiles() const {
    std::vector<std::string> saveFiles;
    
    try {
        if (std::filesystem::exists("saves")) {
            for (const auto& entry : std::filesystem::directory_iterator("saves")) {
                if (entry.path().extension() == ".json") {
                    std::string filename = entry.path().stem().string();
                    saveFiles.push_back(filename);
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error reading save files: {}", e.what());
    }
    
    std::sort(saveFiles.begin(), saveFiles.end());
    return saveFiles;
}

void GameStateManager::startNewGame(GameDifficulty diff) {
    resetGameState();
    
    difficulty = diff;
    playtime = 0;
    
    // Create default player (Luffy)
    auto luffy = std::make_unique<Character>("Monkey D. Luffy", CharacterType::Player);
    luffy->setTitle("Straw Hat");
    luffy->setBounty(3000000000L);
    
    // Set stats based on difficulty
    int statMultiplier = 1;
    switch (diff) {
        case GameDifficulty::Easy: statMultiplier = 2; break;
        case GameDifficulty::Normal: statMultiplier = 1; break;
        case GameDifficulty::Hard: statMultiplier = 1; break;
        case GameDifficulty::Legendary: statMultiplier = 1; break;
    }
    
    luffy->getStats().setBaseStat(StatType::Level, 1);
    luffy->getStats().setBaseStat(StatType::MaxHealth, 120 * statMultiplier);
    luffy->getStats().setBaseStat(StatType::Health, 120 * statMultiplier);
    luffy->getStats().setBaseStat(StatType::Attack, 15 * statMultiplier);
    luffy->getStats().setBaseStat(StatType::Defense, 8 * statMultiplier);
    luffy->getStats().setBaseStat(StatType::Speed, 12 * statMultiplier);
    
    // Give Luffy his Devil Fruit
    luffy->setDevilFruit(DevilFruit::createGomuGomu());
    
    setPlayer(std::move(luffy));
    
    // Initialize world and starting location
    world = std::make_unique<World>();
    inventory = std::make_unique<Inventory>();
    
    // Add starting items based on difficulty
    long startingBerry = 1000;
    switch (diff) {
        case GameDifficulty::Easy: startingBerry = 5000; break;
        case GameDifficulty::Normal: startingBerry = 1000; break;
        case GameDifficulty::Hard: startingBerry = 500; break;
        case GameDifficulty::Legendary: startingBerry = 100; break;
    }
    
    inventory->addBerry(startingBerry);
    
    // Set initial flags
    setFlag("game_started", true);
    setFlag("tutorial_completed", false);
    
    LOG_INFO("🎮 New game started on {} difficulty", static_cast<int>(diff));
    LOG_INFO("👤 Playing as: {} - {}", player->getName(), player->getTitle());
}

void GameStateManager::resetGameState() {
    player.reset();
    crew.clear();
    world.reset();
    inventory.reset();
    
    gameFlags.clear();
    gameCounters.clear();
    completedQuests.clear();
    unlockedLocations.clear();
    
    stats = GameStats{};
    playtime = 0;
    currentSaveFile.clear();
    
    LOG_INFO("Game state reset");
}

nlohmann::json GameStateManager::toJson() const {
    nlohmann::json data = {
        {"version", "1.0"},
        {"difficulty", static_cast<int>(difficulty)},
        {"playtime", playtime},
        {"gameFlags", gameFlags},
        {"gameCounters", gameCounters},
        {"completedQuests", completedQuests},
        {"unlockedLocations", unlockedLocations}
    };
    
    // Player data
    if (player) {
        data["player"] = player->toJson();
    }
    
    // Crew data
    data["crew"] = nlohmann::json::array();
    for (const auto& member : crew) {
        if (member) {
            data["crew"].push_back(member->toJson());
        }
    }
    
    // Inventory
    if (inventory) {
        data["inventory"] = inventory->toJson();
    }
    
    // World
    if (world) {
        data["world"] = world->toJson();
    }
    
    // Statistics
    data["stats"] = stats.toJson();
    
    return data;
}

void GameStateManager::fromJson(const nlohmann::json& data) {
    resetGameState();
    
    difficulty = static_cast<GameDifficulty>(data.value("difficulty", 1));
    playtime = data.value("playtime", 0.0f);
    
    if (data.contains("gameFlags")) {
        gameFlags = data["gameFlags"];
    }
    
    if (data.contains("gameCounters")) {
        gameCounters = data["gameCounters"];
    }
    
    if (data.contains("completedQuests")) {
        completedQuests = data["completedQuests"];
    }
    
    if (data.contains("unlockedLocations")) {
        unlockedLocations = data["unlockedLocations"];
    }
    
    // Load player
    if (data.contains("player")) {
        auto playerData = data["player"];
        std::string name = playerData.value("name", "Unknown");
        CharacterType type = static_cast<CharacterType>(playerData.value("type", 0));
        
        player = std::make_unique<Character>(name, type);
        player->fromJson(playerData);
    }
    
    // Load crew
    if (data.contains("crew")) {
        for (const auto& memberData : data["crew"]) {
            auto member = CharacterFactory::getInstance().createFromJson(memberData);
            if (member) {
                crew.push_back(std::move(member));
            }
        }
    }
    
    // Load inventory
    inventory = std::make_unique<Inventory>();
    if (data.contains("inventory")) {
        inventory->fromJson(data["inventory"]);
    }
    
    // Load world
    world = std::make_unique<World>();
    if (data.contains("world")) {
        world->fromJson(data["world"]);
    }
    
    // Load statistics
    if (data.contains("stats")) {
        stats.fromJson(data["stats"]);
    }
}

void GameStateManager::clear() {
    resetGameState();
}

nlohmann::json GameStateManager::GameStats::toJson() const {
    return {
        {"battlesWon", battlesWon},
        {"battlesLost", battlesLost},
        {"enemiesDefeated", enemiesDefeated},
        {"crewMembersRecruited", crewMembersRecruited},
        {"questsCompleted", questsCompleted},
        {"locationsDiscovered", locationsDiscovered},
        {"berryEarned", berryEarned},
        {"berrySpent", berrySpent}
    };
}

void GameStateManager::GameStats::fromJson(const nlohmann::json& data) {
    battlesWon = data.value("battlesWon", 0);
    battlesLost = data.value("battlesLost", 0);
    enemiesDefeated = data.value("enemiesDefeated", 0);
    crewMembersRecruited = data.value("crewMembersRecruited", 0);
    questsCompleted = data.value("questsCompleted", 0);
    locationsDiscovered = data.value("locationsDiscovered", 0);
    berryEarned = data.value("berryEarned", 0L);
    berrySpent = data.value("berrySpent", 0L);
}