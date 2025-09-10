#include "MainMenuScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "graphics/TextureManager.h"
#include "graphics/FontManager.h"
#include "game/GameStateManager.h"
#include "NewGameScene.h"
#include "LoadGameScene.h"
#include "SettingsScene.h"
#include <cmath>

MainMenuScene::MainMenuScene() 
    : selectedButton(0), transitioning(false), logoAnimationTime(0) {
    
    setupBackground();
    setupUI();
    createButtons();
}

void MainMenuScene::onEnter() {
    LOG_INFO("Entered Main Menu");
    
    // Play main menu music
    auto& app = *Application::getInstance();
    app.getAudioManager().playMusic("main_theme", true);
    
    transitioning = false;
}

void MainMenuScene::onExit() {
    LOG_INFO("Exited Main Menu");
}

void MainMenuScene::setupBackground() {
    auto& textureManager = TextureManager::getInstance();
    
    // Load main menu background (One Piece themed)
    if (textureManager.loadTexture("main_menu_bg", "assets/textures/ui/main_menu_background.png")) {
        backgroundSprite.setTexture(textureManager.getTexture("main_menu_bg"));
        
        // Scale to fit screen
        sf::Vector2u textureSize = textureManager.getTexture("main_menu_bg").getSize();
        backgroundSprite.setScale(1024.0f / textureSize.x, 768.0f / textureSize.y);
    }
}

void MainMenuScene::setupUI() {
    auto& fontManager = FontManager::getInstance();
    
    // Load font
    if (!fontManager.loadFont("main_font", "assets/fonts/pirate_font.ttf")) {
        fontManager.loadFont("main_font", "assets/fonts/arial.ttf"); // Fallback
    }
    
    // Title text
    titleText.setFont(fontManager.getFont("main_font"));
    titleText.setString("ONE PIECE ADVENTURE");
    titleText.setCharacterSize(72);
    titleText.setFillColor(sf::Color(255, 215, 0)); // Gold color
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3);
    
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition((1024 - titleBounds.width) / 2, 100);
    originalLogoPosition = titleText.getPosition();
    
    // Version text
    versionText.setFont(fontManager.getFont("main_font"));
    versionText.setString("v1.0.0 - Alpha");
    versionText.setCharacterSize(24);
    versionText.setFillColor(sf::Color::White);
    versionText.setPosition(10, 768 - 30);
}

void MainMenuScene::createButtons() {
    auto& fontManager = FontManager::getInstance();
    
    buttons.clear();
    
    // Button properties
    float buttonWidth = 300;
    float buttonHeight = 60;
    float buttonSpacing = 20;
    float startY = 350;
    
    sf::Color normalColor(50, 100, 150, 200);
    sf::Color hoverColor(70, 120, 170, 220);
    sf::Color pressColor(30, 80, 130, 240);
    sf::Color disabledColor(100, 100, 100, 150);
    
    // New Game button
    auto newGameBtn = std::make_unique<Button>("New Game", fontManager.getFont("main_font"));
    newGameBtn->setBounds(sf::FloatRect((1024 - buttonWidth) / 2, startY, buttonWidth, buttonHeight));
    newGameBtn->setColors(normalColor, hoverColor, pressColor, disabledColor);
    newGameBtn->setOnClick([this]() { handleButtonClick("new_game"); });
    buttons.push_back(std::move(newGameBtn));
    
    // Continue Game button
    auto continueBtn = std::make_unique<Button>("Continue", fontManager.getFont("main_font"));
    continueBtn->setBounds(sf::FloatRect((1024 - buttonWidth) / 2, startY + (buttonHeight + buttonSpacing) * 1, buttonWidth, buttonHeight));
    continueBtn->setColors(normalColor, hoverColor, pressColor, disabledColor);
    continueBtn->setOnClick([this]() { handleButtonClick("continue"); });
    
    // Check if save file exists
    auto saveFiles = GameStateManager::getInstance().getSaveFiles();
    if (saveFiles.empty()) {
        continueBtn->setInteractive(false);
        continueBtn->setState(UIState::Disabled);
    }
    
    buttons.push_back(std::move(continueBtn));
    
    // Load Game button
    auto loadBtn = std::make_unique<Button>("Load Game", fontManager.getFont("main_font"));
    loadBtn->setBounds(sf::FloatRect((1024 - buttonWidth) / 2, startY + (buttonHeight + buttonSpacing) * 2, buttonWidth, buttonHeight));
    loadBtn->setColors(normalColor, hoverColor, pressColor, disabledColor);
    loadBtn->setOnClick([this]() { handleButtonClick("load_game"); });
    buttons.push_back(std::move(loadBtn));
    
    // Settings button
    auto settingsBtn = std::make_unique<Button>("Settings", fontManager.getFont("main_font"));
    settingsBtn->setBounds(sf::FloatRect((1024 - buttonWidth) / 2, startY + (buttonHeight + buttonSpacing) * 3, buttonWidth, buttonHeight));
    settingsBtn->setColors(normalColor, hoverColor, pressColor, disabledColor);
    settingsBtn->setOnClick([this]() { handleButtonClick("settings"); });
    buttons.push_back(std::move(settingsBtn));
    
    // Exit button
    auto exitBtn = std::make_unique<Button>("Exit", fontManager.getFont("main_font"));
    exitBtn->setBounds(sf::FloatRect((1024 - buttonWidth) / 2, startY + (buttonHeight + buttonSpacing) * 4, buttonWidth, buttonHeight));
    exitBtn->setColors(sf::Color(150, 50, 50, 200), sf::Color(170, 70, 70, 220), 
                      sf::Color(130, 30, 30, 240), disabledColor);
    exitBtn->setOnClick([this]() { handleButtonClick("exit"); });
    buttons.push_back(std::move(exitBtn));
}

void MainMenuScene::handleButtonClick(const std::string& buttonId) {
    if (transitioning) return;
    
    auto& app = *Application::getInstance();
    
    if (buttonId == "new_game") {
        transitioning = true;
        app.pushScene(std::make_unique<NewGameScene>());
    }
    else if (buttonId == "continue") {
        // Load most recent save
        auto saveFiles = GameStateManager::getInstance().getSaveFiles();
        if (!saveFiles.empty()) {
            // Find most recent non-autosave file
            std::string latestSave = saveFiles.back();
            for (const auto& save : saveFiles) {
                if (save.find("autosave") == std::string::npos) {
                    latestSave = save;
                    break;
                }
            }
            
            if (GameStateManager::getInstance().loadGame(latestSave)) {
                transitioning = true;
                app.changeScene(std::make_unique<GameWorldScene>());
            }
        }
    }
    else if (buttonId == "load_game") {
        transitioning = true;
        app.pushScene(std::make_unique<LoadGameScene>());
    }
    else if (buttonId == "settings") {
        transitioning = true;
        app.pushScene(std::make_unique<SettingsScene>());
    }
    else if (buttonId == "exit") {
        app.shutdown();
    }
    
    // Play button click sound
    app.getAudioManager().playSound("button_click");
}

void MainMenuScene::updateAnimations(float deltaTime) {
    logoAnimationTime += deltaTime;
    
    // Floating logo animation
    float bobOffset = std::sin(logoAnimationTime * 2.0f) * 10.0f;
    titleText.setPosition(originalLogoPosition.x, originalLogoPosition.y + bobOffset);
    
    // Color pulse animation
    float pulse = (std::sin(logoAnimationTime * 1.5f) + 1.0f) / 2.0f; // 0 to 1
    sf::Uint8 alpha = static_cast<sf::Uint8>(200 + pulse * 55); // 200 to 255
    sf::Color currentColor = titleText.getFillColor();
    currentColor.a = alpha;
    titleText.setFillColor(currentColor);
}

void MainMenuScene::handleEvent(const sf::Event& event) {
    if (transitioning) return;
    
    // Handle button events
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
    
    for (auto& button : buttons) {
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
    
    // Keyboard navigation
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Up:
                selectedButton = (selectedButton - 1 + buttons.size()) % buttons.size();
                break;
            case sf::Keyboard::Down:
                selectedButton = (selectedButton + 1) % buttons.size();
                break;
            case sf::Keyboard::Enter:
                if (buttons[selectedButton]->isInteractive()) {
                    // Simulate button click
                    sf::Vector2f buttonPos = sf::Vector2f(
                        buttons[selectedButton]->getBounds().left + buttons[selectedButton]->getBounds().width / 2,
                        buttons[selectedButton]->getBounds().top + buttons[selectedButton]->getBounds().height / 2
                    );
                    buttons[selectedButton]->handleMouseClick(buttonPos, sf::Mouse::Left);
                    buttons[selectedButton]->handleMouseRelease(buttonPos, sf::Mouse::Left);
                }
                break;
            default:
                break;
        }
    }
}

void MainMenuScene::update(float deltaTime) {
    updateAnimations(deltaTime);
    
    // Update buttons
    for (auto& button : buttons) {
        button->update(deltaTime);
    }
    
    // Highlight selected button for keyboard navigation
    for (size_t i = 0; i < buttons.size(); ++i) {
        if (i == selectedButton && buttons[i]->isInteractive()) {
            if (buttons[i]->getState() == UIState::Normal) {
                buttons[i]->setState(UIState::Hovered);
            }
        }
    }
}

void MainMenuScene::render(sf::RenderWindow& window) {
    // Draw background
    window.draw(backgroundSprite);
    
    // Draw title
    window.draw(titleText);
    
    // Draw buttons
    for (auto& button : buttons) {
        window.draw(*button);
    }
    
    // Draw version
    window.draw(versionText);
}