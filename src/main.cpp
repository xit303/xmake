#include <iostream>
#include "Version.h"
#include "CmdLineParser.h"
#include "XMakefileParser.h"

int main(int argc, char **argv)
{
    CmdLineParser parser("xmake", VERSION_STRING);
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
    
    std::string xmakefilePath = "xmakefile.json";

    if (parser.IsOptionSet("--xmakefile"))
    {
        xmakefilePath = parser.GetOptionValue("--xmakefile", xmakefilePath);
        std::cout << "Using xmakefile: " << xmakefilePath << std::endl;
    }
    else
    {
        std::cout << "No xmakefile specified, using default." << std::endl;
    }

    XMakefileParser xmakefileParser;
    if (!xmakefileParser.Parse(xmakefilePath))
    {
        std::cerr << "Error: Failed to parse xmakefile." << std::endl;
        return 1;
    }

    std::cout << "Done" << std::endl;

    return 0;
}