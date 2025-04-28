#pragma once

#include <iostream>
#include <string>

class Logger
{
private:
    static bool verboseMode;

public:
    enum class LogLevel
    {
        VERBOSE,
        INFO,
        WARNING,
        ERROR
    };

    static void SetVerbose(bool verbose);
    static void Log(LogLevel level, const std::string &message);
    static void LogVerbose(const std::string &message);
    static void LogInfo(const std::string &message);
    static void LogWarning(const std::string &message);
    static void LogError(const std::string &message);
};
