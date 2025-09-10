#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <any>

enum class EventType {
    // Game events
    GameStart,
    GameEnd,
    LevelUp,
    
    // Battle events
    BattleStart,
    BattleEnd,
    CharacterDeath,
    AbilityUsed,
    
    // Story events
    QuestStart,
    QuestComplete,
    QuestFail,
    DialogueStart,
    DialogueEnd,
    
    // World events
    LocationEnter,
    LocationExit,
    ItemFound,
    CrewMemberJoin,
    
    // Custom events
    Custom
};

struct GameEvent {
    EventType type;
    std::string name;
    std::unordered_map<std::string, std::any> data;
    float timestamp;
    
    GameEvent(EventType t, const std::string& n = "", float time = 0.0f) 
        : type(t), name(n), timestamp(time) {}
    
    template<typename T>
    void setData(const std::string& key, const T& value) {
        data[key] = value;
    }
    
    template<typename T>
    T getData(const std::string& key, const T& defaultValue = T{}) const {
        auto it = data.find(key);
        if (it != data.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    bool hasData(const std::string& key) const {
        return data.find(key) != data.end();
    }
};

using EventListener = std::function<void(const GameEvent&)>;

class EventSystem {
private:
    static EventSystem* instance;
    std::unordered_map<EventType, std::vector<EventListener>> listeners;
    std::vector<GameEvent> eventQueue;
    std::vector<GameEvent> eventHistory;
    
    float currentTime;
    size_t maxHistorySize;

public:
    static EventSystem& getInstance();
    
    // Event registration
    void subscribe(EventType eventType, EventListener listener);
    void unsubscribe(EventType eventType); // Remove all listeners for this event type
    
    // Event dispatching
    void emit(const GameEvent& event);
    void emit(EventType type, const std::string& name = "");
    
    template<typename T>
    void emit(EventType type, const std::string& name, const std::string& dataKey, const T& dataValue) {
        GameEvent event(type, name, currentTime);
        event.setData(dataKey, dataValue);
        emit(event);
    }
    
    // Event processing
    void processEvents();
    void update(float deltaTime);
    
    // History and queries
    const std::vector<GameEvent>& getEventHistory() const { return eventHistory; }
    std::vector<GameEvent> getEventsOfType(EventType type, int maxCount = -1) const;
    std::vector<GameEvent> getRecentEvents(float timeWindow) const;
    
    // Utility
    void clear();
    void setMaxHistorySize(size_t size) { maxHistorySize = size; }
    
    // Convenience methods for common events
    void emitLevelUp(const std::string& characterName, int newLevel);
    void emitBattleStart(const std::string& location, int enemyCount);
    void emitBattleEnd(bool playerWon, int expGained);
    void emitQuestComplete(const std::string& questId, int expReward, long berryReward);
    void emitLocationEnter(const std::string& locationId, const std::string& locationName);
    void emitCrewMemberJoin(const std::string& memberName, const std::string& role);
};