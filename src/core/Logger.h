#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>

class Logger {
private:
    static std::shared_ptr<spdlog::logger> gameLogger;
    static std::shared_ptr<spdlog::logger> systemLogger;
    static std::shared_ptr<spdlog::logger> battleLogger;

public:
    static void init();
    static void shutdown();
    
    // Get specific loggers
    static std::shared_ptr<spdlog::logger>& getGameLogger() { return gameLogger; }
    static std::shared_ptr<spdlog::logger>& getSystemLogger() { return systemLogger; }
    static std::shared_ptr<spdlog::logger>& getBattleLogger() { return battleLogger; }
};

// Convenience macros for game events
#define LOG_TRACE(...)    Logger::getGameLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    Logger::getGameLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)     Logger::getGameLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     Logger::getGameLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    Logger::getGameLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::getGameLogger()->critical(__VA_ARGS__)

// System-specific logging
#define LOG_SYSTEM_DEBUG(...)    Logger::getSystemLogger()->debug(__VA_ARGS__)
#define LOG_SYSTEM_INFO(...)     Logger::getSystemLogger()->info(__VA_ARGS__)
#define LOG_SYSTEM_WARN(...)     Logger::getSystemLogger()->warn(__VA_ARGS__)
#define LOG_SYSTEM_ERROR(...)    Logger::getSystemLogger()->error(__VA_ARGS__)

// Battle-specific logging
#define LOG_BATTLE_DEBUG(...)    Logger::getBattleLogger()->debug(__VA_ARGS__)
#define LOG_BATTLE_INFO(...)     Logger::getBattleLogger()->info(__VA_ARGS__)
#define LOG_BATTLE_WARN(...)     Logger::getBattleLogger()->warn(__VA_ARGS__)
#define LOG_BATTLE_ERROR(...)    Logger::getBattleLogger()->error(__VA_ARGS__)