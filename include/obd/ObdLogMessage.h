#ifndef OBDVG_OBDMESSAGE_H
#define OBDVG_OBDMESSAGE_H

#include "Types.h"
#include <cmath>

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

    ObdLogMessage() : 
        throttle_pos(NAN), 
        engine_load(NAN), 
        engine_coolant_temp(NAN), 
        engine_oil_temp(NAN), 
        intake_map(NAN), 
        vehicle_speed(NAN), 
        intake_air_temp(NAN), 
        engine_speed(NAN), 
        ambient_air_temp(NAN), 
        catalyst_temp(NAN), 
        st_fuel_trim_1(NAN), 
        lt_fuel_trim_1(NAN), 
        engine_fuel_rate(NAN), 
        timestamp(0) {}
};


#endif //OBDVG_OBDMESSAGE_H
