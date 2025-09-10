#pragma once
#include "Character.h"
#include <functional>

enum class RecruitmentMethod {
    Story,        // Recruited through story progression
    Battle,       // Must defeat in battle
    Quest,        // Complete specific quest
    Reputation,   // Need certain bounty/reputation
    Item,         // Need specific item
    Location      // Must be at specific location
};

struct RecruitmentRequirement {
    RecruitmentMethod method;
    std::string description;
    int value; // Level, bounty amount, item count, etc.
    std::string stringValue; // Item name, location name, etc.
    bool completed;
    
    RecruitmentRequirement(RecruitmentMethod m, const std::string& desc, int val = 0, const std::string& strVal = "")
        : method(m), description(desc), value(val), stringValue(strVal), completed(false) {}
};

class CrewMember : public Character {
private:
    std::string role; // Navigator, Cook, Doctor, etc.
    std::string backstory;
    std::vector<RecruitmentRequirement> recruitmentRequirements;
    bool recruited;
    int loyalty; // 0-100
    
    // Special crew abilities
    std::vector<std::string> crewAbilities;
    
    // AI behavior for battles
    std::function<void(CrewMember*, std::vector<Character*>&)> battleAI;

public:
    CrewMember(const std::string& name, const std::string& crewRole);
    
    // Role and backstory
    const std::string& getRole() const { return role; }
    void setRole(const std::string& newRole) { role = newRole; }
    const std::string& getBackstory() const { return backstory; }
    void setBackstory(const std::string& story) { backstory = story; }
    
    // Recruitment
    void addRecruitmentRequirement(const RecruitmentRequirement& requirement);
    const std::vector<RecruitmentRequirement>& getRecruitmentRequirements() const { return recruitmentRequirements; }
    bool canBeRecruited() const;
    void recruit();
    bool isRecruited() const { return recruited; }
    
    // Loyalty
    int getLoyalty() const { return loyalty; }
    void addLoyalty(int amount);
    void setLoyalty(int amount) { loyalty = std::max(0, std::min(100, amount)); }
    
    // Crew abilities
    void addCrewAbility(const std::string& ability);
    const std::vector<std::string>& getCrewAbilities() const { return crewAbilities; }
    bool hasCrewAbility(const std::string& ability) const;
    
    // AI
    void setBattleAI(std::function<void(CrewMember*, std::vector<Character*>&)> ai) { battleAI = ai; }
    void executeBattleAI(std::vector<Character*>& enemies);
    
    // Overrides
    void update(float deltaTime) override;
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& data) override;
    
    // Static factory methods for creating famous crew members
    static std::unique_ptr<CrewMember> createZoro();
    static std::unique_ptr<CrewMember> createNami();
    static std::unique_ptr<CrewMember> createUsopp();
    static std::unique_ptr<CrewMember> createSanji();
    static std::unique_ptr<CrewMember> createChopper();
    static std::unique_ptr<CrewMember> createRobin();
    static std::unique_ptr<CrewMember> createFranky();
    static std::unique_ptr<CrewMember> createBrook();
    static std::unique_ptr<CrewMember> createJinbe();
};