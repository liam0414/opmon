#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>

class AudioManager {
private:
    std::unordered_map<std::string, sf::SoundBuffer> soundBuffers;
    std::unordered_map<std::string, std::unique_ptr<sf::Sound>> sounds;
    std::unordered_map<std::string, std::unique_ptr<sf::Music>> music;
    
    float masterVolume;
    float musicVolume;
    float sfxVolume;
    
    std::string currentMusic;

public:
    AudioManager();
    ~AudioManager();
    
    // Sound effects
    bool loadSound(const std::string& name, const std::string& filepath);
    void playSound(const std::string& name, bool loop = false);
    void stopSound(const std::string& name);
    void pauseSound(const std::string& name);
    
    // Music
    bool loadMusic(const std::string& name, const std::string& filepath);
    void playMusic(const std::string& name, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    
    // Volume control
    void setMasterVolume(float volume);
    void setMusicVolume(float volume);
    void setSfxVolume(float volume);
    
    float getMasterVolume() const { return masterVolume; }
    float getMusicVolume() const { return musicVolume; }
    float getSfxVolume() const { return sfxVolume; }
    
    // Cleanup
    void clear();
};