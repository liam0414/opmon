#include "FontManager.h"
#include "core/Logger.h"

FontManager* FontManager::instance = nullptr;

FontManager& FontManager::getInstance() {
    if (!instance) {
        instance = new FontManager();
        LOG_SYSTEM_INFO("FontManager instance created");
    }
    return *instance;
}

bool FontManager::loadFont(const std::string& name, const std::string& filepath) {
    if (hasFont(name)) {
        LOG_WARN("Font '{}' already loaded", name);
        return true;
    }
    
    sf::Font font;
    if (font.loadFromFile(filepath)) {
        fonts[name] = std::move(font);
        LOG_INFO("✅ Loaded font: {} from {}", name, filepath);
        
        // Set as default if it's the first font loaded
        if (defaultFontName.empty()) {
            defaultFontName = name;
            LOG_INFO("Set '{}' as default font", name);
        }
        
        return true;
    }
    
    LOG_ERROR("❌ Failed to load font: {} from {}", name, filepath);
    return false;
}

sf::Font& FontManager::getFont(const std::string& name) {
    if (!hasFont(name)) {
        LOG_ERROR("❌ Font '{}' not found! Available fonts: {}", name, fonts.size());
        // Return default font if available
        if (!defaultFontName.empty() && hasFont(defaultFontName)) {
            return fonts[defaultFontName];
        }
        // Return any available font
        if (!fonts.empty()) {
            return fonts.begin()->second;
        }
        // Create emergency font
        static sf::Font emergencyFont;
        return emergencyFont;
    }
    return fonts[name];
}

const sf::Font& FontManager::getFont(const std::string& name) const {
    static sf::Font errorFont;
    auto it = fonts.find(name);
    if (it != fonts.end()) {
        return it->second;
    }
    LOG_ERROR("❌ Font '{}' not found in const context", name);
    return errorFont;
}

bool FontManager::hasFont(const std::string& name) const {
    return fonts.find(name) != fonts.end();
}

void FontManager::setDefaultFont(const std::string& name) {
    if (hasFont(name)) {
        defaultFontName = name;
        LOG_INFO("Set '{}' as default font", name);
    } else {
        LOG_WARN("Cannot set '{}' as default font - not loaded", name);
    }
}

sf::Font& FontManager::getDefaultFont() {
    if (!defaultFontName.empty() && hasFont(defaultFontName)) {
        return fonts[defaultFontName];
    }
    
    if (!fonts.empty()) {
        return fonts.begin()->second;
    }
    
    static sf::Font fallbackFont;
    return fallbackFont;
}

void FontManager::clear() {
    size_t count = fonts.size();
    fonts.clear();
    defaultFontName.clear();
    LOG_SYSTEM_INFO("🗑️ Cleared {} fonts", count);
}

std::vector<std::string> FontManager::getLoadedFontNames() const {
    std::vector<std::string> names;
    names.reserve(fonts.size());
    
    for (const auto& [name, font] : fonts) {
        names.push_back(name);
    }
    
    return names;
}