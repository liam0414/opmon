#pragma once
#include <SFML/Graphics.hpp>
#include "../ui/Button.h"
#include <vector>
#include <memory>

class MainMenuScene {
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Font subtitleFont;
    sf::Text titleText;
    sf::Text versionText;
    sf::Text subtitleText;
    
    // Background elements
    sf::RectangleShape background;
    sf::RectangleShape titleBackground;
    std::vector<sf::CircleShape> backgroundParticles;
    
    // UI elements
    std::vector<std::unique_ptr<Button>> buttons;
    
    // Animation
    float animationTime;
    float titlePulse;
    
    void setupUI();
    void setupBackground();
    void createParticles();
    void handleEvents();
    void update();
    void render();
    void updateAnimations(float deltaTime);
    void updateParticles(float deltaTime);
    
    // Button callbacks
    void onNewGame();
    void onLoadGame();
    void onSettings();
    void onQuit();

public:
    MainMenuScene();
    void run();
};