#include <gtest/gtest.h>
#include "Logger.h"
#include <sstream>
#include <iostream>

// Test fixture for Logger tests
class LoggerTest : public ::testing::Test
{
protected:
    std::stringstream coutBuffer;
    std::stringstream cerrBuffer;
    std::streambuf *oldCoutBuffer;
    std::streambuf *oldCerrBuffer;

    // Delete copy constructor and assignment operator (test fixtures should not be copied)
    LoggerTest(const LoggerTest&) = delete;
    LoggerTest& operator=(const LoggerTest&) = delete;

    void SetUp() override
    {
        // Redirect cout and cerr to our buffers
        oldCoutBuffer = std::cout.rdbuf(coutBuffer.rdbuf());
        oldCerrBuffer = std::cerr.rdbuf(cerrBuffer.rdbuf());

        // Reset verbose mode to default state
        Logger::SetVerbose(false);
    }

    void TearDown() override
    {
        // Restore cout and cerr
        std::cout.rdbuf(oldCoutBuffer);
        std::cerr.rdbuf(oldCerrBuffer);
    }

    std::string getCoutOutput()
    {
        return coutBuffer.str();
    }

    std::string getCerrOutput()
    {
        return cerrBuffer.str();
    }

    void clearBuffers()
    {
        coutBuffer.str("");
        coutBuffer.clear();
        cerrBuffer.str("");
        cerrBuffer.clear();
    }

    LoggerTest()
        : coutBuffer(), cerrBuffer(), oldCoutBuffer(nullptr), oldCerrBuffer(nullptr)
    {
        // Constructor
    }
};

// Test SetVerbose functionality
TEST_F(LoggerTest, SetVerboseTrue)
{
    Logger::SetVerbose(true);
    Logger::LogVerbose("Verbose message");
    EXPECT_EQ(getCoutOutput(), "Verbose message\n");
}

TEST_F(LoggerTest, SetVerboseFalse)
{
    Logger::SetVerbose(false);
    Logger::LogVerbose("Verbose message");
    EXPECT_EQ(getCoutOutput(), "");
}

TEST_F(LoggerTest, SetVerboseToggle)
{
    // Start with verbose off
    Logger::SetVerbose(false);
    Logger::LogVerbose("Should not appear");
    EXPECT_EQ(getCoutOutput(), "");

    clearBuffers();

    // Turn verbose on
    Logger::SetVerbose(true);
    Logger::LogVerbose("Should appear");
    EXPECT_EQ(getCoutOutput(), "Should appear\n");

    clearBuffers();

    // Turn verbose off again
    Logger::SetVerbose(false);
    Logger::LogVerbose("Should not appear again");
    EXPECT_EQ(getCoutOutput(), "");
}

// Test Log with different LogLevels
TEST_F(LoggerTest, LogVerboseLevel)
{
    Logger::Log(Logger::LogLevel::VERBOSE, "Verbose log");
    EXPECT_EQ(getCoutOutput(), "Verbose log\n");
    EXPECT_EQ(getCerrOutput(), "");
}

TEST_F(LoggerTest, LogInfoLevel)
{
    Logger::Log(Logger::LogLevel::INFO, "Info log");
    EXPECT_EQ(getCoutOutput(), "Info log\n");
    EXPECT_EQ(getCerrOutput(), "");
}

TEST_F(LoggerTest, LogWarningLevel)
{
    Logger::Log(Logger::LogLevel::WARNING, "Warning log");
    EXPECT_EQ(getCoutOutput(), "");
    EXPECT_EQ(getCerrOutput(), "[WARNING] Warning log\n");
}

TEST_F(LoggerTest, LogErrorLevel)
{
    Logger::Log(Logger::LogLevel::ERROR, "Error log");
    EXPECT_EQ(getCoutOutput(), "");
    EXPECT_EQ(getCerrOutput(), "[ERROR] Error log\n");
}

// Test LogVerbose method
TEST_F(LoggerTest, LogVerboseWhenVerboseIsOff)
{
    Logger::SetVerbose(false);
    Logger::LogVerbose("This should not appear");
    EXPECT_EQ(getCoutOutput(), "");
}

TEST_F(LoggerTest, LogVerboseWhenVerboseIsOn)
{
    Logger::SetVerbose(true);
    Logger::LogVerbose("This should appear");
    EXPECT_EQ(getCoutOutput(), "This should appear\n");
}

TEST_F(LoggerTest, LogVerboseEmptyString)
{
    Logger::SetVerbose(true);
    Logger::LogVerbose("");
    EXPECT_EQ(getCoutOutput(), "\n");
}

// Test LogInfo method
TEST_F(LoggerTest, LogInfoMessage)
{
    Logger::LogInfo("Info message");
    EXPECT_EQ(getCoutOutput(), "Info message\n");
    EXPECT_EQ(getCerrOutput(), "");
}

TEST_F(LoggerTest, LogInfoEmptyString)
{
    Logger::LogInfo("");
    EXPECT_EQ(getCoutOutput(), "\n");
}

TEST_F(LoggerTest, LogInfoMultipleMessages)
{
    Logger::LogInfo("First message");
    Logger::LogInfo("Second message");
    EXPECT_EQ(getCoutOutput(), "First message\nSecond message\n");
}

// Test LogWarning method
TEST_F(LoggerTest, LogWarningMessage)
{
    Logger::LogWarning("Warning message");
    EXPECT_EQ(getCoutOutput(), "");
    EXPECT_EQ(getCerrOutput(), "[WARNING] Warning message\n");
}

TEST_F(LoggerTest, LogWarningEmptyString)
{
    Logger::LogWarning("");
    EXPECT_EQ(getCerrOutput(), "[WARNING] \n");
}

TEST_F(LoggerTest, LogWarningMultipleMessages)
{
    Logger::LogWarning("First warning");
    Logger::LogWarning("Second warning");
    EXPECT_EQ(getCerrOutput(), "[WARNING] First warning\n[WARNING] Second warning\n");
}

// Test LogError method
TEST_F(LoggerTest, LogErrorMessage)
{
    Logger::LogError("Error message");
    EXPECT_EQ(getCoutOutput(), "");
    EXPECT_EQ(getCerrOutput(), "[ERROR] Error message\n");
}

TEST_F(LoggerTest, LogErrorEmptyString)
{
    Logger::LogError("");
    EXPECT_EQ(getCerrOutput(), "[ERROR] \n");
}

TEST_F(LoggerTest, LogErrorMultipleMessages)
{
    Logger::LogError("First error");
    Logger::LogError("Second error");
    EXPECT_EQ(getCerrOutput(), "[ERROR] First error\n[ERROR] Second error\n");
}

// Test special characters and long strings
TEST_F(LoggerTest, LogSpecialCharacters)
{
    Logger::LogInfo("Special chars: !@#$%^&*(){}[]|\\;:'\"<>?,./`~");
    EXPECT_TRUE(getCoutOutput().find("Special chars:") != std::string::npos);
}

TEST_F(LoggerTest, LogUnicodeCharacters)
{
    Logger::LogInfo("Unicode: 你好 مرحبا Привет");
    EXPECT_TRUE(getCoutOutput().find("Unicode:") != std::string::npos);
}

TEST_F(LoggerTest, LogNewlineInMessage)
{
    Logger::LogInfo("Line1\nLine2");
    EXPECT_EQ(getCoutOutput(), "Line1\nLine2\n");
}

TEST_F(LoggerTest, LogTabInMessage)
{
    Logger::LogInfo("Column1\tColumn2");
    EXPECT_EQ(getCoutOutput(), "Column1\tColumn2\n");
}

TEST_F(LoggerTest, LogLongMessage)
{
    std::string longMessage(1000, 'A');
    Logger::LogInfo(longMessage);
    EXPECT_EQ(getCoutOutput(), longMessage + "\n");
}

// Test mixed log levels
TEST_F(LoggerTest, MixedLogLevels)
{
    Logger::LogInfo("Info");
    Logger::LogWarning("Warning");
    Logger::LogError("Error");

    EXPECT_EQ(getCoutOutput(), "Info\n");
    EXPECT_EQ(getCerrOutput(), "[WARNING] Warning\n[ERROR] Error\n");
}

TEST_F(LoggerTest, MixedLogLevelsWithVerbose)
{
    Logger::SetVerbose(true);

    Logger::LogVerbose("Verbose");
    Logger::LogInfo("Info");
    Logger::LogWarning("Warning");
    Logger::LogError("Error");

    EXPECT_EQ(getCoutOutput(), "Verbose\nInfo\n");
    EXPECT_EQ(getCerrOutput(), "[WARNING] Warning\n[ERROR] Error\n");
}

// Test verbose mode does not affect other log levels
TEST_F(LoggerTest, VerboseModeDoesNotAffectInfo)
{
    Logger::SetVerbose(false);
    Logger::LogInfo("Info message");
    EXPECT_EQ(getCoutOutput(), "Info message\n");
}

TEST_F(LoggerTest, VerboseModeDoesNotAffectWarning)
{
    Logger::SetVerbose(false);
    Logger::LogWarning("Warning message");
    EXPECT_EQ(getCerrOutput(), "[WARNING] Warning message\n");
}

TEST_F(LoggerTest, VerboseModeDoesNotAffectError)
{
    Logger::SetVerbose(false);
    Logger::LogError("Error message");
    EXPECT_EQ(getCerrOutput(), "[ERROR] Error message\n");
}

// Test edge cases
TEST_F(LoggerTest, LogAfterClearingBuffers)
{
    Logger::LogInfo("First");
    clearBuffers();
    Logger::LogInfo("Second");
    EXPECT_EQ(getCoutOutput(), "Second\n");
}

TEST_F(LoggerTest, LogMultipleTypesInSequence)
{
    Logger::LogInfo("Info 1");
    Logger::LogWarning("Warning 1");
    Logger::LogInfo("Info 2");
    Logger::LogError("Error 1");
    Logger::LogInfo("Info 3");

    EXPECT_EQ(getCoutOutput(), "Info 1\nInfo 2\nInfo 3\n");
    EXPECT_EQ(getCerrOutput(), "[WARNING] Warning 1\n[ERROR] Error 1\n");
}

// Test numerical and boolean values in messages
TEST_F(LoggerTest, LogNumericalValues)
{
    Logger::LogInfo("Integer: " + std::to_string(42));
    Logger::LogInfo("Float: " + std::to_string(3.14159));
    Logger::LogInfo("Negative: " + std::to_string(-100));

    EXPECT_TRUE(getCoutOutput().find("42") != std::string::npos);
    EXPECT_TRUE(getCoutOutput().find("3.14159") != std::string::npos);
    EXPECT_TRUE(getCoutOutput().find("-100") != std::string::npos);
}

// Test Log method directly with all levels
TEST_F(LoggerTest, DirectLogMethodAllLevels)
{
    Logger::Log(Logger::LogLevel::VERBOSE, "Verbose");
    Logger::Log(Logger::LogLevel::INFO, "Info");
    Logger::Log(Logger::LogLevel::WARNING, "Warning");
    Logger::Log(Logger::LogLevel::ERROR, "Error");

    EXPECT_EQ(getCoutOutput(), "Verbose\nInfo\n");
    EXPECT_EQ(getCerrOutput(), "[WARNING] Warning\n[ERROR] Error\n");
}