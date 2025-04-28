//**************************************************************
// Includes
//**************************************************************

#include "SecurityHelper.h"
#include "Logger.h"
#include <iostream>
#include <regex>

//**************************************************************
// Local function prototypes
//**************************************************************

static bool IsValidCommand(const std::string &command);

//**************************************************************
// Global functions
//**************************************************************

bool ExecuteCommand(const std::string &command)
{
    if (!IsValidCommand(command))
        return false;

    try
    {
        int result = std::system(command.c_str());
        if (result != 0)
        {
            Logger::LogError("Command execution failed with code: " + std::to_string(result));
            return false;
        }
    }
    catch (const std::exception &e)
    {
        // Handle any exceptions that may occur during command execution
        Logger::LogError("Exception during command execution: " + std::string(e.what()));
        return false;
    }
    catch (...)
    {
        Logger::LogError("Unknown error occurred during command execution.");
        return false;
    }

    return true;
}

//**************************************************************
// Local functions
//**************************************************************

static bool IsValidCommand(const std::string &input)
{
    try
    {
        // Disallow dangerous characters or patterns commonly used in command injection
        static const std::regex dangerousPatternRegex("[;&|><`]");
        std::smatch match;

        if (std::regex_search(input, match, dangerousPatternRegex))
        {
            // Print the first invalid character and its position
            Logger::LogError("Invalid character '" + match.str(0) +
                            "' found at position " + std::to_string(match.position(0)) +
                            " in input: " + input);
            Logger::LogError("The following symbols are not allowed for security reasons. \"[;&|><`]\"");
        }
        else
        {
            Logger::LogVerbose("Input validation completed successfully.");
            return true;
        }
    }
    catch (const std::exception &e)
    {
        Logger::LogError(std::string("Exception during input validation: ") + e.what());
    }
    catch (...)
    {
        Logger::LogError("Unknown error occurred during input validation.");
    }
    return false;
}

