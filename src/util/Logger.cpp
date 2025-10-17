#include "Types.h"

#include "util/Logger.h"
#include <cstdio>

Logger::Logger(): currentLevel(LogLevel::ERROR) {}

void Logger::setLevel(const LogLevel level) { currentLevel = level; }

void Logger::setLevel(const int level) {
  currentLevel = static_cast<LogLevel>(0);
    if (level >= 0 && level <= 3) {
        currentLevel = static_cast<LogLevel>(level);
    }
}

void Logger::printPrefix(const LogLevel level) {
  switch (level) {
    case LogLevel::DEBUG:
      std::printf("[DEBUG] ");
      break;
    case LogLevel::INFO:
      std::printf("[INFO ] ");
      break;
    case LogLevel::WARN:
      std::printf("[WARN ] ");
      break;
    case LogLevel::ERROR:
      std::printf("[ERROR] ");
      break;
  }
}

void Logger::log(const LogLevel level, const String &msg) const {
  if (level > currentLevel)
    return;
  printPrefix(level);
  std::printf("%s\n", msg.c_str());
}