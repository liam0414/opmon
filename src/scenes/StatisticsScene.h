#pragma once
#include "core/Scene.h"
#include "graphics/Button.h"
#include <memory>
#include <vector>

class StatisticsScene : public Scene {
private:
    // UI Elements
    std::unique_ptr<Button> backButton;
    sf::Text titleText;
    sf::RectangleShape backgroundPanel;
    
    // Statistics display
    std::vector<sf::Text> statLabels;
    std::vector<sf::Text> statValues;
    std::vector<sf::RectangleShape> statBars;
    std::vector<sf::RectangleShape> statBarBackgrounds;
    
    void setupUI();
    void updateStatistics();
    void drawStatistic(sf::RenderWindow& window, const std::string& label, 
                      const std::string& value, int current, int max, 
                      const sf::Color& color, float x, float y);

public:
    StatisticsScene();
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};