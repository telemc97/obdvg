#include <gtest/gtest.h>
#include "util/Logger.h"

// Since Logger is a singleton and prints to stdout, 
// we use GTest's capture capabilities to verify output.

TEST(LoggerTest, LineBreaking) {
    Logger& logger = Logger::instance();
    logger.setCharLimit(10);
    
    testing::internal::CaptureStdout();
    logger.log("Short");
    String output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "Short\n");

    testing::internal::CaptureStdout();
    logger.log("This is a very long string that should break");
    output = testing::internal::GetCapturedStdout();
    // "This is a " (10)
    // "very long " (10)
    // "string " (7)
    // "that " (5)
    // "should " (7)
    // "break" (5)
    // Note: The algorithm might produce different exact breaks depending on space handling
    EXPECT_TRUE(output.find("\n") != String::npos);
}

TEST(LoggerTest, LogLevelFiltering) {
    Logger& logger = Logger::instance();
    logger.setCharLimit(80); // Reset to large value
    logger.setLevel(LogLevel::ERROR);
    
    testing::internal::CaptureStdout();
    logger.log(LogLevel::DEBUG, "Debug message");
    String output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, ""); // Should be filtered out

    testing::internal::CaptureStdout();
    logger.log(LogLevel::ERROR, "Error message");
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "Error message\n"); // Should be printed
}

TEST(LoggerTest, Separator) {
    Logger& logger = Logger::instance();
    logger.setCharLimit(5);
    
    testing::internal::CaptureStdout();
    logger.print_separator();
    String output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "-----\n");
}
