#include "EventSystem.h"
#include "core/Logger.h"
#include <algorithm>

EventSystem* EventSystem::instance = nullptr;

EventSystem& EventSystem::getInstance() {
    if (!instance) {
        instance = new EventSystem();
    }
    return *instance;
}

EventSystem::EventSystem() : currentTime(0), maxHistorySize(1000) {
}

void EventSystem::subscribe(EventType eventType, EventListener listener) {
    listeners[eventType].push_back(listener);
    LOG_DEBUG("Subscribed to event type: {}", static_cast<int>(eventType));
}

void EventSystem::unsubscribe(EventType eventType) {
    listeners[eventType].clear();
    LOG_DEBUG("Unsubscribed from event type: {}", static_cast<int>(eventType));
}

void EventSystem::emit(const GameEvent& event) {
    eventQueue.push_back(event);
    LOG_DEBUG("Event queued: {} ({})", event.name, static_cast<int>(event.type));
}

void EventSystem::emit(EventType type, const std::string& name) {
    GameEvent event(type, name, currentTime);
    emit(event);
}

void EventSystem::processEvents() {
    for (const auto& event : eventQueue) {
        // Add to history
        eventHistory.push_back(event);
        
        // Trim history if needed
        if (eventHistory.size() > maxHistorySize) {
            eventHistory.erase(eventHistory.begin(), eventHistory.begin() + (eventHistory.size() - maxHistorySize));
        }
        
        // Call listeners
        auto it = listeners.find(event.type);
        if (it != listeners.end()) {
            for (const auto& listener : it->second) {
                try {
                    listener(event);
                } catch (const std::exception& e) {
                    LOG_ERROR("Event listener error for {}: {}", event.name, e.what());
                }
            }
        }
        
        LOG_DEBUG("Processed event: {} ({})", event.name, static_cast<int>(event.type));
    }
    
    eventQueue.clear();
}

void EventSystem::update(float deltaTime) {
    currentTime += deltaTime;
    processEvents();
}

std::vector<GameEvent> EventSystem::getEventsOfType(EventType type, int maxCount) const {
    std::vector<GameEvent> result;
    
    for (auto it = eventHistory.rbegin(); it != eventHistory.rend(); ++it) {
        if (it->type == type) {
            result.push_back(*it);
            if (maxCount > 0 && static_cast<int>(result.size()) >= maxCount) {
                break;
            }
        }
    }
    
    return result;
}

std::vector<GameEvent> EventSystem::getRecentEvents(float timeWindow) const {
    std::vector<GameEvent> result;
    float cutoffTime = currentTime - timeWindow;
    
    for (auto it = eventHistory.rbegin(); it != eventHistory.rend(); ++it) {
        if (it->timestamp >= cutoffTime) {
            result.push_back(*it);
        } else {
            break; // Events are ordered by time, so we can stop here
        }
    }
    
    return result;
}

void EventSystem::clear() {
    eventQueue.clear();
    eventHistory.clear();
    listeners.clear();
    currentTime = 0;
    LOG_INFO("Event system cleared");
}

void EventSystem::emitLevelUp(const std::string& characterName, int newLevel) {
    GameEvent even