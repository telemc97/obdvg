#include <gtest/gtest.h>
#include "bt/ELM327.h"

class ELM327Test : public ::testing::Test {
protected:
    ELM327 elm;
};

TEST_F(ELM327Test, HandleATZ) {
    // ATZ should reset and return ID string
    String resp = elm.processInput("ATZ");
    EXPECT_TRUE(resp.find("ELM327 v1.5") != String::npos);
    EXPECT_TRUE(resp.find("OK") != String::npos);
    EXPECT_TRUE(resp.back() == '>');
}

TEST_F(ELM327Test, HandleATE0) {
    // Echo Off
    String resp = elm.processInput("ATE0");
    EXPECT_TRUE(resp.find("OK") != String::npos);
    
    // Subsequent command should NOT be echoed
    resp = elm.processInput("ATI");
    EXPECT_TRUE(resp.find("ATI") == String::npos);
}

TEST_F(ELM327Test, UnknownATCommand) {
    // Unknown command should return '?'
    String resp = elm.processInput("ATUNKNOWN");
    EXPECT_TRUE(resp.find("?") != String::npos);
}

TEST_F(ELM327Test, HandleMoreATCommands) {
    elm.processInput("ATE0"); // Turn off echo for cleaner assertions
    EXPECT_EQ(elm.processInput("ATH0"), "OK\r>");
    EXPECT_EQ(elm.processInput("ATH1"), "OK\r>");
    EXPECT_EQ(elm.processInput("ATL0"), "OK\r>");
    EXPECT_EQ(elm.processInput("ATL1"), "OK\r>");
    EXPECT_EQ(elm.processInput("ATDP"), "AUTO, CAN (11/500)\r>");
    EXPECT_EQ(elm.processInput("ATSP0"), "OK\r>"); // ATSP anything returns OK
    EXPECT_EQ(elm.processInput("ATI"), "ELM327 v1.5\r>");
}

TEST_F(ELM327Test, HandleATSH) {
    elm.processInput("ATE0"); // Turn off echo
    // Standard ID
    EXPECT_EQ(elm.processInput("ATSH7E0"), "OK\r>");
    // Extended ID
    EXPECT_EQ(elm.processInput("ATSH18DAF110"), "OK\r>");
    // Invalid
    EXPECT_TRUE(elm.processInput("ATSHZZZ").find("?") != String::npos);
}

TEST_F(ELM327Test, ObdRequestModeOnly) {
    // Mode 04 (Clear DTCs) or Mode 03 (Show DTCs)
    elm.processInput("03");
    
    CanFrame tx;
    EXPECT_TRUE(elm.hasPendingCanRequest(tx));
    EXPECT_EQ(tx.data[0], 0x01); // Length 1
    EXPECT_EQ(tx.data[1], 0x03); // Mode 03
}

TEST_F(ELM327Test, ObdRequestParsing) {
    // Mode 01 PID 0C (RPM)
    elm.processInput("010C");
    
    CanFrame tx;
    EXPECT_TRUE(elm.hasPendingCanRequest(tx));
    EXPECT_EQ(tx.id, 0x7DF);
    EXPECT_EQ(tx.data[0], 0x02); // Length
    EXPECT_EQ(tx.data[1], 0x01); // Mode
    EXPECT_EQ(tx.data[2], 0x0C); // PID
}

TEST_F(ELM327Test, FormatResponse) {
    elm.processInput("010C"); // Request RPM
    
    CanFrame rx;
    rx.id = 0x7E8;
    rx.dlc = 8;
    rx.data[0] = 0x04; // PCI: Single Frame, Length 4
    rx.data[1] = 0x41; // Response Mode (01 + 40)
    rx.data[2] = 0x0C; // PID
    rx.data[3] = 0x1A; // Value A
    rx.data[4] = 0xF8; // Value B
    
    String resp = elm.formatCanResponse(rx);
    // Should be "41 0C 1A F8 \r>" or similar (spaces might vary)
    EXPECT_TRUE(resp.find("41 0C 1A F8") != String::npos);
    EXPECT_TRUE(resp.back() == '>');
}

TEST_F(ELM327Test, ResponseFiltering) {
    elm.processInput("010C"); // Request RPM (0C)
    
    CanFrame rx;
    rx.id = 0x7E8;
    rx.dlc = 8;
    rx.data[0] = 0x04;
    rx.data[1] = 0x41;
    rx.data[2] = 0x0D; // WRONG PID (Speed instead of RPM)
    
    String resp = elm.formatCanResponse(rx);
    EXPECT_EQ(resp, ""); // Should be ignored
}
