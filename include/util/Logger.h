#pragma once
#include "Types.h"

#include <string>

// Simple log levels
enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    Logger();
    
    // Initialize the logger (must be called once before use)
    void init(LogLevel level = LogLevel::INFO);

    // Logging functions
    void log(LogLevel level, const String& msg) const;
    void debug(const String& msg) const;
    void info(const String& msg) const;
    void warn(const String& msg) const;
    void error(const String& msg) const;

    // Set log level at runtime
    void setLevel(LogLevel level);

private:
    static void printPrefix(LogLevel level);

    LogLevel currentLevel;
};