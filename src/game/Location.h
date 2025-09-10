#pragma once
#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

enum class QuestType {
    Main,        // Story progression
    Side,        // Optional side quests
    Bounty,      // Defeat specific enemies
    Collection,  // Collect items
    Delivery,    // Deliver items to NPCs
    Exploration  // Visit locations
};

enum class QuestStatus {
    NotStarted,
    Active,
    Completed,
    Failed
};

struct QuestObjective {
    std::string description;
    std::string type; // "defeat", "collect", "visit", "talk", etc.
    std::string target; // Enemy name, item name, location, NPC, etc.
    int requiredAmount;
    int currentAmount;
    bool completed;
    
    QuestObjective(const std::string& desc, const std::string& t, const std::string& tar, int req = 1)
        : description(desc), type(t), target(tar), requiredAmount(req), currentAmount(0), completed(false) {}
    
    void updateProgress(int amount = 1);
    float getProgress() const;
};

struct QuestReward {
    int experience;
    long berry;
    std::vector<std::string> items;
    std::string unlockLocation; // Unlock new location
    std::string unlockCharacter; // Unlock crew member for recruitment
    
    QuestReward() : experience(0), berry(0) {}
};

class Quest {
private:
    std::string id;
    std::string title;
    std::string description;
    QuestType type;
    QuestStatus status;
    
    std::vector<QuestObjective> objectives;
    QuestReward reward;
    // Prerequisites
    std::vector<std::string> requiredQuests; // Must complete these first
    int requiredLevel;
    std::string requiredLocation;
    
    // Quest giver
    std::string questGiverId;
    std::string questGiverName;
    
    // Callbacks
    std::function<void(Quest*)> onStart;
    std::function<void(Quest*)> onComplete;
    std::function<void(Quest*)> onFail;

public:
    Quest(const std::string& questId, const std::string& questTitle, QuestType questType);
    
    // Basic info
    const std::string& getId() const { return id; }
    const std::string& getTitle() const { return title; }
    const std::string& getDescription() const { return description; }
    QuestType getType() const { return type; }
    QuestStatus getStatus() const { return status; }
    
    void setDescription(const std::string& desc) { description = desc; }
    void setQuestGiver(const std::string& giverId, const std::string& giverName);
    
    // Prerequisites
    void addRequiredQuest(const std::string& questId) { requiredQuests.push_back(questId); }
    void setRequiredLevel(int level) { requiredLevel = level; }
    void setRequiredLocation(const std::string& location) { requiredLocation = location; }
    
    bool canStart(int playerLevel, const std::string& currentLocation, 
                  const std::vector<std::string>& completedQuests) const;
    
    // Objectives
    void addObjective(const QuestObjective& objective);
    const std::vector<QuestObjective>& getObjectives() const { return objectives; }
    void updateObjective(const std::string& type, const std::string& target, int amount = 1);
    bool areObjectivesComplete() const;
    float getOverallProgress() const;
    
    // Rewards
    void setReward(const QuestReward& questReward) { reward = questReward; }
    const QuestReward& getReward() const { return reward; }
    
    // Status management
    void start();
    void complete();
    void fail();
    
    // Callbacks
    void setOnStart(std::function<void(Quest*)> callback) { onStart = callback; }
    void setOnComplete(std::function<void(Quest*)> callback) { onComplete = callback; }
    void setOnFail(std::function<void(Quest*)> callback) { onFail = callback; }
    
    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& data);
    
    // Static factory methods for famous One Piece quests
    static std::unique_ptr<Quest> createFindOnePiece();
    static std::unique_ptr<Quest> createRecruitZoro();
    static std::unique_ptr<Quest> createDefeatArlong();
    static std::unique_ptr<Quest> createSaveAce();
};