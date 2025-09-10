#pragma once
#include "core/Scene.h"
#include "graphics/Button.h"
#include "graphics/UIElement.h"
#include <memory>
#include <vector>

class Slider : public UIElement {
private:
    sf::RectangleShape track;
    sf::CircleShape knob;
    float value; // 0.0 to 1.0
    bool dragging;
    std::function<void(float)> onValueChanged;

public:
    Slider();
    void setValue(float val);
    float getValue() const { return value; }
    void setOnValueChanged(std::function<void(float)> callback) { onValueChanged = callback; }
    
    void setBounds(const sf::FloatRect& newBounds) override;
    bool handleMouseClick(const sf::Vector2f& mousePos, sf::Mouse::Button button) override;
    void handleMouseRelease(const sf::Vector2f& mousePos, sf::Mouse::Button button) override;
    bool handleMouseMove(const sf::Vector2f& mousePos) override;
    void update(float deltaTime) override;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

class SettingsScene : public Scene {
private:
    // UI Elements
    std::unique_ptr<Slider> masterVolumeSlider;
    std::unique_ptr<Slider> musicVolumeSlider;
    std::unique_ptr<Slider> sfxVolumeSlider;
    
    std::vector<std::unique_ptr<Button>> resolutionButtons;
    std::unique_ptr<Button> fullscreenButton;
    std::unique_ptr<Button> vsyncButton;
    std::unique_ptr<Button> applyButton;
    std::unique_ptr<Button> resetButton;
    std::unique_ptr<Button> backButton;
    
    // Settings labels
    std::vector<sf::Text> settingsLabels;
    sf::Text titleText;
    
    // Background
    sf::RectangleShape backgroundPanel;
    
    // Settings values
    struct Settings {
        float masterVolume = 1.0f;
        float musicVolume = 0.8f;
        float sfxVolume = 0.9f;
        sf::Vector2u resolution = {1024, 768};
        bool fullscreen = false;
        bool vsync = true;
    } currentSettings, originalSettings;
    
    void setupUI();
    void loadCurrentSettings();
    void applySettings();
    void resetToDefaults();
    void updateSettingsDisplay();

public:
    SettingsScene();
    
    void onEnter() override;
    void onExit() override;
    
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};