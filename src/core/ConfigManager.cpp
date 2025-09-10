#include "ConfigManager.h"
#include "Logger.h"
#include <fstream>

ConfigManager* ConfigManager::instance = nullptr;

ConfigManager& ConfigManager::getInstance() {
    if (!instance) {
        instance = new ConfigManager();
    }
    return *instance;
}

bool ConfigManager::loadConfig(const std::string& filepath) {
    configPath = filepath;
    
    std::ifstream configFile(filepath);
    if (configFile.is_open()) {
        try {
            configFile >> config;
            LOG_SYSTEM_INFO("✅ Loaded configuration from: {}", filepath);
            LOG_DEBUG("Config loaded with {} top-level keys", config.size());
            return true;
        } catch (const std::exception& e) {
            LOG_SYSTEM_ERROR("❌ Error parsing config file {}: {}", filepath, e.what());
        }
    } else {
        LOG_SYSTEM_WARN("⚠️ Could not load config file: {}, using defaults", filepath);
    }
    
    // Set default configuration
    config = {
        {"window", {
            {"width", 1024},
            {"height", 768},
            {"title", "One Piece Adventure"},
            {"fullscreen", false},
            {"vsync", true},
            {"framerate_limit", 60}
        }},
        {"audio", {
            {"master_volume", 100},
            {"music_volume", 80},
            {"sfx_volume", 90}
        }},
        {"graphics", {
            {"texture_filtering", true},
            {"show_fps", false}
        }},
        {"gameplay", {
            {"starting_character", "luffy"},
            {"max_crew_size", 10},
            {"auto_save_interval", 300}
        }}
    };
    
    return false;
}

bool ConfigManager::saveConfig() const {
    if (configPath.empty()) {
        LOG_SYSTEM_ERROR("❌ Cannot save config: no file path set");
        return false;
    }
    
    std::ofstream configFile(configPath);
    if (configFile.is_open()) {
        configFile << config.dump(4); // Pretty print with 4 spaces
        LOG_SYSTEM_INFO("✅ Configuration saved to: {}", configPath);
        return true;
    }
    
    LOG_SYSTEM_ERROR("❌ Failed to save configuration to: {}", configPath);
    return false;
}