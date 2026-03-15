#include "util/Logger.h"
#include <cstdio>
#include <sstream>

Logger::Logger(uint32 limit) : char_limit(limit) {}

void Logger::setCharLimit(uint32 limit) {
    this->char_limit = limit;
}

Vector(String) Logger::break_line(const String& str) const {
    Vector(String) lines;
    std::stringstream ss(str);
    String word;

    String current_line;
    while (ss >> word) {
        if (current_line.length() + word.length() + 1 > char_limit) {
            lines.push_back(current_line);
            current_line = word;
        } else {
            if (!current_line.empty()) {
                current_line += " ";
            }
            current_line += word;
        }
    }
    if (!current_line.empty()) {
        lines.push_back(current_line);
    }
    return lines;
}

void Logger::log(const String& msg) const {
    // Prevent breaking the separator line itself
    if (msg.length() == char_limit && msg.find_first_not_of('-') == std::string::npos) {
        std::printf("%s\n", msg.c_str());
        return;
    }

    Vector(String) lines = break_line(msg);
    for (const auto& line : lines) {
        std::printf("%s\n", line.c_str());
    }
}

void Logger::print_separator() const {
    log(String(this->char_limit, '-'));
}

void Logger::print_empty_line(int8 lines_to_print) const {
    for (int i = 0; i < lines_to_print; ++i) {
        log("");
    }
}
