#include "AchievementScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "graphics/FontManager.h"
#include "graphics/TextureManager.h"
#include "game/GameStateManager.h"
#include "game/EventSystem.h"
#include <ctime>
#include <iomanip>
#include <sstream>

AchievementScene::AchievementScene() 
    : currentCategory(AchievementCategory::Story), currentPage(0), achievementsPerPage(8) {
    
    setupUI();
    loadAchievements();
    createDefaultAchievements();
}

void AchievementScene::onEnter() {
    LOG_INFO("Opened achievements screen");
    checkAchievementProgress();
    updateAchievementDisplay();
}

void AchievementScene::onExit() {
    LOG_INFO("Closed achievements screen");
}

void AchievementScene::setupUI() {
    auto& fontManager = FontManager::getInstance();
    
    // Background
    backgroundPanel.setSize(sf::Vector2f(980, 700));
    backgroundPanel.setPosition(22, 34);
    backgroundPanel.setFillColor(sf::Color(20, 25, 35, 240));
    backgroundPanel.setOutlineColor(sf::Color(100, 150, 200));
    backgroundPanel.setOutlineThickness(3);
    
    // Title
    titleText.setFont(fontManager.getDefaultFont());
    titleText.setString("ACHIEVEMENTS");
    titleText.setCharacterSize(36);
    titleText.setFillColor(sf::Color(255, 215, 0)); // Gold
    titleText.setPosition(50, 50);
    
    // Stats text
    statsText.setFont(fontManager.getDefaultFont());
    statsText.setCharacterSize(18);
    statsText.setFillColor(sf::Color::White);
    statsText.setPosition(400, 60);
    
    // Category buttons
    std::vector<std::string> categories = {"Story", "Battle", "Explore", "Collect", "Social", "Special"};
    std::vector<AchievementCategory> categoryTypes = {
        AchievementCategory::Story, AchievementCategory::Battle, AchievementCategory::Exploration,
        AchievementCategory::Collection, AchievementCategory::Social, AchievementCategory::Special
    };
    
    float buttonWidth = 120;
    float buttonHeight = 35;
    float startY = 100;
    
    for (size_t i = 0; i < categories.size(); ++i) {
        auto button = std::make_unique<Button>(categories[i], fontManager.getDefaultFont());
        button->setBounds(sf::FloatRect(50 + i * (buttonWidth + 10), startY, buttonWidth, buttonHeight));
        
        sf::Color normalColor(60, 70, 90, 200);
        sf::Color hoverColor(80, 90, 110, 220);
        sf::Color selectedColor(100, 120, 160, 240);
        
        button->setColors(normalColor, hoverColor, selectedColor, sf::Color::Gray);
        
        AchievementCategory category = categoryTypes[i];
        button->setOnClick([this, category]() {
            switchCategory(category);
        });
        
        categoryButtons.push_back(std::move(button));
    }
    
    // Set first category as selected
    categoryButtons[0]->setState(UIState::Pressed);
    
    // Back button
    backButton = std::make_unique<Button>("Back", fontManager.getDefaultFont());
    backButton->setBounds(sf::FloatRect(880, 680, 100, 40));
    backButton->setColors(sf::Color(100, 50, 50, 200), sf::Color(120, 70, 70, 220),
                         sf::Color(80, 30, 30, 240), sf::Color::Gray);
    backButton->setOnClick([this]() {
        Application::getInstance()->popScene();
    });
    
    // Create achievement display slots (2x4 grid)
    float slotWidth = 450;
    float slotHeight = 80;
    float slotSpacingX = 470;
    float slotSpacingY = 90;
    float startX = 50;
    float startYSlots = 160;
    
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 2; ++col) {
            // Achievement slot background
            sf::RectangleShape slot;
            slot.setSize(sf::Vector2f(slotWidth, slotHeight));
            slot.setPosition(startX + col * slotSpacingX, startYSlots + row * slotSpacingY);
            slot.setFillColor(sf::Color(40, 45, 55, 200));
            slot.setOutlineColor(sf::Color::Gray);
            slot.setOutlineThickness(1);
            achievementSlots.push_back(slot);
            
            // Achievement icon placeholder
            sf::Sprite icon;
            icon.setPosition(startX + col * slotSpacingX + 10, startYSlots + row * slotSpacingY + 10);
            achievementIcons.push_back(icon);
            
            // Achievement name
            sf::Text nameText;
            nameText.setFont(fontManager.getDefaultFont());
            nameText.setCharacterSize(16);
            nameText.setFillColor(sf::Color::White);
            nameText.setPosition(startX + col * slotSpacingX + 70, startYSlots + row * slotSpacingY + 10);
            achievementNames.push_back(nameText);
            
            // Achievement description
            sf::Text descText;
            descText.setFont(fontManager.getDefaultFont());
            descText.setCharacterSize(12);
            descText.setFillColor(sf::Color::Gray);
            descText.setPosition(startX + col * slotSpacingX + 70, startYSlots + row * slotSpacingY + 30);
            achievementDescs.push_back(descText);
            
            // Progress bar background
            sf::RectangleShape progressBg;
            progressBg.setSize(sf::Vector2f(300, 8));
            progressBg.setPosition(startX + col * slotSpacingX + 70, startYSlots + row * slotSpacingY + 55);
            progressBg.setFillColor(sf::Color(30, 30, 30));
            progressBarBgs.push_back(progressBg);
            
            // Progress bar
            sf::RectangleShape progressBar;
            progressBar.setSize(sf::Vector2f(300, 8));
            progressBar.setPosition(startX + col * slotSpacingX + 70, startYSlots + row * slotSpacingY + 55);
            progressBar.setFillColor(sf::Color::Green);
            progressBars.push_back(progressBar);
        }
    }
}

void AchievementScene::loadAchievements() {
    // In a full implementation, this would load from save file or configuration
    // For now, we'll create achievements dynamically
}

void AchievementScene::updateAchievementDisplay() {
    // Filter achievements by category
    std::vector<Achievement*> filteredAchievements;
    for (auto& achievement : achievements) {
        // Simple category filtering based on achievement ID prefix
        bool matches = false;
        switch (currentCategory) {
            case AchievementCategory::Story:
                matches = achievement->id.find("story_") == 0;
                break;
            case AchievementCategory::Battle:
                matches = achievement->id.find("battle_") == 0;
                break;
            case AchievementCategory::Exploration:
                matches = achievement->id.find("explore_") == 0;
                break;
            case AchievementCategory::Collection:
                matches = achievement->id.find("collect_") == 0;
                break;
            case AchievementCategory::Social:
                matches = achievement->id.find("social_") == 0;
                break;
            case AchievementCategory::Special:
                matches = achievement->id.find("special_") == 0;
                break;
        }
        
        if (matches) {
            filteredAchievements.push_back(achievement.get());
        }
    }
    
    // Clear current display
    for (size_t i = 0; i < achievementSlots.size(); ++i) {
        achievementSlots[i].setFillColor(sf::Color(40, 45, 55, 200));
        achievementNames[i].setString("");
        achievementDescs[i].setString("");
        progressBars[i].setSize(sf::Vector2f(0, 8));
        
        // Clear icon
        achievementIcons[i].setTexture(nullptr);
    }
    
    // Display achievements for current page
    size_t startIndex = currentPage * achievementsPerPage;
    size_t endIndex = std::min(startIndex + achievementsPerPage, filteredAchievements.size());
    
    auto& textureManager = TextureManager::getInstance();
    
    for (size_t i = startIndex; i < endIndex; ++i) {
        size_t slotIndex = i - startIndex;
        if (slotIndex >= achievementSlots.size()) break;
        
        Achievement* achievement = filteredAchievements[i];
        
        // Set colors based on completion status
        if (achievement->unlocked) {
            achievementSlots[slotIndex].setFillColor(sf::Color(60, 80, 60, 200)); // Green tint
            achievementSlots[slotIndex].setOutlineColor(sf::Color::Green);
            achievementSlots[slotIndex].setOutlineThickness(2);
        } else if (achievement->isComplete()) {
            achievementSlots[slotIndex].setFillColor(sf::Color(80, 80, 60, 200)); // Yellow tint
            achievementSlots[slotIndex].setOutlineColor(sf::Color::Yellow);
        }
        
        // Set achievement name
        achievementNames[slotIndex].setString(achievement->name);
        achievementNames[slotIndex].setFillColor(achievement->unlocked ? sf::Color::White : sf::Color::Gray);
        
        // Set description
        std::string desc = achievement->description;
        if (!achievement->unlocked && achievement->maxProgress > 1) {
            desc += " (" + std::to_string(achievement->progress) + "/" + std::to_string(achievement->maxProgress) + ")";
        }
        achievementDescs[slotIndex].setString(desc);
        
        // Set progress bar
        float progressPercent = achievement->getProgressPercent();
        progressBars[slotIndex].setSize(sf::Vector2f(300 * progressPercent, 8));
        
        if (achievement->unlocked) {
            progressBars[slotIndex].setFillColor(sf::Color::Green);
        } else if (progressPercent > 0.5f) {
            progressBars[slotIndex].setFillColor(sf::Color::Yellow);
        } else {
            progressBars[slotIndex].setFillColor(sf::Color::Red);
        }
        
        // Load icon if available
        if (!achievement->iconTexture.empty()) {
            std::string iconTextureName = "achievement_icon_" + achievement->id;
            if (textureManager.loadTexture(iconTextureName, achievement->iconTexture)) {
                achievementIcons[slotIndex].setTexture(textureManager.getTexture(iconTextureName));
                
                // Scale icon to fit
                sf::Vector2u iconSize = textureManager.getTexture(iconTextureName).getSize();
                float scale = std::min(50.0f / iconSize.x, 50.0f / iconSize.y);
                achievementIcons[slotIndex].setScale(scale, scale);
            }
        }
    }
    
    // Update stats display
    int totalAchievements = achievements.size();
    int unlockedAchievements = 0;
    for (const auto& achievement : achievements) {
        if (achievement->unlocked) unlockedAchievements++;
    }
    
    float completionPercent = totalAchievements > 0 ? 
        (static_cast<float>(unlockedAchievements) / totalAchievements * 100.0f) : 0.0f;
    
    std::ostringstream statsStream;
    statsStream << "Completion: " << unlockedAchievements << "/" << totalAchievements 
                << " (" << std::fixed << std::setprecision(1) << completionPercent << "%)";
    statsText.setString(statsStream.str());
}

void AchievementScene::switchCategory(AchievementCategory category) {
    currentCategory = category;
    currentPage = 0;
    
    // Update button states
    for (size_t i = 0; i < categoryButtons.size(); ++i) {
        if (static_cast<AchievementCategory>(i) == category) {
            categoryButtons[i]->setState(UIState::Pressed);
        } else {
            categoryButtons[i]->setState(UIState::Normal);
        }
    }
    
    updateAchievementDisplay();
}

void AchievementScene::checkAchievementProgress() {
    auto& gameState = GameStateManager::getInstance();
    const auto& stats = gameState.getStats();
    
    // Update achievement progress based on game statistics
    for (auto& achievement : achievements) {
        if (achievement->unlocked) continue;
        
        // Update progress based on achievement type
        if (achievement->id == "battle_first_victory") {
            achievement->progress = std::min(1, stats.battlesWon);
        } else if (achievement->id == "battle_win_10") {
            achievement->progress = std::min(10, stats.battlesWon);
        } else if (achievement->id == "battle_win_100") {
            achievement->progress = std::min(100, stats.battlesWon);
        } else if (achievement->id == "collect_1000000_berry") {
            achievement->progress = std::min(1000000L, stats.berryEarned);
        } else if (achievement->id == "social_recruit_5") {
            achievement->progress = std::min(5, stats.crewMembersRecruited);
        } else if (achievement->id == "story_complete_10_quests") {
            achievement->progress = std::min(10, stats.questsCompleted);
        } else if (achievement->id == "explore_discover_10_locations") {
            achievement->progress = std::min(10, stats.locationsDiscovered);
        }
        
        // Check if achievement should be unlocked
        if (achievement->isComplete() && !achievement->unlocked) {
            unlockAchievement(achievement->id);
        }
    }
}

void AchievementScene::addAchievement(std::unique_ptr<Achievement> achievement) {
    achievements.push_back(std::move(achievement));
}

void AchievementScene::createDefaultAchievements() {
    // Story achievements
    auto firstVictory = std::make_unique<Achievement>("story_first_battle", "First Victory", 
                                                     "Win your first battle");
    firstVictory->iconTexture = "assets/textures/achievements/first_victory.png";
    firstVictory->rewardExp = 100;
    firstVictory->rewardBerry = 500;
    addAchievement(std::move(firstVictory));
    
    auto questMaster = std::make_unique<Achievement>("story_complete_10_quests", "Quest Master", 
                                                    "Complete 10 quests", 10);
    questMaster->iconTexture = "assets/textures/achievements/quest_master.png";
    questMaster->rewardExp = 1000;
    questMaster->rewardBerry = 5000;
    addAchievement(std::move(questMaster));
    
    // Battle achievements
    auto battleWinner = std::make_unique<Achievement>("battle_win_10", "Battle Winner", 
                                                     "Win 10 battles", 10);
    battleWinner->iconTexture = "assets/textures/achievements/battle_winner.png";
    battleWinner->rewardExp = 500;
    battleWinner->rewardBerry = 2000;
    addAchievement(std::move(battleWinner));
    
    auto battleLegend = std::make_unique<Achievement>("battle_win_100", "Battle Legend", 
                                                     "Win 100 battles", 100);
    battleLegend->iconTexture = "assets/textures/achievements/battle_legend.png";
    battleLegend->rewardExp = 5000;
    battleLegend->rewardBerry = 25000;
    addAchievement(std::move(battleLegend));
    
    // Collection achievements
    auto richPirate = std::make_unique<Achievement>("collect_1000000_berry", "Rich Pirate", 
                                                   "Earn 1,000,000 Berry", 1000000);
    richPirate->iconTexture = "assets/textures/achievements/rich_pirate.png";
    richPirate->rewardExp = 2000;
    addAchievement(std::move(richPirate));
    
    // Social achievements
    auto crewBuilder = std::make_unique<Achievement>("social_recruit_5", "Crew Builder", 
                                                    "Recruit 5 crew members", 5);
    crewBuilder->iconTexture = "assets/textures/achievements/crew_builder.png";
    crewBuilder->rewardExp = 1500;
    crewBuilder->rewardBerry = 10000;
    addAchievement(std::move(crewBuilder));
    
    // Exploration achievements
    auto explorer = std::make_unique<Achievement>("explore_discover_10_locations", "Explorer", 
                                                 "Discover 10 new locations", 10);
    explorer->iconTexture = "assets/textures/achievements/explorer.png";
    explorer->rewardExp = 800;
    explorer->rewardBerry = 3000;
    addAchievement(std::move(explorer));
    
    // Special achievements
    auto devilFruitUser = std::make_unique<Achievement>("special_devil_fruit", "Devil Fruit User", 
                                                       "Acquire your first Devil Fruit");
    devilFruitUser->iconTexture = "assets/textures/achievements/devil_fruit_user.png";
    devilFruitUser->rewardExp = 2000;
    devilFruitUser->rewardBerry = 10000;
    addAchievement(std::move(devilFruitUser));
    
    LOG_INFO("Created {} default achievements", achievements.size());
}

void AchievementScene::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
    
    // Handle category buttons
    for (auto& button : categoryButtons) {
        if (event.type == sf::Event::MouseMoved) {
            button->handleMouseMove(mousePos);
        }
        else if (event.type == sf::Event::MouseButtonPressed) {
            button->handleMouseClick(mousePos, event.mouseButton.button);
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            button->handleMouseRelease(mousePos, event.mouseButton.button);
        }
    }
    
    // Handle back button
    if (backButton) {
        if (event.type == sf::Event::MouseMoved) {
            backButton->handleMouseMove(mousePos);
        }
        else if (event.type == sf::Event::MouseButtonPressed) {
            backButton->handleMouseClick(mousePos, event.mouseButton.button);
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            backButton->handleMouseRelease(mousePos, event.mouseButton.button);
        }
    }
    
    // Keyboard shortcuts
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Escape:
                Application::getInstance()->popScene();
                break;
            case sf::Keyboard::Left:
                if (currentPage > 0) {
                    currentPage--;
                    updateAchievementDisplay();
                }
                break;
            case sf::Keyboard::Right:
                // Check if there are more achievements to show
                // This would need proper pagination logic
                currentPage++;
                updateAchievementDisplay();
                break;
            case sf::Keyboard::Num1:
            case sf::Keyboard::Num2:
            case sf::Keyboard::Num3:
            case sf::Keyboard::Num4:
            case sf::Keyboard::Num5:
            case sf::Keyboard::Num6: {
                int categoryIndex = event.key.code - sf::Keyboard::Num1;
                if (categoryIndex < static_cast<int>(categoryButtons.size())) {
                    switchCategory(static_cast<AchievementCategory>(categoryIndex));
                }
                break;
            }
            default:
                break;
        }
    }
}

void AchievementScene::update(float deltaTime) {
    // Update buttons
    for (auto& button : categoryButtons) {
        button->update(deltaTime);
    }
    
    if (backButton) {
        backButton->update(deltaTime);
    }
    
    // Periodically check achievement progress
    static float checkTimer = 0;
    checkTimer += deltaTime;
    if (checkTimer >= 1.0f) { // Check every second
        checkAchievementProgress();
        checkTimer = 0;
    }
}

void AchievementScene::render(sf::RenderWindow& window) {
    // Clear background
    window.clear(sf::Color(10, 15, 20));
    
    // Draw background panel
    window.draw(backgroundPanel);
    
    // Draw title and stats
    window.draw(titleText);
    window.draw(statsText);
    
    // Draw category buttons
    for (auto& button : categoryButtons) {
        window.draw(*button);
    }
    
    // Draw achievement slots
    for (size_t i = 0; i < achievementSlots.size(); ++i) {
        window.draw(achievementSlots[i]);
        
        // Draw progress bar background and bar
        window.draw(progressBarBgs[i]);
        window.draw(progressBars[i]);
        
        // Draw icon if available
        if (achievementIcons[i].getTexture()) {
            window.draw(achievementIcons[i]);
        }
        
        // Draw text
        window.draw(achievementNames[i]);
        window.draw(achievementDescs[i]);
    }
    
    // Draw back button
    if (backButton) {
        window.draw(*backButton);
    }
    
    // Draw instructions
    sf::Text instructionText;
    instructionText.setFont(FontManager::getInstance().getDefaultFont());
    instructionText.setString("1-6: Switch categories | Left/Right: Navigate pages | ESC: Close");
    instructionText.setCharacterSize(12);
    instructionText.setFillColor(sf::Color::Gray);
    instructionText.setPosition(50, 740);
    window.draw(instructionText);
}

void AchievementScene::unlockAchievement(const std::string& achievementId) {
    // This would be called from anywhere in the game when an achievement is earned
    auto& eventSystem = EventSystem::getInstance();
    
    // Find the achievement
    // In a real implementation, this would access the global achievement manager
    LOG_INFO("🏆 Achievement unlocked: {}", achievementId);
    
    // Emit achievement unlock event
    eventSystem.emit(EventType::Custom, "achievement_unlocked", "achievementId", achievementId);
}

void AchievementScene::updateAchievementProgress(const std::string& achievementId, int progress) {
    // This would be called to update progress on multi-step achievements
    LOG_DEBUG("Achievement progress updated: {} = {}", achievementId, progress);
}