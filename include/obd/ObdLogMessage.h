#ifndef OBDVG_OBDMESSAGE_H
#define OBDVG_OBDMESSAGE_H

#include "Types.h"

struct ObdLogMessage {
    float32 throttle_pos;
    float32 engine_load;
    float32 engine_coolant_temp;
    float32 engine_oil_temp;
    float32 intake_map;
    float32 vehicle_speed;
    float32 intake_air_temp;
    float32 engine_speed;
    float32 ambient_air_temp;
    float32 catalyst_temp;
    float32 st_fuel_trim_1;
    float32 lt_fuel_trim_1;
    float32 engine_fuel_rate;
    uint32 timestamp;
};


#endif //OBDVG_OBDMESSAGE_H
