#pragma once

//**************************************************************
// Includes
//**************************************************************

#include <ArduinoJson.h>
#include <string>
#include <vector>

//**************************************************************
// Classes
//**************************************************************

class XMakefileConfig
{
public:
    std::string Name;
    std::string BuildType;
    std::string BuildDir;
    std::string OutputFilename;
    std::string CompilerPath;
    std::string Compiler;
    std::string CCompilerFlags;
    std::string CXXCompilerFlags;

    std::string Linker;
    std::string LinkerFlags;

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

    void FromJSON(const JsonVariant &doc)
    {
        Name = doc["name"].as<std::string>();
        BuildType = doc["build_type"].as<std::string>();
        BuildDir = doc["build_dir"].as<std::string>();
        OutputFilename = doc["output_filename"].as<std::string>();
        CompilerPath = doc["compiler_path"].as<std::string>();
        Compiler = doc["compiler"].as<std::string>();
        CCompilerFlags = doc["c_flags"].as<std::string>();
        CXXCompilerFlags = doc["cxx_flags"].as<std::string>();
        Linker = doc["linker"].as<std::string>();
        LinkerFlags = doc["linker_flags"].as<std::string>();

        // Extract include paths
        JsonArray includePaths = doc["include_paths"].as<JsonArray>();
        for (JsonVariant path : includePaths)
        {
            IncludePaths.push_back(path.as<std::string>());
        }

        // Extract library paths
        JsonArray libraryPaths = doc["library_paths"].as<JsonArray>();
        for (JsonVariant path : libraryPaths)
        {
            LibraryPaths.push_back(path.as<std::string>());
        }

        // Extract libraries
        JsonArray libraries = doc["libraries"].as<JsonArray>();
        for (JsonVariant lib : libraries)
        {
            Libraries.push_back(lib.as<std::string>());
        }

        // Extract pre_build_commands
        JsonArray preBuildCommands = doc["pre_build_commands"].as<JsonArray>();
        for (JsonVariant command : preBuildCommands)
        {
            PreBuildCommands.push_back(command.as<std::string>());
        }
        // Extract post_build_commands
        JsonArray postBuildCommands = doc["post_build_commands"].as<JsonArray>();
        for (JsonVariant command : postBuildCommands)
        {
            PostBuildCommands.push_back(command.as<std::string>());
        }
        //Extract pre_run_commands
        JsonArray preRunCommands = doc["pre_run_commands"].as<JsonArray>();
        for (JsonVariant command : preRunCommands)
        {
            PreRunCommands.push_back(command.as<std::string>());
        }
        // Extract post_run_commands
        JsonArray postRunCommands = doc["post_run_commands"].as<JsonArray>();
        for (JsonVariant command : postRunCommands)
        {
            PostRunCommands.push_back(command.as<std::string>());
        }

        // Extract source paths
        JsonArray sourcePaths = doc["source_paths"].as<JsonArray>();
        for (JsonVariant path : sourcePaths)
        {
            SourcePaths.push_back(path.as<std::string>());
        }

        // Extract excluded source paths
        JsonArray excludePaths = doc["exclude_paths"].as<JsonArray>();
        for (JsonVariant path : excludePaths)
        {
            ExcludePaths.push_back(path.as<std::string>());
        }
        // Extract excluded source files
        JsonArray excludeFiles = doc["exclude_files"].as<JsonArray>();
        for (JsonVariant file : excludeFiles)
        {
            ExcludeFiles.push_back(file.as<std::string>());
        }

        // Extract install commands
        JsonArray installCommands = doc["install_commands"].as<JsonArray>();
        for (JsonVariant command : installCommands)
        {
            InstallCommands.push_back(command.as<std::string>());
        }
        // Extract uninstall commands
        JsonArray uninstallCommands = doc["uninstall_commands"].as<JsonArray>();
        for (JsonVariant command : uninstallCommands)
        {
            UninstallCommands.push_back(command.as<std::string>());
        }
        // Extract clean commands
        JsonArray cleanCommands = doc["clean_commands"].as<JsonArray>();
        for (JsonVariant command : cleanCommands)
        {
            CleanCommands.push_back(command.as<std::string>());
        }
    }
};
