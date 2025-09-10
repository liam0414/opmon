#include "TextureManager.h"
#include "core/Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

TextureManager* TextureManager::instance = nullptr;

TextureManager& TextureManager::getInstance() {
    if (!instance) {
        instance = new TextureManager();
        LOG_SYSTEM_INFO("TextureManager instance created");
    }
    return *instance;
}

bool TextureManager::loadTexture(const std::string& name, const std::string& filepath) {
    if (hasTexture(name)) {
        LOG_WARN("Texture '{}' already loaded", name);
        return true;
    }
    
    TextureInfo info;
    info.filepath = filepath;
    
    if (info.texture.loadFromFile(filepath)) {
        info.size = info.texture.getSize();
        info.memoryUsage = info.size.x * info.size.y * 4; // RGBA
        
        textures[name] = std::move(info);
        totalMemoryUsage += textures[name].memoryUsage;
        
        LOG_INFO("✅ Loaded texture: {} ({}x{}, {:.2f} KB)", 
                name, info.size.x, info.size.y, info.memoryUsage / 1024.0f);
        return true;
    }
    
    LOG_ERROR("❌ Failed to load texture: {} from {}", name, filepath);
    return false;
}

bool TextureManager::loadTextureFromMemory(const std::string& name, const std::string& filepath) {
    if (hasTexture(name)) {
        return true;
    }
    
    LOG_DEBUG("Loading texture '{}' with STB from: {}", name, filepath);
    
    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
    
    if (!data) {
        LOG_ERROR("❌ STB failed to load image: {} ({})", filepath, stbi_failure_reason());
        return false;
    }
    
    sf::Image image;
    image.create(width, height, data);
    
    TextureInfo info;
    info.filepath = filepath;
    info.size = {static_cast<unsigned int>(width), static_cast<unsigned int>(height)};
    info.memoryUsage = width * height * 4;
    
    if (info.texture.loadFromImage(image)) {
        textures[name] = std::move(info);
        totalMemoryUsage += textures[name].memoryUsage;
        
        LOG_INFO("✅ Loaded texture with STB: {} ({}x{}, {} channels, {:.2f} KB)", 
                name, width, height, channels, info.memoryUsage / 1024.0f);
    } else {
        LOG_ERROR("❌ Failed to create SFML texture from STB data: {}", name);
    }
    
    stbi_image_free(data);
    return hasTexture(name);
}

bool TextureManager::loadTextureSheet(const std::string& name, const std::string& filepath, 
                                    sf::Vector2u tileSize, sf::Vector2u spacing) {
    if (loadTexture(name, filepath)) {
        LOG_INFO("Loaded sprite sheet: {} with tile size {}x{}", name, tileSize.x, tileSize.y);
        return true;
    }
    return false;
}

sf::Texture& TextureManager::getTexture(const std::string& name) {
    if (!hasTexture(name)) {
        LOG_ERROR("❌ Texture '{}' not found! Available textures: {}", name, textures.size());
        // Create and return error texture
        static sf::Texture errorTexture;
        static bool errorTextureCreated = false;
        
        if (!errorTextureCreated) {
            sf::Image errorImage;
            errorImage.create(64, 64, sf::Color::Magenta);
            errorTexture.loadFromImage(errorImage);
            errorTextureCreated = true;
            LOG_WARN("Created error texture (magenta 64x64)");
        }
        
        return errorTexture;
    }
    return textures[name].texture;
}

const sf::Texture& TextureManager::getTexture(const std::string& name) const {
    static sf::Texture errorTexture;
    auto it = textures.find(name);
    if (it != textures.end()) {
        return it->second.texture;
    }
    LOG_ERROR("❌ Texture '{}' not found in const context", name);
    return errorTexture;
}

bool TextureManager::hasTexture(const std::string& name) const {
    return textures.find(name) != textures.end();
}

sf::Vector2u TextureManager::getTextureSize(const std::string& name) const {
    auto it = textures.find(name);
    if (it != textures.end()) {
        return it->second.size;
    }
    return {0, 0};
}

std::vector<std::string> TextureManager::getLoadedTextureNames() const {
    std::vector<std::string> names;
    names.reserve(textures.size());
    
    for (const auto& [name, info] : textures) {
        names.push_back(name);
    }
    
    return names;
}

void TextureManager::unloadTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        totalMemoryUsage -= it->second.memoryUsage;
        textures.erase(it);
        LOG_INFO("Unloaded texture: {}", name);
    }
}

void TextureManager::clear() {
    size_t count = textures.size();
    textures.clear();
    totalMemoryUsage = 0;
    LOG_SYSTEM_INFO("🗑️ Cleared {} textures ({:.2f} MB freed)", 
                   count, totalMemoryUsage / (1024.0f * 1024.0f));
}

bool TextureManager::loadFromManifest(const std::string& manifestPath) {
    std::ifstream file(manifestPath);
    if (!file.is_open()) {
        LOG_ERROR("❌ Could not open texture manifest: {}", manifestPath);
        return false;
    }
    
    nlohmann::json manifest;
    file >> manifest;
    
    LOG_INFO("Loading textures from manifest: {}", manifestPath);
    
    int loaded = 0;
    int failed = 0;
    
    for (const auto& [name, data] : manifest["textures"].items()) {
        std::string filepath = data["file"];
        
        if (loadTexture(name, filepath)) {
            loaded++;
        } else {
            failed++;
        }
    }
    
    LOG_INFO("Texture manifest loading complete: {} loaded, {} failed", loaded, failed);
    return failed == 0;
}