#include <iostream>
#include "Version.h"
#include "CmdLineParser.h"
#include "xmake.h"
#include <filesystem>

int main(int argc, char **argv)
{
    CmdLineParser parser("xmake", VERSION_STRING);
    parser.RegisterOption("-h", "Show help message");
    parser.RegisterOption("--version", "Show version information");
    parser.RegisterOption("-v", "Enable verbose output");
    parser.RegisterOption("--xmakefile", "Path to the xmakefile to use");
    parser.RegisterOption("clean", "Clean the build files");
    parser.RegisterOption("cleanall", "Clean all files");
    parser.RegisterOption("run", "Run the output file after building");
    parser.RegisterOption("install", "Install the output file");

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

    bool verbose = parser.IsOptionSet("-v");
    if (verbose)
    {
        std::cout << "Verbose output enabled" << std::endl;
    }

    std::string xmakefilePath = std::filesystem::current_path().string() + "/xmakefile.json";

    if (parser.IsOptionSet("--xmakefile"))
    {
        xmakefilePath = parser.GetOptionValue("--xmakefile", xmakefilePath);
        if (verbose)
            std::cout << "Using xmakefile: " << xmakefilePath << std::endl;
    }
    else
    {
        if (verbose)
            std::cout << "No xmakefile specified, using default." << std::endl;
    }


    XMake xmake(parser);
    if (!xmake.Init(xmakefilePath))
    {
        std::cerr << "Error: Failed to parse xmakefile." << std::endl;
        return 1;
    }

    if (parser.IsOptionSet("clean"))
    {
        xmake.Clean();
        std::cout << "Cleaned build files." << std::endl;
        return 0;
    }

    if (parser.IsOptionSet("cleanall"))
    {
        xmake.CleanAll();
        std::cout << "Cleaned all files." << std::endl;
        return 0;
    }

    xmake.Build();
    return 0;
}