#include "CmdLineParser.h"

#include <iostream>

#include <algorithm>

CmdLineParser::CmdLineParser()
{
}

bool CmdLineParser::Parse(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
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
    for (const auto &option : registeredOptions)
    {
        std::cout << option.first << ": " << option.second << std::endl;
    }
}