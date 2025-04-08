#include "CmdLineParser.h"
#include <iostream>
#include <algorithm>

CmdLineParser::CmdLineParser(const std::string &programName, const std::string &programVersion)
    : programName(programName), programVersion(programVersion)
{
}

bool CmdLineParser::Parse(int argc, char *argv[])
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
    std::cout << std::endl << programName
              << " version " << programVersion << std::endl
              << std::endl;
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