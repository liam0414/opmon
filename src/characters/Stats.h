#pragma once
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>

enum class StatType {
    Health,
    MaxHealth,
    Attack,
    Defense,
    Speed,
    DevilFruitPower,
    Haki,
    Experience,
    Level
};

class Stats {
private:
    std::unordered_map<StatType, int> baseStats;
    std::unordered_map<StatType, int> modifiers; // Temporary bonuses/penalties
    std::unordered_map<StatType, float> multipliers; // Percentage modifiers

public:
    Stats();
    
    // Base stat management
    void setBaseStat(StatType type, int value);
    int getBaseStat(StatType type) const;
    
    // Modifiers
    void addModifier(StatType type, int value);
    void setMultiplier(StatType type, float multiplier);
    void clearModifiers();
    void clearModifier(StatType type);
    
    // Final calculated values
    int getFinalStat(StatType type) const;
    int getCurrentHealth() const;
    int getMaxHealth() const;
    
    // Level and experience
    void addExperience(int exp);
    bool canLevelUp() const;
    void levelUp();
    int getExperienceToNextLevel() const;
    
    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& data);
    
    // Utility
    float getHealthPercentage() const;
    bool isAlive() const;
    void heal(int amount);
    void takeDamage(int damage);
    void fullHeal();
};