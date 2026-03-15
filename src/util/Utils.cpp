#include "Types.h"
#include <ctime>
#include "util/Utils.h"
#include "pico/stdlib.h"

uint64 Utils::getUnixTimestamp() {
    return static_cast<uint64>(time(nullptr));
}
