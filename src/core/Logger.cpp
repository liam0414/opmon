#include "Logger.h"
#include <spdlog/pattern_formatter.h>
#include <iostream>

std::shared_ptr<spdlog::logger> Logger::gameLogger;
std::shared_ptr<spdlog::logger> Logger::systemLogger;
std::shared_ptr<spdlog::logger> Logger::battleLogger;

void Logger::init() {
    try {
        // Create sinks
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");

        // Rotating file sink for general game logs (5MB, 3 files)
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/game.log", 1024 * 1024 * 5, 3);
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");

        // Battle-specific log file
        auto battle_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/battle.log", 1024 * 1024 * 2, 2);
        battle_file_sink->set_level(spdlog::level::debug);

        // System-specific log file
        auto system_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            "logs/system.log", true);
        system_file_sink->set_level(spdlog::level::info);

        // Create loggers
        std::vector<spdlog::sink_ptr> game_sinks{console_sink, file_sink};
        gameLogger = std::make_shared<spdlog::logger>("GAME", game_sinks.begin(), game_sinks.end());
        gameLogger->set_level(spdlog::level::trace);

        std::vector<spdlog::sink_ptr> system_sinks{console_sink, system_file_sink};
        systemLogger = std::make_shared<spdlog::logger>("SYSTEM", system_sinks.begin(), system_sinks.end());
        systemLogger->set_level(spdlog::level::debug);

        std::vector<spdlog::sink_ptr> battle_sinks{console_sink, battle_file_sink};
        battleLogger = std::make_shared<spdlog::logger>("BATTLE", battle_sinks.begin(), battle_sinks.end());
        battleLogger->set_level(spdlog::level::debug);

        // Register loggers
        spdlog::register_logger(gameLogger);
        spdlog::register_logger(systemLogger);
        spdlog::register_logger(battleLogger);

        // Set flush policies
        gameLogger->flush_on(spdlog::level::warn);
        systemLogger->flush_on(spdlog::level::info);
        battleLogger->flush_on(spdlog::level::info);

        LOG_INFO("Logging system initialized successfully");
        LOG_SYSTEM_INFO("System logger initialized");
        LOG_BATTLE_INFO("Battle logger initialized");

    } catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
}

void Logger::shutdown() {
    if (gameLogger) {
        LOG_INFO("Shutting down logging system");
    }
    
    spdlog::drop_all();
    spdlog::shutdown();
}