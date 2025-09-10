#include "Application.h"
#include "Logger.h"
#include "ConfigManager.h"
#include "InputManager.h"
#include "AudioManager.h"
#include "Scene.h"
#include "graphics/TextureManager.h"
#include <filesystem>

static Application* s_instance = nullptr;

Application::Application() : running(true), frameCount(0) {
    s_instance = this;
    
    // Create necessary directories
    std::filesystem::create_directories("logs");
    std::filesystem::create_directories("saves");
    
    Logger::init();
    LOG_INFO("🚀 Starting One Piece Adventure");
    
    initializeSubsystems();
}

Application::~Application() {
    cleanup();
    s_instance = nullptr;
}

Application* Application::getInstance() {
    return s_instance;
}

void Application::initializeSubsystems() {
    LOG_SYSTEM_INFO("Initializing application subsystems...");
    
    // Load configuration
    ConfigManager::getInstance().loadConfig("assets/data/game_config.json");
    
    // Create window
    auto& config = ConfigManager::getInstance();
    int width = config.get<int>("window.width", 1024);
    int height = config.get<int>("window.height", 768);
    std::string title = config.get<std::string>("window.title", "One Piece Adventure");
    bool fullscreen = config.get<bool>("window.fullscreen", false);
    
    sf::Uint32 style = fullscreen ? sf::Style::Fullscreen : sf::Style::Default;
    window.create(sf::VideoMode(width, height), title, style);
    window.setFramerateLimit(config.get<int>("window.framerate_limit", 60));
    
    if (config.get<bool>("window.vsync", true)) {
        window.setVerticalSyncEnabled(true);
    }
    
    // Initialize managers
    inputManager = std::make_unique<InputManager>();
    audioManager = std::make_unique<AudioManager>();
    
    // Initialize texture manager
    TextureManager::getInstance();
    
    LOG_SYSTEM_INFO("✅ Window created: {}x{} - '{}'", width, height, title);
    LOG_SYSTEM_INFO("✅ All subsystems initialized");
}

void Application::run() {
    LOG_INFO("🎮 Starting main application loop");
    
    while (running && window.isOpen()) {
        float deltaTime = deltaClock.restart().asSeconds();
        
        processEvents();
        update(deltaTime);
        render();
        calculateFPS();
    }
    
    LOG_INFO("🏁 Application loop ended");
}

void Application::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            LOG_INFO("👋 Window close requested");
            shutdown();
        }
        
        // Let input manager handle events
        inputManager->handleEvent(event);
        
        // Let current scene handle events
        if (!sceneStack.empty()) {
            sceneStack.top()->handleEvent(event);
        }
    }
}

void Application::update(float deltaTime) {
    inputManager->update();
    
    if (!sceneStack.empty()) {
        sceneStack.top()->update(deltaTime);
    }
}

void Application::render() {
    window.clear(sf::Color(20, 25, 40)); // Dark blue-grey background
    
    if (!sceneStack.empty()) {
        sceneStack.top()->render(window);
    }
    
    window.display();
}

void Application::calculateFPS() {
    frameCount++;
    if (fpsTimer.getElapsedTime().asSeconds() >= 5.0f) {
        float fps = frameCount / fpsTimer.getElapsedTime().asSeconds();
        LOG_DEBUG("FPS: {:.1f}", fps);
        frameCount = 0;
        fpsTimer.restart();
    }
}

void Application::pushScene(std::unique_ptr<Scene> scene) {
    if (!sceneStack.empty()) {
        sceneStack.top()->onPause();
    }
    
    sceneStack.push(std::move(scene));
    sceneStack.top()->onEnter();
    
    LOG_INFO("Pushed scene, stack size: {}", sceneStack.size());
}

void Application::popScene() {
    if (!sceneStack.empty()) {
        sceneStack.top()->onExit();
        sceneStack.pop();
        
        if (!sceneStack.empty()) {
            sceneStack.top()->onResume();
        }
        
        LOG_INFO("Popped scene, stack size: {}", sceneStack.size());
    }
}

void Application::changeScene(std::unique_ptr<Scene> scene) {
    // Clear all scenes and push new one
    while (!sceneStack.empty()) {
        popScene();
    }
    pushScene(std::move(scene));
}

void Application::cleanup() {
    LOG_SYSTEM_INFO("🧹 Starting application cleanup...");
    
    // Clear scenes
    while (!sceneStack.empty()) {
        sceneStack.pop();
    }
    
    // Cleanup managers
    TextureManager::getInstance().clear();
    audioManager.reset();
    inputManager.reset();
    
    LOG_SYSTEM_INFO("✅ Application cleanup complete");
    Logger::shutdown();
}

void Application::shutdown() {
    running = false;
}