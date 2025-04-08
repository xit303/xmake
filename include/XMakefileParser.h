#pragma once

#include "XMakefile.h"

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
