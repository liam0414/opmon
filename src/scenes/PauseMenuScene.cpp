#include "PauseMenuScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "graphics/FontManager.h"
#include "game/GameStateManager.h"
#include "InventoryScene.h"
#include "SettingsScene.h"
#include "MainMenuScene.h"

PauseMenuScene::PauseMenuScene() : selectedButton(0) {
    setupUI();
}

void PauseMenuScene::onEnter() {
    LOG_INFO("Game paused");
}

void PauseMenuScene::onExit() {
    LOG_INFO("Game resumed");
}

void PauseMenuScene::setupUI() {
    // Semi-transparent background
    backgroundOverlay.setSize(sf::Vector2f(1024, 768));
    backgroundOverlay.setFillColor(sf::Color(0, 0, 0, 150));
    
    auto& fontManager = FontManager::getInstance();
    
    // Title
    titleText.setFont(fontManager.getDefaultFont());
    titleText.setString("PAUSED");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::White);
    
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition((1024 - titleBounds.width) / 2, 200);
    
    // Create menu buttons
    float buttonWidth = 200;
    float buttonHeight = 50;
    float buttonSpacing = 20;
    float startY = 300;
    
    sf::Color normalColor(70, 70, 70, 200);
    sf::Color hoverColor(100, 100, 100, 220);
    sf::Color pressColor(50, 50, 50, 240);
    
    // Resume button
    auto resumeBtn = std::make_unique<Button>("Resume", fontManager.getDefaultFont());
    resumeBtn->setBounds(sf::FloatRect((1024 - buttonWidth) / 2, startY, buttonWidth, buttonHeight));
    resumeBtn->setColors(normalColor, hoverColor, pressColor, sf::Color::Gray);
    resumeBtn->setOnClick([this]() { handleButtonClick("resume"); });
    menuButtons.push_back(std::move(resumeBtn));
    
    // Inventory button
    auto inventoryBtn = std::make_unique<Button>("Inventory", fontManager.getDefaultFont());
    inventoryBtn->setBounds(sf::FloatRect((1024 - buttonWidth) / 2, startY + (buttonHeight + buttonSpacing) * 1, buttonWidth, buttonHeight));
    inventoryBtn->setColors(normalColor, hoverColor, pressColor, sf::Color::Gray);
    inventoryBtn->setOnClick([this]() { handleButtonClick("inventory"); });
    menuButtons.push_back(std::move(inventoryBtn));
    
    // Save Game button
    auto saveBtn = std::make_unique<Button>("Save Game", fontManager.getDefaultFont());
    saveBtn->setBounds(sf::FloatRect((1024 - buttonWidth) / 2, startY + (buttonHeight + buttonSpacing) * 2, buttonWidth, buttonHeight));
    saveBtn->setColors(normalColor, hoverColor, pressColor, sf::Color::Gray);
    saveBtn->setOnClick([this]() { handleButtonClick("save"); });
    menuButtons.push_back(std::move(saveBtn));
    
    // Settings button
    auto settingsBtn = std::make_unique<Button>("Settings", fontManager.getDefaultFont());
    settingsBtn->setBounds(sf::FloatRect((1024 - buttonWidth) / 2, startY + (buttonHeight + buttonSpacing) * 3, buttonWidth, buttonHeight));
    settingsBtn->setColors(normalColor, hoverColor, pressColor, sf::Color::Gray);
    settingsBtn->setOnClick([this]() { handleButtonClick("settings"); });
    menuButtons.push_back(std::move(settingsBtn));
    
    // Quit to Menu button
    auto quitBtn = std::make_unique<Button>("Quit to Menu", fontManager.getDefaultFont());
    quitBtn->setBounds(sf::FloatRect((1024 - buttonWidth) / 2, startY + (buttonHeight + buttonSpacing) * 4, buttonWidth, buttonHeight));
    quitBtn->setColors(sf::Color(150, 50, 50, 200), sf::Color(170, 70, 70, 220), 
                      sf::Color(130, 30, 30, 240), sf::Color::Gray);
    quitBtn->setOnClick([this]() { handleButtonClick("quit"); });
    menuButtons.push_back(std::move(quitBtn));
}

void PauseMenuScene::handleButtonClick(const std::string& buttonId) {
    auto& app = *Application::getInstance();
    
    if (buttonId == "resume") {
        app.popScene(); // Return to game
    }
    else if (buttonId == "inventory") {
        app.pushScene(std::make_unique<InventoryScene>());
    }
    else if (buttonId == "save") {
        // Quick save
        std::string saveFileName = "quicksave";
        if (GameStateManager::getInstance().saveGame(saveFileName)) {
            LOG_INFO("Game saved successfully!");
            // TODO: Show save confirmation UI
        }
    }
    else if (buttonId == "settings") {
        app.pushScene(std::make_unique<SettingsScene>());
    }
    else if (buttonId == "quit") {
        // Auto-save before quitting
        GameStateManager::getInstance().autoSave();
        app.changeScene(std::make_unique<MainMenuScene>());
    }
}

void PauseMenuScene::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
    
    // Handle button events
    for (auto& button : menuButtons) {
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
    
    // Keyboard input
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Escape:
                Application::getInstance()->popScene(); // Resume game
                break;
            case sf::Keyboard::Up:
                selectedButton = (selectedButton - 1 + menuButtons.size()) % menuButtons.size();
                break;
            case sf::Keyboard::Down:
                selectedButton = (selectedButton + 1) % menuButtons.size();
                break;
            case sf::Keyboard::Enter:
                if (selectedButton < menuButtons.size()) {
                    sf::Vector2f buttonCenter = sf::Vector2f(
                        menuButtons[selectedButton]->getBounds().left + menuButtons[selectedButton]->getBounds().width / 2,
                        menuButtons[selectedButton]->getBounds().top + menuButtons[selectedButton]->getBounds().height / 2
                    );
                    menuButtons[selectedButton]->handleMouseClick(buttonCenter, sf::Mouse::Left);
                    menuButtons[selectedButton]->handleMouseRelease(buttonCenter, sf::Mouse::Left);
                }
                break;
            default:
                break;
        }
    }
}

void PauseMenuScene::update(float deltaTime) {
    // Update buttons
    for (auto& button : menuButtons) {
        button->update(deltaTime);
    }
    
    // Highlight selected button for keyboard navigation
    for (size_t i = 0; i < menuButtons.size(); ++i) {
        if (i == selectedButton) {
            if (menuButtons[i]->getState() == UIState::Normal) {
                menuButtons[i]->setState(UIState::Hovered);
            }
        }
    }
}

void PauseMenuScene::render(sf::RenderWindow& window) {
    // Draw semi-transparent overlay
    window.draw(backgroundOverlay);
    
    // Draw title
    window.draw(titleText);
    
    // Draw buttons
    for (auto& button : menuButtons) {
        window.draw(*button);
    }
}