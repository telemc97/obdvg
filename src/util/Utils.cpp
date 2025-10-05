#include "Types.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "util/Utils.h"


void Utils::floatToChars(float32 const value, char out[5]) {
    // Handle NaN or Inf
    if (!std::isfinite(value)) {
        strncpy(out, "ERR ", 5);
        return;
    }

    // Round to one decimal place for 4-char display
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", value);  // e.g. 12.3, -0.5, 3.1

    // If longer than 4, truncate (Display::setText will also pad/truncate)
    size_t len = strlen(buf);
    if (len > 4) len = 4;

    memcpy(out, buf, len);

    // Pad with spaces if shorter
    if (len < 4)
        memset(out + len, ' ', 4 - len);

    out[4] = '\0';
}
