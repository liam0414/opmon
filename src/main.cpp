#include "core/Application.h"
#include "core/Logger.h"
#include "game/GameStateManager.h"
#include "game/EventSystem.h"
#include "scenes/MainMenuScene.h"
#include <memory>
#include <exception>

int main() {
    try {
        // Initialize core systems
        Application app;
        
        // Setup event listeners for logging important game events
        auto& eventSystem = EventSystem::getInstance();
        
        eventSystem.subscribe(EventType::LevelUp, [](const GameEvent& event) {
            std::string character = event.getData<std::string>("character", "Unknown");
            int level = event.getData<int>("level", 1);
            LOG_INFO("🌟 {} reached level {}!", character, level);
        });
        
        eventSystem.subscribe(EventType::BattleEnd, [](const GameEvent& event) {
            bool victory = event.getData<bool>("victory", false);
            int exp = event.getData<int>("experience", 0);
            if (victory) {
                LOG_INFO("⚔️ Battle won! Gained {} experience", exp);
            } else {
                LOG_INFO("💀 Battle lost...");
            }
        });
        
        eventSystem.subscribe(EventType::QuestComplete, [](const GameEvent& event) {
            std::string questId = event.getData<std::string>("questId", "unknown");
            int exp = event.getData<int>("experience", 0);
            long berry = event.getData<long>("berry", 0L);
            LOG_INFO("✅ Quest '{}' completed! Rewards: {} EXP, {} Berry", questId, exp, berry);
        });
        
        eventSystem.subscribe(EventType::CrewMemberJoin, [](const GameEvent& event) {
            std::string name = event.getData<std::string>("memberName", "Unknown");
            std::string role = event.getData<std::string>("role", "Unknown");
            LOG_INFO("🎉 {} ({}) joined the crew!", name, role);
        });
        
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