#include "DialogueScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "graphics/FontManager.h"
#include "graphics/TextureManager.h"

DialogueScene::DialogueScene(const std::vector<DialoguePage>& dialoguePages) 
    : pages(dialoguePages), currentPage(0), textTimer(0), textComplete(false), autoAdvance(false) {
    setupUI();
}

void DialogueScene::onEnter() {
    LOG_INFO("Started dialogue with {} pages", pages.size());
    
    if (!pages.empty()) {
        loadPage(0);
    }
    
    // Pause game music and play dialogue music if needed
    // Application::getInstance()->getAudioManager().playMusic("dialogue_theme");
}

void DialogueScene::onExit() {
    LOG_INFO("Ended dialogue");
    
    if (onDialogueComplete) {
        onDialogueComplete();
    }
}

void DialogueScene::setupUI() {
    auto& fontManager = FontManager::getInstance();
    
    // Dialogue box
    dialogueBox.setSize(sf::Vector2f(900, 200));
    dialogueBox.setPosition(62, 500);
    dialogueBox.setFillColor(sf::Color(20, 20, 40, 230));
    dialogueBox.setOutlineColor(sf::Color::White);
    dialogueBox.setOutlineThickness(3);
    
    // Name box
    nameBox.setSize(sf::Vector2f(200, 40));
    nameBox.setPosition(62, 460);
    nameBox.setFillColor(sf::Color(40, 40, 80, 230));
    nameBox.setOutlineColor(sf::Color::White);
    nameBox.setOutlineThickness(2);
    
    // Portrait frame
    portraitFrame.setSize(sf::Vector2f(150, 180));
    portraitFrame.setPosition(980, 510);
    portraitFrame.setFillColor(sf::Color(30, 30, 50, 200));
    portraitFrame.setOutlineColor(sf::Color::Gray);
    portraitFrame.setOutlineThickness(2);
    
    // Name text
    nameText.setFont(fontManager.getDefaultFont());
    nameText.setCharacterSize(20);
    nameText.setFillColor(sf::Color::White);
    nameText.setPosition(72, 465);
    
    // Dialogue text
    dialogueText.setFont(fontManager.getDefaultFont());
    dialogueText.setCharacterSize(18);
    dialogueText.setFillColor(sf::Color::White);
    dialogueText.setPosition(80, 520);
    
    // Continue button
    continueButton = std::make_unique<Button>("Continue", fontManager.getDefaultFont());
    continueButton->setBounds(sf::FloatRect(800, 650, 100, 30));
    continueButton->setColors(sf::Color(50, 100, 150, 200), sf::Color(70, 120, 170, 220),
                             sf::Color(30, 80, 130, 240), sf::Color::Gray);
    continueButton->setOnClick([this]() { nextPage(); });
    
    // Skip button
    skipButton = std::make_unique<Button>("Skip", fontManager.getDefaultFont());
    skipButton->setBounds(sf::FloatRect(910, 650, 80, 30));
    skipButton->setColors(sf::Color(100, 50, 50, 200), sf::Color(120, 70, 70, 220),
                         sf::Color(80, 30, 30, 240), sf::Color::Gray);
    skipButton->setOnClick([this]() { skipText(); });
}

void DialogueScene::loadPage(int pageIndex) {
    if (pageIndex < 0 || pageIndex >= static_cast<int>(pages.size())) {
        // End of dialogue
        Application::getInstance()->popScene();
        return;
    }
    
    currentPage = pageIndex;
    const DialoguePage& page = pages[currentPage];
    
    // Set speaker name
    nameText.setString(page.speakerName);
    
    // Setup text animation
    fullText = page.text;
    displayedText = "";
    textTimer = 0;
    textComplete = false;
    
    // Load speaker portrait
    if (!page.portraitTexture.empty()) {
        auto& textureManager = TextureManager::getInstance();
        if (textureManager.loadTexture("dialogue_portrait", page.portraitTexture)) {
            speakerPortrait.setTexture(textureManager.getTexture("dialogue_portrait"));
            
            // Scale to fit frame
            sf::Vector2u textureSize = textureManager.getTexture("dialogue_portrait").getSize();
            float scaleX = 140.0f / textureSize.x;
            float scaleY = 170.0f / textureSize.y;
            float scale = std::min(scaleX, scaleY);
            
            speakerPortrait.setScale(scale, scale);
            speakerPortrait.setPosition(985, 515);
        }
    }
    
    // Hide choices initially
    hideChoices();
    
    LOG_DEBUG("Loaded dialogue page {}: {}", pageIndex, page.speakerName);
}

void DialogueScene::updateTextAnimation(float deltaTime) {
    if (textComplete) return;
    
    const DialoguePage& page = pages[currentPage];
    textTimer += deltaTime;
    
    float charsToShow = textTimer * page.displaySpeed;
    size_t targetLength = std::min(static_cast<size_t>(charsToShow), fullText.length());
    
    if (targetLength != displayedText.length()) {
        displayedText = fullText.substr(0, targetLength);
        
        // Word wrap for dialogue box
        std::string wrappedText = "";
        std::string currentLine = "";
        std::istringstream words(displayedText);
        std::string word;
        
        while (std::getline(words, word, ' ')) {
            if (currentLine.length() + word.length() + 1 > 50) { // Approximate character limit
                wrappedText += currentLine + "\n";
                currentLine = word;
            } else {
                if (!currentLine.empty()) currentLine += " ";
                currentLine += word;
            }
        }
        wrappedText += currentLine;
        
        dialogueText.setString(wrappedText);
    }
    
    if (targetLength >= fullText.length()) {
        textComplete = true;
        
        // Show choices or continue button
        if (!page.choices.empty()) {
            showChoices();
        } else if (autoAdvance && currentPage + 1 < static_cast<int>(pages.size())) {
            // Auto-advance to next page after a delay
            static float autoAdvanceTimer = 0;
            autoAdvanceTimer += deltaTime;
            if (autoAdvanceTimer >= 2.0f) {
                autoAdvanceTimer = 0;
                nextPage();
            }
        }
    }
}

void DialogueScene::showChoices() {
    hideChoices();
    
    const DialoguePage& page = pages[currentPage];
    if (page.choices.empty()) return;
    
    float buttonWidth = 400;
    float buttonHeight = 40;
    float buttonSpacing = 10;
    float startY = 350;
    
    for (size_t i = 0; i < page.choices.size(); ++i) {
        const DialogueChoice& choice = page.choices[i];
        
        auto button = std::make_unique<Button>(choice.text, FontManager::getInstance().getDefaultFont());
        button->setBounds(sf::FloatRect(300, startY - i * (buttonHeight + buttonSpacing), buttonWidth, buttonHeight));
        
if (choice.available) {
            button->setColors(sf::Color(60, 80, 100, 200), sf::Color(80, 100, 120, 220),
                             sf::Color(40, 60, 80, 240), sf::Color::Gray);
            button->setOnClick([this, choice]() {
                if (choice.action) {
                    choice.action();
                }
                nextPage();
            });
        } else {
            button->setColors(sf::Color::Gray, sf::Color::Gray, sf::Color::Gray, sf::Color::Gray);
            button->setInteractive(false);
        }
        
        choiceButtons.push_back(std::move(button));
    }
}

void DialogueScene::hideChoices() {
    choiceButtons.clear();
}

void DialogueScene::nextPage() {
    currentPage++;
    if (currentPage < static_cast<int>(pages.size())) {
        loadPage(currentPage);
    } else {
        Application::getInstance()->popScene();
    }
}

void DialogueScene::skipText() {
    if (!textComplete) {
        // Complete current text immediately
        displayedText = fullText;
        dialogueText.setString(displayedText);
        textComplete = true;
        
        const DialoguePage& page = pages[currentPage];
        if (!page.choices.empty()) {
            showChoices();
        }
    } else {
        // Skip to next page
        nextPage();
    }
}

void DialogueScene::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
    
    // Handle choice buttons
    for (auto& button : choiceButtons) {
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
    
    // Handle continue and skip buttons
    if (textComplete && choiceButtons.empty()) {
        if (continueButton) {
            if (event.type == sf::Event::MouseMoved) {
                continueButton->handleMouseMove(mousePos);
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                continueButton->handleMouseClick(mousePos, event.mouseButton.button);
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                continueButton->handleMouseRelease(mousePos, event.mouseButton.button);
            }
        }
    }
    
    if (skipButton) {
        if (event.type == sf::Event::MouseMoved) {
            skipButton->handleMouseMove(mousePos);
        }
        else if (event.type == sf::Event::MouseButtonPressed) {
            skipButton->handleMouseClick(mousePos, event.mouseButton.button);
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            skipButton->handleMouseRelease(mousePos, event.mouseButton.button);
        }
    }
    
    // Keyboard shortcuts
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Space:
            case sf::Keyboard::Enter:
                if (textComplete) {
                    if (choiceButtons.empty()) {
                        nextPage();
                    }
                } else {
                    skipText();
                }
                break;
            case sf::Keyboard::Escape:
                Application::getInstance()->popScene();
                break;
            default:
                break;
        }
    }
    
    // Click anywhere to advance (if no choices)
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (textComplete && choiceButtons.empty()) {
            nextPage();
        } else if (!textComplete) {
            skipText();
        }
    }
}

void DialogueScene::update(float deltaTime) {
    updateTextAnimation(deltaTime);
    
    // Update buttons
    for (auto& button : choiceButtons) {
        button->update(deltaTime);
    }
    
    if (continueButton) continueButton->update(deltaTime);
    if (skipButton) skipButton->update(deltaTime);
}

void DialogueScene::render(sf::RenderWindow& window) {
    // Darken background
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(1024, 768));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);
    
    // Draw dialogue box
    window.draw(dialogueBox);
    window.draw(nameBox);
    
    // Draw portrait if available
    if (speakerPortrait.getTexture()) {
        window.draw(portraitFrame);
        window.draw(speakerPortrait);
    }
    
    // Draw text
    window.draw(nameText);
    window.draw(dialogueText);
    
    // Draw choice buttons
    for (auto& button : choiceButtons) {
        window.draw(*button);
    }
    
    // Draw continue button if text is complete and no choices
    if (textComplete && choiceButtons.empty() && continueButton) {
        window.draw(*continueButton);
    }
    
    // Draw skip button
    if (skipButton) {
        window.draw(*skipButton);
    }
    
    // Draw instructions
    sf::Text instructionText;
    instructionText.setFont(FontManager::getInstance().getDefaultFont());
    if (!textComplete) {
        instructionText.setString("Space/Click to skip text");
    } else if (choiceButtons.empty()) {
        instructionText.setString("Space/Click to continue | ESC to exit");
    } else {
        instructionText.setString("Click a choice to respond");
    }
    instructionText.setCharacterSize(14);
    instructionText.setFillColor(sf::Color::Gray);
    instructionText.setPosition(80, 720);
    window.draw(instructionText);
}

std::unique_ptr<DialogueScene> DialogueScene::createWelcomeDialogue() {
    std::vector<DialoguePage> pages;
    
    pages.emplace_back("Narrator", "Welcome to the world of One Piece! You are about to embark on an incredible adventure to become the Pirate King!");
    pages.emplace_back("Narrator", "Your journey will take you across the Grand Line, where you'll meet allies, face powerful enemies, and search for the ultimate treasure...");
    pages.emplace_back("Narrator", "The One Piece!");
    
    auto dialogue = std::make_unique<DialogueScene>(pages);
    dialogue->setAutoAdvance(true);
    return dialogue;
}

std::unique_ptr<DialogueScene> DialogueScene::createRecruitmentDialogue(const std::string& characterName) {
    std::vector<DialoguePage> pages;
    
    if (characterName == "Zoro") {
        pages.emplace_back("Zoro", "You want me to join your crew?");
        
        DialoguePage choicePage("Zoro", "I'll only join if you're serious about becoming Pirate King.");
        choicePage.choices.push_back(DialogueChoice("I'm going to be the Pirate King!", []() {
            LOG_INFO("Player chose confident response");
        }));
        choicePage.choices.push_back(DialogueChoice("Maybe someday...", []() {
            LOG_INFO("Player chose uncertain response");
        }));
        pages.push_back(choicePage);
        
        pages.emplace_back("Zoro", "Alright then. I'll join your crew. But don't get in my way of becoming the world's greatest swordsman!");
    } else {
        pages.emplace_back(characterName, "Hello there! Are you a pirate?");
        pages.emplace_back(characterName, "Well, every great crew needs good people. I'd be honored to join!");
    }
    
    return std::make_unique<DialogueScene>(pages);
}

std::unique_ptr<DialogueScene> DialogueScene::createBattleVictoryDialogue() {
    std::vector<DialoguePage> pages;
    
    pages.emplace_back("Luffy", "Yosh! We did it, everyone!");
    pages.emplace_back("Narrator", "With another victory under your belt, you're one step closer to your dream of becoming Pirate King.");
    pages.emplace_back("Narrator", "But many more adventures await on the Grand Line...");
    
    return std::make_unique<DialogueScene>(pages);
}