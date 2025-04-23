#pragma once

//**************************************************************
// Includes
//**************************************************************

#include "CmdLineParser.h"
#include "XMakefileParser.h"
#include <atomic>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>

//**************************************************************
// Classes
//**************************************************************

class XMake
{
private:
    XMakefileParser parser;
    const CmdLineParser &cmdLineParser;
    bool verbose = false;

    void SaveBuildTimes();

public:
    XMake(const CmdLineParser &cmdLineParser);

    bool Init(const std::string &makefileName);

    bool Build();
    void Clean();
    void Run();
    void Install();
    void Uninstall();
};