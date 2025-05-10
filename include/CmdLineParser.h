#pragma once

//**************************************************************
// Includes
//**************************************************************

#include <map>
#include <string>
#include <vector>

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
    std::string usageString;

public:
    CmdLineParser(const std::string &programName, const std::string &description, const std::string &programVersion);
    bool Parse(int argc, char *argv[]);

    void SetUsage(const std::string &usage);

    bool IsOptionSet(const std::string &option) const;
    std::string GetOptionValue(const std::string &option) const;
    std::string GetOptionValue(const std::string &option, const std::string &defaultValue) const;
    std::string GetOptionValue(const std::string &option, int defaultValue) const;

    std::vector<std::string> GetOptionValues(const std::string &option) const;

    void RegisterOption(const std::string &option, const std::string &description, bool hasArgs = false);
    std::string Find(const std::string &option) const;
    void PrintHelp() const;
    void PrintVersion() const;
};