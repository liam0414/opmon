#include "LoadGameScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "graphics/FontManager.h"
#include "game/GameStateManager.h"
#include "GameWorldScene.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

LoadGameScene::LoadGameScene() : selectedSave(-1) {
    setupUI();
    loadSaveFileList();
}

void LoadGameScene::onEnter() {
    LOG_INFO("Entered Load Game Scene");
    loadSaveFileList(); // Refresh the list
}

void LoadGameScene::onExit() {
    LOG_INFO("Exited Load Game Scene");
}

void LoadGameScene::setupUI() {
    auto& fontManager = FontManager::getInstance();
    
    // Background panel
    backgroundPanel.setSize(sf::Vector2f(900, 650));
    backgroundPanel.setPosition(62, 59);
    backgroundPanel.setFillColor(sf::Color(30, 30, 50, 240));
    backgroundPanel.setOutlineColor(sf::Color::White);
    backgroundPanel.setOutlineThickness(2);
    
    // Preview panel
    previewPanel.setSize(sf::Vector2f(350, 500));
    previewPanel.setPosition(580, 100);
    previewPanel.setFillColor(sf::Color(40, 40, 60, 200));
    previewPanel.setOutlineColor(sf::Color::Gray);
    previewPanel.setOutlineThickness(1);
    
    // Title
    titleText.setFont(fontManager.getDefaultFont());
    titleText.setString("LOAD GAME");
    titleText.setCharacterSize(36);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(80, 80);
    
    // Save info text
    saveInfoText.setFont(fontManager.getDefaultFont());
    saveInfoText.setCharacterSize(16);
    saveInfoText.setFillColor(sf::Color::White);
    saveInfoText.setPosition(590, 110);
    
    // Control buttons
    loadButton = std::make_unique<Button>("Load Game", fontManager.getDefaultFont());
    loadButton->setBounds(sf::FloatRect(80, 620, 120, 40));
    loadButton->setColors(sf::Color(0, 150, 0, 200), sf::Color(0, 180, 0, 220),
                         sf::Color(0, 120, 0, 240), sf::Color::Gray);
    loadButton->setOnClick([this]() { loadSelectedGame(); });
    loadButton->setInteractive(false);
    
    deleteButton = std::make_unique<Button>("Delete", fontManager.getDefaultFont());
    deleteButton->setBounds(sf::FloatRect(220, 620, 100, 40));
    deleteButton->setColors(sf::Color(150, 50, 50, 200), sf::Color(180, 70, 70, 220),
                           sf::Color(120, 30, 30, 240), sf::Color::Gray);
    deleteButton->setOnClick([this]() { deleteSelectedSave(); });
    deleteButton->setInteractive(false);
    
    backButton = std::make_unique<Button>("Back", fontManager.getDefaultFont());
    backButton->setBounds(sf::FloatRect(840, 620, 100, 40));
    backButton->setColors(sf::Color(100, 100, 100, 200), sf::Color(120, 120, 120, 220),
                         sf::Color(80, 80, 80, 240), sf::Color::Gray);
    backButton->setOnClick([this]() {
        Application::getInstance()->popScene();
    });
}

void LoadGameScene::loadSaveFileList() {
    saveFiles.clear();
    saveFileButtons.clear();
    
    auto availableSaves = GameStateManager::getInstance().getSaveFiles();
    
    float buttonHeight = 40;
    float buttonSpacing = 5;
    float startY = 140;
    
    for (size_t i = 0; i < availableSaves.size(); ++i) {
        SaveFileInfo saveInfo = parseSaveFile(availableSaves[i]);
        saveFiles.push_back(saveInfo);
        
        // Create button for this save file
        auto button = std::make_unique<Button>(saveInfo.displayName, FontManager::getInstance().getDefaultFont());
        button->setBounds(sf::FloatRect(80, startY + i * (buttonHeight + buttonSpacing), 480, buttonHeight));
        
        sf::Color normalColor(60, 60, 80, 200);
        sf::Color hoverColor(80, 80, 100, 220);
        sf::Color selectedColor(100, 150, 200, 220);
        
        button->setColors(normalColor, hoverColor, selectedColor, sf::Color::Gray);
        
        int saveIndex = i;
        button->setOnClick([this, saveIndex]() {
            selectedSave = saveIndex;
            updateSaveInfo();
            
            // Update button states
            for (size_t j = 0; j < saveFileButtons.size(); ++j) {
                if (j == selectedSave) {
                    saveFileButtons[j]->setState(UIState::Pressed);
                } else {
                    saveFileButtons[j]->setState(UIState::Normal);
                }
            }
            
            // Enable control buttons
            loadButton->setInteractive(true);
            loadButton->setState(UIState::Normal);
            deleteButton->setInteractive(true);
            deleteButton->setState(UIState::Normal);
        });
        
        saveFileButtons.push_back(std::move(button));
    }
    
    if (saveFiles.empty()) {
        saveInfoText.setString("No save files found.\nCreate a new game to get started!");
    } else {
        saveInfoText.setString("Select a save file to load");
    }
}

void LoadGameScene::updateSaveInfo() {
    if (selectedSave < 0 || selectedSave >= static_cast<int>(saveFiles.size())) {
        saveInfoText.setString("No save file selected");
        return;
    }
    
    const SaveFileInfo& save = saveFiles[selectedSave];
    
    std::string info = "=== " + save.displayName + " ===\n\n";
    info += "Last Modified: " + save.lastModified + "\n";
    info += "Playtime: " + save.playtime + "\n";
    info += "Level: " + std::to_string(save.level) + "\n";
    info += "Location: " + save.location + "\n";
    info += "Berry: " + std::to_string(save.berry) + " ฿\n\n";
    info += "Click 'Load Game' to continue\nthis adventure!";
    
    saveInfoText.setString(info);
}

void LoadGameScene::loadSelectedGame() {
    if (selectedSave < 0 || selectedSave >= static_cast<int>(saveFiles.size())) {
        LOG_WARN("No save file selected for loading");
        return;
    }
    
    const SaveFileInfo& save = saveFiles[selectedSave];
    
    if (GameStateManager::getInstance().loadGame(save.filename)) {
        LOG_INFO("Successfully loaded game: {}", save.displayName);
        Application::getInstance()->changeScene(std::make_unique<GameWorldScene>());
    } else {
        LOG_ERROR("Failed to load game: {}", save.displayName);
        // TODO: Show error message to player
    }
}

void LoadGameScene::deleteSelectedSave() {
    if (selectedSave < 0 || selectedSave >= static_cast<int>(saveFiles.size())) {
        LOG_WARN("No save file selected for deletion");
        return;
    }
    
    const SaveFileInfo& save = saveFiles[selectedSave];
    
    try {
        std::string fullPath = "saves/" + save.filename + ".json";
        if (std::filesystem::remove(fullPath)) {
            LOG_INFO("Deleted save file: {}", save.displayName);
            
            // Refresh the list
            selectedSave = -1;
            loadSaveFileList();
            
            // Disable control buttons
            loadButton->setInteractive(false);
            loadButton->setState(UIState::Disabled);
            deleteButton->setInteractive(false);
            deleteButton->setState(UIState::Disabled);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete save file {}: {}", save.displayName, e.what());
    }
}

SaveFileInfo LoadGameScene::parseSaveFile(const std::string& filename) {
    SaveFileInfo info;
    info.filename = filename;
    info.displayName = filename;
    info.lastModified = "Unknown";
    info.playtime = "00:00:00";
    info.level = 1;
    info.location = "Unknown";
    info.berry = 0;
    
    try {
        // Get file modification time
        std::string fullPath = "saves/" + filename + ".json";
        if (std::filesystem::exists(fullPath)) {
            auto ftime = std::filesystem::last_write_time(fullPath);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
            auto tt = std::chrono::system_clock::to_time_t(sctp);
            
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S");
            info.lastModified = oss.str();
            
            // Try to parse save file for more details
            std::ifstream file(fullPath);
            if (file.is_open()) {
                nlohmann::json saveData;
                file >> saveData;
                
                if (saveData.contains("player")) {
                    auto playerData = saveData["player"];
                    if (playerData.contains("name")) {
                        info.displayName = playerData["name"];
                    }
                    
                    if (playerData.contains("stats")) {
                        auto statsData = playerData["stats"]["baseStats"];
                        if (statsData.contains("1")) { // Level stat
                            info.level = statsData["1"];
                        }
                    }
                }
                
                if (saveData.contains("inventory")) {
                    auto inventoryData = saveData["inventory"];
                    if (inventoryData.contains("berry")) {
                        info.berry = inventoryData["berry"];
                    }
                }
                
                if (saveData.contains("playtime")) {
                    float playtimeSeconds = saveData["playtime"];
                    int hours = static_cast<int>(playtimeSeconds) / 3600;
                    int minutes = (static_cast<int>(playtimeSeconds) % 3600) / 60;
                    int seconds = static_cast<int>(playtimeSeconds) % 60;
                    
                    std::ostringstream timeStream;
                    timeStream << std::setfill('0') << std::setw(2) << hours << ":"
                              << std::setw(2) << minutes << ":" << std::setw(2) << seconds;
                    info.playtime = timeStream.str();
                }
                
                if (saveData.contains("world")) {
                    auto worldData = saveData["world"];
                    if (worldData.contains("currentLocationId")) {
                        info.location = worldData["currentLocationId"];
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_WARN("Failed to parse save file {}: {}", filename, e.what());
    }
    
    return info;
}

void LoadGameScene::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
    
    // Handle save file buttons
    for (auto& button : saveFileButtons) {
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
    
    // Handle control buttons
    std::vector<Button*> controlButtons = {loadButton.get(), deleteButton.get(), backButton.get()};
    
    for (auto* button : controlButtons) {
        if (button && button->isInteractive()) {
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
    }
    
    // Keyboard shortcuts
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Escape:
                Application::getInstance()->popScene();
                break;
            case sf::Keyboard::Enter:
                if (selectedSave >= 0) {
                    loadSelectedGame();
                }
                break;
            case sf::Keyboard::Delete:
                if (selectedSave >= 0) {
                    deleteSelectedSave();
                }
                break;
            case sf::Keyboard::Up:
                if (selectedSave > 0) {
                    selectedSave--;
                    saveFileButtons[selectedSave]->handleMouseClick(
                        sf::Vector2f(saveFileButtons[selectedSave]->getBounds().left + 10,
                                   saveFileButtons[selectedSave]->getBounds().top + 10), 
                        sf::Mouse::Left);
                    saveFileButtons[selectedSave]->handleMouseRelease(
                        sf::Vector2f(saveFileButtons[selectedSave]->getBounds().left + 10,
                                   saveFileButtons[selectedSave]->getBounds().top + 10), 
                        sf::Mouse::Left);
                }
                break;
            case sf::Keyboard::Down:
                if (selectedSave < static_cast<int>(saveFileButtons.size()) - 1) {
                    selectedSave++;
                    saveFileButtons[selectedSave]->handleMouseClick(
                        sf::Vector2f(saveFileButtons[selectedSave]->getBounds().left + 10,
                                   saveFileButtons[selectedSave]->getBounds().top + 10), 
                        sf::Mouse::Left);
                    saveFileButtons[selectedSave]->handleMouseRelease(
                        sf::Vector2f(saveFileButtons[selectedSave]->getBounds().left + 10,
                                   saveFileButtons[selectedSave]->getBounds().top + 10), 
                        sf::Mouse::Left);
                }
                break;
            default:
                break;
        }
    }
}

void LoadGameScene::update(float deltaTime) {
    // Update all buttons
    for (auto& button : saveFileButtons) {
        button->update(deltaTime);
    }
    
    if (loadButton) loadButton->update(deltaTime);
    if (deleteButton) deleteButton->update(deltaTime);
    if (backButton) backButton->update(deltaTime);
}

void LoadGameScene::render(sf::RenderWindow& window) {
    // Clear background
    window.clear(sf::Color(20, 25, 35));
    
    // Draw panels
    window.draw(backgroundPanel);
    window.draw(previewPanel);
    
    // Draw title
    window.draw(titleText);
    
    // Draw save file buttons
    for (auto& button : saveFileButtons) {
        window.draw(*button);
    }
    
    // Draw save info
    window.draw(saveInfoText);
    
    // Draw control buttons
    if (loadButton) window.draw(*loadButton);
    if (deleteButton) window.draw(*deleteButton);
    if (backButton) window.draw(*backButton);
    
    // Draw instructions
    sf::Text instructionText;
    instructionText.setFont(FontManager::getInstance().getDefaultFont());
    instructionText.setString("Up/Down arrows to navigate | Enter to load | Delete to remove | ESC to go back");
    instructionText.setCharacterSize(14);
    instructionText.setFillColor(sf::Color::Gray);
    instructionText.setPosition(80, 680);
    window.draw(instructionText);
}