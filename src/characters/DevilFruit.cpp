#include "DevilFruit.h"
#include "core/Logger.h"

nlohmann::json Ability::toJson() const {
    return {
        {"name", name},
        {"description", description},
        {"powerCost", powerCost},
        {"baseDamage", baseDamage},
        {"cooldown", cooldown},
        {"currentCooldown", currentCooldown},
        {"levelRequirement", levelRequirement}
    };
}

void Ability::fromJson(const nlohmann::json& data) {
    name = data.value("name", "Unknown Ability");
    description = data.value("description", "");
    powerCost = data.value("powerCost", 0);
    baseDamage = data.value("baseDamage", 0);
    cooldown = data.value("cooldown", 1.0f);
    currentCooldown = data.value("currentCooldown", 0.0f);
    levelRequirement = data.value("levelRequirement", 1);
}

DevilFruit::DevilFruit() 
    : name("None"), type(DevilFruitType::None), description("No Devil Fruit"),
      masteryLevel(0), masteryPoints(0), awakened(false) {
}

DevilFruit::DevilFruit(const std::string& fruitName, DevilFruitType fruitType, const std::string& desc)
    : name(fruitName), type(fruitType), description(desc),
      masteryLevel(1), masteryPoints(0), awakened(false) {
}

void DevilFruit::addAbility(std::unique_ptr<Ability> ability) {
    abilities.push_back(std::move(ability));
    LOG_DEBUG("Added ability '{}' to Devil Fruit '{}'", abilities.back()->name, name);
}

Ability* DevilFruit::getAbility(const std::string& abilityName) {
    for (auto& ability : abilities) {
        if (ability->name == abilityName) {
            return ability.get();
        }
    }
    return nullptr;
}

std::vector<Ability*> DevilFruit::getAvailableAbilities(int characterLevel) const {
    std::vector<Ability*> available;
    
    for (const auto& ability : abilities) {
        if (characterLevel >= ability->levelRequirement && ability->canUse()) {
            available.push_back(ability.get());
        }
    }
    
    return available;
}

void DevilFruit::addMasteryPoints(int points) {
    if (type == DevilFruitType::None) return;
    
    masteryPoints += points;
    LOG_DEBUG("Gained {} mastery points for {}. Total: {}", points, name, masteryPoints);
    
    while (canLevelUpMastery()) {
        levelUpMastery();
    }
}

bool DevilFruit::canLevelUpMastery() const {
    if (type == DevilFruitType::None || masteryLevel >= 10) return false;
    
    int requiredPoints = masteryLevel * 50; // 50, 100, 150, etc.
    return masteryPoints >= requiredPoints;
}

void DevilFruit::levelUpMastery() {
    if (!canLevelUpMastery()) return;
    
    int requiredPoints = masteryLevel * 50;
    masteryPoints -= requiredPoints;
    masteryLevel++;
    
    LOG_INFO("🌟 {} mastery increased to level {}!", name, masteryLevel);
    
    // Unlock awakening at mastery level 10
    if (masteryLevel >= 10 && !awakened) {
        LOG_INFO("✨ {} can now be awakened!", name);
    }
}

void DevilFruit::awaken() {
    if (masteryLevel >= 10 && !awakened) {
        awakened = true;
        LOG_INFO("🔥 {} has been awakened! Ultimate power unlocked!", name);
        
        // Enhance all abilities
        for (auto& ability : abilities) {
            ability->baseDamage = static_cast<int>(ability->baseDamage * 1.5f);
            ability->cooldown *= 0.8f; // Reduce cooldown by 20%
        }
    }
}

void DevilFruit::update(float deltaTime) {
    for (auto& ability : abilities) {
        ability->update(deltaTime);
    }
}

nlohmann::json DevilFruit::toJson() const {
    nlohmann::json data = {
        {"name", name},
        {"type", static_cast<int>(type)},
        {"description", description},
        {"masteryLevel", masteryLevel},
        {"masteryPoints", masteryPoints},
        {"awakened", awakened}
    };
    
    data["abilities"] = nlohmann::json::array();
    for (const auto& ability : abilities) {
        data["abilities"].push_back(ability->toJson());
    }
    
    return data;
}

void DevilFruit::fromJson(const nlohmann::json& data) {
    name = data.value("name", "None");
    type = static_cast<DevilFruitType>(data.value("type", 0));
    description = data.value("description", "No Devil Fruit");
    masteryLevel = data.value("masteryLevel", 0);
    masteryPoints = data.value("masteryPoints", 0);
    awakened = data.value("awakened", false);
    
    abilities.clear();
    if (data.contains("abilities")) {
        for (const auto& abilityData : data["abilities"]) {
            auto ability = std::make_unique<Ability>("", "", 0, 0, 0);
            ability->fromJson(abilityData);
            abilities.push_back(std::move(ability));
        }
    }
}

std::unique_ptr<DevilFruit> DevilFruit::createGomuGomu() {
    auto fruit = std::make_unique<DevilFruit>("Gomu Gomu no Mi", DevilFruitType::Paramecia, 
                                             "Rubber powers that make the user's body stretch like rubber");
    
    fruit->addAbility(std::make_unique<Ability>("Gomu Gomu no Pistol", 
                                               "Basic stretching punch attack", 
                                               10, 25, 1.0f, 1));
    
    fruit->addAbility(std::make_unique<Ability>("Gomu Gomu no Gatling", 
                                               "Rapid-fire punches", 
                                               25, 15, 3.0f, 5));
    
    fruit->addAbility(std::make_unique<Ability>("Gear Second", 
                                               "Increases speed and power temporarily", 
                                               50, 0, 10.0f, 10));
    
    fruit->addAbility(std::make_unique<Ability>("Gear Third", 
                                               "Giant limb attack with massive damage", 
                                               75, 100, 15.0f, 15));
    
    return fruit;
}

std::unique_ptr<DevilFruit> DevilFruit::createMeraMera() {
    auto fruit = std::make_unique<DevilFruit>("Mera Mera no Mi", DevilFruitType::Logia, 
                                             "Fire powers that allow control over flames");
    
    fruit->addAbility(std::make_unique<Ability>("Fire Fist", 
                                               "Launch a fist-shaped fire projectile", 
                                               15, 35, 1.5f, 1));
    
    fruit->addAbility(std::make_unique<Ability>("Flame Spear", 
                                               "Create spears of fire", 
                                               25, 45, 2.0f, 3));
    
    fruit->addAbility(std::make_unique<Ability>("Flame Emperor", 
                                               "Massive fireball attack", 
                                               80, 120, 12.0f, 12));
    
    return fruit;
}

std::unique_ptr<DevilFruit> DevilFruit::createHieHie() {
    auto fruit = std::make_unique<DevilFruit>("Hie Hie no Mi", DevilFruitType::Logia, 
                                             "Ice powers that allow control over ice and cold");
    
    fruit->addAbility(std::make_unique<Ability>("Ice Saber", 
                                               "Create weapons from ice", 
                                               12, 30, 1.2f, 1));
    
    fruit->addAbility(std::make_unique<Ability>("Ice Age", 
                                               "Freeze the surrounding area", 
                                               40, 20, 8.0f, 8));
    
    fruit->addAbility(std::make_unique<Ability>("Absolute Zero", 
                                               "Ultimate freezing attack", 
                                               100, 150, 20.0f, 20));
    
    return fruit;
}