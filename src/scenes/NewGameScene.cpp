#include "NewGameScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "graphics/FontManager.h"
#include "GameWorldScene.h"
#include "MainMenuScene.h"

NewGameScene::NewGameScene() : selectedDifficulty(GameDifficulty::Normal) {
    setupUI();
}

void NewGameScene::onEnter() {
    LOG_INFO("Entered New Game Scene");
}

void NewGameScene::onExit() {
    LOG_INFO("Exited New Game Scene");
}

void NewGameScene::setupUI() {
    auto& fontManager = FontManager::getInstance();
    
    // Title
    titleText.setFont(fontManager.getDefaultFont());
    titleText.setString("New Adventure");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(50, 50);
    
    // Description
    descriptionText.setFont(fontManager.getDefaultFont());
    descriptionText.setString("Choose your difficulty and begin your journey to become Pirate King!");
    descriptionText.setCharacterSize(24);
    descriptionText.setFillColor(sf::Color::White);
    descriptionText.setPosition(50, 120);
    
    // Difficulty description
    difficultyDescText.setFont(fontManager.getDefaultFont());
    difficultyDescText.setCharacterSize(20);
    difficultyDescText.setFillColor(sf::Color::Yellow);
    difficultyDescText.setPosition(50, 500);
    
    // Create difficulty buttons
    std::vector<std::string> difficulties = {"Easy", "Normal", "Hard", "Legendary"};
    std::vector<sf::Color> colors = {
        sf::Color::Green,
        sf::Color::Blue,
        sf::Color(255, 165, 0), // Orange
        sf::Color::Red
    };
    
    float buttonWidth = 200;
    float buttonHeight = 50;
    float startX = 50;
    float startY = 200;
    
    for (size_t i = 0; i < difficulties.size(); ++i) {
        auto button = std::make_unique<Button>(difficulties[i], fontManager.getDefaultFont());
        button->setBounds(sf::FloatRect(startX + i * (buttonWidth + 20), startY, buttonWidth, buttonHeight));
        
        sf::Color normal = colors[i];
        normal.a = 180;
        sf::Color hover = colors[i];
        hover.a = 220;
        sf::Color press = colors[i];
        press.a = 255;
        
        button->setColors(normal, hover, press, sf::Color::Gray);
        
        GameDifficulty difficulty = static_cast<GameDifficulty>(i);
        button->setOnClick([this, difficulty]() {
            selectedDifficulty = difficulty;
            updateDifficultyDescription();
        });
        
        difficultyButtons.push_back(std::move(button));
    }
    
    // Initially select Normal difficulty
    difficultyButtons[1]->setState(UIState::Pressed);
    updateDifficultyDescription();
    
    // Start button
    startButton = std::make_unique<Button>("Start Adventure!", fontManager.getDefaultFont());
    startButton->setBounds(sf::FloatRect(50, 600, 200, 60));
    startButton->setColors(sf::Color(0, 150, 0, 200), sf::Color(0, 180, 0, 220), 
                          sf::Color(0, 120, 0, 240), sf::Color::Gray);
    startButton->setOnClick([this]() { startNewGame(); });
    
    // Back button
    backButton = std::make_unique<Button>("Back", fontManager.getDefaultFont());
    backButton->setBounds(sf::FloatRect(300, 600, 120, 60));
    backButton->setColors(sf::Color(100, 100, 100, 200), sf::Color(120, 120, 120, 220),
                         sf::Color(80, 80, 80, 240), sf::Color::Gray);
    backButton->setOnClick([this]() {
        Application::getInstance()->popScene();
    });
}

void NewGameScene::updateDifficultyDescription() {
    std::string description;
    
    switch (selectedDifficulty) {
        case GameDifficulty::Easy:
            description = "Easy: For new pirates! Double health, more berries, weaker enemies.";
            break;
        case GameDifficulty::Normal:
            description = "Normal: The classic One Piece experience. Balanced and fair.";
            break;
        case GameDifficulty::Hard:
            description = "Hard: For veteran pirates! Tougher enemies, less resources.";
            break;
        case GameDifficulty::Legendary:
            description = "Legendary: For Pirate King candidates only! Extremely challenging.";
            break;
    }
    
    difficultyDescText.setString(description);
    
    // Update button states
    for (size_t i = 0; i < difficultyButtons.size(); ++i) {
        if (static_cast<GameDifficulty>(i) == selectedDifficulty) {
            difficultyButtons[i]->setState(UIState::Pressed);
        } else {
            difficultyButtons[i]->setState(UIState::Normal);
        }
    }
}

void NewGameScene::startNewGame() {
    LOG_INFO("Starting new game with difficulty: {}", static_cast<int>(selectedDifficulty));
    
    // Initialize new game state
    GameStateManager::getInstance().startNewGame(selectedDifficulty);
    
    // Transition to game world
    Application::getInstance()->changeScene(std::make_unique<GameWorldScene>());
}

void NewGameScene::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
    
    // Handle difficulty buttons
    for (auto& button : difficultyButtons) {
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
    
    // Handle other buttons
    if (startButton) {
        if (event.type == sf::Event::MouseMoved) {
            startButton->handleMouseMove(mousePos);
        }
        else if (event.type == sf::Event::MouseButtonPressed) {
            startButton->handleMouseClick(mousePos, event.mouseButton.button);
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            startButton->handleMouseRelease(mousePos, event.mouseButton.button);
        }
    }
    
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
        if (event.key.code == sf::Keyboard::Escape) {
            Application::getInstance()->popScene();
        }
        else if (event.key.code == sf::Keyboard::Enter) {
            startNewGame();
        }
    }
}

void NewGameScene::update(float deltaTime) {
    // Update buttons
    for (auto& button : difficultyButtons) {
        button->update(deltaTime);
    }
    
    if (startButton) startButton->update(deltaTime);
    if (backButton) backButton->update(deltaTime);
}

void NewGameScene::render(sf::RenderWindow& window) {
    window.clear(sf::Color(20, 30, 50));
    
    // Draw UI elements
    window.draw(titleText);
    window.draw(descriptionText);
    window.draw(difficultyDescText);
    
    // Draw buttons
    for (auto& button : difficultyButtons) {
        window.draw(*button);
    }
    
    if (startButton) window.draw(*startButton);
    if (backButton) window.draw(*backButton);
}