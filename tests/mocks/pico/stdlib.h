#ifndef MOCK_STDLIB_H
#define MOCK_STDLIB_H

#include <cstdint>

typedef uint64_t absolute_time_t;

// Simulate advancing time for timeout loops
inline absolute_time_t get_absolute_time() { 
    static uint64_t simulated_time = 0;
    simulated_time += 10; // Advance by 10ms per call
    return simulated_time; 
}

inline uint32_t to_ms_since_boot(absolute_time_t t) { return (uint32_t)t; }

#endif
