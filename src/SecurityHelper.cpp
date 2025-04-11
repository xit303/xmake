#include "SecurityHelper.h"
#include <regex>
#include <iostream>

static bool IsValidCommand(const std::string &command)
{
    // Allow only alphanumeric characters, spaces, dashes, and slashes
    static const std::regex validCommandRegex("^[a-zA-Z0-9_\-./ ]+$");
    return std::regex_match(command, validCommandRegex);
}

// Updated system calls to validate commands before execution
bool ExecuteCommand(const std::string &command)
{
    if (!IsValidCommand(command))
    {
        std::cerr << "Error: Invalid command detected." << std::endl;
        return false;
    }
    int result = system(command.c_str());
    if (result != 0)
    {
        std::cerr << "Error: Command execution failed with code " << result << std::endl;
        return false;
    }
    return true;
}