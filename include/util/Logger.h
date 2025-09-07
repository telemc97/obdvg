#pragma once
#include <cstdio>

class Logger {
    
    public:
        template<typename... Args>

        static void info(const char* fmt, Args... args) {
            printf(fmt, args...);
            printf("\n");
        }
};