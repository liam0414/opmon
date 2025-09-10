#include "TextureManager.h"
#include "core/Logger.h"

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
    
    sf::Texture texture;
    if (texture.loadFromFile(filepath)) {
        textures[name] = std::move(texture);
        LOG_INFO("✅ Loaded texture: {} from {}", name, filepath);
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
    
    sf::Texture texture;
    if (texture.loadFromImage(image)) {
        textures[name] = std::move(texture);
        LOG_INFO("✅ Loaded texture with STB: {} ({}x{}, {} channels)", name, width, height, channels);
    } else {
        LOG_ERROR("❌ Failed to create SFML texture from STB data: {}", name);
    }
    
    stbi_image_free(data);
    return hasTexture(name);
}

sf::Texture& TextureManager::getTexture(const std::string& name) {
    if (!hasTexture(name)) {
        LOG_ERROR("❌ Texture '{}' not found! Available textures: {}", name, textures.size());
        static sf::Texture errorTexture;
        return errorTexture;
    }
    return textures[name];
}

bool TextureManager::hasTexture(const std::string& name) const {
    return textures.find(name) != textures.end();
}

void TextureManager::clear() {
    size_t count = textures.size();
    textures.clear();
    LOG_SYSTEM_INFO("🗑️ Cleared {} textures", count);
}