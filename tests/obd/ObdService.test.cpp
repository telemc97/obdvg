#include <gtest/gtest.h>
#include "obd/ObdService.h"
#include "obd/ObdPids.h"
#include <vector>

TEST(ObdServiceTest, BuildRequest) {
    CanFrame tx;
    ObdService::buildCanFrameForPID(ObdPid::ENGINE_RPM, tx);
    
    EXPECT_EQ(tx.id, 0x7DF);
    EXPECT_EQ(tx.data[0], 0x02); // Length
    EXPECT_EQ(tx.data[1], 0x01); // Mode 01
    EXPECT_EQ(tx.data[2], (uint8)ObdPid::ENGINE_RPM);
}

void TestDecodePid(ObdPid pid, int bytes) {
    CanFrame rx;
    rx.id = 0x7E8;
    rx.dlc = 8;
    rx.data[0] = 1 + 1 + bytes; // pciLen
    rx.data[1] = 0x41; // Response Mode
    rx.data[2] = (uint8)pid;
    rx.data[3] = 0x10;
    rx.data[4] = 0x20;
    
    float32 val = 0;
    EXPECT_TRUE(ObdService::pollResponse(rx, pid, val));
}

TEST(ObdServiceTest, DecodeAllPids) {
    // 1-byte PIDs
    std::vector<ObdPid> oneBytePids = {
        ObdPid::ENGINE_LOAD, ObdPid::ENGINE_COOLANT_TEMP, ObdPid::SHORT_TERM_FUEL_TRIM_1,
        ObdPid::LONG_TERM_FUEL_TRIM_1, ObdPid::SHORT_TERM_FUEL_TRIM_2, ObdPid::LONG_TERM_FUEL_TRIM_2,
        ObdPid::FUEL_PRESSURE, ObdPid::INTAKE_MAP, ObdPid::VEHICLE_SPEED, ObdPid::TIMING_ADVANCE,
        ObdPid::INTAKE_AIR_TEMP, ObdPid::THROTTLE_POS, ObdPid::COMMANDED_EGR, ObdPid::EGR_ERROR,
        ObdPid::COMMANDED_EVAP_PURGE, ObdPid::FUEL_LEVEL, ObdPid::WARMUPS_SINCE_CLEARED,
        ObdPid::BAROMETRIC_PRESSURE, ObdPid::RELATIVE_THROTTLE_POS, ObdPid::ABSOLUTE_THROTTLE_POS_B,
        ObdPid::ABSOLUTE_THROTTLE_POS_C, ObdPid::ACCELERATOR_PEDAL_POS_D, ObdPid::ACCELERATOR_PEDAL_POS_E,
        ObdPid::ACCELERATOR_PEDAL_POS_F, ObdPid::COMMANDED_THROTTLE_ACTUATOR, ObdPid::AMBIENT_AIR_TEMP,
        ObdPid::MAX_MAF_FLOW, ObdPid::ETHANOL_FUEL_PERCENT, ObdPid::RELATIVE_ACCEL_PEDAL_POS,
        ObdPid::HYBRID_BATTERY_REMAINING, ObdPid::ENGINE_OIL_TEMP, ObdPid::DEMAND_ENGINE_TORQUE,
        ObdPid::ACTUAL_ENGINE_TORQUE, ObdPid::ENGINE_PERCENT_TORQUE, ObdPid::ENGINE_COOLANT_TEMP_SENSOR,
        ObdPid::INTAKE_AIR_TEMP_SENSOR
    };

    for (ObdPid pid : oneBytePids) {
        TestDecodePid(pid, 1);
    }

    // 2-byte PIDs
    std::vector<ObdPid> twoBytePids = {
        ObdPid::ENGINE_RPM, ObdPid::MAF_FLOW, ObdPid::RUNTIME_SINCE_START, ObdPid::DISTANCE_WITH_MIL,
        ObdPid::FUEL_RAIL_PRESSURE, ObdPid::FUEL_RAIL_GAUGE_PRESSURE, ObdPid::DISTANCE_SINCE_CLEARED,
        ObdPid::EVAP_VAPOR_PRESSURE, ObdPid::CATALYST_TEMP_B1S1, ObdPid::CATALYST_TEMP_B2S1,
        ObdPid::CATALYST_TEMP_B1S2, ObdPid::CATALYST_TEMP_B2S2, ObdPid::CONTROL_MODULE_VOLTAGE,
        ObdPid::ABSOLUTE_LOAD_VALUE, ObdPid::COMMANDED_EQUIV_RATIO, ObdPid::ABS_EVAP_VAPOR_PRESSURE,
        ObdPid::EVAP_VAPOR_PRESSURE_ALT, ObdPid::FUEL_RAIL_ABS_PRESSURE, ObdPid::TIME_RUN_WITH_MIL,
        ObdPid::TIME_SINCE_CODES_CLEARED, ObdPid::FUEL_INJECTION_TIMING, ObdPid::ENGINE_FUEL_RATE,
        ObdPid::ENGINE_REFERENCE_TORQUE
    };

    for (ObdPid pid : twoBytePids) {
        TestDecodePid(pid, 2);
    }
}

TEST(ObdServiceTest, DefaultCase) {
    float32 val = 0;
    CanFrame rx;
    rx.id = 0x7E8;
    rx.dlc = 8;
    rx.data[0] = 3;
    rx.data[1] = 0x41;
    rx.data[2] = 0x99; // Unknown PID, 1 byte expected length fall-through
    EXPECT_FALSE(ObdService::pollResponse(rx, static_cast<ObdPid>(0x99), val));
}

TEST(ObdServiceTest, InvalidResponseId) {
    CanFrame rx;
    rx.id = 0x123; // WRONG ID
    rx.dlc = 8;
    rx.data[0] = 0x04;
    rx.data[1] = 0x41;
    rx.data[2] = (uint8)ObdPid::ENGINE_RPM;
    
    float32 val = 0;
    EXPECT_FALSE(ObdService::pollResponse(rx, ObdPid::ENGINE_RPM, val));
}

TEST(ObdServiceTest, InvalidResponsePid) {
    CanFrame rx;
    rx.id = 0x7E8;
    rx.dlc = 8;
    rx.data[0] = 0x04;
    rx.data[1] = 0x41;
    rx.data[2] = (uint8)ObdPid::VEHICLE_SPEED; // WRONG PID
    
    float32 val = 0;
    EXPECT_FALSE(ObdService::pollResponse(rx, ObdPid::ENGINE_RPM, val));
}

TEST(ObdServiceTest, InvalidResponseLength) {
    CanFrame rx;
    rx.id = 0x7E8;
    rx.dlc = 8;
    rx.data[0] = 0x02; // TOO SHORT
    rx.data[1] = 0x41;
    rx.data[2] = (uint8)ObdPid::ENGINE_RPM;
    
    float32 val = 0;
    EXPECT_FALSE(ObdService::pollResponse(rx, ObdPid::ENGINE_RPM, val));
}

TEST(ObdServiceTest, InvalidDlc) {
    CanFrame rx;
    rx.id = 0x7E8;
    rx.dlc = 2; // DLC < 3
    rx.data[0] = 0x04;
    rx.data[1] = 0x41;
    rx.data[2] = (uint8)ObdPid::ENGINE_RPM;
    
    float32 val = 0;
    EXPECT_FALSE(ObdService::pollResponse(rx, ObdPid::ENGINE_RPM, val));
}

TEST(ObdServiceTest, DlcTooSmallForPci) {
    CanFrame rx;
    rx.id = 0x7E8;
    rx.dlc = 4; // DLC < pciLen + 1 (which is 5)
    rx.data[0] = 0x04;
    rx.data[1] = 0x41;
    rx.data[2] = (uint8)ObdPid::ENGINE_RPM;
    
    float32 val = 0;
    EXPECT_FALSE(ObdService::pollResponse(rx, ObdPid::ENGINE_RPM, val));
}