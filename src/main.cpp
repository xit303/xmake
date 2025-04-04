#include <iostream>
#include "CmdLineParser.h"
#include "XMakefileParser.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_STRING "v" + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + "." + std::to_string(VERSION_PATCH)

int main(int argc, char **argv)
{
    CmdLineParser parser;
    parser.RegisterOption("-h", "Show help message");
    parser.RegisterOption("--version", "Show version information");
    parser.RegisterOption("-v", "Enable verbose output");
    parser.RegisterOption("--xmakefile", "Path to the xmakefile to use");
    
    if (!parser.Parse(argc, argv))
    {
        return 1;
    }

    if (parser.IsOptionSet("-h"))
    {
        parser.PrintHelp();
        return 0;
    }
    if (parser.IsOptionSet("--version"))
    {
        std::cout << "Version: " << VERSION_STRING << std::endl;
        return 0;
    }
    if (parser.IsOptionSet("-v"))
    {
        std::cout << "Verbose output enabled" << std::endl;
    }
    if (parser.IsOptionSet("--xmakefile"))
    {
        std::string xmakefile = parser.GetOptionValue("--xmakefile");
        std::cout << "Using xmakefile: " << xmakefile << std::endl;
    }
    else
    {
        std::cout << "No xmakefile specified, using default." << std::endl;
    }

    // Initialize the XMakefileParser with the xmakefile path
    std::string xmakefilePath = parser.GetOptionValue("--xmakefile", "xmakefile.json");

    XMakefileParser xmakefileParser;
    if (!xmakefileParser.Parse(xmakefilePath))
    {
        std::cerr << "Error: Failed to parse xmakefile." << std::endl;
        return 1;
    }

    return 0;
}