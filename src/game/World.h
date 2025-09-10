#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <nlohmann/json.hpp>

class Character;
class Location;
class Quest;

enum class WeatherType {
    Clear,
    Rainy,
    Stormy,
    Foggy,
    Snowy
};

struct WeatherSystem {
    WeatherType currentWeather;
    float weatherDuration;
    float weatherTimer;
    
    WeatherSystem() : currentWeather(WeatherType::Clear), weatherDuration(300.0f), weatherTimer(0) {}
    
    void update(float deltaTime);
    void changeWeather(WeatherType newWeather, float duration = -1.0f);
};

class World {
private:
    std::string currentLocationId;
    std::unordered_map<std::string, std::unique_ptr<Location>> locations;
    std::vector<std::unique_ptr<Character>> npcs;
    std::unique_ptr<WeatherSystem> weather;
    
    // World time
    float gameTime; // In hours (0-24)
    int gameDay;
    float timeScale; // How fast time passes
    
    // World events
    std::vector<std::unique_ptr<Quest>> activeQuests;
    std::vector<std::unique_ptr<Quest>> completedQuests;
    
public:
    World();
    ~World() = default;
    
    // Location management
    void addLocation(std::unique_ptr<Location> location);
    Location* getLocation(const std::string& locationId);
    Location* getCurrentLocation();
    bool travelToLocation(const std::string& locationId);
    std::vector<std::string> getAvailableDestinations() const;
    
    // NPC management
    void addNPC(std::unique_ptr<Character> npc);
    Character* findNPC(const std::string& name);
    std::vector<Character*> getNPCsAtLocation(const std::string& locationId);
    
    // Time system
    void setTimeScale(float scale) { timeScale = scale; }
    float getGameTime() const { return gameTime; }
    int getGameDay() const { return gameDay; }
    std::string getTimeString() const;
    bool isDay() const { return gameTime >= 6.0f && gameTime < 18.0f; }
    bool isNight() const { return !isDay(); }
    
    // Weather
    WeatherSystem& getWeather() { return *weather; }
    const WeatherSystem& getWeather() const { return *weather; }
    
    // Quest system
    void addQuest(std::unique_ptr<Quest> quest);
    Quest* getQuest(const std::string& questId);
    std::vector<Quest*> getActiveQuests();
    std::vector<Quest*> getCompletedQuests();
    bool completeQuest(const std::string& questId);
    
    // Update
    void update(float deltaTime);
    
    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& data);
    
    // Utility
    void clear();
};