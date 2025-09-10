#pragma once
#include <nlohmann/json.hpp>
#include <string>

class ConfigManager {
private:
    nlohmann::json config;
    std::string configPath;
    static ConfigManager* instance;

public:
    static ConfigManager& getInstance();
    
    bool loadConfig(const std::string& filepath);
    bool saveConfig() const;
    
    // Template methods for getting nested values
    template<typename T>
    T get(const std::string& key, const T& defaultValue = T{}) const {
        try {
            // Support dot notation like "window.width"
            nlohmann::json current = config;
            std::string delimiter = ".";
            std::string keyPath = key;
            
            size_t pos = 0;
            while ((pos = keyPath.find(delimiter)) != std::string::npos) {
                std::string token = keyPath.substr(0, pos);
                current = current.at(token);
                keyPath.erase(0, pos + delimiter.length());
            }
            
            return current.at(keyPath).get<T>();
        } catch (...) {
            return defaultValue;
        }
    }
    
    template<typename T>
    void set(const std::string& key, const T& value) {
        // Support dot notation for setting nested values
        nlohmann::json* current = &config;
        std::string delimiter = ".";
        std::string keyPath = key;
        
        size_t pos = 0;
        while ((pos = keyPath.find(delimiter)) != std::string::npos) {
            std::string token = keyPath.substr(0, pos);
            current = &((*current)[token]);
            keyPath.erase(0, pos + delimiter.length());
        }
        
        (*current)[keyPath] = value;
    }
    
    nlohmann::json& getJson() { return config; }
    const nlohmann::json& getJson() const { return config; }
};