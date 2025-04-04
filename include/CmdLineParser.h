#pragma once

#include <map>
#include <string>

class CmdLineParser
{
private:
    std::map<std::string, std::string> registeredOptions;
    std::map<std::string, std::string> options;

public:
    CmdLineParser();
    bool Parse(int argc, char *argv[]);

    bool IsOptionSet(const std::string &option) const;
    std::string GetOptionValue(const std::string &option) const;
    std::string GetOptionValue(const std::string &option, const std::string &defaultValue) const;
    std::string GetOptionValue(const std::string &option, int defaultValue) const;

    void RegisterOption(const std::string &option, const std::string &description);
    void PrintHelp() const;
};