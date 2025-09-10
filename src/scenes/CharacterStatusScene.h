#pragma once
#include "core/Scene.h"
#include "graphics/Button.h"
#include "characters/Character.h"
#include <memory>
#include <vector>

class CharacterStatusScene : public Scene {
private:
    // UI Elements
    std::vector<std::unique_ptr<Button>> characterTabs;
    std::unique_ptr<Button> backButton;
    
    sf::Text titleText;
    sf::Text characterInfoText;
    sf::Text statsText;
    sf::Text devilFruitText;
    sf::Text abilitiesText;
    
    // Character display
    sf::Sprite characterPortrait;
    sf::RectangleShape backgroundPanel;
    sf::RectangleShape portraitFrame;
    
    // Stats bars
    std::vector<sf::RectangleShape> statBars;
    std::vector<sf::RectangleShape> statBarBackgrounds;
    std::vector<sf::Text> statLabels;
    
    // Selection
    int selectedCharacter;
    
    void setupUI();
    void updateCharacterDisplay();
    void drawStatBar(sf::RenderWindow& window, const std::string& label, int current, int max, 
                    const sf::Color& color, float x, float y, float width = 200);

public:
    CharacterStatusScene();
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};