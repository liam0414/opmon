#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include "Character.h"
#include "CrewMember.h"

class CharacterFactory {
private:
    static CharacterFactory* instance;
    std::unordered_map<std::string, std::function<std::unique_ptr<Character>()>> characterCreators;
    std::unordered_map<std::string, std::function<std::unique_ptr<CrewMember>()>> crewCreators;
    
    void registerBuiltInCharacters();

public:
    static CharacterFactory& getInstance();
    
    // Registration
    void registerCharacter(const std::string& name, std::function<std::unique_ptr<Character>()> creator);
    void registerCrewMember(const std::string& name, std::function<std::unique_ptr<CrewMember>()> creator);
    
    // Creation
    std::unique_ptr<Character> createCharacter(const std::string& name);
    std::unique_ptr<CrewMember> createCrewMember(const std::string& name);
    std::unique_ptr<Character> createFromJson(const nlohmann::json& data);
    
    // Info
    std::vector<std::string> getAvailableCharacters() const;
    std::vector<std::string> getAvailableCrewMembers() const;
};