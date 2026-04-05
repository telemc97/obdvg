#include <gtest/gtest.h>
#include "can/UartCanBus.h"
#include "hardware/uart.h"

class UartCanBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        MockUart::clear();
    }
};

TEST_F(UartCanBusTest, SendStandardFrame) {
    UartCanBus bus(uart1);
    CanFrame frame;
    frame.id = 0x123;
    frame.dlc = 2;
    frame.isExtended = false;
    frame.data[0] = 0xAA;
    frame.data[1] = 0xBB;
    
    EXPECT_TRUE(bus.send(frame));
    EXPECT_EQ(MockUart::get_tx(), "t1232AABB\r");
}

TEST_F(UartCanBusTest, SendExtendedFrame) {
    UartCanBus bus(uart1);
    CanFrame frame;
    frame.id = 0x12345678;
    frame.dlc = 1;
    frame.isExtended = true;
    frame.data[0] = 0xFF;
    
    EXPECT_TRUE(bus.send(frame));
    EXPECT_EQ(MockUart::get_tx(), "T123456781FF\r");
}

TEST_F(UartCanBusTest, ReceiveStandardFrame) {
    UartCanBus bus(uart1);
    MockUart::inject_rx("t1232AABB\r");
    
    CanFrame frame;
    EXPECT_TRUE(bus.receive(frame));
    EXPECT_EQ(frame.id, 0x123);
    EXPECT_EQ(frame.dlc, 2);
    EXPECT_FALSE(frame.isExtended);
    EXPECT_EQ(frame.data[0], 0xAA);
    EXPECT_EQ(frame.data[1], 0xBB);
}

TEST_F(UartCanBusTest, ReceiveInvalidLength) {
    UartCanBus bus(uart1);
    MockUart::inject_rx("t12\r"); // Too short
    CanFrame frame;
    EXPECT_FALSE(bus.receive(frame));
}

TEST_F(UartCanBusTest, ReceiveInvalidHex) {
    UartCanBus bus(uart1);
    MockUart::inject_rx("t1232AAXX\r"); // XX is not valid hex
    CanFrame frame;
    EXPECT_FALSE(bus.receive(frame));
}

TEST_F(UartCanBusTest, IsConnectedSuccess) {
    UartCanBus bus(uart1);
    MockUart::inject_rx("OK\r"); // Send OK before carriage return
    EXPECT_TRUE(bus.isConnected());
    EXPECT_EQ(MockUart::get_tx(), "AT\r");
}

TEST_F(UartCanBusTest, IsConnectedTimeout) {
    UartCanBus bus(uart1);
    // Do not inject anything, it should time out
    EXPECT_FALSE(bus.isConnected());
}

TEST_F(UartCanBusTest, ReceiveExtendedFrame) {
    UartCanBus bus(uart1);
    MockUart::inject_rx("T123456781FF\r");
    
    CanFrame frame;
    EXPECT_TRUE(bus.receive(frame));
    EXPECT_EQ(frame.id, 0x12345678);
    EXPECT_EQ(frame.dlc, 1);
    EXPECT_TRUE(frame.isExtended);
    EXPECT_EQ(frame.data[0], 0xFF);
}

TEST_F(UartCanBusTest, BufferOverflowProtection) {
    UartCanBus bus(uart1);
    CanFrame frame;
    frame.dlc = 9; // INVALID DLC
    
    EXPECT_FALSE(bus.send(frame));
}
