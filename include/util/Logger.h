#ifndef OBDVG_LOGGER_H
#define OBDVG_LOGGER_H

#include <string>
#include <vector>
#include <cstdio>
#include "Types.h"

// Keep LogLevel for backward compatibility
enum class LogLevel {
    ERROR,
    WARN,
    INFO,
    DEBUG
};

/**
 * @brief A simple singleton logger class.
 *
 * This logger provides a simple way to log messages.
 * It supports formatted strings in a printf-like manner.
 */
class Logger {
public:
    /**
     * @brief Gets the singleton instance of the Logger.
     * @return A reference to the Logger instance.
     */
    static Logger& instance() {
        static Logger logger(80);
        return logger;
    }

    /**
     * @brief Logs a simple string message.
     *
     * @param msg The message to log.
     */
    void log(const String& msg) const;

    /**
     * @brief Backward compatibility: Logs a simple string message with a log level.
     * 
     * @param level The log level (ignored in this version).
     * @param msg The message to log.
     */
    void log(LogLevel level, const String& msg) const {
        (void)level;
        log(msg);
    }

    /**
     * @brief Prints a separator line to the console.
     *        This is useful for sectioning off different parts of the log output.
     */
    void print_separator() const;

    /**
     * @brief Prints a specified number of empty lines to the console.
     *        This can be used to add vertical spacing to the log output.
     * @param lines_to_print The number of empty lines to print. Defaults to 1.
     */
    void print_empty_line(int8 lines_to_print = 1) const;

    /**
     * @brief Sets a new character limit for line breaking.
     * @param limit The new character limit.
     */
    void setCharLimit(uint32 limit);

    /**
     * @brief Logs a formatted string message.
     *
     * This function uses a printf-like format string and a variable number of
     * arguments.
     *
     * @tparam Args The types of the arguments.
     * @param format The format string.
     * @param args The arguments for the format string.
     */
    template<typename... Args>
    void log(const char* format, Args... args) const {
        int size = std::snprintf(nullptr, 0, format, args...);
        if (size < 0) {
            return;
        }

        Vector(char) buf(size + 1);
        std::snprintf(buf.data(), buf.size(), format, args...);

        log(String(buf.data()));
    }

    /**
     * @brief Backward compatibility: Logs a formatted string message with a log level.
     *
     * @tparam Args The types of the arguments.
     * @param level The log level (ignored in this version).
     * @param format The format string.
     * @param args The arguments for the format string.
     */
    template<typename... Args>
    void log(LogLevel level, const char* format, Args... args) const {
        (void)level;
        log(format, args...);
    }

    // Dummy for backward compatibility
    void setLevel(LogLevel level) { (void)level; }
    void setLevel(int level) { (void)level; }

private:
    /**
     * @brief Private constructor for the singleton pattern.
     * @param limit The character limit for line breaking.
     */
    Logger(uint32 limit = 80);

    /**
     * @brief Breaks a string into multiple lines at a specified character limit without breaking words.
     * @param str The string to break.
     * @return A vector of strings, where each string is a line.
     */
    Vector(String) break_line(const String& str) const;

    uint32 char_limit;
};

#endif //OBDVG_LOGGER_H
