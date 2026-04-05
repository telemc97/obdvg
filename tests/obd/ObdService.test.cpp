#include <gtest/gtest.h>
#include "obd/ObdService.h"

TEST(ObdServiceTest, BuildRequest) {
    CanFrame tx;
    ObdService::buildCanFrameForPID(ObdPid::ENGINE_RPM, tx);
    
    EXPECT_EQ(tx.id, 0x7DF);
    EXPECT_EQ(tx.data[0], 0x02); // Length
    EXPECT_EQ(tx.data[1], 0x01); // Mode 01
    EXPECT_EQ(tx.data[2], (uint8)ObdPid::ENGINE_RPM);
}

TEST(ObdServiceTest, ValidResponse) {
    CanFrame rx;
    rx.id = 0x7E8;
    rx.dlc = 8;
    rx.data[0] = 0x04; // Length (Service + PID + 2 bytes data)
    rx.data[1] = 0x41; // Response Mode
    rx.data[2] = (uint8)ObdPid::ENGINE_RPM;
    
    float32 val = 0;
    EXPECT_TRUE(ObdService::pollResponse(rx, ObdPid::ENGINE_RPM, val));
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
    rx.data[0] = 0x02; // TOO SHORT (claimed length doesn't fit Service+PID+RPM_Data)
    rx.data[1] = 0x41;
    rx.data[2] = (uint8)ObdPid::ENGINE_RPM;
    
    float32 val = 0;
    EXPECT_FALSE(ObdService::pollResponse(rx, ObdPid::ENGINE_RPM, val));
}
