#pragma once

//**************************************************************
// Includes
//**************************************************************

#include <ArduinoJson.h>
#include <string>
#include <vector>
#include <filesystem>

//**************************************************************
// Classes
//**************************************************************

class XMakefileConfig
{
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

    static bool IsParentDirectoryTraversalPath(const std::string &path)
    {
        return path[0] == '.' && path[1] == '.' && path[2] == '/';
    }

    static bool IsRelativePath(const std::string &path)
    {
        return path[0] != '/' && path[1] != ':';
    }

    static std::string GetRealPath(const std::string &path, const std::string &basePath)
    {
        if (IsParentDirectoryTraversalPath(path))
        {
            // get current working directory
            std::string currentPath = std::filesystem::current_path().string();

            // check how many directories we need to go up
            std::string relativePath = path;
            size_t pos = 0;
            size_t searchPos = 0;
            int count = 0;
            while ((searchPos = relativePath.find("../", searchPos)) != std::string::npos)
            {
                count++;
                searchPos += 3; // Move past "../"
                pos += 3; // Move past "../"
            }

            // check how many directories we have in the base path
            size_t basePathPos = 0;
            while ((basePathPos = basePath.find("/", basePathPos)) != std::string::npos)
            {
                // subtract from count
                count--;
                basePathPos++;
            }

            if (count < 0)
            {
                count = 0;
            }

            // Remove the "../" from the relative path
            relativePath = relativePath.substr(pos);

            // remove the number of directories from the current path depending on the count
            for (int i = 0; i < count; i++)
            {
                currentPath = currentPath.substr(0, currentPath.find_last_of('/'));
            }
            return currentPath + "/" + relativePath;
        }
        else if (IsRelativePath(path))
        {
            return basePath + path;
        }
        return path;
    }

    void FromJSON(const JsonVariant &doc, const std::string &basePath)
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
        Archiver = doc["archiver"].as<std::string>();
        ArchiverFlags = doc["archiver_flags"].as<std::string>();

        std::string tmpPath = basePath.empty() ? "" : basePath + "/";

        OutputDir = tmpPath;

        if (!BuildDir.empty() && !Name.empty())
            OutputDir += BuildDir + "/" + Name;
        else if (!BuildDir.empty())
            OutputDir += BuildDir;
        else if (!Name.empty())
            OutputDir += Name;
        else
            OutputDir += "";

        // Extract defines
        JsonArray defines = doc["defines"].as<JsonArray>();
        for (JsonVariant define : defines)
        {
            Defines.push_back(define.as<std::string>());
        }

        // Extract include paths
        JsonArray includePaths = doc["include_paths"].as<JsonArray>();
        for (JsonVariant path : includePaths)
        {
            IncludePaths.push_back(GetRealPath(path.as<std::string>(), tmpPath));
        }

        // Extract library paths
        JsonArray libraryPaths = doc["library_paths"].as<JsonArray>();
        for (JsonVariant path : libraryPaths)
        {
            LibraryPaths.push_back(GetRealPath(path.as<std::string>(), tmpPath));
        }

        // Extract libraries
        JsonArray libraries = doc["libraries"].as<JsonArray>();
        for (JsonVariant lib : libraries)
        {
            Libraries.push_back(GetRealPath(lib.as<std::string>(), tmpPath));
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
        // Extract pre_run_commands
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
            SourcePaths.push_back(GetRealPath(path.as<std::string>(), tmpPath));
        }

        // Extract excluded source paths
        JsonArray excludePaths = doc["exclude_paths"].as<JsonArray>();
        for (JsonVariant path : excludePaths)
        {
            ExcludePaths.push_back(GetRealPath(path.as<std::string>(), tmpPath));
        }
        // Extract excluded source files
        JsonArray excludeFiles = doc["exclude_files"].as<JsonArray>();
        for (JsonVariant file : excludeFiles)
        {
            ExcludeFiles.push_back(GetRealPath(file.as<std::string>(), tmpPath));
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
