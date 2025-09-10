#include "Game.h"
#include "Logger.h"
#include "graphics/TextureManager.h"
#include <fstream>
#include <filesystem>

Game::Game() : running(true) {
    // Create logs directory
    std::filesystem::create_directories("logs");
    
    // Initialize logging first
    Logger::init();
    
    LOG_INFO("🚀 Starting One Piece Adventure");
    LOG_SYSTEM_INFO("Initializing game systems...");
    
    loadConfiguration();
    
    // Create window
    int width = gameConfig.value("window", nlohmann::json{}).value("width", 1024);
    int height = gameConfig.value("window", nlohmann::json{}).value("height", 768);
    std::string title = gameConfig.value("window", nlohmann::json{}).value("title", "One Piece Adventure");
    
    window.create(sf::VideoMode(width, height), title);
    window.setFramerateLimit(60);
    
    LOG_SYSTEM_INFO("Window created: {}x{} - '{}'", width, height, title);
    
    initializeSubsystems();
    
    LOG_INFO("✅ Game initialized successfully");
}

Game::~Game() {
    LOG_INFO("🛑 Game destructor called");
    cleanup();
}

void Game::loadConfiguration() {
    LOG_SYSTEM_DEBUG("Loading game configuration...");
    
    std::ifstream configFile("assets/data/game_config.json");
    if (configFile.is_open()) {
        configFile >> gameConfig;
        LOG_SYSTEM_INFO("✅ Loaded game configuration from file");
        LOG_DEBUG("Config loaded with {} top-level keys", gameConfig.size());
    } else {
        LOG_SYSTEM_WARN("⚠️ Could not load game configuration file, using defaults");
        gameConfig = {
            {"window", {{"width", 1024}, {"height", 768}, {"title", "One Piece Adventure"}}},
            {"gameplay", {{"starting_character", "luffy"}}}
        };
    }
}

void Game::initializeSubsystems() {
    LOG_SYSTEM_INFO("Initializing game subsystems...");
    
    // Initialize texture manager
    TextureManager::getInstance();
    
    // Load some test textures
    // TextureManager::getInstance().loadTexture("test", "assets/textures/test.png");
    
    LOG_SYSTEM_INFO("✅ All subsystems initialized");
}

void Game::run() {
    LOG_INFO("🎮 Starting main game loop");
    
    sf::Clock fpsTimer;
    int frameCount = 0;
    
    while (running && window.isOpen()) {
        float deltaTime = deltaClock.restart().asSeconds();
        
        processEvents();
        update(deltaTime);
        render();
        
        // Log FPS every 5 seconds
        frameCount++;
        if (fpsTimer.getElapsedTime().asSeconds() >= 5.0f) {
            float fps = frameCount / fpsTimer.getElapsedTime().asSeconds();
            LOG_DEBUG("FPS: {:.1f}", fps);
            frameCount = 0;
            fpsTimer.restart();
        }
    }
    
    LOG_INFO("🏁 Main game loop ended");
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                LOG_INFO("👋 Window close requested");
                running = false;
                window.close();
                break;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Escape) {
                    LOG_INFO("🚪 ESC key pressed - exiting game");
                    running = false;
                }
                break;
            default:
                break;
        }
    }
}

void Game::update(float deltaTime) {
    // Game logic updates
    (void)deltaTime; // Suppress unused parameter warning
}

void Game::render() {
    window.clear(sf::Color(30, 30, 60)); // Dark blue background
    
    // Test rendering
    sf::CircleShape testShape(50);
    testShape.setFillColor(sf::Color::Red);
    testShape.setPosition(100, 100);
    window.draw(testShape);
    
    window.display();
}

void Game::cleanup() {
    LOG_SYSTEM_INFO("🧹 Starting cleanup...");
    
    TextureManager::getInstance().clear();
    
    LOG_SYSTEM_INFO("✅ Cleanup complete");
    Logger::shutdown();
}

void Game::shutdown() {
    LOG_INFO("🛑 Shutdown requested");
    running = false;
}