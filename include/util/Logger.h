#ifndef OBDVG_LOGGER_H
#define OBDVG_LOGGER_H

#include "Types.h"

#include <string>
#include <cstdio>
#include <vector>

#include "Config.h"

// Simple log levels
enum class LogLevel {
    ERROR,
    WARN,
    INFO,
    DEBUG
};

/**
 * @brief A simple singleton logger class.
 *
 * This logger provides a simple way to log messages at different levels.
 * It supports formatted strings in a printf-like manner.
 */
class Logger {
public:
    /**
     * @brief Gets the singleton instance of the Logger.
     * @return A reference to the Logger instance.
     */
    static Logger& instance() {
        static Logger logger;
        logger.setLevel(Config::LOG_LEVEL);
        return logger;
    }

    /**
     * @brief Sets the logging level.
     *
     * Messages with a level numerically higher (less important) than the set level will not be logged.
     *
     * @param level The log level to set.
     */
    void setLevel(LogLevel level);

    /**
     * @brief Sets the logging level using an integer.
     *
     * 0 = ERROR, 1 = WARN, 2 = INFO, 3 = DEBUG.
     * Invalid integers are ignored.
     *
     * @param level The integer representation of the log level.
     */
    void setLevel(int level);

    /**
     * @brief Logs a simple string message.
     *
     * @param level The log level of the message.
     * @param msg The message to log.
     */
    void log(LogLevel level, const String& msg) const;

    /**
     * @brief Logs a formatted string message.
     *
     * This function uses a printf-like format string and a variable number of
     * arguments.
     *
     * @tparam Args The types of the arguments.
     * @param level The log level of the message.
     * @param format The format string.
     * @param args The arguments for the format string.
     */
    template<typename... Args>
    void log(LogLevel level, const char* format, Args... args) const {
        if (level > currentLevel) {
            return;
        }

        int size = std::snprintf(nullptr, 0, format, args...);
        if (size < 0) {
            return;
        }

        std::vector<char> buf(size + 1);
        std::snprintf(buf.data(), buf.size(), format, args...);

        log(level, std::string(buf.data()));
    }

private:
    /**
     * @brief Private constructor for the singleton pattern.
     */
    Logger();

    /**
     * @brief Prints the prefix for a log message (e.g., "[INFO] ").
     * @param level The log level of the message.
     */
    static void printPrefix(LogLevel level);

    LogLevel currentLevel;
};

#endif //OBDVG_LOGGER_H
