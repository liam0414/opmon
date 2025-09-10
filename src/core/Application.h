#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <stack>

class Scene;
class InputManager;
class AudioManager;

class Application {
private:
    sf::RenderWindow window;
    std::stack<std::unique_ptr<Scene>> sceneStack;
    std::unique_ptr<InputManager> inputManager;
    std::unique_ptr<AudioManager> audioManager;
    
    bool running;
    sf::Clock deltaClock;
    sf::Clock fpsTimer;
    int frameCount;
    
    void initializeSubsystems();
    void processEvents();
    void update(float deltaTime);
    void render();
    void cleanup();
    void calculateFPS();

public:
    Application();
    ~Application();
    
    void run();
    void shutdown();
    
    // Scene management
    void pushScene(std::unique_ptr<Scene> scene);
    void popScene();
    void changeScene(std::unique_ptr<Scene> scene);
    
    // Getters
    sf::RenderWindow& getWindow() { return window; }
    InputManager& getInputManager() { return *inputManager; }
    AudioManager& getAudioManager() { return *audioManager; }
    
    static Application* getInstance();
};