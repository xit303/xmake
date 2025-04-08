#pragma once

#include "XMakefile.h"
#include <atomic>

class XMakefileParser
{
private:
    std::string xmakefilePath;
    std::string xmakefileContent;
    std::string xmakefileName;
    std::string xmakefileDir;

    std::atomic<size_t> buildStringIndex = 0; // Atomic index for build strings to ensure thread safety
    std::vector<std::string> sourceFiles; // List of source files to be compiled
    std::vector<std::string> buildStrings;
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
    XMakefileConfig GetCurrentConfig() const { return currentConfig; }

    void CreateBuildList();
    void ResetBuildIndex() { buildStringIndex = 0; }
    
    const std::string &GetNextBuildString();
    const std::string &GetLinkerString();
};
