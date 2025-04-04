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
    xmakefile.xmakeFileName = jsonDoc["name"].as<std::string>();
    xmakefile.xmakeFileVersion = jsonDoc["version"].as<std::string>();
    xmakefile.xmakeFileDescription = jsonDoc["description"].as<std::string>();
    xmakefile.xmakeFileAuthor = jsonDoc["author"].as<std::string>();
    xmakefile.xmakeFileLicense = jsonDoc["license"].as<std::string>();

    // Extract the configurations
    JsonArray configs = jsonDoc["configurations"].as<JsonArray>();

    for (JsonVariant config : configs)
    {
        currentConfig.Name = config["name"].as<std::string>();
        currentConfig.BuildType = config["build_type"].as<std::string>();
        currentConfig.BuildDir = config["build_dir"].as<std::string>();
        currentConfig.OutputFilename = config["output_filename"].as<std::string>();
        currentConfig.CompilerPath = config["compiler_path"].as<std::string>();
        currentConfig.Compiler = config["compiler"].as<std::string>();
        currentConfig.CompilerFlags = config["compiler_flags"].as<std::string>();
        currentConfig.LinkerFlags = config["linker_flags"].as<std::string>();

        // Extract include paths
        JsonArray includePaths = config["include_paths"].as<JsonArray>();
        for (JsonVariant path : includePaths)
        {
            currentConfig.IncludePaths.push_back(path.as<std::string>());
        }

        // Extract library paths
        JsonArray libraryPaths = config["library_paths"].as<JsonArray>();
        for (JsonVariant path : libraryPaths)
        {
            currentConfig.LibraryPaths.push_back(path.as<std::string>());
        }

        // Extract libraries
        JsonArray libraries = config["libraries"].as<JsonArray>();
        for (JsonVariant lib : libraries)
        {
            currentConfig.Libraries.push_back(lib.as<std::string>());
        }

        // Extract source paths
        JsonArray sourcePaths = config["source_paths"].as<JsonArray>();
        for (JsonVariant path : sourcePaths)
        {
            currentConfig.SourcePaths.push_back(path.as<std::string>());
        }

        xmakefile.configs.push_back(currentConfig); // Add the configuration to the list
    }

    file.close();
    return true;
}
