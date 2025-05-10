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
                auto it = registeredOptions.find(arg);

                // Check if the argument is a registered option
                if (it == registeredOptions.end())
                {
                    it = registeredOptions.find(arg + " <arg>");
                    // Check if the argument is a registered option with an argument
                    if (it == registeredOptions.end())
                    {
                        std::cerr << "Unknown option: " << arg << std::endl;
                        PrintHelp();
                        return false;
                    }
                }

                // Check if the option requires an argument
                if (it->first.find("<arg>") != std::string::npos)
                {
                    if (i + 1 < argc && argv[i + 1][0] != '-')
                    {
                        std::string value;
                        while (i + 1 < argc && argv[i + 1][0] != '-')
                        {
                            if (!value.empty())
                            {
                                value += " ";
                            }

                            std::string nextArg = argv[i + 1];
                            // Check if the next argument is a registered option
                            if (registeredOptions.find(nextArg) != registeredOptions.end())
                            {
                                break;
                            }
                            // Check if the next argument is a registered option with an argument
                            if (registeredOptions.find(nextArg + " <arg>") != registeredOptions.end())
                            {
                                break;
                            }

                            // Otherwise, add it to the value
                            value += argv[++i];
                        }

                        // Store the value in the options map and trim whitespace
                        value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
                        if (value.empty())
                        {
                            std::cerr << "Option " << arg << " requires a non-empty argument." << std::endl;
                            PrintHelp();
                            return false;
                        }
                        options[arg] = value;
                    }
                    else
                    {
                        std::cerr << "Option " << arg << " requires an argument." << std::endl;
                        PrintHelp();
                        return false;
                    }
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

        Logger::LogVerbose("Command-line arguments parsed successfully.");
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

void CmdLineParser::SetUsage(const std::string &usage)
{
    usageString = usage;
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

std::vector<std::string> CmdLineParser::GetOptionValues(const std::string &option) const
{
    std::vector<std::string> values;
    auto it = options.find(option);
    if (it != options.end())
    {
        std::string value = it->second;
        size_t pos = 0;
        while ((pos = value.find(' ')) != std::string::npos)
        {
            values.push_back(value.substr(0, pos));
            value.erase(0, pos + 1);
        }
        values.push_back(value);
    }
    return values;
}

void CmdLineParser::RegisterOption(const std::string &option, const std::string &description, bool hasArgs)
{
    if (hasArgs)
    {
        registeredOptions[option + " <arg>"] = description;
    }
    else
    {
        registeredOptions[option] = description;
    }
}

std::string CmdLineParser::Find(const std::string &option) const
{
    auto it = std::find_if(options.begin(), options.end(),
                           [&option](const auto &pair)
                           {
                               // check if first contains the option string within
                               return pair.first.find(option) != std::string::npos;
                           });
    if (it != options.end())
    {
        return it->first;
    }
    it = std::find_if(options.begin(), options.end(),
                      [&option](const auto &pair)
                      {
                          // check if second contains the option string within
                          return pair.second.find(option) != std::string::npos;
                      });
    if (it != options.end())
    {
        return it->second;
    }
    // If no match is found, return an empty string
    return "";
}

void CmdLineParser::PrintHelp() const
{
    PrintVersion();

    if (!usageString.empty())
    {
        std::cout << usageString << std::endl;
    }
    else
    {
        std::cout << "Usage: " << programName << " [options]" << std::endl;
    }

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