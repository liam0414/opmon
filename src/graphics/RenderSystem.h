#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>

enum class RenderLayer {
    Background = 0,
    Terrain = 1,
    Objects = 2,
    Characters = 3,
    Effects = 4,
    UI = 5,
    Debug = 6
};

struct RenderCommand {
    RenderLayer layer;
    float depth; // For sorting within layers
    std::function<void(sf::RenderWindow&)> renderFunc;
    
    RenderCommand(RenderLayer l, float d, std::function<void(sf::RenderWindow&)> func)
        : layer(l), depth(d), renderFunc(func) {}
};

class RenderSystem {
private:
    std::vector<RenderCommand> renderQueue;
    sf::View gameView;
    sf::View uiView;
    sf::RenderWindow* window;
    
    // Render states
    bool wireframeMode;
    bool showDebugInfo;
    sf::Color clearColor;
    
    void sortRenderQueue();

public:
    RenderSystem();
    
    void setWindow(sf::RenderWindow* win) { window = win; }
    
    // Queue management
    void submitDrawable(const sf::Drawable& drawable, RenderLayer layer, float depth = 0.0f);
    void submitCustomRender(std::function<void(sf::RenderWindow&)> renderFunc, 
                           RenderLayer layer, float depth = 0.0f);
    void clear();
    
    // Views
    void setGameView(const sf::View& view) { gameView = view; }
    void setUIView(const sf::View& view) { uiView = view; }
    const sf::View& getGameView() const { return gameView; }
    const sf::View& getUIView() const { return uiView; }
    
    // Rendering
    void render();
    
    // Settings
    void setClearColor(const sf::Color& color) { clearColor = color; }
    void setWireframeMode(bool enabled) { wireframeMode = enabled; }
    void setShowDebugInfo(bool show) { showDebugInfo = show; }
    
    // Utility
    sf::Vector2f screenToWorld(const sf::Vector2i& screenPos) const;
    sf::Vector2i worldToScreen(const sf::Vector2f& worldPos) const;
};