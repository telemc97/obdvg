#include "util/Logger.h"

Logger::Logger() : currentLevel(LogLevel::INFO) {}

void Logger::init(const LogLevel level) {
  currentLevel = level;
}

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

void Logger::debug(const String &msg) const { log(LogLevel::DEBUG, msg); }
void Logger::info(const String &msg) const { log(LogLevel::INFO, msg); }
void Logger::warn(const String &msg) const { log(LogLevel::WARN, msg); }
void Logger::error(const String &msg) const { log(LogLevel::ERROR, msg); }
