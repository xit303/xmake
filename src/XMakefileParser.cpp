//**************************************************************
// Includes
//**************************************************************

#include "XMakefileParser.h"
#include "Logger.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>

//**************************************************************
// Public functions
//**************************************************************

XMakefileParser::XMakefileParser()
{
}

bool XMakefileParser::Parse(const std::string &path)
{
    try
    {
        xmakefilePath = path;
        xmakefileName = path.substr(path.find_last_of("/\\") + 1);
        xmakefileDir = path.substr(0, path.find_last_of("/\\"));
        xmakefileContent = "";

        std::ifstream file(xmakefilePath);
        if (!file.is_open())
        {
            Logger::LogError("Could not open xmakefile: " + xmakefilePath);
            return false;
        }

        std::string line;
        while (std::getline(file, line))
        {
            xmakefileContent += line;
        }

        deserializeJson(jsonDoc, xmakefileContent);
        if (jsonDoc.isNull())
        {
            Logger::LogError("Failed to parse xmakefile content.");
            return false;
        }

        xmakefile.FromJSON(jsonDoc);
        file.close();

        if (xmakefile.configs.empty())
        {
            Logger::LogError("No configurations found in the xmakefile.");
            return false;
        }
        else
        {
            currentConfig = xmakefile.configs[0];
        }

        Logger::LogVerbose("xmakefile parsed successfully.");

#ifdef DEBUG_MORE
        // print the parsed data for debugging
        std::cout << std::endl
                  << std::endl;
        std::cout << "Parsed xmakefile:" << std::endl;
        std::cout << "Name: " << xmakefile.xmakeFileName << std::endl;
        std::cout << "Version: " << xmakefile.xmakeFileVersion << std::endl;
        std::cout << "Description: " << xmakefile.xmakeFileDescription << std::endl;
        std::cout << "Author: " << xmakefile.xmakeFileAuthor << std::endl;
        std::cout << "License: " << xmakefile.xmakeFileLicense << std::endl;
        std::cout << "Configurations:" << std::endl;

        for (const auto &config : xmakefile.configs)
        {
            std::cout << "  Name: " << config.Name << std::endl;
            std::cout << "  Build Type: " << config.BuildType << std::endl;
            std::cout << "  Build Dir: " << config.BuildDir << std::endl;
            std::cout << "  Output Filename: " << config.OutputFilename << std::endl;
            std::cout << "  Compiler Path: " << config.CompilerPath << std::endl;
            std::cout << "  Compiler: " << config.Compiler << std::endl;
            std::cout << "  C Compiler Flags: " << config.CCompilerFlags << std::endl;
            std::cout << "  C++ Compiler Flags: " << config.CXXCompilerFlags << std::endl;
            std::cout << "  Linker: " << config.Linker << std::endl;
            std::cout << "  Linker Flags: " << config.LinkerFlags << std::endl;
            std::cout << "  Include Paths: ";
            for (const auto &path : config.IncludePaths)
                std::cout << path << " ";
            std::cout << std::endl;

            // Print library paths
            std::cout << "  Library Paths: ";
            for (const auto &path : config.LibraryPaths)
                std::cout << path << " ";
            std::cout << std::endl;

            // Print libraries
            std::cout << "  Libraries: ";
            for (const auto &lib : config.Libraries)
                std::cout << lib << " ";
            std::cout << std::endl;

            // Print source paths
            std::cout << "  Source Paths: ";
            for (const auto &path : config.SourcePaths)
                std::cout << path << " ";
            std::cout << std::endl;
        }
        std::cout << "Parsed successfully." << std::endl;
#endif

        return true;
    }
    catch (const std::exception &e)
    {
        Logger::LogError(std::string("Exception during parsing: ") + e.what());
        return false;
    }
    catch (...)
    {
        Logger::LogError("Unknown error occurred during parsing.");
        return false;
    }
}

bool XMakefileParser::SetConfig(const std::string &configName)
{
    // Find the configuration by name
    auto it = std::find_if(xmakefile.configs.begin(), xmakefile.configs.end(),
                           [&configName](const XMakefileConfig &config) { return config.Name == configName; });

    if (it != xmakefile.configs.end())
    {
        currentConfig = *it;
        Logger::LogVerbose("Configuration set to: " + configName);
        return true;
    }
    else
    {
        Logger::LogError("Configuration not found: " + configName);
        return false;
    }
}

void XMakefileParser::CreateBuildList()
{
    // create the build list from the current config
    buildStructureIndex = 0; // Reset the index for build strings
    buildStructures.clear(); // Clear previous build strings
    linkString.clear();      // Clear previous linker string

    UpdateFileLists();

    std::vector<std::string> objectFiles;

    // Create the build string for every source file
    for (const auto &sourceFile : sourceFiles)
    {
        std::string buildString = (currentConfig.CompilerPath.empty() ? "" : currentConfig.CompilerPath + "/") + currentConfig.Compiler + " ";

        // check file extension to add specific compiler flags
        std::string ext = sourceFile.substr(sourceFile.find_last_of("."));
        if (ext == ".c")
        {
            buildString += currentConfig.CCompilerFlags;
        }
        else if (ext == ".cpp" || ext == ".cc" || ext == ".cxx" || ext == ".m" || ext == ".mm")
        {
            buildString += currentConfig.CXXCompilerFlags;
        }
        else
        {
            Logger::LogWarning("Unknown file extension for file: " + sourceFile);
            continue; // Skip unknown file types
        }

        // Add include paths
        for (const auto &includePath : currentConfig.IncludePaths)
        {
            buildString += " -I" + includePath;
        }

        // Add defines
        for (const auto &define : currentConfig.Defines)
        {
            if (define.starts_with("-D"))
            {
                buildString += " " + define;
            }
            else
            {
                buildString += " -D" + define;
            }
        }

        // Add source file
        buildString += " " + sourceFile;

        std::string objectFile;

        // replace path if object file with build dir
        if (!currentConfig.OutputDir.empty())
        {
            std::filesystem::path sourceFilePath = sourceFile;
            std::filesystem::path objectFilePath;

            // check if source path is relative or absolute
            if (sourceFile[0] != '/' && sourceFile[1] != ':')
            {
                // relative path
                objectFilePath = currentConfig.OutputDir / sourceFilePath;
            }
            else
            {
                // absolute path
                objectFilePath = currentConfig.OutputDir / sourceFilePath.filename();
            }

            std::string substring = objectFilePath.string().substr(0, objectFilePath.string().find_last_of('.'));
            objectFile = substring + ".o";
        }
        else
        {
            // No build dir specified, use the same directory as the source file
            objectFile = sourceFile.substr(0, sourceFile.find_last_of('.')) + ".o";
        }

        // Add object file to the list
        objectFiles.push_back(objectFile);

        // Add output filename
        buildString += " -c -o " + objectFile;

        BuildStruct buildStruct;
        buildStruct.sourceFile = sourceFile;
        buildStruct.buildString = buildString;
        buildStruct.objectFile = objectFile;

        // Store the build string
        buildStructures.push_back(buildStruct);
    }

    // Create the linker string based on the build type
    if (currentConfig.BuildType == "Executable")
    {
        linkString = (currentConfig.CompilerPath.empty() ? "" : currentConfig.CompilerPath + "/") + currentConfig.Linker;

        // Add object files to the linker string
        for (const auto &objectFile : objectFiles)
        {
            linkString += " " + objectFile;
        }

        // Add output filename
        linkString += " -o " + currentConfig.OutputDir + "/" + currentConfig.OutputFilename;
    }
    else if (currentConfig.BuildType == "StaticLibrary")
    {
        linkString = (currentConfig.CompilerPath.empty() ? "" : currentConfig.CompilerPath + "/") + currentConfig.Archiver;

        // Add archiver flags
        linkString += " " + currentConfig.ArchiverFlags;

        // Add object files to the static library
        for (const auto &objectFile : objectFiles)
        {
            linkString += " " + objectFile;
        }
    }
    else if (currentConfig.BuildType == "SharedLibrary")
    {
        linkString = (currentConfig.CompilerPath.empty() ? "" : currentConfig.CompilerPath + "/") + currentConfig.Linker;

        // Add shared library flags
        linkString += " -shared";

        // Add object files to the linker string
        for (const auto &objectFile : objectFiles)
        {
            linkString += " " + objectFile;
        }

        // Add output filename
        linkString += " -o " + currentConfig.OutputDir + "/" + currentConfig.OutputFilename;
    }

    // Add library paths
    for (const auto &libraryPath : currentConfig.LibraryPaths)
    {
        linkString += " -L" + libraryPath;
    }

    // Add libraries
    for (const auto &library : currentConfig.Libraries)
    {
        // check if library is absolute or relative
        if (library[0] != '/' && library[1] != ':')
        {
            linkString += " -l" + library;
        }
        else
        {
            linkString += " " + library;
        }
    }

    // Add include paths
    for (const auto &includePath : currentConfig.IncludePaths)
    {
        linkString += " -I" + includePath;
    }

    // Add linker flags
    linkString += " " + currentConfig.LinkerFlags;

#ifdef DEBUG_MORE
    std::cout << "Build strings:" << std::endl;
    for (const auto &buildString : buildStrings)
    {
        std::cout << buildString << std::endl;
    }
    std::cout << "Linker string: " << linkString << std::endl;
#endif
}
void XMakefileParser::ResetBuildIndex()
{
    buildStructureIndex = 0;
}

RebuildScheme XMakefileParser::CheckRebuild()
{
    if (CheckFileModifications(headerFiles, "Header"))
    {
        if (verbose)
            std::cout << "Header files changed, startung full rebuild..." << std::endl;
        return RebuildScheme::Full;
    }
    else if (CheckFileModifications(sourceFiles, "Source"))
    {
        if (verbose)
            std::cout << "Source files changed, rebuilding sources..." << std::endl;
        return RebuildScheme::Sources;
    }
    else if (CheckFileModifications(currentConfig.Libraries, "Library"))
    {
        if (verbose)
            std::cout << "Libraries changed, linking..." << std::endl;
        return RebuildScheme::Link;
    }
    // TODO check if makefile has changed

    return RebuildScheme::None;
}

void XMakefileParser::LoadBuildTimes()
{
    // Load the last build times from a file in the build directory
    std::string buildTimeFile = currentConfig.OutputDir + "/build_times.txt";
    std::ifstream file(buildTimeFile);
    if (!file.is_open())
    {
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        size_t separator = line.find('|');
        if (separator != std::string::npos)
        {
            std::string filename = line.substr(0, separator);
            std::string timestamp = line.substr(separator + 1);
            lastModifiedTimes[filename] = timestamp; // Store filename and timestamp
        }
    }
    file.close();
}
void XMakefileParser::SaveBuildTimes()
{
    for (const std::string &file : sourceFiles)
    {
        if (file.empty())
            continue;

        std::filesystem::path path(file);
        if (!std::filesystem::exists(path))
            continue;

        auto lastWriteTime = std::filesystem::last_write_time(path);
        lastModifiedTimes[file] = FileTimestampToString(lastWriteTime);
    }

    for (const std::string &file : headerFiles)
    {
        if (file.empty())
            continue;

        std::filesystem::path path(file);
        if (!std::filesystem::exists(path))
            continue;

        auto lastWriteTime = std::filesystem::last_write_time(path);
        lastModifiedTimes[file] = FileTimestampToString(lastWriteTime);
    }

    // Save the last build times to a file in the build directory
    std::string buildTimeFile = currentConfig.OutputDir + "/build_times.txt";
    std::ofstream file(buildTimeFile);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open build time file for writing: " << buildTimeFile << std::endl;
        return;
    }
    for (const auto &entry : lastModifiedTimes)
    {
        file << entry.first << "|" << entry.second << std::endl; // Write filename and timestamp
    }
    file.close();

    if (verbose)
        std::cout << "Build times saved to: " << buildTimeFile << std::endl;
}

//**************************************************************
// Private functions
//**************************************************************

void XMakefileParser::UpdateFileLists()
{
    // Find all header files in include paths
    UpdateLists(currentConfig.IncludePaths, {".h", ".hpp"}, headerFiles);

    // Find all source files in source paths
    UpdateLists(currentConfig.SourcePaths, {".cpp", ".c", ".cc", ".cxx", ".m", ".mm"}, sourceFiles);

    // Find all library files in library paths
    UpdateLists(currentConfig.LibraryPaths, {".a", ".so", ".dll"}, libraryFiles);
}
void XMakefileParser::UpdateLists(const std::vector<std::string> &paths, const std::vector<std::string> &extensions, std::vector<std::string> &outputFiles)
{
    outputFiles.clear(); // Clear previous files

    for (const auto &includePath : paths)
    {
        // check if include path is relative or absolute
        if (includePath[0] != '/' && includePath[1] != ':')
        {
            // relative path, convert to absolute
            std::filesystem::path absPath = std::filesystem::current_path() / includePath;
            FindFiles(absPath.string(), extensions, outputFiles);
        }
        else
        {
            // absolute path
            FindFiles(includePath, extensions, outputFiles);
        }
    }
}
void XMakefileParser::FindFiles(const std::string &path, const std::vector<std::string> &extensions, std::vector<std::string> &outputFiles)
{
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
    {
        std::cerr << "Error: Path does not exist or is not a directory: " << path << std::endl;
        return;
    }

    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        bool isExcluded = false;

        for (const auto &excludePath : currentConfig.ExcludePaths)
        {
            // check if exclude path has * at the beginning and the end
            if (excludePath[0] == '*' && excludePath[excludePath.size() - 1] == '*')
            {
                // check if the path contains the exclude path
                if (entry.path().string().find(excludePath.substr(1, excludePath.size() - 2)) != std::string::npos)
                {
                    isExcluded = true;
                    break;
                }
            }
            else if (excludePath[0] == '*')
            {
                // check if the path ends with the exclude path
                if (entry.path().string().ends_with(excludePath.substr(1)))
                {
                    isExcluded = true;
                    break;
                }
            }
            else if (excludePath[excludePath.size() - 1] == '*')
            {
                // check if the path starts with the exclude path
                if (entry.path().string().starts_with(excludePath.substr(0, excludePath.size() - 1)))
                {
                    isExcluded = true;
                    break;
                }
            }
            else if (entry.path().string().find(excludePath) != std::string::npos)
            {
                isExcluded = true;
                break;
            }
        }

        if (isExcluded)
            continue;

        if (entry.is_directory())
        {
            FindFiles(entry.path().string(), extensions, outputFiles);
        }
        else if (entry.is_regular_file())
        {
            std::string ext = entry.path().extension().string();
            if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end())
            {
                for (const auto &excludeFile : currentConfig.ExcludeFiles)
                {
                    if (entry.path().filename() == excludeFile)
                    {
                        isExcluded = true;
                        break;
                    }
                }

                if (isExcluded)
                    continue;

                outputFiles.push_back(entry.path().string());
            }
        }
    }
}

bool XMakefileParser::CheckFileModifications(const std::vector<std::string> &files, const std::string &fileType)
{
    if (lastModifiedTimes.empty())
    {
        // We need to rebuild if no previous build times are available
        return true;
    }

    // Check if files exist and their last modified times
    for (const auto &file : files)
    {
        std::filesystem::path filePath(file);
        if (!std::filesystem::exists(filePath))
        {
            std::cerr << "Error: " << fileType << " file does not exist: " << file << std::endl;
            continue;
        }

        // Check last modified time
        auto lastWriteTime = std::filesystem::last_write_time(filePath);

        const std::string &timestamp = lastModifiedTimes[filePath.string()];
        if (timestamp.empty())
        {
            continue;
        }

        auto lastBuildTime = StringToFileTimestamp(lastModifiedTimes[filePath.string()]);

        // Check if the last write time is different from the last build time
        if (lastWriteTime > lastBuildTime)
        {
            if (verbose)
                std::cout << fileType << " file changed: " << file << std::endl;
            return true; // File has changed
        }
    }
    return false;
}

std::string XMakefileParser::FileTimestampToString(const std::filesystem::file_time_type &fileTime)
{
    auto sctp = std::chrono::time_point_cast<std::chrono::seconds>(fileTime);
    auto epoch = sctp.time_since_epoch();
    return std::to_string(epoch.count()); // Convert to Unix timestamp string
}
std::filesystem::file_time_type XMakefileParser::StringToFileTimestamp(const std::string &timestamp)
{
    auto seconds = std::stoll(timestamp);                                  // Convert string to long long
    return std::filesystem::file_time_type(std::chrono::seconds(seconds)); // Convert to file_time_type
}
