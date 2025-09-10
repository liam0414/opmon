#include "CharacterFactory.h"
#include "core/Logger.h"

CharacterFactory* CharacterFactory::instance = nullptr;

CharacterFactory& CharacterFactory::getInstance() {
    if (!instance) {
        instance = new CharacterFactory();
        instance->registerBuiltInCharacters();
    }
    return *instance;
}

void CharacterFactory::registerBuiltInCharacters() {
    // Register crew members
    registerCrewMember("zoro", []() { return CrewMember::createZoro(); });
    registerCrewMember("nami", []() { return CrewMember::createNami(); });
    registerCrewMember("sanji", []() { return CrewMember::createSanji(); });
    // Add more as needed...
    
    LOG_SYSTEM_INFO("Registered {} built-in crew members", crewCreators.size());
}

void CharacterFactory::registerCharacter(const std::string& name, std::function<std::unique_ptr<Character>()> creator) {
    characterCreators[name] = creator;
    LOG_DEBUG("Registered character creator: {}", name);
}

void CharacterFactory::registerCrewMember(const std::string& name, std::function<std::unique_ptr<CrewMember>()> creator) {
    crewCreators[name] = creator;
    LOG_DEBUG("Registered crew member creator: {}", name);
}

std::unique_ptr<Character> CharacterFactory::createCharacter(const std::string& name) {
    auto it = characterCreators.find(name);
    if (it != characterCreators.end()) {
        return it->second();
    }
    
    LOG_WARN("Character '{}' not found in factory", name);
    return nullptr;
}

std::unique_ptr<CrewMember> CharacterFactory::createCrewMember(const std::string& name) {
    auto it = crewCreators.find(name);
    if (it != crewCreators.end()) {
        return it->second();
    }
    
    LOG_WARN("Crew member '{}' not found in factory", name);
    return nullptr;
}

std::unique_ptr<Character> CharacterFactory::createFromJson(const nlohmann::json& data) {
    std::string name = data.value("name", "unknown");
    CharacterType type = static_cast<CharacterType>(data.value("type", 0));
    
    std::unique_ptr<Character> character;
    
    if (type == CharacterType::CrewMember) {
        character = std::make_unique<CrewMember>(name, data.value("role", "Unknown"));
    } else {
        character = std::make_unique<Character>(name, type);
    }
    
    character->fromJson(data);
    return character;
}

std::vector<std::string> CharacterFactory::getAvailableCharacters() const {
    std::vector<std::string> names;
    for (const auto& [name, creator] : characterCreators) {
        names.push_back(name);
    }
    return names;
}

std::vector<std::string> CharacterFactory::getAvailableCrewMembers() const {
    std::vector<std::string> names;
    for (const auto& [name, creator] : crewCreators) {
        names.push_back(name);
    }
    return names;
}