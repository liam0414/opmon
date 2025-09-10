#include "core/Application.h"
#include "core/Logger.h"
#include "scenes/MainMenuScene.h"
#include <memory>
#include <exception>

int main() {
    try {
        Application app;
        
        // Start with the main menu
        app.pushScene(std::make_unique<MainMenuScene>());
        
        app.run();
        
    } catch (const std::exception& e) {
        LOG_CRITICAL("💥 Unhandled exception: {}", e.what());
        return -1;
    } catch (...) {
        LOG_CRITICAL("💥 Unknown exception occurred");
        return -2;
    }
    
    return 0;
}