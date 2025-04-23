#pragma once

//**************************************************************
// Includes
//**************************************************************

#include "XMakefile.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>

//**************************************************************
// Structures
//**************************************************************

struct BuildStruct
{
    std::string buildString;
    std::string objectFile;
    std::string sourceFile;

    bool empty() const
    {
        return buildString.empty() && objectFile.empty();
    }
};

//**************************************************************
// Enums
//**************************************************************

enum RebuildScheme
{
    None,
    Full,
    Sources,
    Link
};

//**************************************************************
// Classes
//**************************************************************

class XMakefileParser
{
private:
    bool verbose = false; // Verbose mode flag

    std::string xmakefilePath;
    std::string xmakefileContent;
    std::string xmakefileName;
    std::string xmakefileDir;

    std::atomic<size_t> buildStructureIndex = 0; // Atomic index for build strings to ensure thread safety
    std::vector<std::string> sourceFiles;        // List of source files to be compiled
    std::vector<std::string> headerFiles;        // List of header files to check date
    std::vector<std::string> libraryFiles;       // Storage for library files
    std::vector<BuildStruct> buildStructures;
    std::string linkString;

    // storage for last modified times
    std::map<std::string, std::string> lastModifiedTimes;

    JsonDocument jsonDoc; // JSON document to hold the parsed content

    XMakefile xmakefile;           // Parsed xmakefile structure
    XMakefileConfig currentConfig; // Current configuration being parsed

    void UpdateFileLists();
    void UpdateLists(const std::vector<std::string> &paths, const std::vector<std::string> &extensions, std::vector<std::string> &outputFiles);
    void FindFiles(const std::string &path, const std::vector<std::string> &extensions, std::vector<std::string> &outputFiles);

    bool CheckFileModifications(const std::vector<std::string> &files, const std::string &fileType);

    std::string FileTimestampToString(const std::filesystem::file_time_type &fileTime);
    std::filesystem::file_time_type StringToFileTimestamp(const std::string &timestamp);

public:
    XMakefileParser();

    void SetVerbose(bool verbose) { this->verbose = verbose; }
    std::string GetXMakefileName() const { return xmakefileName; }
    std::string GetXMakefileDir() const { return xmakefileDir; }
    std::string GetXMakefileContent() const { return xmakefileContent; }
    std::string GetOutputFilename() const { return currentConfig.OutputFilename; }
    XMakefileConfig GetCurrentConfig() const { return currentConfig; }
    const std::string &GetLinkerString() const { return linkString; }
    const std::vector<BuildStruct> &GetBuildStructures() { return buildStructures; }

    bool Parse(const std::string &path);

    void CreateBuildList();
    void ResetBuildIndex();

    RebuildScheme CheckRebuild();

    void LoadBuildTimes();
    void SaveBuildTimes();
};
