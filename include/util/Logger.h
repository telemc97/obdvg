#ifndef OBDVG_LOGGER_H
#define OBDVG_LOGGER_H

#include "Types.h"

#include <string>
#include <stdio.h>

// Simple log levels
enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void setLevel(LogLevel level);
    void log(LogLevel level, const String& msg) const;

private:
    Logger();
    static void printPrefix(LogLevel level);
    LogLevel currentLevel;
};

#endif //OBDVG_LOGGER_H
