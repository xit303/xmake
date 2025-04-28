#include "Logger.h"

bool Logger::verboseMode = false;

void Logger::SetVerbose(bool verbose)
{
    verboseMode = verbose;
}

void Logger::Log(LogLevel level, const std::string &message)
{
    switch (level)
    {
    case LogLevel::VERBOSE:
        std::cout << message << std::endl;
        break;
    case LogLevel::INFO:
        std::cout << message << std::endl;
        break;
    case LogLevel::WARNING:
        std::cerr << "[WARNING] " << message << std::endl;
        break;
    case LogLevel::ERROR:
        std::cerr << "[ERROR] " << message << std::endl;
        break;
    }
}

void Logger::LogVerbose(const std::string &message)
{
    if (verboseMode)
        Log(LogLevel::VERBOSE, message);
}

void Logger::LogInfo(const std::string &message)
{
    Log(LogLevel::INFO, message);
}
void Logger::LogWarning(const std::string &message)
{
    Log(LogLevel::WARNING, message);
}
void Logger::LogError(const std::string &message)
{
    Log(LogLevel::ERROR, message);
}
