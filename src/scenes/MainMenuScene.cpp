#include "MainMenuScene.h"
#include <iostream>
#include <cmath>
#include <random>

#include "MainMenuScene.h"
#include <iostream>
#include <cmath>
#include <random>

MainMenuScene::MainMenuScene() : window(sf::VideoMode(1280, 720), "OPMON Red"), animationTime(0), titlePulse(0) {
    window.setFramerateLimit(60);
    setupBackground();
    setupUI();
    createParticles();
}

void MainMenuScene::setupBackground() {
    // Create gradient background
    background.setSize(sf::Vector2f(1280, 720));
    background.setFillColor(sf::Color(15, 25, 45)); // Deep blue
    
    // Title background panel
    titleBackground.setSize(sf::Vector2f(800, 120));
    titleBackground.setPosition(240, 120);
    titleBackground.setFillColor(sf::Color(20, 30, 50, 180));
    titleBackground.setOutlineThickness(3);
    titleBackground.setOutlineColor(sf::Color(100, 150, 200, 100));
}

void MainMenuScene::createParticles() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(0, 1280);
    std::uniform_real_distribution<float> yDist(0, 720);
    std::uniform_real_distribution<float> sizeDist(1, 4);
    
    backgroundParticles.clear();
    for (int i = 0; i < 50; ++i) {
        sf::CircleShape particle;
        particle.setRadius(sizeDist(gen));
        particle.setPosition(xDist(gen), yDist(gen));
        particle.setFillColor(sf::Color(100, 150, 200, 50));
        backgroundParticles.push_back(particle);
    }
}

void MainMenuScene::setupUI() {
    // Load font
    if (!font.loadFromFile("assets/fonts/arial.ttf") or !subtitleFont.loadFromFile("assets/fonts/Mplus1-Regular.ttf")) {
        std::cout << "Warning: Could not load font. Using default font.\n";
    }
    
    // Main title
    titleText.setFont(font);
    titleText.setString("OPMON RED");
    titleText.setCharacterSize(64);
    titleText.setFillColor(sf::Color(255, 215, 0)); // Gold
    titleText.setStyle(sf::Text::Bold);
    
    // Add text outline
    titleText.setOutlineThickness(3);
    titleText.setOutlineColor(sf::Color(139, 69, 19)); // Saddle brown
    
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition((1280 - titleBounds.width) / 2, 140);
    
    // Subtitle
    subtitleText.setFont(subtitleFont);
    subtitleText.setString("海賊王に俺はなる！");
    subtitleText.setCharacterSize(24);
    subtitleText.setFillColor(sf::Color(200, 200, 255));
    subtitleText.setStyle(sf::Text::Italic);
    
    sf::FloatRect subtitleBounds = subtitleText.getLocalBounds();
    subtitleText.setPosition((1280 - subtitleBounds.width) / 2, 200);
    
    // Version text
    versionText.setFont(font);
    versionText.setString("v0.1.0 - Development Build");
    versionText.setCharacterSize(18);
    versionText.setFillColor(sf::Color(150, 150, 150, 200));
    versionText.setPosition(20, 680);
    
    // Create enhanced buttons
    buttons.clear();
    
    float buttonWidth = 250;
    float buttonHeight = 60;
    float buttonX = (1280 - buttonWidth) / 2;
    float startY = 320;
    float spacing = 80;
    
    // New Game button
    auto newGameBtn = std::make_unique<Button>("New Game", font, buttonX, startY, buttonWidth, buttonHeight);
    newGameBtn->setColors(
        sf::Color(60, 120, 180, 220),   // Normal: Blue
        sf::Color(80, 140, 200, 240),   // Hover: Lighter blue
        sf::Color(40, 100, 160, 255)    // Press: Darker blue
    );
    newGameBtn->setOnClick([this]() { onNewGame(); });
    buttons.push_back(std::move(newGameBtn));
    
    // Load Game button
    auto loadGameBtn = std::make_unique<Button>("Load Game", font, buttonX, startY + spacing, buttonWidth, buttonHeight);
    loadGameBtn->setColors(
        sf::Color(80, 120, 80, 220),    // Normal: Green
        sf::Color(100, 140, 100, 240),  // Hover: Lighter green
        sf::Color(60, 100, 60, 255)     // Press: Darker green
    );
    loadGameBtn->setOnClick([this]() { onLoadGame(); });
    buttons.push_back(std::move(loadGameBtn));
    
    // Settings button
    auto settingsBtn = std::make_unique<Button>("Settings", font, buttonX, startY + spacing * 2, buttonWidth, buttonHeight);
    settingsBtn->setColors(
        sf::Color(120, 80, 120, 220),   // Normal: Purple
        sf::Color(140, 100, 140, 240),  // Hover: Lighter purple
        sf::Color(100, 60, 100, 255)    // Press: Darker purple
    );
    settingsBtn->setOnClick([this]() { onSettings(); });
    buttons.push_back(std::move(settingsBtn));
    
    // Quit button
    auto quitBtn = std::make_unique<Button>("Quit", font, buttonX, startY + spacing * 3, buttonWidth, buttonHeight);
    quitBtn->setColors(
        sf::Color(180, 60, 60, 220),    // Normal: Red
        sf::Color(200, 80, 80, 240),    // Hover: Lighter red
        sf::Color(160, 40, 40, 255)     // Press: Darker red
    );
    quitBtn->setOnClick([this]() { onQuit(); });
    buttons.push_back(std::move(quitBtn));
}

void MainMenuScene::run() {
    sf::Clock deltaClock;
    
    while (window.isOpen()) {
        float deltaTime = deltaClock.restart().asSeconds();
        animationTime += deltaTime;
        
        handleEvents();
        update();
        updateAnimations(deltaTime);
        render();
    }
}

void MainMenuScene::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                for (auto& button : buttons) {
                    button->handleClick(mousePos);
                }
            }
        }
        
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }
    }
}

void MainMenuScene::update() {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    for (auto& button : buttons) {
        button->update(mousePos, 1.0f/60.0f); // Assuming 60 FPS
    }
}

void MainMenuScene::updateAnimations(float deltaTime) {
    // Title pulsing effect
    titlePulse += deltaTime * 2.0f;
    float pulseFactor = 1.0f + 0.05f * std::sin(titlePulse);
    
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setScale(pulseFactor, pulseFactor);
    titleText.setPosition(
        (1280 - titleBounds.width * pulseFactor) / 2,
        140 - (titleBounds.height * (pulseFactor - 1.0f)) / 2
    );
    
    // Update particles
    updateParticles(deltaTime);
}

void MainMenuScene::updateParticles(float deltaTime) {
    for (auto& particle : backgroundParticles) {
        sf::Vector2f pos = particle.getPosition();
        pos.y -= 20 * deltaTime; // Slow upward movement
        
        // Reset particle if it goes off screen
        if (pos.y < -10) {
            pos.y = 730;
            pos.x = rand() % 1280;
        }
        
        particle.setPosition(pos);
        
        // Subtle alpha animation
        sf::Uint8 alpha = 30 + 20 * std::sin(animationTime * 2 + pos.x * 0.01f);
        particle.setFillColor(sf::Color(100, 150, 200, alpha));
    }
}

void MainMenuScene::render() {
    window.clear();
    
    // Draw background
    window.draw(background);
    
    // Draw particles
    for (const auto& particle : backgroundParticles) {
        window.draw(particle);
    }
    
    // Draw title background
    window.draw(titleBackground);
    
    // Draw title and subtitle
    window.draw(titleText);
    window.draw(subtitleText);
    
    // Draw buttons
    for (auto& button : buttons) {
        button->draw(window);
    }
    
    // Draw version
    window.draw(versionText);
    
    window.display();
}

void MainMenuScene::onNewGame() {
    std::cout << "New Game clicked! Setting sail for the Grand Line...\n";
    // TODO: Switch to character selection or game world scene
}

void MainMenuScene::onLoadGame() {
    std::cout << "Load Game clicked! Loading your pirate adventure...\n";
    // TODO: Open load game menu
}

void MainMenuScene::onSettings() {
    std::cout << "Settings clicked! Adjusting ship settings...\n";
    // TODO: Open settings scene
}

void MainMenuScene::onQuit() {
    std::cout << "Quit clicked! Thanks for sailing with the Straw Hats!\n";
    window.close();
}