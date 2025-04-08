#pragma once

#include <ArduinoJson.h>
#include <string>
#include <vector>

class XMakefileConfig
{
public:
    std::string Name;
    std::string BuildType;
    std::string BuildDir;
    std::string OutputFilename;
    std::string CompilerPath;
    std::string Compiler;
    std::string CompilerFlags;
    std::string LinkerFlags;

    std::vector<std::string> IncludePaths;
    std::vector<std::string> LibraryPaths;
    std::vector<std::string> Libraries;
    std::vector<std::string> SourcePaths;

    void FromJSON(const JsonVariant &doc)
    {
        Name = doc["name"].as<std::string>();
        BuildType = doc["build_type"].as<std::string>();
        BuildDir = doc["build_dir"].as<std::string>();
        OutputFilename = doc["output_filename"].as<std::string>();
        CompilerPath = doc["compiler_path"].as<std::string>();
        Compiler = doc["compiler"].as<std::string>();
        CompilerFlags = doc["compiler_flags"].as<std::string>();
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

        // Extract source paths
        JsonArray sourcePaths = doc["source_paths"].as<JsonArray>();
        for (JsonVariant path : sourcePaths)
        {
            SourcePaths.push_back(path.as<std::string>());
        }
    }
};

class XMakefile
{
public:
    std::string xmakeFileName;
    std::string xmakeFileVersion;
    std::string xmakeFileDescription;
    std::string xmakeFileAuthor;
    std::string xmakeFileLicense;

    std::vector<XMakefileConfig> configs; // List of configurations

    void FromJSON(const JsonVariant &doc)
    {
        xmakeFileName = doc["name"].as<std::string>();
        xmakeFileVersion = doc["version"].as<std::string>();
        xmakeFileDescription = doc["description"].as<std::string>();
        xmakeFileAuthor = doc["author"].as<std::string>();
        xmakeFileLicense = doc["license"].as<std::string>();

        // Extract configurations
        JsonArray configsArray = doc["configurations"].as<JsonArray>();
        for (JsonVariant config : configsArray)
        {
            XMakefileConfig cfg;
            cfg.FromJSON(config);
            configs.push_back(cfg);
        }
    }
};

class XMakefileParser
{
private:
    std::string xmakefilePath;
    std::string xmakefileContent;
    std::string xmakefileName;
    std::string xmakefileDir;

    JsonDocument jsonDoc; // JSON document to hold the parsed content

    XMakefile xmakefile;           // Parsed xmakefile structure
    XMakefileConfig currentConfig; // Current configuration being parsed

public:
    XMakefileParser();
    bool Parse(const std::string &path);
    std::string GetXMakefileName() const { return xmakefileName; }
    std::string GetXMakefileDir() const { return xmakefileDir; }
    std::string GetXMakefileContent() const { return xmakefileContent; }
};
