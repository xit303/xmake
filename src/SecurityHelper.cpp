#include "SecurityHelper.h"
#include <iostream>
#include <regex>

static bool IsValidCommand(const std::string &command)
{
    // Allow alphanumeric characters, underscores, dashes, slashes, spaces, dots, and plus signs
    static const std::regex validCommandRegex("^[a-zA-Z0-9_.+/\\-=: ]+$");
    return std::regex_match(command, validCommandRegex);
}

// Updated system calls to validate commands before execution
bool ExecuteCommand(const std::string &command)
{
    if (!IsValidCommand(command))
    {
        std::cerr << "Error: Invalid command detected: " << command << std::endl;
        return false;
    }

    try
    {
        int result = std::system(command.c_str());
        if (result != 0)
        {
            std::cerr << "Error: Command execution failed with code " << result << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception occurred while executing command: " << e.what() << std::endl;
        return false;
    }

    return true;
}