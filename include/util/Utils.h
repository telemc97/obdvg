#ifndef OBDVG_UTILS_H
#define OBDVG_UTILS_H

#include "Types.h"

/**
 * @brief A collection of utility functions.
 */
class Utils {

public:
    /**
     * @brief Gets the current Unix timestamp in seconds.
     * 
     * Note: On the Raspberry Pi Pico, this will return seconds since boot
     * unless the RTC has been synchronized with a real-time source.
     * 
     * @return uint64 The Unix timestamp.
     */
    static uint64 getUnixTimestamp();
};

#endif //OBDVG_UTILS_H
