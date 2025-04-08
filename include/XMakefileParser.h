#pragma once

#include "XMakefile.h"
#include <atomic>

struct BuildStruct
{
    std::string buildString;
    std::string objectFile;

    bool empty() const
    {
        return buildString.empty() && objectFile.empty();
    }
};

class XMakefileParser
{
private:
    std::string xmakefilePath;
    std::string xmakefileContent;
    std::string xmakefileName;
    std::string xmakefileDir;

    std::atomic<size_t> buildStructureIndex = 0; // Atomic index for build strings to ensure thread safety
    std::vector<std::string> sourceFiles; // List of source files to be compiled
    std::vector<BuildStruct> buildStructures;
    std::string linkString;

    JsonDocument jsonDoc; // JSON document to hold the parsed content

    XMakefile xmakefile;           // Parsed xmakefile structure
    XMakefileConfig currentConfig; // Current configuration being parsed

    void FindSources(const std::string &path, const std::vector<std::string> &extensions);

public:
    XMakefileParser();

    bool Parse(const std::string &path);

    std::string GetXMakefileName() const { return xmakefileName; }
    std::string GetXMakefileDir() const { return xmakefileDir; }
    std::string GetXMakefileContent() const { return xmakefileContent; }
    std::string GetOutputFilename() const { return currentConfig.OutputFilename; }
    XMakefileConfig GetCurrentConfig() const { return currentConfig; }

    void CreateBuildList();
    void ResetBuildIndex() { buildStructureIndex = 0; }
    
    const std::vector<BuildStruct> &GetBuildStructures() { return buildStructures; }
    const std::string &GetLinkerString();
};
