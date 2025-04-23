//**************************************************************
// Includes
//**************************************************************

#include "SecurityHelper.h"
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

//**************************************************************
// Local functions
//**************************************************************

static bool IsValidCommand(const std::string &command)
{
    // Disallow dangerous characters or patterns commonly used in command injection
    static const std::regex dangerousPatternRegex("[;&|><`]");
    std::smatch match;

    if (std::regex_search(command, match, dangerousPatternRegex))
    {
        // Print the first invalid character and its position
        std::cerr << "Error: Invalid character '" << match.str(0)
                  << "' found at position " << match.position(0) << " in command: " << std::endl;

        std::cerr << command << std::endl;
        std::cerr << "The following symbols are not allowed for security reasons. \"[;&|><`]\"" << std::endl;

        return false; // Command contains potentially dangerous characters
    }

    // Additional checks can be added here if needed
    return true;
}

