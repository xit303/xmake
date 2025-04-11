#include <iostream>
#include "Version.h"
#include "CmdLineParser.h"
#include "xmake.h"
#include <filesystem>

int main(int argc, char **argv)
{
    CmdLineParser parser("xmake", "xit's little make tool", VERSION_STRING);
    parser.RegisterOption("--help", "Show help message");
    parser.RegisterOption("--version", "Show version information");
    parser.RegisterOption("--xmakefile", "Path to the xmakefile to use");
    parser.RegisterOption("-v", "Enable verbose output");
    parser.RegisterOption("clean", "Clean all build files (clean_commands needs to be set in xakefile)");
    parser.RegisterOption("run", "Run the output file after building");
    parser.RegisterOption("install", "Install the output file");
    parser.RegisterOption("uninstall", "Uninstall the output file");

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
        parser.PrintVersion();
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

    if (parser.IsOptionSet("run"))
    {
        xmake.Run();
        return 0;
    }

    if (parser.IsOptionSet("install"))
    {
        xmake.Install();
        return 0;
    }

    if (parser.IsOptionSet("uninstall"))
    {
        xmake.Uninstall();
        return 0;
    }

    // measure build time
    auto start = std::chrono::high_resolution_clock::now();

    xmake.Build();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Build completed in " << elapsed.count() << " seconds." << std::endl;

    return 0;
}