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
