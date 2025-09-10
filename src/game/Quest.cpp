#include "Quest.h"
#include "core/Logger.h"
#include <algorithm>
#include <numeric>

void QuestObjective::updateProgress(int amount) {
    currentAmount = std::min(requiredAmount, currentAmount + amount);
    completed = (currentAmount >= requiredAmount);
    
    LOG_DEBUG("Quest objective updated: {} ({}/{})", description, currentAmount, requiredAmount);
}

float QuestObjective::getProgress() const {
    if (requiredAmount <= 0) return completed ? 1.0f : 0.0f;
    return static_cast<float>(currentAmount) / requiredAmount;
}

Quest::Quest(const std::string& questId, const std::string& questTitle, QuestType questType)
    : id(questId), title(questTitle), type(questType), status(QuestStatus::NotStarted),
      requiredLevel(1) {
}

void Quest::setQuestGiver(const std::string& giverId, const std::string& giverName) {
    questGiverId = giverId;
    questGiverName = giverName;
}

bool Quest::canStart(int playerLevel, const std::string& currentLocation, 
                    const std::vector<std::string>& completedQuests) const {
    
    if (status != QuestStatus::NotStarted) return false;
    
    // Check level requirement
    if (playerLevel < requiredLevel) return false;
    
    // Check location requirement
    if (!requiredLocation.empty() && currentLocation != requiredLocation) return false;
    
    // Check prerequisite quests
    for (const std::string& reqQuest : requiredQuests) {
        if (std::find(completedQuests.begin(), completedQuests.end(), reqQuest) == completedQuests.end()) {
            return false;
        }
    }
    
    return true;
}

void Quest::addObjective(const QuestObjective& objective) {
    objectives.push_back(objective);
    LOG_DEBUG("Added objective to quest '{}': {}", title, objective.description);
}

void Quest::updateObjective(const std::string& type, const std::string& target, int amount) {
    for (auto& objective : objectives) {
        if (objective.type == type && objective.target == target && !objective.completed) {
            objective.updateProgress(amount);
            
            if (objective.completed) {
                LOG_INFO("✅ Quest objective completed: {}", objective.description);
            }
            
            // Check if all objectives are complete
            if (areObjectivesComplete() && status == QuestStatus::Active) {
                complete();
            }
            break;
        }
    }
}

bool Quest::areObjectivesComplete() const {
    return std::all_of(objectives.begin(), objectives.end(),
        [](const QuestObjective& obj) { return obj.completed; });
}

float Quest::getOverallProgress() const {
    if (objectives.empty()) return 0.0f;
    
    float totalProgress = std::accumulate(objectives.begin(), objectives.end(), 0.0f,
        [](float sum, const QuestObjective& obj) { return sum + obj.getProgress(); });
    
    return totalProgress / objectives.size();
}

void Quest::start() {
    if (status == QuestStatus::NotStarted) {
        status = QuestStatus::Active;
        LOG_INFO("🎯 Quest started: {}", title);
        
        if (onStart) {
            onStart(this);
        }
    }
}

void Quest::complete() {
    if (status == QuestStatus::Active) {
        status = QuestStatus::Completed;
        LOG_INFO("🎉 Quest completed: {}", title);
        
        if (onComplete) {
            onComplete(this);
        }
    }
}

void Quest::fail() {
    if (status == QuestStatus::Active) {
        status = QuestStatus::Failed;
        LOG_WARN("❌ Quest failed: {}", title);
        
        if (onFail) {
            onFail(this);
        }
    }
}

nlohmann::json Quest::toJson() const {
    nlohmann::json data = {
        {"id", id},
        {"title", title},
        {"description", description},
        {"type", static_cast<int>(type)},
        {"status", static_cast<int>(status)},
        {"requiredLevel", requiredLevel},
        {"requiredLocation", requiredLocation},
        {"questGiverId", questGiverId},
        {"questGiverName", questGiverName},
        {"requiredQuests", requiredQuests}
    };
    
    // Objectives
    data["objectives"] = nlohmann::json::array();
    for (const auto& obj : objectives) {
        data["objectives"].push_back({
            {"description", obj.description},
            {"type", obj.type},
            {"target", obj.target},
            {"requiredAmount", obj.requiredAmount},
            {"currentAmount", obj.currentAmount},
            {"completed", obj.completed}
        });
    }
    
    // Reward
    data["reward"] = {
        {"experience", reward.experience},
        {"berry", reward.berry},
        {"items", reward.items},
        {"unlockLocation", reward.unlockLocation},
        {"unlockCharacter", reward.unlockCharacter}
    };
    
    return data;
}

void Quest::fromJson(const nlohmann::json& data) {
    id = data.value("id", "");
    title = data.value("title", "");
    description = data.value("description", "");
    type = static_cast<QuestType>(data.value("type", 0));
    status = static_cast<QuestStatus>(data.value("status", 0));
    requiredLevel = data.value("requiredLevel", 1);
    requiredLocation = data.value("requiredLocation", "");
    questGiverId = data.value("questGiverId", "");
    questGiverName = data.value("questGiverName", "");
    
    if (data.contains("requiredQuests")) {
        requiredQuests = data["requiredQuests"];
    }
    
    // Objectives
    objectives.clear();
    if (data.contains("objectives")) {
        for (const auto& objData : data["objectives"]) {
            QuestObjective obj(
                objData.value("description", ""),
                objData.value("type", ""),
                objData.value("target", ""),
                objData.value("requiredAmount", 1)
            );
            obj.currentAmount = objData.value("currentAmount", 0);
            obj.completed = objData.value("completed", false);
            objectives.push_back(obj);
        }
    }
    
    // Reward
    if (data.contains("reward")) {
        const auto& rewardData = data["reward"];
        reward.experience = rewardData.value("experience", 0);
        reward.berry = rewardData.value("berry", 0L);
        reward.unlockLocation = rewardData.value("unlockLocation", "");
        reward.unlockCharacter = rewardData.value("unlockCharacter", "");
        
        if (rewardData.contains("items")) {
            reward.items = rewardData["items"];
        }
    }
}

std::unique_ptr<Quest> Quest::createFindOnePiece() {
    auto quest = std::make_unique<Quest>("find_one_piece", "Find the One Piece", QuestType::Main);
    quest->setDescription("The ultimate treasure left by the Pirate King Gol D. Roger. Reach Laugh Tale and claim the One Piece!");
    quest->setRequiredLevel(50);
    
    quest->addObjective(QuestObjective("Gather all four Road Poneglyphs", "collect", "road_poneglyph", 4));
    quest->addObjective(QuestObjective("Reach Laugh Tale", "visit", "laugh_tale", 1));
    quest->addObjective(QuestObjective("Become Pirate King worthy", "achieve", "pirate_king_status", 1));
    
    QuestReward reward;
    reward.experience = 10000;
    reward.berry = 5000000000L;
    reward.items = {"one_piece_treasure", "pirate_king_crown"};
    quest->setReward(reward);
    
    return quest;
}

std::unique_ptr<Quest> Quest::createRecruitZoro() {
    auto quest = std::make_unique<Quest>("recruit_zoro", "Recruit the Pirate Hunter", QuestType::Side);
    quest->setDescription("Help Roronoa Zoro escape from the Marine base and recruit him to your crew.");
    quest->setRequiredLocation("shells_town");
    
    quest->addObjective(QuestObjective("Find Zoro at the Marine base", "talk", "zoro", 1));
    quest->addObjective(QuestObjective("Defeat Captain Morgan", "defeat", "captain_morgan", 1));
    quest->addObjective(QuestObjective("Free Zoro from his restraints", "interact", "zoro_restraints", 1));
    
    QuestReward reward;
    reward.experience = 500;
    reward.berry = 50000;
    reward.unlockCharacter = "zoro";
    quest->setReward(reward);
    
    return quest;
}

std::unique_ptr<Quest> Quest::createDefeatArlong() {
    auto quest = std::make_unique<Quest>("defeat_arlong", "Free Cocoyasi Village", QuestType::Main);
    quest->setDescription("Defeat the fish-man pirate Arlong and free Nami's village from his tyranny.");
    quest->setRequiredLocation("cocoyasi_village");
    quest->setRequiredLevel(8);
    
    quest->addObjective(QuestObjective("Investigate Arlong Park", "visit", "arlong_park", 1));
    quest->addObjective(QuestObjective("Defeat Arlong's officers", "defeat", "arlong_officer", 3));
    quest->addObjective(QuestObjective("Defeat Arlong", "defeat", "arlong", 1));
    quest->addObjective(QuestObjective("Destroy the room with Nami's maps", "destroy", "map_room", 1));
    
    QuestReward reward;
    reward.experience = 1000;
    reward.berry = 200000;
    reward.items = {"arlong_sword", "navigator_tools"};
    reward.unlockCharacter = "nami";
    quest->setReward(reward);
    
    return quest;
}