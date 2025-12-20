//
// Created by Tilemahos Mitroudas on 20/12/25.
//

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
    uint32 timestamp;
};


#endif //OBDVG_OBDMESSAGE_H