//**************************************************************
// Includes
//**************************************************************

#include <XMakefileConfig.h>
#include <filesystem>
#include <iostream>

//**************************************************************
// Classes
//**************************************************************

bool XMakefileConfig::IsParentDirectoryTraversalPath(const std::string &path)
{
    return path[0] == '.' && path[1] == '.' && path[2] == '/';
}

bool XMakefileConfig::IsRelativePath(const std::string &path)
{
    return path[0] != '/' && path[1] != ':';
}

std::string XMakefileConfig::ResolvePath(const std::string &path, const std::string &basePath)
{
    if (IsParentDirectoryTraversalPath(path))
    {
        std::filesystem::path base(basePath);
        std::filesystem::path relativePath = path;

        // Resolve "../" in the relative path
        std::filesystem::path resolvedPath = base / relativePath;
        resolvedPath = std::filesystem::weakly_canonical(resolvedPath);
        return resolvedPath.string();
    }
    else if (IsRelativePath(path) && !path.starts_with(basePath) && !basePath.empty())
    {
        return (std::filesystem::path(basePath) / path).lexically_normal().string();
    }
    return path;
}

std::string XMakefileConfig::ResolveLibraryPath(const std::string &path, const std::string &basePath)
{
    // check if path is just a filename
    // If the path is just a filename (no directory components), return it as-is
    if (path.find('/') == std::string::npos && path.find('\\') == std::string::npos && path.find(':') == std::string::npos)
    {
        return path;
    }

    if (IsParentDirectoryTraversalPath(path))
    {
        std::filesystem::path base(basePath);
        std::filesystem::path relativePath = path;

        // Resolve "../" in the relative path
        std::filesystem::path resolvedPath = base / relativePath;
        resolvedPath = std::filesystem::weakly_canonical(resolvedPath);
        return resolvedPath.string();
    }
    else if (IsRelativePath(path) && !path.starts_with(basePath) && !basePath.empty())
    {
        return (std::filesystem::path(basePath) / path).lexically_normal().string();
    }
    return path;
}

void XMakefileConfig::FromJSON(const JsonVariant &doc, const std::string &basePath)
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

    envVars["name"] = Name;
    envVars["build_dir"] = ResolvePath(BuildDir, tmpPath);
    envVars["output_dir"] = ResolvePath(OutputDir, tmpPath);
    envVars["output_filename"] = ResolvePath(OutputFilename, tmpPath);
    envVars["output_file"] = OutputDir + "/" + OutputFilename;

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
        IncludePaths.push_back(ResolvePath(path.as<std::string>(), tmpPath));
    }

    // Extract library paths
    JsonArray libraryPaths = doc["library_paths"].as<JsonArray>();
    for (JsonVariant path : libraryPaths)
    {
        LibraryPaths.push_back(ResolvePath(path.as<std::string>(), tmpPath));
    }

    // Extract libraries
    JsonArray libraries = doc["libraries"].as<JsonArray>();
    for (JsonVariant lib : libraries)
    {
        Libraries.push_back(ResolveLibraryPath(lib.as<std::string>(), tmpPath));
    }

    // Extract pre_build_commands
    JsonArray preBuildCommands = doc["pre_build_commands"].as<JsonArray>();
    for (JsonVariant command : preBuildCommands)
    {
        PreBuildCommands.push_back(Resolve(command.as<std::string>(), tmpPath, envVars));
    }
    // Extract post_build_commands
    JsonArray postBuildCommands = doc["post_build_commands"].as<JsonArray>();
    for (JsonVariant command : postBuildCommands)
    {
        PostBuildCommands.push_back(Resolve(command.as<std::string>(), tmpPath, envVars));
    }
    // Extract pre_run_commands
    JsonArray preRunCommands = doc["pre_run_commands"].as<JsonArray>();
    for (JsonVariant command : preRunCommands)
    {
        PreRunCommands.push_back(Resolve(command.as<std::string>(), tmpPath, envVars));
    }
    // Extract post_run_commands
    JsonArray postRunCommands = doc["post_run_commands"].as<JsonArray>();
    for (JsonVariant command : postRunCommands)
    {
        PostRunCommands.push_back(Resolve(command.as<std::string>(), tmpPath, envVars));
    }

    // Extract source paths
    JsonArray sourcePaths = doc["source_paths"].as<JsonArray>();
    for (JsonVariant path : sourcePaths)
    {
        SourcePaths.push_back(ResolvePath(path.as<std::string>(), tmpPath));
    }

    // Extract excluded source paths
    JsonArray excludePaths = doc["exclude_paths"].as<JsonArray>();
    for (JsonVariant path : excludePaths)
    {
        ExcludePaths.push_back(ResolvePath(path.as<std::string>(), tmpPath));
    }
    // Extract excluded source files
    JsonArray excludeFiles = doc["exclude_files"].as<JsonArray>();
    for (JsonVariant file : excludeFiles)
    {
        ExcludeFiles.push_back(ResolvePath(file.as<std::string>(), tmpPath));
    }

    // Extract install commands
    JsonArray installCommands = doc["install_commands"].as<JsonArray>();
    for (JsonVariant command : installCommands)
    {
        InstallCommands.push_back(Resolve(command.as<std::string>(), tmpPath, envVars));
    }
    // Extract uninstall commands
    JsonArray uninstallCommands = doc["uninstall_commands"].as<JsonArray>();
    for (JsonVariant command : uninstallCommands)
    {
        UninstallCommands.push_back(Resolve(command.as<std::string>(), tmpPath, envVars));
    }
    // Extract clean commands
    JsonArray cleanCommands = doc["clean_commands"].as<JsonArray>();
    for (JsonVariant command : cleanCommands)
    {
        CleanCommands.push_back(Resolve(command.as<std::string>(), tmpPath, envVars));
    }
}

void XMakefileConfig::PrintEnvironmentVariables()
{
    for (const auto &pair : envVars)
    {
        std::cout << pair.first.c_str() << "=" << pair.second.c_str() << std::endl;
    }
}

std::string XMakefileConfig::ResolveEnvironmentVariables(const std::string &path, const std::map<std::string, std::string> &envVars)
{
    // Replace environment variables in the path
    std::string resolvedPath = path;
    size_t pos = 0;
    while ((pos = resolvedPath.find("${", pos)) != std::string::npos)
    {
        size_t endPos = resolvedPath.find("}", pos);
        if (endPos == std::string::npos)
            break;

        std::string varName = resolvedPath.substr(pos + 2, endPos - pos - 2);

        // Check if the variable is in the envVars list
        auto it = envVars.find(varName);
        if (it == envVars.end())
        {
            // Check if the variable is in the environment variables
            const char *envValue = getenv(varName.c_str());
            if (envValue)
            {
                // Replace the variable with its value
                resolvedPath.replace(pos, endPos - pos + 1, envValue);
                pos += strlen(envValue);
            }
            else
            {
                // If not found, just remove the variable
                resolvedPath.erase(pos, endPos - pos + 1);
                pos = endPos;
            }
        }
        else
        {
            // Replace the variable with its value
            std::string envValue = it->second;
            resolvedPath.replace(pos, endPos - pos + 1, envValue);
            pos += envValue.length();
        }
    }
    return resolvedPath;
}

static std::vector<std::string> Split(std::string str, const std::string &delimiter)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    while ((pos = str.find(delimiter)) != std::string::npos)
    {
        tokens.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }
    tokens.push_back(str);
    return tokens;
}

std::string XMakefileConfig::ResolveCommand(const std::string &command, const std::string &basePath)
{
    std::string resolvedCommand;
    std::vector<std::string> parts = Split(command, " ");
    for (const auto &part : parts)
    {
        // check if the command contains a path
        if (part.find("/") != std::string::npos || part.find("\\") != std::string::npos)
        {
            resolvedCommand += ResolvePath(part, basePath) + " ";
        }
        else
        {
            resolvedCommand += part + " ";
        }
    }

    return resolvedCommand;
}

std::string XMakefileConfig::Resolve(const std::string &command, const std::string &basePath, const std::map<std::string, std::string> &envVars)
{
    std::string resolved = ResolveCommand(command, basePath);
    return ResolveEnvironmentVariables(resolved, envVars);
}