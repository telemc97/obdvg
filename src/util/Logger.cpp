#include "Types.h"

#include "Config.h"
#include "util/Logger.h"

Logger::Logger() : currentLevel(Config::LOG_LEVEL) {}

void Logger::setLevel(const LogLevel level) { currentLevel = level; }

void Logger::printPrefix(const LogLevel level) {
  switch (level) {
  case LogLevel::DEBUG:
    printf("[DEBUG] ");
    break;
  case LogLevel::INFO:
    printf("[INFO ] ");
    break;
  case LogLevel::WARN:
    printf("[WARN ] ");
    break;
  case LogLevel::ERROR:
    printf("[ERROR] ");
    break;
  }
}

void Logger::log(const LogLevel level, const String &msg) const {
  if (level < currentLevel)
    return;
  printPrefix(level);
  printf("%s\n", msg.c_str());
}