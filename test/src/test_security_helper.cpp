#include <gtest/gtest.h>
#include "SecurityHelper.h"
#include "Logger.h"
#include <sstream>
#include <iostream>

// Test fixture for SecurityHelper tests
class SecurityHelperTest : public ::testing::Test
{
protected:
    std::stringstream coutBuffer;
    std::stringstream cerrBuffer;
    std::streambuf *oldCoutBuffer;
    std::streambuf *oldCerrBuffer;

    // Delete copy constructor and assignment operator (test fixtures should not be copied)
    SecurityHelperTest(const SecurityHelperTest &) = delete;
    SecurityHelperTest &operator=(const SecurityHelperTest &) = delete;

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

    SecurityHelperTest()
        : coutBuffer(), cerrBuffer(), oldCoutBuffer(nullptr), oldCerrBuffer(nullptr)
    {
        // Constructor
    }
};

// Test valid commands that should pass validation
TEST_F(SecurityHelperTest, ValidSimpleCommand)
{
    // Use a simple, safe command that exists on most systems
    bool result = ExecuteCommand("echo test");
    EXPECT_TRUE(result);
}

TEST_F(SecurityHelperTest, ValidCommandWithArguments)
{
    bool result = ExecuteCommand("echo hello world");
    EXPECT_TRUE(result);
}

TEST_F(SecurityHelperTest, ValidCommandWithPath)
{
    bool result = ExecuteCommand("ls /tmp");
    EXPECT_TRUE(result);
}

TEST_F(SecurityHelperTest, ValidCommandWithHyphenOptions)
{
    bool result = ExecuteCommand("echo -n test");
    EXPECT_TRUE(result);
}

TEST_F(SecurityHelperTest, ValidCommandWithDoubleHyphenOptions)
{
    bool result = ExecuteCommand("echo --help");
    EXPECT_TRUE(result);
}

TEST_F(SecurityHelperTest, ValidCommandWithNumbers)
{
    bool result = ExecuteCommand("echo 12345");
    EXPECT_TRUE(result);
}

TEST_F(SecurityHelperTest, ValidCommandWithUnderscores)
{
    bool result = ExecuteCommand("echo test_file");
    EXPECT_TRUE(result);
}

TEST_F(SecurityHelperTest, ValidCommandWithDots)
{
    bool result = ExecuteCommand("echo test.txt");
    EXPECT_TRUE(result);
}

TEST_F(SecurityHelperTest, ValidCommandWithSpaces)
{
    bool result = ExecuteCommand("echo multiple words here");
    EXPECT_TRUE(result);
}

TEST_F(SecurityHelperTest, ValidCommandWithForwardSlash)
{
    bool result = ExecuteCommand("echo /path/to/file");
    EXPECT_TRUE(result);
}

// Test invalid commands with dangerous characters
TEST_F(SecurityHelperTest, InvalidCommandWithSemicolon)
{
    bool result = ExecuteCommand("echo test; rm -rf /");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Invalid character ';'") != std::string::npos);
    EXPECT_TRUE(errorOutput.find("[;&|><`]") != std::string::npos);
}

TEST_F(SecurityHelperTest, InvalidCommandWithAmpersand)
{
    bool result = ExecuteCommand("echo test & malicious_command");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Invalid character '&'") != std::string::npos);
}

TEST_F(SecurityHelperTest, InvalidCommandWithPipe)
{
    bool result = ExecuteCommand("cat /etc/passwd | grep root");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Invalid character '|'") != std::string::npos);
}

TEST_F(SecurityHelperTest, InvalidCommandWithRedirectOutput)
{
    bool result = ExecuteCommand("echo test > /tmp/output.txt");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Invalid character '>'") != std::string::npos);
}

TEST_F(SecurityHelperTest, InvalidCommandWithRedirectInput)
{
    bool result = ExecuteCommand("cat < /etc/passwd");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Invalid character '<'") != std::string::npos);
}

TEST_F(SecurityHelperTest, InvalidCommandWithBacktick)
{
    bool result = ExecuteCommand("echo `whoami`");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Invalid character '`'") != std::string::npos);
}

TEST_F(SecurityHelperTest, InvalidCommandWithMultipleDangerousChars)
{
    bool result = ExecuteCommand("echo test; cat /etc/passwd | grep root");
    EXPECT_FALSE(result);

    // Should report the first dangerous character found
    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Invalid character") != std::string::npos);
}

// Test edge cases
TEST_F(SecurityHelperTest, EmptyCommand)
{
    // Empty commands pass validation (no dangerous chars) and system("") returns 0
    bool result = ExecuteCommand("");
    // Note: system("") typically returns 0 indicating shell is available
    EXPECT_TRUE(result) << "Empty command passes validation and succeeds with system()";
}

TEST_F(SecurityHelperTest, CommandWithOnlySpaces)
{
    // Whitespace-only commands pass validation and succeed
    bool result = ExecuteCommand("   ");
    EXPECT_TRUE(result) << "Whitespace passes validation and succeeds with system()";
}

TEST_F(SecurityHelperTest, ValidCommandThatFails)
{
    // This command should pass validation but fail execution
    bool result = ExecuteCommand("false");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Command execution failed") != std::string::npos);
}

TEST_F(SecurityHelperTest, ValidCommandWithNonExistentExecutable)
{
    bool result = ExecuteCommand("nonexistent_command_12345");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Command execution failed") != std::string::npos);
}

// Test position reporting of invalid characters
TEST_F(SecurityHelperTest, InvalidCharacterPositionReporting)
{
    bool result = ExecuteCommand("echo test");
    clearBuffers();

    result = ExecuteCommand("test;malicious");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("position 4") != std::string::npos);
}

TEST_F(SecurityHelperTest, InvalidCharacterAtBeginning)
{
    bool result = ExecuteCommand(";malicious");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("position 0") != std::string::npos);
}

TEST_F(SecurityHelperTest, InvalidCharacterAtEnd)
{
    bool result = ExecuteCommand("echo test;");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Invalid character ';'") != std::string::npos);
}

// Test special command injection attempts
TEST_F(SecurityHelperTest, CommandInjectionAttemptWithNewline)
{
    bool result = ExecuteCommand("echo test\nrm -rf /");
    (void)result; // Newline is not in the blocked character set
    // The behavior depends on the system's command processor
    // This test documents that newlines are currently allowed
}

TEST_F(SecurityHelperTest, CommandInjectionWithDollarSubstitution)
{
    // $(command) substitution doesn't use backticks but could be dangerous
    // However, it's not in the current blocked list - this documents current behavior
    // Note: $ alone is not blocked, but $() might be executed by shell
}

// Test that validation error messages are logged
TEST_F(SecurityHelperTest, ValidationErrorIsLogged)
{
    clearBuffers();

    bool result = ExecuteCommand("echo test | grep test");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_FALSE(errorOutput.empty());
    EXPECT_TRUE(errorOutput.find("[ERROR]") != std::string::npos);
}

// Test multiple sequential command attempts
TEST_F(SecurityHelperTest, MultipleValidCommands)
{
    bool result1 = ExecuteCommand("echo test1");
    clearBuffers();

    bool result2 = ExecuteCommand("echo test2");
    clearBuffers();

    bool result3 = ExecuteCommand("echo test3");

    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_TRUE(result3);
}

TEST_F(SecurityHelperTest, MixedValidAndInvalidCommands)
{
    bool result1 = ExecuteCommand("echo valid");
    EXPECT_TRUE(result1);
    clearBuffers();

    bool result2 = ExecuteCommand("echo invalid; rm test");
    EXPECT_FALSE(result2);
    clearBuffers();

    bool result3 = ExecuteCommand("echo valid_again");
    EXPECT_TRUE(result3);
}

// Test long commands
TEST_F(SecurityHelperTest, LongValidCommand)
{
    std::string longCommand = "echo ";
    longCommand += std::string(500, 'a');

    bool result = ExecuteCommand(longCommand);
    EXPECT_TRUE(result);
}

TEST_F(SecurityHelperTest, LongInvalidCommand)
{
    std::string longCommand = "echo ";
    longCommand += std::string(500, 'a');
    longCommand += ";malicious";

    bool result = ExecuteCommand(longCommand);
    EXPECT_FALSE(result);
}

// Test commands with quotes (quotes are not blocked)
TEST_F(SecurityHelperTest, CommandWithSingleQuotes)
{
    bool result = ExecuteCommand("echo 'hello world'");
    EXPECT_TRUE(result);
}

TEST_F(SecurityHelperTest, CommandWithDoubleQuotes)
{
    bool result = ExecuteCommand("echo \"hello world\"");
    EXPECT_TRUE(result);
}

// Test parentheses (not currently blocked, documenting behavior)
TEST_F(SecurityHelperTest, CommandWithParentheses)
{
    bool result = ExecuteCommand("echo (test)");
    (void)result; // Parentheses are not in the blocked list
    // This test documents current behavior - parentheses are allowed
}

// Test command with equals sign (not blocked, used in assignments)
TEST_F(SecurityHelperTest, CommandWithEquals)
{
    bool result = ExecuteCommand("echo test=value");
    EXPECT_TRUE(result);
}

// Test asterisk and wildcards (not currently blocked)
TEST_F(SecurityHelperTest, CommandWithAsterisk)
{
    bool result = ExecuteCommand("echo *.txt");
    // Asterisk is not in the blocked list
    EXPECT_TRUE(result);
}

// Test that execution errors are properly caught and reported
TEST_F(SecurityHelperTest, ExecutionErrorReporting)
{
    clearBuffers();

    // Command that will definitely fail
    bool result = ExecuteCommand("exit 1");
    EXPECT_FALSE(result);

    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Command execution failed with code:") != std::string::npos ||
                errorOutput.find("execution failed") != std::string::npos);
}
