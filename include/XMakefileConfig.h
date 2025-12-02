#pragma once

//**************************************************************
// Includes
//**************************************************************

#include <ArduinoJson.h>
#include <string>
#include <vector>
#include <map>

//**************************************************************
// Classes
//**************************************************************

class XMakefileConfig
{
private:
    std::map<std::string, std::string> envVars;

    static std::string ResolveEnvironmentVariables(const std::string &path, const std::map<std::string, std::string> &envVars);
    static std::string ResolveCommand(const std::string &command, const std::string &basePath);
    static std::string Resolve(const std::string &command, const std::string &basePath, const std::map<std::string, std::string> &envVars);

public:
    std::string Name;
    std::string BuildType;
    std::string BuildDir;
    std::string OutputDir;
    std::string OutputFilename;
    std::string CompilerPath;
    std::string Compiler;
    std::string CCompilerFlags;
    std::string CXXCompilerFlags;

    std::string Linker;
    std::string LinkerFlags;

    std::string Archiver;
    std::string ArchiverFlags;

    std::vector<std::string> Defines;
    std::vector<std::string> IncludePaths;
    std::vector<std::string> LibraryPaths;
    std::vector<std::string> Libraries;
    std::vector<std::string> SourcePaths;
    std::vector<std::string> ExcludePaths;
    std::vector<std::string> ExcludeFiles;
    std::vector<std::string> PreBuildCommands;
    std::vector<std::string> PostBuildCommands;
    std::vector<std::string> PreRunCommands;
    std::vector<std::string> PostRunCommands;
    std::vector<std::string> InstallCommands;
    std::vector<std::string> UninstallCommands;
    std::vector<std::string> CleanCommands;

    XMakefileConfig();

    static bool IsParentDirectoryTraversalPath(const std::string &path);
    static bool IsRelativePath(const std::string &path);
    static std::string ResolvePath(const std::string &path, const std::string &basePath);
    static std::string ResolveLibraryPath(const std::string &path, const std::string &basePath);

    void PrintEnvironmentVariables();

    void FromJSON(const JsonVariant &doc, const std::string &basePath);
};
