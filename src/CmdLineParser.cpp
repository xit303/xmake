//**************************************************************
// Includes
//**************************************************************

#include "CmdLineParser.h"
#include "Logger.h"
#include <iostream>
#include <algorithm>

//**************************************************************
// Public functions
//**************************************************************

CmdLineParser::CmdLineParser(const std::string &programName, const std::string &description, const std::string &programVersion)
    : programName(programName), description(description), programVersion(programVersion)
{
}

bool CmdLineParser::Parse(int argc, char *argv[])
{
    try
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg.empty())
            {
                continue;
            }

            if (arg[0] == '-')
            {
                if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    options[arg] = argv[++i];
                }
                else
                {
                    options[arg] = "";
                }
            }
            else
            {
                options[arg] = argv[i];
            }
        }

        // check if the options are in the registered options
        for (const auto &option : options)
        {
            if (registeredOptions.find(option.first) == registeredOptions.end())
            {
                std::cerr << "Unknown option: " << option.first << std::endl;
                PrintHelp();
                return false;
            }
        }

        Logger::LogInfo("Command-line arguments parsed successfully.");
    }
    catch (const std::exception &e)
    {
        Logger::LogError(std::string("Exception during command-line parsing: ") + e.what());
    }
    catch (...)
    {
        Logger::LogError("Unknown error occurred during command-line parsing.");
    }

    return true;
}

bool CmdLineParser::IsOptionSet(const std::string &option) const
{
    return options.find(option) != options.end();
}

std::string CmdLineParser::GetOptionValue(const std::string &option) const
{
    auto it = options.find(option);
    if (it != options.end())
    {
        return it->second;
    }
    return "";
}

std::string CmdLineParser::GetOptionValue(const std::string &option, const std::string &defaultValue) const
{
    auto it = options.find(option);
    if (it != options.end())
    {
        return it->second;
    }
    return defaultValue;
}

std::string CmdLineParser::GetOptionValue(const std::string &option, int defaultValue) const
{
    auto it = options.find(option);
    if (it != options.end())
    {
        return it->second;
    }
    return std::to_string(defaultValue);
}

void CmdLineParser::RegisterOption(const std::string &option, const std::string &description)
{
    registeredOptions[option] = description;
}

void CmdLineParser::PrintHelp() const
{
    PrintVersion();

    std::cout << "Usage: xmake [options]" << std::endl
              << std::endl;
    std::cout << "Options:" << std::endl;

    for (const auto &option : registeredOptions)
    {
        // Print the option and its description with same size
        std::cout << "  " << option.first;
        std::cout << std::string(20 - option.first.size(), ' ') << option.second << std::endl;
    }
}

void CmdLineParser::PrintVersion() const
{
    std::cout << programName << " - " << description << " - " << " version " << programVersion << std::endl;
}