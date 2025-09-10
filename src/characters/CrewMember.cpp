#include "CrewMember.h"
#include "core/Logger.h"
#include <algorithm>

CrewMember::CrewMember(const std::string& name, const std::string& crewRole) 
    : Character(name, CharacterType::CrewMember), role(crewRole), recruited(false), loyalty(50) {
    
    LOG_DEBUG("Created crew member: {} - {}", name, role);
}

void CrewMember::addRecruitmentRequirement(const RecruitmentRequirement& requirement) {
    recruitmentRequirements.push_back(requirement);
    LOG_DEBUG("Added recruitment requirement for {}: {}", getName(), requirement.description);
}

bool CrewMember::canBeRecruited() const {
    if (recruited) return false;
    
    for (const auto& req : recruitmentRequirements) {
        if (!req.completed) {
            return false;
        }
    }
    
    return true;
}

void CrewMember::recruit() {
    if (canBeRecruited()) {
        recruited = true;
        loyalty = 75; // Start with good loyalty when recruited
        LOG_INFO("🎉 {} has joined the crew as {}!", getName(), role);
    }
}

void CrewMember::addLoyalty(int amount) {
    int oldLoyalty = loyalty;
    loyalty = std::max(0, std::min(100, loyalty + amount));
    
    if (loyalty != oldLoyalty) {
        LOG_DEBUG("{} loyalty: {} -> {}", getName(), oldLoyalty, loyalty);
        
        if (loyalty >= 90) {
            LOG_INFO("💖 {} is extremely loyal to the crew!", getName());
        } else if (loyalty <= 20) {
            LOG_WARN("💔 {} is becoming disloyal...", getName());
        }
    }
}

void CrewMember::addCrewAbility(const std::string& ability) {
    if (!hasCrewAbility(ability)) {
        crewAbilities.push_back(ability);
        LOG_INFO("{} learned crew ability: {}", getName(), ability);
    }
}

bool CrewMember::hasCrewAbility(const std::string& ability) const {
    return std::find(crewAbilities.begin(), crewAbilities.end(), ability) != crewAbilities.end();
}

void CrewMember::executeBattleAI(std::vector<Character*>& enemies) {
    if (battleAI && isAlive() && canAct()) {
        battleAI(this, enemies);
    }
}

void CrewMember::update(float deltaTime) {
    Character::update(deltaTime);
    
    // Loyalty naturally increases over time if recruited
    if (recruited && loyalty < 100) {
        static float loyaltyTimer = 0;
        loyaltyTimer += deltaTime;
        
        if (loyaltyTimer >= 60.0f) { // Every minute
            addLoyalty(1);
            loyaltyTimer = 0;
        }
    }
}

nlohmann::json CrewMember::toJson() const {
    nlohmann::json data = Character::toJson();
    
    data["role"] = role;
    data["backstory"] = backstory;
    data["recruited"] = recruited;
    data["loyalty"] = loyalty;
    data["crewAbilities"] = crewAbilities;
    
    data["recruitmentRequirements"] = nlohmann::json::array();
    for (const auto& req : recruitmentRequirements) {
        data["recruitmentRequirements"].push_back({
            {"method", static_cast<int>(req.method)},
            {"description", req.description},
            {"value", req.value},
            {"stringValue", req.stringValue},
            {"completed", req.completed}
        });
    }
    
    return data;
}

void CrewMember::fromJson(const nlohmann::json& data) {
    Character::fromJson(data);
    
    role = data.value("role", "Unknown");
    backstory = data.value("backstory", "");
    recruited = data.value("recruited", false);
    loyalty = data.value("loyalty", 50);
    
    if (data.contains("crewAbilities")) {
        crewAbilities = data["crewAbilities"];
    }
    
    if (data.contains("recruitmentRequirements")) {
        recruitmentRequirements.clear();
        for (const auto& reqData : data["recruitmentRequirements"]) {
            RecruitmentRequirement req(
                static_cast<RecruitmentMethod>(reqData.value("method", 0)),
                reqData.value("description", ""),
                reqData.value("value", 0),
                reqData.value("stringValue", "")
            );
            req.completed = reqData.value("completed", false);
            recruitmentRequirements.push_back(req);
        }
    }
}

std::unique_ptr<CrewMember> CrewMember::createZoro() {
    auto zoro = std::make_unique<CrewMember>("Roronoa Zoro", "Swordsman");
    
    // Set stats
    zoro->getStats().setBaseStat(StatType::Level, 2);
    zoro->getStats().setBaseStat(StatType::MaxHealth, 130);
    zoro->getStats().setBaseStat(StatType::Health, 130);
    zoro->getStats().setBaseStat(StatType::Attack, 18);
    zoro->getStats().setBaseStat(StatType::Defense, 12);
    zoro->getStats().setBaseStat(StatType::Speed, 10);
    
    zoro->setBounty(1111000000);
    zoro->setBackstory("A master swordsman seeking to become the world's greatest swordsman. Uses the three-sword style.");
    
    // Recruitment requirements
    zoro->addRecruitmentRequirement(RecruitmentRequirement(
        RecruitmentMethod::Battle, "Defeat Zoro in single combat", 0));
    zoro->addRecruitmentRequirement(RecruitmentRequirement(
        RecruitmentMethod::Story, "Complete the East Blue arc", 0));
    
    // Crew abilities
    zoro->addCrewAbility("Navigation (Poor)"); // He gets lost easily
    zoro->addCrewAbility("Three Sword Style");
    zoro->addCrewAbility("Intimidation");
    
    // Battle AI - Aggressive melee fighter
    zoro->setBattleAI([](CrewMember* self, std::vector<Character*>& enemies) {
        if (enemies.empty()) return;
        
        // Find closest enemy
        Character* target = nullptr;
        float closestDistance = std::numeric_limits<float>::max();
        
        for (auto* enemy : enemies) {
            if (enemy && enemy->isAlive()) {
                float distance = self->getDistanceTo(*enemy);
                if (distance < closestDistance) {
                    closestDistance = distance;
                    target = enemy;
                }
            }
        }
        
        if (target) {
            // If close enough, attack
            if (closestDistance < 50.0f) {
                int damage = self->getStats().getFinalStat(StatType::Attack) + 5; // Bonus for swordsman
                target->takeDamage(damage, self);
                LOG_INFO("{} slashes {} with his swords!", self->getName(), target->getName());
            } else {
                // Move closer
                sf::Vector2f direction = target->getPosition() - self->getPosition();
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length > 0) {
                    direction /= length;
                    self->setVelocity(direction * self->getMoveSpeed() * 1.2f); // Slightly faster
                }
            }
        }
    });
    
    return zoro;
}

std::unique_ptr<CrewMember> CrewMember::createNami() {
    auto nami = std::make_unique<CrewMember>("Nami", "Navigator");
    
    // Set stats - More support oriented
    nami->getStats().setBaseStat(StatType::Level, 1);
    nami->getStats().setBaseStat(StatType::MaxHealth, 90);
    nami->getStats().setBaseStat(StatType::Health, 90);
    nami->getStats().setBaseStat(StatType::Attack, 8);
    nami->getStats().setBaseStat(StatType::Defense, 6);
    nami->getStats().setBaseStat(StatType::Speed, 12);
    
    nami->setBounty(366000000);
    nami->setBackstory("A skilled navigator and thief who can predict weather patterns. Wields the Clima-Tact.");
    
    // Recruitment requirements
    nami->addRecruitmentRequirement(RecruitmentRequirement(
        RecruitmentMethod::Story, "Free Cocoyasi Village from Arlong", 0));
    nami->addRecruitmentRequirement(RecruitmentRequirement(
        RecruitmentMethod::Item, "Gather 100,000,000 berries", 100000000));
    
    // Crew abilities
    nami->addCrewAbility("Weather Navigation");
    nami->addCrewAbility("Cartography");
    nami->addCrewAbility("Weather Control");
    nami->addCrewAbility("Treasure Detection");
    
    // Battle AI - Support and ranged attacks
    nami->setBattleAI([](CrewMember* self, std::vector<Character*>& enemies) {
        // Nami focuses on supporting allies and using weather attacks
        // For now, basic ranged attack implementation
        if (!enemies.empty()) {
            Character* target = enemies[0]; // Attack first enemy
            if (target && target->isAlive()) {
                int damage = self->getStats().getFinalStat(StatType::Attack);
                target->takeDamage(damage, self);
                LOG_INFO("{} strikes {} with lightning!", self->getName(), target->getName());
            }
        }
    });
    
    return nami;
}

std::unique_ptr<CrewMember> CrewMember::createSanji() {
    auto sanji = std::make_unique<CrewMember>("Sanji", "Cook");
    
    // Set stats - Balanced fighter with high speed
    sanji->getStats().setBaseStat(StatType::Level, 2);
    sanji->getStats().setBaseStat(StatType::MaxHealth, 120);
    sanji->getStats().setBaseStat(StatType::Health, 120);
    sanji->getStats().setBaseStat(StatType::Attack, 15);
    sanji->getStats().setBaseStat(StatType::Defense, 10);
    sanji->getStats().setBaseStat(StatType::Speed, 14);
    
    sanji->setBounty(1032000000);
    sanji->setBackstory("A master chef who fights using only his legs. Dreams of finding the All Blue.");
    
    // Recruitment requirements
    sanji->addRecruitmentRequirement(RecruitmentRequirement(
        RecruitmentMethod::Battle, "Defeat Don Krieg's pirates", 0));
    sanji->addRecruitmentRequirement(RecruitmentRequirement(
        RecruitmentMethod::Story, "Save the Baratie restaurant", 0));
    
    // Crew abilities
    sanji->addCrewAbility("Master Cooking");
    sanji->addCrewAbility("Black Leg Style");
    sanji->addCrewAbility("Food Preparation");
    sanji->addCrewAbility("Nutrition Expert");
    
    // Battle AI - Fast kick-based attacks
    sanji->setBattleAI([](CrewMember* self, std::vector<Character*>& enemies) {
        if (enemies.empty()) return;
        
        Character* target = enemies[0];
        if (target && target->isAlive()) {
            float distance = self->getDistanceTo(*target);
            
            if (distance < 40.0f) {
                int damage = self->getStats().getFinalStat(StatType::Attack) + 3; // Kick bonus
                target->takeDamage(damage, self);
                LOG_INFO("{} delivers a powerful kick to {}!", self->getName(), target->getName());
            } else {
                // Move closer with high speed
                sf::Vector2f direction = target->getPosition() - self->getPosition();
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length > 0) {
                    direction /= length;
                    self->setVelocity(direction * self->getMoveSpeed() * 1.4f); // Faster movement
                }
            }
        }
    });
    
    return sanji;
}