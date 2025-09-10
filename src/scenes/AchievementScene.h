#pragma once
#include "core/Scene.h"
#include "graphics/Button.h"
#include <memory>
#include <vector>

struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    std::string iconTexture;
    bool unlocked;
    std::string unlockedDate;
    int progress;
    int maxProgress;
    int rewardExp;
    long rewardBerry;
    std::vector<std::string> rewardItems;
    
    Achievement(const std::string& achievementId, const std::string& achievementName, 
                const std::string& desc, int maxProg = 1)
        : id(achievementId), name(achievementName), description(desc), 
          unlocked(false), progress(0), maxProgress(maxProg), rewardExp(0), rewardBerry(0) {}
    
    float getProgressPercent() const {
        return maxProgress > 0 ? static_cast<float>(progress) / maxProgress : 0.0f;
    }
    
    bool isComplete() const { return progress >= maxProgress; }
};

enum class AchievementCategory {
    Story,
    Battle,
    Exploration,
    Collection,
    Social,
    Special
};

class AchievementScene : public Scene {
private:
    // UI Elements
    std::vector<std::unique_ptr<Button>> categoryButtons;
    std::unique_ptr<Button> backButton;
    
    sf::Text titleText;
    sf::Text statsText;
    sf::RectangleShape backgroundPanel;
    
    // Achievement display
    std::vector<sf::RectangleShape> achievementSlots;
    std::vector<sf::Sprite> achievementIcons;
    std::vector<sf::Text> achievementNames;
    std::vector<sf::Text> achievementDescs;
    std::vector<sf::RectangleShape> progressBars;
    std::vector<sf::RectangleShape> progressBarBgs;
    
    // Data
    std::vector<std::unique_ptr<Achievement>> achievements;
    AchievementCategory currentCategory;
    
    // Pagination
    int currentPage;
    int achievementsPerPage;
    
    void setupUI();
    void loadAchievements();
    void updateAchievementDisplay();
    void switchCategory(AchievementCategory category);
    void checkAchievementProgress();
    
    // Achievement creation
    void addAchievement(std::unique_ptr<Achievement> achievement);
    void createDefaultAchievements();

public:
    AchievementScene();
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    
    // Achievement management
    static void unlockAchievement(const std::string& achievementId);
    static void updateAchievementProgress(const std::string& achievementId, int progress);
};