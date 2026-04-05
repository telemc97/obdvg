#include <gtest/gtest.h>
#include "obd/PidDecoder.h"
#include <cmath>

TEST(PidDecoderTest, DecodeRpm) {
    // Formula: ((A*256)+B)/4
    // 3000 RPM -> (3000 * 4) = 12000 -> 0x2EE0
    EXPECT_FLOAT_EQ(PidDecoder::decodeRpm(0x2E, 0xE0), 3000.0f);
    EXPECT_FLOAT_EQ(PidDecoder::decodeRpm(0x00, 0x00), 0.0f);
}

TEST(PidDecoderTest, DecodeSpeed) {
    // Formula: A
    EXPECT_FLOAT_EQ(PidDecoder::decodeSpeed(100), 100.0f);
    EXPECT_FLOAT_EQ(PidDecoder::decodeSpeed(0), 0.0f);
}

TEST(PidDecoderTest, DecodeTemperature) {
    // Formula: A - 40
    EXPECT_FLOAT_EQ(PidDecoder::decodeTemperature(40), 0.0f);   // 40 - 40 = 0
    EXPECT_FLOAT_EQ(PidDecoder::decodeTemperature(100), 60.0f); // 100 - 40 = 60
    EXPECT_FLOAT_EQ(PidDecoder::decodeTemperature(0), -40.0f);  // 0 - 40 = -40
}

TEST(PidDecoderTest, DecodeFuelPressure) {
    // Formula: A * 3
    // Testing the fix for overflow (A > 85)
    EXPECT_FLOAT_EQ(PidDecoder::decodeFuelPressure(100), 300.0f);
    EXPECT_FLOAT_EQ(PidDecoder::decodeFuelPressure(255), 765.0f);
}

TEST(PidDecoderTest, DecodeTorquePercentage) {
    // Formula: A - 125
    // Testing the fix for overflow (A > 127)
    EXPECT_FLOAT_EQ(PidDecoder::decodeTorquePercentage(125), 0.0f);
    EXPECT_FLOAT_EQ(PidDecoder::decodeTorquePercentage(128), 3.0f);
    EXPECT_FLOAT_EQ(PidDecoder::decodeTorquePercentage(255), 130.0f);
    EXPECT_FLOAT_EQ(PidDecoder::decodeTorquePercentage(0), -125.0f);
}

TEST(PidDecoderTest, DecodeEngineLoad) {
    // Formula: A * 100 / 255
    EXPECT_NEAR(PidDecoder::decodeEngineLoad(255), 100.0f, 0.01f);
    EXPECT_FLOAT_EQ(PidDecoder::decodeEngineLoad(0), 0.0f);
    EXPECT_NEAR(PidDecoder::decodeEngineLoad(127), 49.8f, 0.1f);
}
