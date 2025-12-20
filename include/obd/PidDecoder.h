#ifndef OBDVG_PIDDECODER_H
#define OBDVG_PIDDECODER_H

#include "Types.h"

class PidDecoder {
public:
    // 0x04 - Calculated Engine Load (%)
    static float decodeEngineLoad(uint8 const A) { return A * 100.0f / 255.0f; }

    // 0x05 - Engine Coolant Temperature (°C)
    // 0x0F - Intake Air Temperature (°C)
    // 0x46 - Ambient Air Temperature (°C)
    // 0x5C - Engine Oil Temperature (°C)
    // 0x67 - Engine Coolant Temperature Sensor (°C)
    // 0x68 - Intake Air Temperature Sensor (°C)
    static float decodeTemperature(uint8 const A) { return static_cast<float>(A) - 40.0f; }

    // 0x06 - Short Term Fuel Trim - Bank 1 (%)
    // 0x07 - Long Term Fuel Trim - Bank 1 (%)
    // 0x08 - Short Term Fuel Trim - Bank 2 (%)
    // 0x09 - Long Term Fuel Trim - Bank 2 (%)
    static float decodeFuelTrim(uint8 const A) { return (A - 128.0f) * 100.0f / 128.0f; }

    // 0x0A - Fuel Pressure (kPa)
    static uint8 decodeFuelPressure(uint8 const A) { return A * 3; }

    // 0x0B - Intake Manifold Absolute Pressure (kPa)
    // 0x33 - Barometric Pressure (kPa)
    static uint8 decodePressure(uint8 const A) { return A; }

    // 0x0C - Engine RPM (rpm)
    static float decodeRpm(uint8 const A, uint8 const B) { return ((A << 8) | B) / 4.0f; }

    // 0x0D - Vehicle Speed (km/h)
    static uint8 decodeSpeed(uint8 const A) { return A; }

    // 0x0E - Timing Advance (°)
    static float decodeTimingAdvance(uint8 const A) { return (A - 128.0f) / 2.0f; }

    // 0x10 - MAF Air Flow Rate (g/s)
    static float decodeMafFlow(uint8 const A, uint8 const B) { return ((A << 8) | B) / 100.0f; }

    // 0x11 - Throttle Position (%)
    // 0x2C - Commanded EGR (%)
    // 0x2E - Commanded Evaporative Purge (%)
    // 0x2F - Fuel Tank Level Input (%)
    // 0x45 - Relative Throttle Position (%)
    // 0x47 - Absolute Throttle Position B (%)
    // 0x48 - Absolute Throttle Position C (%)
    // 0x49 - Accelerator Pedal Position D (%)
    // 0x4A - Accelerator Pedal Position E (%)
    // 0x4B - Accelerator Pedal Position F (%)
    // 0x4C - Commanded Throttle Actuator (%)
    // 0x52 - Ethanol Fuel % (%)
    // 0x5A - Relative Accelerator Pedal Position (%)
    // 0x5B - Hybrid Battery Pack Remaining Life (%)
    static float decodePercentage(uint8 const A) { return A * 100.0f / 255.0f; }

    // 0x1F - Run time since engine start (sec)
    // 0x4D - Time run with MIL on (min)
    // 0x4E - Time since trouble codes cleared (min)
    static uint16 decodeTime(uint8 const A, uint8 const B) { return (A << 8) | B; }

    // 0x21 - Distance Traveled with MIL on (km)
    // 0x31 - Distance traveled since codes cleared (km)
    static uint16 decodeDistance(uint8 const A, uint8 const B) { return (A << 8) | B; }

    // 0x22 - Fuel Rail Pressure (Relative to manifold vacuum) (kPa)
    static float decodeFuelRailPressureRel(uint8 const A, uint8 const B) { return ((A << 8) | B) * 0.079f; }

    // 0x23 - Fuel Rail Gauge Pressure (diesel, or gas direct inject) (kPa)
    // 0x59 - Fuel Rail Absolute Pressure (kPa)
    static float decodeFuelRailGaugePressure(uint8 const A, uint8 const B) { return ((A << 8) | B) * 10.0f; }

    // 0x30 - Warm-ups since codes cleared (count)
    static uint8 decodeWarmups(uint8 const A) { return A; }

    // 0x32 - Evap. System Vapor Pressure (Pa)
    static float decodeEvapVaporPressure(uint8 const A, uint8 const B) { return static_cast<int16>((A << 8) | B) / 4.0f; }

    // 0x3C - Catalyst Temperature: Bank 1, Sensor 1 (°C)
    // ...
    static float decodeCatalystTemp(uint8 const A, uint8 const B) { return ((A << 8) | B) / 10.0f - 40.0f; }

    // 0x42 - Control Module Voltage (V)
    static float decodeControlModuleVoltage(uint8 const A, uint8 const B) { return ((A << 8) | B) / 1000.0f; }

    // 0x43 - Absolute Load Value (%)
    static float decodeAbsoluteLoad(uint8 const A, uint8 const B) { return ((A << 8) | B) * 100.0f / 255.0f; }

    // 0x44 - Commanded Equivalence Ratio (lambda)
    static float decodeCommandedEquivRatio(uint8 const A, uint8 const B) { return ((A << 8) | B) / 32768.0f; }

    // 0x50 - Maximum value for Fuel Flow Analysis (g/s)
    static float decodeMaxMafFlow(uint8 const A) { return A * 10.0f; }

    // 0x53 - Absolute Evap System Vapor Pressure (kPa)
    static float decodeAbsEvapVaporPressure(uint8 const A, uint8 const B) { return ((A << 8) | B) / 200.0f; }

    // 0x54 - Evap System Vapor Pressure (Pa)
    static float decodeEvapVaporPressureAlt(uint8 const A, uint8 const B) { return ((A << 8) | B) - 32767.0f; }

    // 0x5D - Fuel Injection Timing (°)
    static float decodeFuelInjectionTiming(uint8 const A, uint8 const B) { return ((A << 8) | B) / 128.0f - 210.0f; }

    // 0x5E - Engine Fuel Rate (L/h)
    static float decodeEngineFuelRate(uint8 const A, uint8 const B) { return ((A << 8) | B) / 20.0f; }

    // 0x61 - Driver's Demand Engine - Percent Torque (%)
    // 0x62 - Actual Engine - Percent Torque (%)
    // 0x64 - Engine Percent Torque Data (%)
    static float decodeTorquePercentage(uint8 const A) { return static_cast<int8>(A) - 125.0f; }

    // 0x63 - Engine Reference Torque (Nm)
    static uint16 decodeReferenceTorque(uint8 const A, uint8 const B) { return (A << 8) | B; }
};

#endif //OBDVG_PIDDECODER_H
