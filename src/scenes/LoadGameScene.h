#pragma once
#include "core/Scene.h"
#include "graphics/Button.h"
#include <memory>
#include <vector>

struct SaveFileInfo {
    std::string filename;
    std::string displayName;
    std::string lastModified;
    std::string playtime;
    int level;
    std::string location;
    long berry;
};

class LoadGameScene : public Scene {
private:
    // UI Elements
    std::vector<std::unique_ptr<Button>> saveFileButtons;
    std::unique_ptr<Button> loadButton;
    std::unique_ptr<Button> deleteButton;
    std::unique_ptr<Button> backButton;
    
    sf::Text titleText;
    sf::Text saveInfoText;
    sf::RectangleShape backgroundPanel;
    sf::RectangleShape previewPanel;
    
    // Save file data
    std::vector<SaveFileInfo> saveFiles;
    int selectedSave;
    
    void setupUI();
    void loadSaveFileList();
    void updateSaveInfo();
    void loadSelectedGame();
    void deleteSelectedSave();
    SaveFileInfo parseSaveFile(const std::string& filename);

public:
    LoadGameScene();
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};