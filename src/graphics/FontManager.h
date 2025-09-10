#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

class FontManager {
private:
    std::unordered_map<std::string, sf::Font> fonts;
    static FontManager* instance;
    std::string defaultFontName;

public:
    static FontManager& getInstance();
    
    bool loadFont(const std::string& name, const std::string& filepath);
    sf::Font& getFont(const std::string& name);
    const sf::Font& getFont(const std::string& name) const;
    bool hasFont(const std::string& name) const;
    
    void setDefaultFont(const std::string& name);
    sf::Font& getDefaultFont();
    
    void clear();
    std::vector<std::string> getLoadedFontNames() const;
};