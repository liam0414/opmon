#include "Stats.h"
#include "core/Logger.h"
#include <algorithm>
#include <cmath>

Stats::Stats() {
    // Initialize default stats
    baseStats[StatType::Level] = 1;
    baseStats[StatType::MaxHealth] = 100;
    baseStats[StatType::Health] = 100;
    baseStats[StatType::Attack] = 10;
    baseStats[StatType::Defense] = 5;
    baseStats[StatType::Speed] = 8;
    baseStats[StatType::DevilFruitPower] = 0;
    baseStats[StatType::Haki] = 0;
    baseStats[StatType::Experience] = 0;
}

void Stats::setBaseStat(StatType type, int value) {
    baseStats[type] = std::max(0, value);
    
    // Special handling for health
    if (type == StatType::MaxHealth) {
        int currentHealthPercent = (baseStats[StatType::Health] * 100) / 
                                  std::max(1, getBaseStat(StatType::MaxHealth));
        baseStats[StatType::Health] = (value * currentHealthPercent) / 100;
    }
}

int Stats::getBaseStat(StatType type) const {
    auto it = baseStats.find(type);
    return it != baseStats.end() ? it->second : 0;
}

void Stats::addModifier(StatType type, int value) {
    modifiers[type] += value;
}

void Stats::setMultiplier(StatType type, float multiplier) {
    multipliers[type] = multiplier;
}

void Stats::clearModifiers() {
    modifiers.clear();
    multipliers.clear();
}

void Stats::clearModifier(StatType type) {
    modifiers.erase(type);
    multipliers.erase(type);
}

int Stats::getFinalStat(StatType type) const {
    int base = getBaseStat(type);
    
    // Apply flat modifiers
    auto modIt = modifiers.find(type);
    if (modIt != modifiers.end()) {
        base += modIt->second;
    }
    
    // Apply multipliers
    auto multIt = multipliers.find(type);
    if (multIt != multipliers.end()) {
        base = static_cast<int>(base * multIt->second);
    }
    
    return std::max(0, base);
}

int Stats::getCurrentHealth() const {
    return getBaseStat(StatType::Health);
}

int Stats::getMaxHealth() const {
    return getFinalStat(StatType::MaxHealth);
}

void Stats::addExperience(int exp) {
    if (exp <= 0) return;
    
    baseStats[StatType::Experience] += exp;
    LOG_DEBUG("Gained {} experience. Total: {}", exp, baseStats[StatType::Experience]);
    
    // Check for level up
    while (canLevelUp()) {
        levelUp();
    }
}

bool Stats::canLevelUp() const {
    int currentLevel = getBaseStat(StatType::Level);
    int currentExp = getBaseStat(StatType::Experience);
    int requiredExp = currentLevel * 100; // Simple formula: level * 100
    
    return currentExp >= requiredExp;
}

void Stats::levelUp() {
    int currentLevel = getBaseStat(StatType::Level);
    int expRequired = currentLevel * 100;
    
    baseStats[StatType::Level]++;
    baseStats[StatType::Experience] -= expRequired;
    
    // Stat growth on level up
    int healthGrowth = 15 + (currentLevel / 5) * 2;
    int attackGrowth = 2 + (currentLevel / 10);
    int defenseGrowth = 1 + (currentLevel / 8);
    int speedGrowth = 1 + (currentLevel / 12);
    
    baseStats[StatType::MaxHealth] += healthGrowth;
    baseStats[StatType::Attack] += attackGrowth;
    baseStats[StatType::Defense] += defenseGrowth;
    baseStats[StatType::Speed] += speedGrowth;
    
    // Full heal on level up
    baseStats[StatType::Health] = baseStats[StatType::MaxHealth];
    
    LOG_INFO("🌟 Level up! Now level {} (+{} HP, +{} ATK, +{} DEF, +{} SPD)", 
             baseStats[StatType::Level], healthGrowth, attackGrowth, defenseGrowth, speedGrowth);
}

int Stats::getExperienceToNextLevel() const {
    int currentLevel = getBaseStat(StatType::Level);
    int currentExp = getBaseStat(StatType::Experience);
    int requiredExp = currentLevel * 100;
    
    return std::max(0, requiredExp - currentExp);
}

float Stats::getHealthPercentage() const {
    int maxHp = getMaxHealth();
    if (maxHp <= 0) return 0.0f;
    
    return static_cast<float>(getCurrentHealth()) / maxHp;
}

bool Stats::isAlive() const {
    return getCurrentHealth() > 0;
}

void Stats::heal(int amount) {
    if (amount <= 0) return;
    
    int newHealth = std::min(getMaxHealth(), getCurrentHealth() + amount);
    baseStats[StatType::Health] = newHealth;
    
    LOG_DEBUG("Healed for {} HP. Current: {}/{}", amount, newHealth, getMaxHealth());
}

void Stats::takeDamage(int damage) {
    if (damage <= 0) return;
    
    int newHealth = std::max(0, getCurrentHealth() - damage);
    baseStats[StatType::Health] = newHealth;
    
    LOG_DEBUG("Took {} damage. Current: {}/{}", damage, newHealth, getMaxHealth());
}

void Stats::fullHeal() {
    baseStats[StatType::Health] = getMaxHealth();
    LOG_DEBUG("Full heal! Health: {}", getMaxHealth());
}

nlohmann::json Stats::toJson() const {
    nlohmann::json data;
    
    for (const auto& [type, value] : baseStats) {
        data["baseStats"][static_cast<int>(type)] = value;
    }
    
    for (const auto& [type, value] : modifiers) {
        data["modifiers"][static_cast<int>(type)] = value;
    }
    
    for (const auto& [type, value] : multipliers) {
        data["multipliers"][static_cast<int>(type)] = value;
    }
    
    return data;
}

void Stats::fromJson(const nlohmann::json& data) {
    baseStats.clear();
    modifiers.clear();
    multipliers.clear();
    
    if (data.contains("baseStats")) {
        for (const auto& [key, value] : data["baseStats"].items()) {
            StatType type = static_cast<StatType>(std::stoi(key));
            baseStats[type] = value;
        }
    }
    
    if (data.contains("modifiers")) {
        for (const auto& [key, value] : data["modifiers"].items()) {
            StatType type = static_cast<StatType>(std::stoi(key));
            modifiers[type] = value;
        }
    }
    
    if (data.contains("multipliers")) {
        for (const auto& [key, value] : data["multipliers"].items()) {
            StatType type = static_cast<StatType>(std::stoi(key));
            multipliers[type] = value;
        }
    }
}