#include <ctime>
#include "util/Utils.h"
#include "Types.h"

uint64 Utils::getUnixTimestamp() {
    return static_cast<uint64>(time(nullptr));
}
