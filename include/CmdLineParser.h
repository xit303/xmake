#pragma once

//**************************************************************
// Includes
//**************************************************************

#include <map>
#include <string>

//**************************************************************
// Classes
//**************************************************************

class CmdLineParser
{
private:
    std::map<std::string, std::string> registeredOptions;
    std::map<std::string, std::string> options;

    std::string programName;
    std::string description;
    std::string programVersion;

public:
    CmdLineParser(const std::string &programName, const std::string &description, const std::string &programVersion);
    bool Parse(int argc, char *argv[]);

    bool IsOptionSet(const std::string &option) const;
    std::string GetOptionValue(const std::string &option) const;
    std::string GetOptionValue(const std::string &option, const std::string &defaultValue) const;
    std::string GetOptionValue(const std::string &option, int defaultValue) const;

    void RegisterOption(const std::string &option, const std::string &description, bool hasArgs = false);
    void PrintHelp() const;
    void PrintVersion() const;
};