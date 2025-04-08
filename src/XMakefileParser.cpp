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

#ifdef DEBUG
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
    buildStringIndex = 0; // Reset the index for build strings
    buildStrings.clear(); // Clear previous build strings
    linkString.clear();   // Clear previous linker string

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

    // Create the build string for every source file

    for (const auto &sourceFile : sourceFiles)
    {
        std::string buildString = currentConfig.CompilerPath + "/" + currentConfig.Compiler + " ";

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

        // create object file name
        std::string objectFile = sourceFile.substr(0, sourceFile.find_last_of('.')) + ".o";

        // Add output filename
        buildString += " -o " + objectFile;

        // Store the build string
        buildStrings.push_back(buildString);
    }

    // Create the linker string
    linkString = currentConfig.CompilerPath + "/" + currentConfig.Linker + " " + currentConfig.LinkerFlags;

#ifdef DEBUG
    std::cout << "Build strings:" << std::endl;
    for (const auto &buildString : buildStrings)
    {
        std::cout << buildString << std::endl;
    }
    std::cout << "Linker string: " << linkString << std::endl;
#endif
}

const std::string &XMakefileParser::GetNextBuildString()
{
    static const std::string empty;

    if (buildStringIndex >= buildStrings.size())
    {
        // reached the end of the build strings
        return empty;
    }

    return buildStrings[buildStringIndex++];
}

const std::string &XMakefileParser::GetLinkerString()
{
    return linkString;
}

void XMakefileParser::FindSources(const std::string &path, const std::vector<std::string> &extensions)
{
    // Check if the path is a directory and if it exists
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
    {
        std::cerr << "Error: Path does not exist or is not a directory: " << path << std::endl;
        return;
    }

    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        if (entry.is_directory())
        {
            FindSources(entry.path().string(), extensions);
        }
        else if (entry.is_regular_file())
        {
            std::string ext = entry.path().extension().string();
            if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end())
            {
                sourceFiles.push_back(entry.path().string());
            }
        }
    }
}