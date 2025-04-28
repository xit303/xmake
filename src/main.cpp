//**************************************************************
// Includes
//**************************************************************

#include <iostream>
#include "Version.h"
#include "CmdLineParser.h"
#include "xmake.h"
#include <filesystem>
#include <Logger.h>

//**************************************************************
// Main program
//**************************************************************

int main(int argc, char **argv)
{
    CmdLineParser parser("xmake", "xit's little make tool", VERSION_STRING);
    parser.RegisterOption("--help", "Show help message");
    parser.RegisterOption("--version", "Show version information");
    parser.RegisterOption("--xmakefile", "Path to the xmakefile to use");
    parser.RegisterOption("--config", "Configuration to use (default: first one in xmakefile)");
    parser.RegisterOption("-v", "Enable verbose output");
    parser.RegisterOption("-j", "Number of jobs to run simultaneously");
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
        Logger::SetVerbose(true);
        Logger::LogInfo("Verbose mode enabled");
    }

    std::string xmakefilePath = std::filesystem::current_path().string() + "/xmakefile.json";

    if (parser.IsOptionSet("--xmakefile"))
    {
        xmakefilePath = parser.GetOptionValue("--xmakefile", xmakefilePath);
        Logger::LogVerbose("Using xmakefile: " + xmakefilePath);
    }
    else
    {
        Logger::LogVerbose("No xmakefile specified, using default.");
    }

    XMake xmake(parser);
    if (!xmake.Init(xmakefilePath))
    {
        Logger::LogError("Failed to initialize xmake.");
        return 1;
    }

    if (parser.IsOptionSet("clean"))
    {
        xmake.Clean();
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