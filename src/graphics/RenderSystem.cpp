#include "RenderSystem.h"
#include "core/Logger.h"
#include <algorithm>

RenderSystem::RenderSystem() 
    : window(nullptr), wireframeMode(false), showDebugInfo(false), 
      clearColor(sf::Color(20, 25, 40)) {
    
    // Initialize default views
    gameView.setSize(1024, 768);
    gameView.setCenter(512, 384);
    
    uiView.setSize(1024, 768);
    uiView.setCenter(512, 384);
}

void RenderSystem::submitDrawable(const sf::Drawable& drawable, RenderLayer layer, float depth) {
    renderQueue.emplace_back(layer, depth, [&drawable](sf::RenderWindow& win) {
        win.draw(drawable);
    });
}

void RenderSystem::submitCustomRender(std::function<void(sf::RenderWindow&)> renderFunc, 
                                     RenderLayer layer, float depth) {
    renderQueue.emplace_back(layer, depth, renderFunc);
}

void RenderSystem::sortRenderQueue() {
    std::sort(renderQueue.begin(), renderQueue.end(), 
        [](const RenderCommand& a, const RenderCommand& b) {
            if (a.layer != b.layer) {
                return static_cast<int>(a.layer) < static_cast<int>(b.layer);
            }
            return a.depth < b.depth;
        });
}

void RenderSystem::render() {
    if (!window) {
        LOG_ERROR("RenderSystem: No window set!");
        return;
    }
    
    window->clear(clearColor);
    
    if (renderQueue.empty()) {
        window->display();
        return;
    }
    
    sortRenderQueue();
    
    RenderLayer currentLayer = static_cast<RenderLayer>(-1);
    
    for (const auto& command : renderQueue) {
        // Switch views based on layer
        if (command.layer != currentLayer) {
            currentLayer = command.layer;
            
            if (currentLayer == RenderLayer::UI || currentLayer == RenderLayer::Debug) {
                window->setView(uiView);
            } else {
                window->setView(gameView);
            }
        }
        
        // Execute render command
        command.renderFunc(*window);
    }
    
    // Debug info
    if (showDebugInfo) {
        window->setView(uiView);
        renderDebugInfo();
    }
    
    window->display();
    clear();
}

void RenderSystem::clear() {
    renderQueue.clear();
}

sf::Vector2f RenderSystem::screenToWorld(const sf::Vector2i& screenPos) const {
    if (window) {
        return window->mapPixelToCoords(screenPos, gameView);
    }
    return sf::Vector2f(screenPos);
}

sf::Vector2i RenderSystem::worldToScreen(const sf::Vector2f& worldPos) const {
    if (window) {
        return window->mapCoordsToPixel(worldPos, gameView);
    }
    return sf::Vector2i(worldPos);
}