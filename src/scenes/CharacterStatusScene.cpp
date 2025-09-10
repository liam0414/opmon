#include "CharacterStatusScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "graphics/FontManager.h"
#include "graphics/TextureManager.h"
#include "game/GameStateManager.h"
#include <sstream>
#include <iomanip>

CharacterStatusScene::CharacterStatusScene() : selectedCharacter(0) {
    setupUI();
}

void CharacterStatusScene::onEnter() {
    LOG_INFO("Opened character status screen");
    updateCharacterDisplay();
}

void CharacterStatusScene::onExit() {
    LOG_INFO("Closed character status screen");
}

void CharacterStatusScene::setupUI() {
    auto& fontManager = FontManager::getInstance();
    
    // Background panel
    backgroundPanel.setSize(sf::Vector2f(980, 700));
    backgroundPanel.setPosition(22, 34);
    backgroundPanel.setFillColor(sf::Color(25, 30, 45, 240));
    backgroundPanel.setOutlineColor(sf::Color(100, 150, 200));
    backgroundPanel.setOutlineThickness(3);
    
    // Title
    titleText.setFont(fontManager.getDefaultFont());
    titleText.setString("CHARACTER STATUS");
    titleText.setCharacterSize(32);
    titleText.setFillColor(sf::Color(255, 215, 0)); // Gold
    titleText.setPosition(50, 50);
    
    // Portrait frame
    portraitFrame.setSize(sf::Vector2f(200, 250));
    portraitFrame.setPosition(50, 100);
    portraitFrame.setFillColor(sf::Color(40, 40, 60, 200));
    portraitFrame.setOutlineColor(sf::Color::White);
    portraitFrame.setOutlineThickness(2);
    
    // Character info text
    characterInfoText.setFont(fontManager.getDefaultFont());
    characterInfoText.setCharacterSize(18);
    characterInfoText.setFillColor(sf::Color::White);
    characterInfoText.setPosition(280, 100);
    
    // Stats text
    statsText.setFont(fontManager.getDefaultFont());
    statsText.setCharacterSize(16);
    statsText.setFillColor(sf::Color::White);
    statsText.setPosition(280, 200);
    
    // Devil Fruit info
    devilFruitText.setFont(fontManager.getDefaultFont());
    devilFruitText.setCharacterSize(16);
    devilFruitText.setFillColor(sf::Color(255, 100, 255)); // Magenta for DF
    devilFruitText.setPosition(50, 380);
    
    // Abilities text
    abilitiesText.setFont(fontManager.getDefaultFont());
    abilitiesText.setCharacterSize(14);
    abilitiesText.setFillColor(sf::Color(100, 255, 100)); // Light green
    abilitiesText.setPosition(50, 500);
    
    // Back button
    backButton = std::make_unique<Button>("Back", fontManager.getDefaultFont());
    backButton->setBounds(sf::FloatRect(880, 680, 100, 40));
    backButton->setColors(sf::Color(100, 50, 50, 200), sf::Color(120, 70, 70, 220),
                         sf::Color(80, 30, 30, 240), sf::Color::Gray);
    backButton->setOnClick([this]() {
        Application::getInstance()->popScene();
    });
    
    // Create character tabs
    auto& gameState = GameStateManager::getInstance();
    std::vector<Character*> allCharacters;
    
    // Add player first
    if (gameState.getPlayer()) {
        allCharacters.push_back(gameState.getPlayer());
    }
    
    // Add crew members
    const auto& crew = gameState.getCrew();
    for (const auto& member : crew) {
        if (member) {
            allCharacters.push_back(member.get());
        }
    }
    
    // Create tabs for each character
    float tabWidth = 120;
    float tabHeight = 40;
    float tabSpacing = 10;
    float startX = 50;
    
    for (size_t i = 0; i < allCharacters.size(); ++i) {
        auto button = std::make_unique<Button>(allCharacters[i]->getName(), fontManager.getDefaultFont());
        button->setBounds(sf::FloatRect(startX + i * (tabWidth + tabSpacing), 680, tabWidth, tabHeight));
        
        sf::Color normalColor(60, 80, 100, 200);
        sf::Color hoverColor(80, 100, 120, 220);
        sf::Color selectedColor(100, 150, 200, 240);
        
        button->setColors(normalColor, hoverColor, selectedColor, sf::Color::Gray);
        
        int charIndex = i;
        button->setOnClick([this, charIndex]() {
            selectedCharacter = charIndex;
            updateCharacterDisplay();
            
            // Update tab states
            for (size_t j = 0; j < characterTabs.size(); ++j) {
                if (j == selectedCharacter) {
                    characterTabs[j]->setState(UIState::Pressed);
                } else {
                    characterTabs[j]->setState(UIState::Normal);
                }
            }
        });
        
        characterTabs.push_back(std::move(button));
    }
    
    // Select first character by default
    if (!characterTabs.empty()) {
        characterTabs[0]->setState(UIState::Pressed);
    }
}

void CharacterStatusScene::updateCharacterDisplay() {
    auto& gameState = GameStateManager::getInstance();
    std::vector<Character*> allCharacters;
    
    // Build character list
    if (gameState.getPlayer()) {
        allCharacters.push_back(gameState.getPlayer());
    }
    
    const auto& crew = gameState.getCrew();
    for (const auto& member : crew) {
        if (member) {
            allCharacters.push_back(member.get());
        }
    }
    
    if (selectedCharacter >= static_cast<int>(allCharacters.size()) || selectedCharacter < 0) {
        selectedCharacter = 0;
    }
    
    if (allCharacters.empty()) return;
    
    Character* character = allCharacters[selectedCharacter];
    if (!character) return;
    
    // Update character info
    std::ostringstream infoStream;
    infoStream << character->getName();
    if (!character->getTitle().empty()) {
        infoStream << "\n\"" << character->getTitle() << "\"";
    }
    infoStream << "\n\nLevel: " << character->getStats().getFinalStat(StatType::Level);
    infoStream << "\nBounty: " << std::fixed << std::setprecision(0) << character->getBounty() << " ฿";
    
    // Add type-specific info
    if (character->getType() == CharacterType::CrewMember) {
        // Cast to CrewMember for additional info
        // This would require a safe cast or additional methods
        infoStream << "\nCrew Member";
    } else if (character->getType() == CharacterType::Player) {
        infoStream << "\nCaptain";
    }
    
    characterInfoText.setString(infoStream.str());
    
    // Update stats display
    std::ostringstream statsStream;
    const auto& stats = character->getStats();
    
    statsStream << "=== STATS ===\n";
    statsStream << "Health: " << stats.getCurrentHealth() << "/" << stats.getMaxHealth() << "\n";
    statsStream << "Attack: " << stats.getFinalStat(StatType::Attack) << "\n";
    statsStream << "Defense: " << stats.getFinalStat(StatType::Defense) << "\n";
    statsStream << "Speed: " << stats.getFinalStat(StatType::Speed) << "\n";
    
    if (character->hasDevilFruit()) {
        statsStream << "DF Power: " << stats.getFinalStat(StatType::DevilFruitPower) << "\n";
    }
    
    statsStream << "Experience: " << stats.getBaseStat(StatType::Experience) << "\n";
    int expToNext = stats.getExperienceToNextLevel();
    if (expToNext > 0) {
        statsStream << "To Next Level: " << expToNext << "\n";
    }
    
    statsText.setString(statsStream.str());
    
    // Update Devil Fruit info
    if (character->hasDevilFruit()) {
        const auto& devilFruit = character->getDevilFruit();
        std::ostringstream dfStream;
        
        dfStream << "=== DEVIL FRUIT ===\n";
        dfStream << devilFruit.getName() << "\n";
        dfStream << devilFruit.getDescription() << "\n\n";
        dfStream << "Type: ";
        
        switch (devilFruit.getType()) {
            case DevilFruitType::Paramecia: dfStream << "Paramecia"; break;
            case DevilFruitType::Zoan: dfStream << "Zoan"; break;
            case DevilFruitType::Logia: dfStream << "Logia"; break;
            default: dfStream << "Unknown"; break;
        }
        
        dfStream << "\nMastery Level: " << devilFruit.getMasteryLevel() << "/10";
        
        if (devilFruit.isAwakened()) {
            dfStream << "\n✨ AWAKENED ✨";
        }
        
        devilFruitText.setString(dfStream.str());
    } else {
        devilFruitText.setString("=== DEVIL FRUIT ===\nNo Devil Fruit");
    }
    
    // Update abilities info
    std::ostringstream abilitiesStream;
    abilitiesStream << "=== ABILITIES ===\n";
    
    if (character->hasDevilFruit()) {
        const auto& abilities = character->getDevilFruit().getAbilities();
        int playerLevel = character->getStats().getFinalStat(StatType::Level);
        
        for (const auto& ability : abilities) {
            if (ability) {
                abilitiesStream << "• " << ability->name;
                
                if (playerLevel >= ability->levelRequirement) {
                    abilitiesStream << " (Ready)";
                    abilitiesStream << "\n  Power: " << ability->baseDamage;
                    abilitiesStream << " | Cost: " << ability->powerCost;
                    abilitiesStream << " | CD: " << std::fixed << std::setprecision(1) << ability->cooldown << "s";
                } else {
                    abilitiesStream << " (Req. Lv" << ability->levelRequirement << ")";
                }
                abilitiesStream << "\n";
            }
        }
    } else {
        abilitiesStream << "No special abilities\n";
        abilitiesStream << "(Acquire a Devil Fruit to unlock abilities)";
    }
    
    abilitiesText.setString(abilitiesStream.str());
    
    // Try to load character portrait
    auto& textureManager = TextureManager::getInstance();
    std::string portraitPath = "assets/textures/portraits/" + character->getName() + ".png";
    
    if (textureManager.loadTexture("character_portrait", portraitPath)) {
        characterPortrait.setTexture(textureManager.getTexture("character_portrait"));
        
        // Scale to fit portrait frame
        sf::Vector2u textureSize = textureManager.getTexture("character_portrait").getSize();
        float scaleX = 190.0f / textureSize.x; // Leave 10px border
        float scaleY = 240.0f / textureSize.y;
        float scale = std::min(scaleX, scaleY);
        
        characterPortrait.setScale(scale, scale);
        characterPortrait.setPosition(55, 105);
    }
}

void CharacterStatusScene::drawStatBar(sf::RenderWindow& window, const std::string& label, 
                                      int current, int max, const sf::Color& color, 
                                      float x, float y, float width) {
    auto& fontManager = FontManager::getInstance();
    
    // Label
    sf::Text labelText;
    labelText.setFont(fontManager.getDefaultFont());
    labelText.setString(label);
    labelText.setCharacterSize(14);
    labelText.setFillColor(sf::Color::White);
    labelText.setPosition(x, y);
    window.draw(labelText);
    
    // Background bar
    sf::RectangleShape bgBar;
    bgBar.setSize(sf::Vector2f(width, 16));
    bgBar.setPosition(x, y + 20);
    bgBar.setFillColor(sf::Color(50, 50, 50));
    bgBar.setOutlineColor(sf::Color::Gray);
    bgBar.setOutlineThickness(1);
    window.draw(bgBar);
    
    // Filled bar
    if (max > 0) {
        float fillPercent = static_cast<float>(current) / max;
        sf::RectangleShape fillBar;
        fillBar.setSize(sf::Vector2f(width * fillPercent, 16));
        fillBar.setPosition(x, y + 20);
        fillBar.setFillColor(color);
        window.draw(fillBar);
    }
    
    // Value text
    sf::Text valueText;
    valueText.setFont(fontManager.getDefaultFont());
    valueText.setString(std::to_string(current) + "/" + std::to_string(max));
    valueText.setCharacterSize(12);
    valueText.setFillColor(sf::Color::White);
    valueText.setPosition(x + width + 10, y + 22);
    window.draw(valueText);
}

void CharacterStatusScene::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
    
    // Handle character tabs
    for (auto& tab : characterTabs) {
        if (event.type == sf::Event::MouseMoved) {
            tab->handleMouseMove(mousePos);
        }
        else if (event.type == sf::Event::MouseButtonPressed) {
            tab->handleMouseClick(mousePos, event.mouseButton.button);
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            tab->handleMouseRelease(mousePos, event.mouseButton.button);
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
    
    // Keyboard navigation
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Escape:
                Application::getInstance()->popScene();
                break;
            case sf::Keyboard::Left:
                if (selectedCharacter > 0) {
                    selectedCharacter--;
                    updateCharacterDisplay();
                    
                    for (size_t i = 0; i < characterTabs.size(); ++i) {
                        characterTabs[i]->setState(i == selectedCharacter ? UIState::Pressed : UIState::Normal);
                    }
                }
                break;
            case sf::Keyboard::Right:
                if (selectedCharacter < static_cast<int>(characterTabs.size()) - 1) {
                    selectedCharacter++;
                    updateCharacterDisplay();
                    
                    for (size_t i = 0; i < characterTabs.size(); ++i) {
                        characterTabs[i]->setState(i == selectedCharacter ? UIState::Pressed : UIState::Normal);
                    }
                }
                break;
            default:
                break;
        }
    }
}

void CharacterStatusScene::update(float deltaTime) {
    // Update tabs
    for (auto& tab : characterTabs) {
        tab->update(deltaTime);
    }
    
    // Update back button
    if (backButton) {
        backButton->update(deltaTime);
    }
}

void CharacterStatusScene::render(sf::RenderWindow& window) {
    // Clear background
    window.clear(sf::Color(10, 15, 25));
    
    // Draw background panel
    window.draw(backgroundPanel);
    
    // Draw title
    window.draw(titleText);
    
    // Draw portrait frame and character portrait
    window.draw(portraitFrame);
    if (characterPortrait.getTexture()) {
        window.draw(characterPortrait);
    }
    
    // Draw character info
    window.draw(characterInfoText);
    window.draw(statsText);
    window.draw(devilFruitText);
    window.draw(abilitiesText);
    
    // Draw stat bars for current character
    auto& gameState = GameStateManager::getInstance();
    std::vector<Character*> allCharacters;
    
    if (gameState.getPlayer()) {
        allCharacters.push_back(gameState.getPlayer());
    }
    
    const auto& crew = gameState.getCrew();
    for (const auto& member : crew) {
        if (member) {
            allCharacters.push_back(member.get());
        }
    }
    
    if (selectedCharacter >= 0 && selectedCharacter < static_cast<int>(allCharacters.size())) {
        Character* character = allCharacters[selectedCharacter];
        if (character) {
            const auto& stats = character->getStats();
            float barX = 600;
            float barY = 200;
            float barSpacing = 50;
            
            // Health bar
            drawStatBar(window, "Health", stats.getCurrentHealth(), stats.getMaxHealth(), 
                       sf::Color::Green, barX, barY, 200);
            
            // Attack bar (visual representation, max 100 for display)
            int attackStat = stats.getFinalStat(StatType::Attack);
            drawStatBar(window, "Attack", attackStat, std::max(100, attackStat), 
                       sf::Color::Red, barX, barY + barSpacing, 200);
            
            // Defense bar
            int defenseStat = stats.getFinalStat(StatType::Defense);
            drawStatBar(window, "Defense", defenseStat, std::max(100, defenseStat), 
                       sf::Color::Blue, barX, barY + barSpacing * 2, 200);
            
            // Speed bar
            int speedStat = stats.getFinalStat(StatType::Speed);
            drawStatBar(window, "Speed", speedStat, std::max(100, speedStat), 
                       sf::Color::Yellow, barX, barY + barSpacing * 3, 200);
            
            // Experience bar (to next level)
            int currentExp = stats.getBaseStat(StatType::Experience);
            int expToNext = stats.getExperienceToNextLevel();
            int expForLevel = currentExp + expToNext;
            
            if (expForLevel > 0) {
                drawStatBar(window, "Experience", currentExp, expForLevel, 
                           sf::Color::Cyan, barX, barY + barSpacing * 4, 200);
            }
        }
    }
    
    // Draw character tabs
    for (auto& tab : characterTabs) {
        window.draw(*tab);
    }
    
    // Draw back button
    if (backButton) {
        window.draw(*backButton);
    }
    
    // Draw navigation instructions
    sf::Text instructionText;
    instructionText.setFont(FontManager::getInstance().getDefaultFont());
    instructionText.setString("Left/Right arrows to switch characters | ESC to close");
    instructionText.setCharacterSize(14);
    instructionText.setFillColor(sf::Color::Gray);
    instructionText.setPosition(50, 750);
    window.draw(instructionText);
}