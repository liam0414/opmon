#include "AudioManager.h"
#include "Logger.h"
#include "ConfigManager.h"

AudioManager::AudioManager() {
    auto& config = ConfigManager::getInstance();
    
    masterVolume = config.get<float>("audio.master_volume", 100.0f);
    musicVolume = config.get<float>("audio.music_volume", 80.0f);
    sfxVolume = config.get<float>("audio.sfx_volume", 90.0f);
    
    LOG_SYSTEM_INFO("AudioManager initialized - Master: {}%, Music: {}%, SFX: {}%", 
                   masterVolume, musicVolume, sfxVolume);
}

AudioManager::~AudioManager() {
    clear();
}

bool AudioManager::loadSound(const std::string& name, const std::string& filepath) {
    if (soundBuffers.find(name) != soundBuffers.end()) {
        LOG_WARN("Sound '{}' already loaded", name);
        return true;
    }
    
    sf::SoundBuffer buffer;
    if (buffer.loadFromFile(filepath)) {
        soundBuffers[name] = std::move(buffer);
        sounds[name] = std::make_unique<sf::Sound>(soundBuffers[name]);
        sounds[name]->setVolume((masterVolume / 100.0f) * (sfxVolume / 100.0f) * 100.0f);
        
        LOG_INFO("✅ Loaded sound: {} from {}", name, filepath);
        return true;
    }
    
    LOG_ERROR("❌ Failed to load sound: {} from {}", name, filepath);
    return false;
}

void AudioManager::playSound(const std::string& name, bool loop) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        it->second->setLoop(loop);
        it->second->play();
        LOG_DEBUG("🔊 Playing sound: {}", name);
    } else {
        LOG_WARN("Sound '{}' not found", name);
    }
}

void AudioManager::stopSound(const std::string& name) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        it->second->stop();
        LOG_DEBUG("⏹️ Stopped sound: {}", name);
    }
}

void AudioManager::pauseSound(const std::string& name) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        it->second->pause();
    }
}

bool AudioManager::loadMusic(const std::string& name, const std::string& filepath) {
    if (music.find(name) != music.end()) {
        LOG_WARN("Music '{}' already loaded", name);
        return true;
    }
    
    auto musicPtr = std::make_unique<sf::Music>();
    if (musicPtr->openFromFile(filepath)) {
        musicPtr->setVolume((masterVolume / 100.0f) * (musicVolume / 100.0f) * 100.0f);
        music[name] = std::move(musicPtr);
        
        LOG_INFO("✅ Loaded music: {} from {}", name, filepath);
        return true;
    }
    
    LOG_ERROR("❌ Failed to load music: {} from {}", name, filepath);
    return false;
}

void AudioManager::playMusic(const std::string& name, bool loop) {
    auto it = music.find(name);
    if (it != music.end()) {
        // Stop current music if any
        stopMusic();
        
        it->second->setLoop(loop);
        it->second->play();
        currentMusic = name;
        
        LOG_INFO("🎵 Playing music: {}", name);
    } else {
        LOG_WARN("Music '{}' not found", name);
    }
}

void AudioManager::stopMusic() {
    if (!currentMusic.empty()) {
        auto it = music.find(currentMusic);
        if (it != music.end()) {
            it->second->stop();
            LOG_DEBUG("⏹️ Stopped music: {}", currentMusic);
        }
        currentMusic.clear();
    }
}

void AudioManager::pauseMusic() {
    if (!currentMusic.empty()) {
        auto it = music.find(currentMusic);
        if (it != music.end()) {
            it->second->pause();
        }
    }
}

void AudioManager::resumeMusic() {
    if (!currentMusic.empty()) {
        auto it = music.find(currentMusic);
        if (it != music.end()) {
            it->second->play();
        }
    }
}

void AudioManager::setMasterVolume(float volume) {
    masterVolume = std::max(0.0f, std::min(100.0f, volume));
    
    // Update all sounds and music
    for (auto& [name, sound] : sounds) {
        sound->setVolume((masterVolume / 100.0f) * (sfxVolume / 100.0f) * 100.0f);
    }
    
    for (auto& [name, musicPtr] : music) {
        musicPtr->setVolume((masterVolume / 100.0f) * (musicVolume / 100.0f) * 100.0f);
    }
}

void AudioManager::setMusicVolume(float volume) {
    musicVolume = std::max(0.0f, std::min(100.0f, volume));
    
    for (auto& [name, musicPtr] : music) {
        musicPtr->setVolume((masterVolume / 100.0f) * (musicVolume / 100.0f) * 100.0f);
    }
}

void AudioManager::setSfxVolume(float volume) {
    sfxVolume = std::max(0.0f, std::min(100.0f, volume));
    
    for (auto& [name, sound] : sounds) {
        sound->setVolume((masterVolume / 100.0f) * (sfxVolume / 100.0f) * 100.0f);
    }
}

void AudioManager::clear() {
    sounds.clear();
    soundBuffers.clear();
    music.clear();
    currentMusic.clear();
    
    LOG_SYSTEM_INFO("🗑️ Cleared all audio resources");
}