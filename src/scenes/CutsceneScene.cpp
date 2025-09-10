#include "CutsceneScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "graphics/TextureManager.h"
#include "graphics/FontManager.h"

CutsceneScene::CutsceneScene(const std::vector<CutsceneFrame>& cutsceneFrames) 
    : frames(cutsceneFrames), currentFrame(0), frameTimer(0), fadeAlpha(0), 
      fading(false), skipRequested(false), textTimer(0), textSpeed(30.0f), textComplete(false) {
    
    effectSystem = std::make_unique<ParticleSystem>();
    
    // Setup UI
    auto& fontManager = FontManager::getInstance();
    
    // Text box
    textBox.setSize(sf::Vector2f(900, 150));
    textBox.setPosition(62, 550);
    textBox.setFillColor(sf::Color(0, 0, 0, 200));
    textBox.setOutlineColor(sf::Color::White);
    textBox.setOutlineThickness(2);
    
    // Dialogue text
    dialogueText.setFont(fontManager.getDefaultFont());
    dialogueText.setCharacterSize(18);
    dialogueText.setFillColor(sf::Color::White);
    dialogueText.setPosition(80, 570);
    
    // Speaker text
    speakerText.setFont(fontManager.getDefaultFont());
    speakerText.setCharacterSize(22);
    speakerText.setFillColor(sf::Color::Yellow);
    speakerText.setPosition(80, 520);
    
    // Fade overlay
    fadeOverlay.setSize(sf::Vector2f(1024, 768));
    fadeOverlay.setFillColor(sf::Color(0, 0, 0, 0));
    
    // Setup particle system for effects
    effectSystem->setEmissionShape(EmissionShape::Rectangle, sf::Vector2f(1024, 100));
    effectSystem->setEmissionRate(20.0f);
    effectSystem->setLifeRange(2.0f, 5.0f);
    effectSystem->setSizeRange(1.0f, 4.0f);
    effectSystem->setColors(sf::Color(255, 255, 255, 100), sf::Color::Transparent);
}

void CutsceneScene::onEnter() {
    LOG_INFO("Started cutscene with {} frames", frames.size());
    
    if (!frames.empty()) {
        loadFrame(0);
    }
}

void CutsceneScene::onExit() {
    LOG_INFO("Ended cutscene");
    
    if (onCutsceneComplete) {
        onCutsceneComplete();
    }
}

void CutsceneScene::loadFrame(int frameIndex) {
    if (frameIndex < 0 || frameIndex >= static_cast<int>(frames.size())) {
        // End of cutscene
        Application::getInstance()->popScene();
        return;
    }
    
    currentFrame = frameIndex;
    frameTimer = 0;
    
    const CutsceneFrame& frame = frames[currentFrame];
    auto& textureManager = TextureManager::getInstance();
    
    // Load background
    if (!frame.backgroundTexture.empty()) {
        if (textureManager.loadTexture("cutscene_bg", frame.backgroundTexture)) {
            backgroundSprite.setTexture(textureManager.getTexture("cutscene_bg"));
            
            // Scale to fit screen
            sf::Vector2u bgSize = textureManager.getTexture("cutscene_bg").getSize();
            float scaleX = 1024.0f / bgSize.x;
            float scaleY = 768.0f / bgSize.y;
            backgroundSprite.setScale(scaleX, scaleY);
        }
    }
    
    // Load character sprites
    characterSprites.clear();
    for (size_t i = 0; i < frame.characterTextures.size(); ++i) {
        sf::Sprite charSprite;
        std::string textureName = "cutscene_char_" + std::to_string(i);
        
        if (textureManager.loadTexture(textureName, frame.characterTextures[i])) {
            charSprite.setTexture(textureManager.getTexture(textureName));
            
            // Position character
            if (i < frame.characterPositions.size()) {
                charSprite.setPosition(frame.characterPositions[i]);
            } else {
                // Default positioning
                float x = 200.0f + i * 300.0f;
                charSprite.setPosition(x, 200.0f);
            }
            
            characterSprites.push_back(charSprite);
        }
    }
    
    // Setup dialogue
    speakerText.setString(frame.speakerName);
    fullText = frame.dialogue;
    displayedText = "";
    textTimer = 0;
    textComplete = false;
    
    // Play music if specified
    if (!frame.musicTrack.empty()) {
        Application::getInstance()->getAudioManager().playMusic(frame.musicTrack, true);
    }
    
    // Setup effects based on frame content
    if (frame.speakerName.find("Devil Fruit") != std::string::npos || 
        frame.dialogue.find("power") != std::string::npos) {
        // Magical effects for Devil Fruit scenes
        effectSystem->setColors(sf::Color(255, 100, 255, 150), sf::Color::Transparent);
        effectSystem->setEmissionPosition(sf::Vector2f(512, 200));
        effectSystem->start();
    }
    
    LOG_DEBUG("Loaded cutscene frame {}: {}", frameIndex, frame.speakerName);
}

void CutsceneScene::updateTextAnimation(float deltaTime) {
    if (textComplete) return;
    
    textTimer += deltaTime;
    float charsToShow = textTimer * textSpeed;
    size_t targetLength = std::min(static_cast<size_t>(charsToShow), fullText.length());
    
    if (targetLength != displayedText.length()) {
        displayedText = fullText.substr(0, targetLength);
        
        // Simple word wrapping
        std::string wrappedText = "";
        std::string currentLine = "";
        std::istringstream words(displayedText);
        std::string word;
        
        while (std::getline(words, word, ' ')) {
            if (currentLine.length() + word.length() + 1 > 60) {
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
    }
}

void CutsceneScene::updateFadeEffect(float deltaTime) {
    if (fading) {
        fadeAlpha += deltaTime * 255.0f; // Fade in 1 second
        
        if (fadeAlpha >= 255.0f) {
            fadeAlpha = 255.0f;
            fading = false;
            // Could trigger next frame or end cutscene here
        }
        
        sf::Uint8 alpha = static_cast<sf::Uint8>(fadeAlpha);
        fadeOverlay.setFillColor(sf::Color(0, 0, 0, alpha));
    }
}

void CutsceneScene::nextFrame() {
    currentFrame++;
    if (currentFrame < static_cast<int>(frames.size())) {
        loadFrame(currentFrame);
    } else {
        Application::getInstance()->popScene();
    }
}

void CutsceneScene::skipCutscene() {
    LOG_INFO("Cutscene skipped by user");
    Application::getInstance()->popScene();
}

void CutsceneScene::handleEvent(const sf::Event& event) {
    // Skip cutscene on ESC
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        skipCutscene();
        return;
    }
    
    // Advance cutscene on various inputs
    if (event.type == sf::Event::KeyPressed || 
        (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)) {
        
        if (!textComplete) {
            // Skip text animation
            displayedText = fullText;
            dialogueText.setString(displayedText);
            textComplete = true;
        } else {
            // Advance to next frame
            nextFrame();
        }
    }
}

void CutsceneScene::update(float deltaTime) {
    const CutsceneFrame& frame = frames[currentFrame];
    
    // Update text animation
    updateTextAnimation(deltaTime);
    
    // Update effects
    effectSystem->update(deltaTime);
    
    // Update fade effects
    updateFadeEffect(deltaTime);
    
    // Auto-advance frame if duration is set
    if (frame.duration > 0) {
        frameTimer += deltaTime;
        if (frameTimer >= frame.duration && textComplete) {
            nextFrame();
        }
    }
    
    // Character animation (simple floating effect)
    static float animationTimer = 0;
    animationTimer += deltaTime;
    
    for (size_t i = 0; i < characterSprites.size(); ++i) {
        sf::Vector2f originalPos = characterSprites[i].getPosition();
        float bobOffset = std::sin(animationTimer * 2.0f + i * 0.5f) * 3.0f;
        characterSprites[i].setPosition(originalPos.x, originalPos.y + bobOffset);
    }
}

void CutsceneScene::render(sf::RenderWindow& window) {
    // Draw background
    if (backgroundSprite.getTexture()) {
        window.draw(backgroundSprite);
    } else {
        window.clear(sf::Color::Black);
    }
    
    // Draw effects behind characters
    window.draw(*effectSystem);
    
    // Draw characters
    for (const auto& sprite : characterSprites) {
        window.draw(sprite);
    }
    
    // Draw dialogue box and text
    if (!fullText.empty()) {
        window.draw(textBox);
        if (!speakerText.getString().isEmpty()) {
            window.draw(speakerText);
        }
        window.draw(dialogueText);
    }
    
    // Draw fade overlay
    window.draw(fadeOverlay);
    
    // Draw skip instructions
    sf::Text skipText;
    skipText.setFont(FontManager::getInstance().getDefaultFont());
    skipText.setString("Press ESC to skip | Space/Click to advance");
    skipText.setCharacterSize(14);
    skipText.setFillColor(sf::Color::White);
    skipText.setPosition(10, 10);
    window.draw(skipText);
    
    // Draw progress indicator
    sf::Text progressText;
    progressText.setFont(FontManager::getInstance().getDefaultFont());
    progressText.setString(std::to_string(currentFrame + 1) + "/" + std::to_string(frames.size()));
    progressText.setCharacterSize(16);
    progressText.setFillColor(sf::Color::Gray);
    progressText.setPosition(950, 10);
    window.draw(progressText);
}

std::unique_ptr<CutsceneScene> CutsceneScene::createGameIntro() {
    std::vector<CutsceneFrame> frames;
    
    // Frame 1: Ocean scene
    CutsceneFrame frame1;
    frame1.backgroundTexture = "assets/textures/cutscenes/ocean_wide.png";
    frame1.musicTrack = "grand_line_theme";
    frame1.speakerName = "Narrator";
    frame1.dialogue = "Long ago, the legendary Pirate King Gol D. Roger conquered the Grand Line and acquired everything the world had to offer.";
    frame1.duration = 5.0f;
    frames.push_back(frame1);
    
    // Frame 2: Execution scene
    CutsceneFrame frame2;
    frame2.backgroundTexture = "assets/textures/cutscenes/execution_platform.png";
    frame2.speakerName = "Narrator";
    frame2.dialogue = "His final words before his execution sent countless pirates to the seas...";
    frame2.duration = 4.0f;
    frames.push_back(frame2);
    
    // Frame 3: Roger's speech
    CutsceneFrame frame3;
    frame3.backgroundTexture = "assets/textures/cutscenes/execution_platform.png";
    frame3.characterTextures = {"assets/textures/characters/gol_d_roger.png"};
    frame3.characterPositions = {{400, 200}};
    frame3.speakerName = "Gol D. Roger";
    frame3.dialogue = "My treasure? If you want it, I'll let you have it. Look for it! I left all of it at that place!";
    frames.push_back(frame3);
    
    // Frame 4: Great Pirate Era begins
    CutsceneFrame frame4;
    frame4.backgroundTexture = "assets/textures/cutscenes/pirate_ships.png";
    frame4.speakerName = "Narrator";
    frame4.dialogue = "And so began the Great Pirate Era! Now, a new adventure is about to unfold...";
    frame4.duration = 4.0f;
    frames.push_back(frame4);
    
    return std::make_unique<CutsceneScene>(frames);
}

std::unique_ptr<CutsceneScene> CutsceneScene::createDevilFruitAwakening() {
    std::vector<CutsceneFrame> frames;
    
    CutsceneFrame frame1;
    frame1.backgroundTexture = "assets/textures/cutscenes/mystical_aura.png";
    frame1.musicTrack = "devil_fruit_power";
    frame1.speakerName = "Narrator";
    frame1.dialogue = "A strange power flows through you... Your Devil Fruit abilities are awakening!";
    frames.push_back(frame1);
    
    CutsceneFrame frame2;
    frame2.backgroundTexture = "assets/textures/cutscenes/power_surge.png";
    frame2.speakerName = "???";
    frame2.dialogue = "This power... it transcends the limits of a normal Devil Fruit user!";
    frames.push_back(frame2);
    
    CutsceneFrame frame3;
    frame3.backgroundTexture = "assets/textures/cutscenes/awakened_power.png";
    frame3.speakerName = "Narrator";
    frame3.dialogue = "Your Devil Fruit has awakened! New abilities are now at your command!";
    frames.push_back(frame3);
    
    return std::make_unique<CutsceneScene>(frames);
}

std::unique_ptr<CutsceneScene> CutsceneScene::createCrewReunion() {
    std::vector<CutsceneFrame> frames;
    
    CutsceneFrame frame1;
    frame1.backgroundTexture = "assets/textures/cutscenes/sunny_deck.png";
    frame1.musicTrack = "nakama_theme";
    frame1.speakerName = "Luffy";
    frame1.dialogue = "Everyone! It's so good to see you all again!";
    frames.push_back(frame1);
    
    CutsceneFrame frame2;
    frame2.backgroundTexture = "assets/textures/cutscenes/sunny_deck.png";
    frame2.characterTextures = {
        "assets/textures/characters/luffy.png",
        "assets/textures/characters/zoro.png", 
        "assets/textures/characters/nami.png"
    };
    frame2.characterPositions = {{300, 200}, {500, 200}, {700, 200}};
    frame2.speakerName = "Narrator";
    frame2.dialogue = "The Straw Hat crew is reunited at last! Their bond stronger than ever, they're ready for the challenges ahead.";
    frames.push_back(frame2);
    
    return std::make_unique<CutsceneScene>(frames);
}