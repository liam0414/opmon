#include "SettingsScene.h"
#include "core/Application.h"
#include "core/Logger.h"
#include "core/ConfigManager.h"
#include "graphics/FontManager.h"

// Slider Implementation
Slider::Slider() : value(0.5f), dragging(false) {
    track.setSize(sf::Vector2f(200, 6));
    track.setFillColor(sf::Color(100, 100, 100));
    
    knob.setRadius(12);
    knob.setFillColor(sf::Color(200, 200, 200));
    knob.setOutlineColor(sf::Color::White);
    knob.setOutlineThickness(2);
}

void Slider::setValue(float val) {
    value = std::max(0.0f, std::min(1.0f, val));
    
    // Update knob position
    sf::Vector2f trackPos = track.getPosition();
    float knobX = trackPos.x + (track.getSize().x * value) - knob.getRadius();
    knob.setPosition(knobX, trackPos.y - knob.getRadius() + track.getSize().y / 2);
    
    if (onValueChanged) {
        onValueChanged(value);
    }
}

void Slider::setBounds(const sf::FloatRect& newBounds) {
    UIElement::setBounds(newBounds);
    track.setPosition(newBounds.left, newBounds.top + newBounds.height / 2 - 3);
    track.setSize(sf::Vector2f(newBounds.width, 6));
    setValue(value); // Update knob position
}

bool Slider::handleMouseClick(const sf::Vector2f& mousePos, sf::Mouse::Button button) {
    if (!visible || !interactive) return false;
    
    if (button == sf::Mouse::Left) {
        sf::FloatRect knobBounds = knob.getGlobalBounds();
        sf::FloatRect trackBounds = track.getGlobalBounds();
        
        if (knobBounds.contains(mousePos)) {
            dragging = true;
            return true;
        } else if (trackBounds.contains(mousePos)) {
            // Click on track - jump to position
            float newValue = (mousePos.x - trackBounds.left) / trackBounds.width;
            setValue(newValue);
            dragging = true;
            return true;
        }
    }
    
    return false;
}

void Slider::handleMouseRelease(const sf::Vector2f& mousePos, sf::Mouse::Button button) {
    if (button == sf::Mouse::Left) {
        dragging = false;
    }
}

bool Slider::handleMouseMove(const sf::Vector2f& mousePos) {
    if (dragging) {
        sf::FloatRect trackBounds = track.getGlobalBounds();
        float newValue = (mousePos.x - trackBounds.left) / trackBounds.width;
        setValue(newValue);
        return true;
    }
    
    return false;
}

void Slider::update(float deltaTime) {
    (void)deltaTime; // Suppress unused warning
}

void Slider::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!visible) return;
    
    states.transform *= getTransform();
    target.draw(track, states);
    target.draw(knob, states);
}

// SettingsScene Implementation
SettingsScene::SettingsScene() {
    setupUI();
    loadCurrentSettings();
}

void SettingsScene::onEnter() {
    LOG_INFO("Entered Settings Scene");
    loadCurrentSettings();
    updateSettingsDisplay();
}

void SettingsScene::onExit() {
    LOG_INFO("Exited Settings Scene");
}

void SettingsScene::setupUI() {
    auto& fontManager = FontManager::getInstance();
    
    // Background
    backgroundPanel.setSize(sf::Vector2f(800, 600));
    backgroundPanel.setPosition(112, 84);
    backgroundPanel.setFillColor(sf::Color(40, 40, 60, 240));
    backgroundPanel.setOutlineColor(sf::Color::White);
    backgroundPanel.setOutlineThickness(2);
    
    // Title
    titleText.setFont(fontManager.getDefaultFont());
    titleText.setString("SETTINGS");
    titleText.setCharacterSize(36);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(130, 100);
    
    // Audio Settings
    float startY = 180;
    float spacing = 60;
    
    // Master Volume
    sf::Text masterLabel;
    masterLabel.setFont(fontManager.getDefaultFont());
    masterLabel.setString("Master Volume:");
    masterLabel.setCharacterSize(20);
    masterLabel.setFillColor(sf::Color::White);
    masterLabel.setPosition(150, startY);
    settingsLabels.push_back(masterLabel);
    
    masterVolumeSlider = std::make_unique<Slider>();
    masterVolumeSlider->setBounds(sf::FloatRect(350, startY, 300, 30));
    masterVolumeSlider->setOnValueChanged([this](float value) {
        currentSettings.masterVolume = value;
    });
    
    // Music Volume
    sf::Text musicLabel;
    musicLabel.setFont(fontManager.getDefaultFont());
    musicLabel.setString("Music Volume:");
    musicLabel.setCharacterSize(20);
    musicLabel.setFillColor(sf::Color::White);
    musicLabel.setPosition(150, startY + spacing);
    settingsLabels.push_back(musicLabel);
    
    musicVolumeSlider = std::make_unique<Slider>();
    musicVolumeSlider->setBounds(sf::FloatRect(350, startY + spacing, 300, 30));
    musicVolumeSlider->setOnValueChanged([this](float value) {
        currentSettings.musicVolume = value;
    });
    
    // SFX Volume
// SFX Volume
    sf::Text sfxLabel;
    sfxLabel.setFont(fontManager.getDefaultFont());
    sfxLabel.setString("SFX Volume:");
    sfxLabel.setCharacterSize(20);
    sfxLabel.setFillColor(sf::Color::White);
    sfxLabel.setPosition(150, startY + spacing * 2);
    settingsLabels.push_back(sfxLabel);
    
    sfxVolumeSlider = std::make_unique<Slider>();
    sfxVolumeSlider->setBounds(sf::FloatRect(350, startY + spacing * 2, 300, 30));
    sfxVolumeSlider->setOnValueChanged([this](float value) {
        currentSettings.sfxVolume = value;
    });
    
    // Graphics Settings
    sf::Text graphicsLabel;
    graphicsLabel.setFont(fontManager.getDefaultFont());
    graphicsLabel.setString("GRAPHICS");
    graphicsLabel.setCharacterSize(24);
    graphicsLabel.setFillColor(sf::Color::Yellow);
    graphicsLabel.setPosition(150, startY + spacing * 3.5f);
    settingsLabels.push_back(graphicsLabel);
    
    // Resolution buttons
    sf::Text resLabel;
    resLabel.setFont(fontManager.getDefaultFont());
    resLabel.setString("Resolution:");
    resLabel.setCharacterSize(20);
    resLabel.setFillColor(sf::Color::White);
    resLabel.setPosition(150, startY + spacing * 4.5f);
    settingsLabels.push_back(resLabel);
    
    std::vector<sf::Vector2u> resolutions = {{800, 600}, {1024, 768}, {1280, 720}, {1920, 1080}};
    float buttonY = startY + spacing * 4.5f;
    
    for (size_t i = 0; i < resolutions.size(); ++i) {
        std::string resText = std::to_string(resolutions[i].x) + "x" + std::to_string(resolutions[i].y);
        auto button = std::make_unique<Button>(resText, fontManager.getDefaultFont());
        button->setBounds(sf::FloatRect(350 + i * 110, buttonY, 100, 30));
        
        sf::Color normal(60, 80, 100, 200);
        sf::Color hover(80, 100, 120, 220);
        sf::Color selected(100, 150, 200, 240);
        
        button->setColors(normal, hover, selected, sf::Color::Gray);
        
        sf::Vector2u resolution = resolutions[i];
        button->setOnClick([this, resolution]() {
            currentSettings.resolution = resolution;
            updateSettingsDisplay();
        });
        
        resolutionButtons.push_back(std::move(button));
    }
    
    // Fullscreen toggle
    fullscreenButton = std::make_unique<Button>("Fullscreen: OFF", fontManager.getDefaultFont());
    fullscreenButton->setBounds(sf::FloatRect(350, startY + spacing * 5.5f, 200, 40));
    fullscreenButton->setColors(sf::Color(80, 80, 80, 200), sf::Color(100, 100, 100, 220),
                               sf::Color(60, 60, 60, 240), sf::Color::Gray);
    fullscreenButton->setOnClick([this]() {
        currentSettings.fullscreen = !currentSettings.fullscreen;
        updateSettingsDisplay();
    });
    
    // VSync toggle
    vsyncButton = std::make_unique<Button>("VSync: ON", fontManager.getDefaultFont());
    vsyncButton->setBounds(sf::FloatRect(570, startY + spacing * 5.5f, 150, 40));
    vsyncButton->setColors(sf::Color(80, 80, 80, 200), sf::Color(100, 100, 100, 220),
                          sf::Color(60, 60, 60, 240), sf::Color::Gray);
    vsyncButton->setOnClick([this]() {
        currentSettings.vsync = !currentSettings.vsync;
        updateSettingsDisplay();
    });
    
    // Control buttons
    applyButton = std::make_unique<Button>("Apply", fontManager.getDefaultFont());
    applyButton->setBounds(sf::FloatRect(300, 620, 100, 40));
    applyButton->setColors(sf::Color(0, 150, 0, 200), sf::Color(0, 180, 0, 220),
                          sf::Color(0, 120, 0, 240), sf::Color::Gray);
    applyButton->setOnClick([this]() { applySettings(); });
    
    resetButton = std::make_unique<Button>("Reset", fontManager.getDefaultFont());
    resetButton->setBounds(sf::FloatRect(420, 620, 100, 40));
    resetButton->setColors(sf::Color(150, 150, 0, 200), sf::Color(180, 180, 0, 220),
                          sf::Color(120, 120, 0, 240), sf::Color::Gray);
    resetButton->setOnClick([this]() { resetToDefaults(); });
    
    backButton = std::make_unique<Button>("Back", fontManager.getDefaultFont());
    backButton->setBounds(sf::FloatRect(540, 620, 100, 40));
    backButton->setColors(sf::Color(100, 100, 100, 200), sf::Color(120, 120, 120, 220),
                         sf::Color(80, 80, 80, 240), sf::Color::Gray);
    backButton->setOnClick([this]() {
        Application::getInstance()->popScene();
    });
}

void SettingsScene::loadCurrentSettings() {
    auto& config = ConfigManager::getInstance();
    
    // Load audio settings
    currentSettings.masterVolume = config.get<float>("audio.master_volume", 100.0f) / 100.0f;
    currentSettings.musicVolume = config.get<float>("audio.music_volume", 80.0f) / 100.0f;
    currentSettings.sfxVolume = config.get<float>("audio.sfx_volume", 90.0f) / 100.0f;
    
    // Load graphics settings
    currentSettings.resolution.x = config.get<int>("window.width", 1024);
    currentSettings.resolution.y = config.get<int>("window.height", 768);
    currentSettings.fullscreen = config.get<bool>("window.fullscreen", false);
    currentSettings.vsync = config.get<bool>("window.vsync", true);
    
    // Store original settings for comparison
    originalSettings = currentSettings;
}

void SettingsScene::applySettings() {
    auto& config = ConfigManager::getInstance();
    auto& app = *Application::getInstance();
    
    // Apply audio settings
    config.set("audio.master_volume", currentSettings.masterVolume * 100.0f);
    config.set("audio.music_volume", currentSettings.musicVolume * 100.0f);
    config.set("audio.sfx_volume", currentSettings.sfxVolume * 100.0f);
    
    app.getAudioManager().setMasterVolume(currentSettings.masterVolume * 100.0f);
    app.getAudioManager().setMusicVolume(currentSettings.musicVolume * 100.0f);
    app.getAudioManager().setSfxVolume(currentSettings.sfxVolume * 100.0f);
    
    // Apply graphics settings
    config.set("window.width", static_cast<int>(currentSettings.resolution.x));
    config.set("window.height", static_cast<int>(currentSettings.resolution.y));
    config.set("window.fullscreen", currentSettings.fullscreen);
    config.set("window.vsync", currentSettings.vsync);
    
    // Apply window changes if they're different
    if (currentSettings.resolution != originalSettings.resolution ||
        currentSettings.fullscreen != originalSettings.fullscreen ||
        currentSettings.vsync != originalSettings.vsync) {
        
        // Recreate window with new settings
        sf::Uint32 style = currentSettings.fullscreen ? sf::Style::Fullscreen : sf::Style::Default;
        app.getWindow().create(sf::VideoMode(currentSettings.resolution.x, currentSettings.resolution.y),
                              config.get<std::string>("window.title", "One Piece Adventure"), style);
        
        if (currentSettings.vsync) {
            app.getWindow().setVerticalSyncEnabled(true);
        } else {
            app.getWindow().setFramerateLimit(config.get<int>("window.framerate_limit", 60));
        }
        
        LOG_INFO("Applied graphics settings: {}x{}, Fullscreen: {}, VSync: {}",
                currentSettings.resolution.x, currentSettings.resolution.y,
                currentSettings.fullscreen, currentSettings.vsync);
    }
    
    // Save configuration to file
    config.saveConfig();
    
    // Update original settings
    originalSettings = currentSettings;
    
    LOG_INFO("Settings applied and saved");
    
    // Show confirmation (could be a temporary message)
    // For now, just log it
}

void SettingsScene::resetToDefaults() {
    currentSettings.masterVolume = 1.0f;
    currentSettings.musicVolume = 0.8f;
    currentSettings.sfxVolume = 0.9f;
    currentSettings.resolution = {1024, 768};
    currentSettings.fullscreen = false;
    currentSettings.vsync = true;
    
    updateSettingsDisplay();
    
    LOG_INFO("Settings reset to defaults");
}

void SettingsScene::updateSettingsDisplay() {
    // Update sliders
    masterVolumeSlider->setValue(currentSettings.masterVolume);
    musicVolumeSlider->setValue(currentSettings.musicVolume);
    sfxVolumeSlider->setValue(currentSettings.sfxVolume);
    
    // Update resolution button states
    for (size_t i = 0; i < resolutionButtons.size(); ++i) {
        sf::Vector2u buttonRes;
        switch (i) {
            case 0: buttonRes = {800, 600}; break;
            case 1: buttonRes = {1024, 768}; break;
            case 2: buttonRes = {1280, 720}; break;
            case 3: buttonRes = {1920, 1080}; break;
        }
        
        if (buttonRes == currentSettings.resolution) {
            resolutionButtons[i]->setState(UIState::Pressed);
        } else {
            resolutionButtons[i]->setState(UIState::Normal);
        }
    }
    
    // Update toggle button text
    fullscreenButton->setText(currentSettings.fullscreen ? "Fullscreen: ON" : "Fullscreen: OFF");
    vsyncButton->setText(currentSettings.vsync ? "VSync: ON" : "VSync: OFF");
}

void SettingsScene::handleEvent(const sf::Event& event) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
    
    // Handle sliders
    std::vector<Slider*> sliders = {masterVolumeSlider.get(), musicVolumeSlider.get(), sfxVolumeSlider.get()};
    
    for (auto* slider : sliders) {
        if (slider) {
            if (event.type == sf::Event::MouseMoved) {
                slider->handleMouseMove(mousePos);
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                slider->handleMouseClick(mousePos, event.mouseButton.button);
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                slider->handleMouseRelease(mousePos, event.mouseButton.button);
            }
        }
    }
    
    // Handle buttons
    std::vector<Button*> buttons = {
        fullscreenButton.get(), vsyncButton.get(),
        applyButton.get(), resetButton.get(), backButton.get()
    };
    
    // Add resolution buttons
    for (auto& btn : resolutionButtons) {
        buttons.push_back(btn.get());
    }
    
    for (auto* button : buttons) {
        if (button) {
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
    }
    
    // Keyboard shortcuts
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Escape:
                Application::getInstance()->popScene();
                break;
            case sf::Keyboard::Enter:
                applySettings();
                break;
            case sf::Keyboard::R:
                if (event.key.control) {
                    resetToDefaults();
                }
                break;
            default:
                break;
        }
    }
}

void SettingsScene::update(float deltaTime) {
    // Update sliders
    masterVolumeSlider->update(deltaTime);
    musicVolumeSlider->update(deltaTime);
    sfxVolumeSlider->update(deltaTime);
    
    // Update buttons
    for (auto& button : resolutionButtons) {
        button->update(deltaTime);
    }
    
    fullscreenButton->update(deltaTime);
    vsyncButton->update(deltaTime);
    applyButton->update(deltaTime);
    resetButton->update(deltaTime);
    backButton->update(deltaTime);
}

void SettingsScene::render(sf::RenderWindow& window) {
    // Clear background
    window.clear(sf::Color(15, 20, 30));
    
    // Draw background panel
    window.draw(backgroundPanel);
    
    // Draw title
    window.draw(titleText);
    
    // Draw labels
    for (const auto& label : settingsLabels) {
        window.draw(label);
    }
    
    // Draw sliders
    window.draw(*masterVolumeSlider);
    window.draw(*musicVolumeSlider);
    window.draw(*sfxVolumeSlider);
    
    // Draw volume percentages
    auto& fontManager = FontManager::getInstance();
    
    sf::Text masterPercent, musicPercent, sfxPercent;
    masterPercent.setFont(fontManager.getDefaultFont());
    musicPercent.setFont(fontManager.getDefaultFont());
    sfxPercent.setFont(fontManager.getDefaultFont());
    
    masterPercent.setString(std::to_string(static_cast<int>(currentSettings.masterVolume * 100)) + "%");
    musicPercent.setString(std::to_string(static_cast<int>(currentSettings.musicVolume * 100)) + "%");
    sfxPercent.setString(std::to_string(static_cast<int>(currentSettings.sfxVolume * 100)) + "%");
    
    masterPercent.setCharacterSize(18);
    musicPercent.setCharacterSize(18);
    sfxPercent.setCharacterSize(18);
    
    masterPercent.setFillColor(sf::Color::White);
    musicPercent.setFillColor(sf::Color::White);
    sfxPercent.setFillColor(sf::Color::White);
    
    masterPercent.setPosition(670, 180);
    musicPercent.setPosition(670, 240);
    sfxPercent.setPosition(670, 300);
    
    window.draw(masterPercent);
    window.draw(musicPercent);
    window.draw(sfxPercent);
    
    // Draw resolution buttons
    for (auto& button : resolutionButtons) {
        window.draw(*button);
    }
    
    // Draw toggle buttons
    window.draw(*fullscreenButton);
    window.draw(*vsyncButton);
    
    // Draw control buttons
    window.draw(*applyButton);
    window.draw(*resetButton);
    window.draw(*backButton);
    
    // Draw instructions
    sf::Text instructionText;
    instructionText.setFont(fontManager.getDefaultFont());
    instructionText.setString("Enter to Apply | Ctrl+R to Reset | ESC to Cancel");
    instructionText.setCharacterSize(14);
    instructionText.setFillColor(sf::Color::Gray);
    instructionText.setPosition(150, 680);
    window.draw(instructionText);
}