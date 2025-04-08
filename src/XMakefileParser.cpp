#include "XMakefileParser.h"
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

#ifdef DEBUG
    // print the parsed data for debugging
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
        std::cout << "  Compiler Flags: " << config.CompilerFlags << std::endl;
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
