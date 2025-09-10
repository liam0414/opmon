#pragma once
#include "core/Scene.h"
#include "graphics/Button.h"
#include "characters/Character.h"
#include <memory>
#include <vector>
#include <functional>

struct DialogueChoice {
    std::string text;
    std::function<void()> action;
    bool available;
    
    DialogueChoice(const std::string& t, std::function<void()> a, bool avail = true)
        : text(t), action(a), available(avail) {}
};

struct DialoguePage {
    std::string speakerName;
    std::string text;
    std::string portraitTexture;
    std::vector<DialogueChoice> choices;
    float displaySpeed; // Characters per second
    
    DialoguePage(const std::string& speaker, const std::string& dialogueText, 
                const std::string& portrait = "", float speed = 30.0f)
        : speakerName(speaker), text(dialogueText), portraitTexture(portrait), displaySpeed(speed) {}
};

class DialogueScene : public Scene {
private:
    // UI Elements
    sf::RectangleShape dialogueBox;
    sf::RectangleShape nameBox;
    sf::RectangleShape portraitFrame;
    sf::Sprite speakerPortrait;
    sf::Text nameText;
    sf::Text dialogueText;
    
    std::vector<std::unique_ptr<Button>> choiceButtons;
    std::unique_ptr<Button> continueButton;
    std::unique_ptr<Button> skipButton;
    
    // Dialogue data
    std::vector<DialoguePage> pages;
    int currentPage;
    
    // Text animation
    std::string fullText;
    std::string displayedText;
    float textTimer;
    bool textComplete;
    bool autoAdvance;
    
    // Callbacks
    std::function<void()> onDialogueComplete;
    
    void setupUI();
    void loadPage(int pageIndex);
    void updateTextAnimation(float deltaTime);
    void showChoices();
    void hideChoices();
    void nextPage();
    void skipText();

public:
    DialogueScene(const std::vector<DialoguePage>& dialoguePages);
    
    void setOnDialogueComplete(std::function<void()> callback) { onDialogueComplete = callback; }
    void setAutoAdvance(bool auto_advance) { autoAdvance = auto_advance; }
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    
    // Static factory methods for common dialogues
    static std::unique_ptr<DialogueScene> createWelcomeDialogue();
    static std::unique_ptr<DialogueScene> createRecruitmentDialogue(const std::string& characterName);
    static std::unique_ptr<DialogueScene> createBattleVictoryDialogue();
};