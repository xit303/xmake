#include "XMakefileParser.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

XMakefileParser::XMakefileParser()
{
}

bool XMakefileParser::Parse(const std::string &path)
{
    // Initialize the parser with the xmakefile path
    xmakefilePath = path;
    xmakefileName = path.substr(path.find_last_of("/\\") + 1);
    xmakefileDir = path.substr(0, path.find_last_of("/\\"));
    xmakefileContent = ""; // Initialize with empty content

    // Open the xmakefile and read its content
    std::ifstream file(xmakefilePath);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open xmakefile: " << xmakefilePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        xmakefileContent += line;
    }

    deserializeJson(jsonDoc, xmakefileContent); // Parse the JSON content
    if (jsonDoc.isNull())
    {
        std::cerr << "Error: Failed to parse xmakefile content." << std::endl;
        return false;
    }

    // Extract the xmakefile metadata
    xmakefile.FromJSON(jsonDoc);

    file.close();

    if (xmakefile.configs.size() > 0)
        currentConfig = xmakefile.configs[0]; // Set the first config as the current one

    LoadBuildTimes();

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
            std::cerr << "Warning: Unknown file extension for file: " << sourceFile << std::endl;
            continue; // Skip unknown file types
        }

        // Add include paths
        for (const auto &includePath : currentConfig.IncludePaths)
        {
            buildString += " -I" + includePath;
        }

        // Add source file
        buildString += " " + sourceFile;

        std::string objectFile;

        // replace path if object file with build dir
        if (!currentConfig.BuildDir.empty())
        {
            std::filesystem::path buildDirPath = currentConfig.BuildDir + "/" + currentConfig.BuildType;
            std::filesystem::path sourceFilePath = sourceFile;
            std::filesystem::path objectFilePath = buildDirPath / sourceFilePath.filename();
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

    // Create the linker string
    linkString = (currentConfig.CompilerPath.empty() ? "" : currentConfig.CompilerPath + "/") + currentConfig.Linker;

    // Add object files to the linker string
    for (const auto &objectFile : objectFiles)
    {
        linkString += " " + objectFile;
    }

    // Add output filename
    if (!currentConfig.BuildDir.empty() && !currentConfig.BuildType.empty())
        linkString += " -o " + currentConfig.BuildDir + "/" + currentConfig.BuildType + "/" + currentConfig.OutputFilename;
    else if (!currentConfig.BuildDir.empty())
        linkString += " -o " + currentConfig.BuildDir + "/" + currentConfig.OutputFilename;
    else if (!currentConfig.BuildType.empty())
        linkString += " -o " + currentConfig.BuildType + "/" + currentConfig.OutputFilename;
    else
        linkString += " -o " + currentConfig.OutputFilename;

    // Add library paths
    for (const auto &libraryPath : currentConfig.LibraryPaths)
    {
        linkString += " -L" + libraryPath;
    }
    // Add libraries
    for (const auto &library : currentConfig.Libraries)
    {
        linkString += " -l" + library;
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

const std::string &XMakefileParser::GetLinkerString()
{
    return linkString;
}

RebuildScheme XMakefileParser::CheckRebuild()
{
    if (CheckFileModifications(headerFiles, "Header"))
    {
        std::cout << "Header files changed, rebuilding..." << std::endl;
        return RebuildScheme::Full;
    }
    else if (CheckFileModifications(sourceFiles, "Source"))
    {
        std::cout << "Source files changed, rebuilding..." << std::endl;
        return RebuildScheme::Sources;
    }
    else if (CheckFileModifications(currentConfig.Libraries, "Library"))
    {
        std::cout << "Libraries changed, rebuilding..." << std::endl;
        return RebuildScheme::Link;
    }
    return RebuildScheme::None;
}

void XMakefileParser::LoadBuildTimes()
{
    // Load the last build times from a file in the build directory
    std::string buildTimeFile = currentConfig.BuildDir + "/" + currentConfig.BuildType + "/build_times.txt";
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
    std::string buildTimeFile = currentConfig.BuildDir + "/" + currentConfig.BuildType + "/build_times.txt";
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
    std::cout << "Build times saved to: " << buildTimeFile << std::endl;
}

void XMakefileParser::UpdateFileLists()
{
    headerFiles.clear();
    sourceFiles.clear();
    libraryFiles.clear();

    // Find all header files in include paths
    for (const auto &includePath : currentConfig.IncludePaths)
    {
        // check if include path is relative or absolute
        if (includePath[0] != '/' && includePath[1] != ':')
        {
            // relative path, convert to absolute
            std::filesystem::path absPath = std::filesystem::current_path() / includePath;
            FindHeaders(absPath.string(), {".h", ".hpp"});
        }
        else
        {
            // absolute path
            FindHeaders(includePath, {".h", ".hpp"});
        }
    }

    const std::vector<std::string> extensions = {".cpp", ".c", ".cc", ".cxx", ".m", ".mm"};

    // Find all files in source paths
    for (const auto &sourcePath : currentConfig.SourcePaths)
    {
        // check if source path is relative or absolute
        if (sourcePath[0] != '/' && sourcePath[1] != ':')
        {
            // relative path, convert to absolute
            std::filesystem::path absPath = std::filesystem::current_path() / sourcePath;
            FindSources(absPath.string(), extensions);
        }
        else
        {
            // absolute path
            FindSources(sourcePath, extensions);
        }
    }

    // TODO find all library files
}

void XMakefileParser::FindFiles(const std::string &path, const std::vector<std::string> &extensions, const std::vector<std::string> &excludePaths, const std::vector<std::string> &excludeFiles, std::vector<std::string> &outputFiles)
{
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
    {
        std::cerr << "Error: Path does not exist or is not a directory: " << path << std::endl;
        return;
    }

    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        bool isExcluded = false;

        for (const auto &excludePath : excludePaths)
        {
            if (entry.path().string().find(excludePath) != std::string::npos)
            {
                isExcluded = true;
                break;
            }
        }

        if (isExcluded)
            continue;

        if (entry.is_directory())
        {
            FindFiles(entry.path().string(), extensions, excludePaths, excludeFiles, outputFiles);
        }
        else if (entry.is_regular_file())
        {
            std::string ext = entry.path().extension().string();
            if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end())
            {
                for (const auto &excludeFile : excludeFiles)
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

void XMakefileParser::FindHeaders(const std::string &path, const std::vector<std::string> &extensions)
{
    FindFiles(path, extensions, currentConfig.ExcludePaths, currentConfig.ExcludeFiles, headerFiles);
}

void XMakefileParser::FindSources(const std::string &path, const std::vector<std::string> &extensions)
{
    FindFiles(path, extensions, currentConfig.ExcludePaths, currentConfig.ExcludeFiles, sourceFiles);
}

void XMakefileParser::FindLibraries(const std::string &path, const std::vector<std::string> &extensions)
{
    FindFiles(path, extensions, currentConfig.ExcludePaths, currentConfig.ExcludeFiles, libraryFiles);
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