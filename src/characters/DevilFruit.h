#pragma once
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

enum class DevilFruitType {
    None,
    Paramecia,
    Zoan,
    Logia
};

class Ability {
public:
    std::string name;
    std::string description;
    int powerCost;
    int baseDamage;
    float cooldown;
    float currentCooldown;
    int levelRequirement;
    
    Ability(const std::string& n, const std::string& desc, int cost, int damage, 
            float cd, int levelReq = 1)
        : name(n), description(desc), powerCost(cost), baseDamage(damage), 
          cooldown(cd), currentCooldown(0), levelRequirement(levelReq) {}
    
    bool canUse() const { return currentCooldown <= 0; }
    void use() { currentCooldown = cooldown; }
    void update(float deltaTime) { currentCooldown = std::max(0.0f, currentCooldown - deltaTime); }
    
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& data);
};

class DevilFruit {
private:
    std::string name;
    DevilFruitType type;
    std::string description;
    std::vector<std::unique_ptr<Ability>> abilities;
    int masteryLevel;
    int masteryPoints;
    bool awakened;

public:
    DevilFruit();
    DevilFruit(const std::string& fruitName, DevilFruitType fruitType, const std::string& desc);
    
    // Basic info
    const std::string& getName() const { return name; }
    DevilFruitType getType() const { return type; }
    const std::string& getDescription() const { return description; }
    
    // Abilities
    void addAbility(std::unique_ptr<Ability> ability);
    const std::vector<std::unique_ptr<Ability>>& getAbilities() const { return abilities; }
    Ability* getAbility(const std::string& abilityName);
    std::vector<Ability*> getAvailableAbilities(int characterLevel) const;
    
    // Mastery
    int getMasteryLevel() const { return masteryLevel; }
    void addMasteryPoints(int points);
    bool canLevelUpMastery() const;
    void levelUpMastery();
    
    // Awakening
    bool isAwakened() const { return awakened; }
    void awaken();
    
    // Update
    void update(float deltaTime);
    
    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& data);
    
    // Static factory methods
    static std::unique_ptr<DevilFruit> createGomuGomu();
    static std::unique_ptr<DevilFruit> createMeraMera();
    static std::unique_ptr<DevilFruit> createHieHie();
};