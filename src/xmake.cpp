//**************************************************************
// Includes
//**************************************************************

#include "xmake.h"
#include "SecurityHelper.h"
#include <Logger.h>

//**************************************************************
// Public functions
//**************************************************************

XMake::XMake(const CmdLineParser &cmdLineParser) : cmdLineParser(cmdLineParser)
{
    // Check if verbose option is set
    if (cmdLineParser.IsOptionSet("-v"))
    {
        verbose = true;
    }

    selectedConfig = cmdLineParser.GetOptionValue("-c", "");
}

bool XMake::Init(const std::string &makefileName)
{
    parser.SetVerbose(verbose);

    if (!parser.Parse(makefileName))
    {
        return false;
    }

    if (!selectedConfig.empty())
    {
        if (!parser.SetConfig(selectedConfig))
            return false;
    }
    else
    {
        // If no config is specified, use the first one
        Logger::LogInfo("No configuration specified, using [" + parser.GetCurrentConfig().Name + "]");
    }

    parser.LoadBuildTimes();

    return true;
}

bool XMake::Build()
{
    parser.CreateBuildList();

    // check if dependencies are up to date
    if (parser.GetBuildStructures().empty())
    {
        Logger::LogError("No build structures found.");
        return false;
    }

    RebuildScheme rebuildScheme = parser.CheckRebuild();

    if (rebuildScheme == RebuildScheme::None)
    {
        std::cout << "No changes in files." << std::endl;
        return true;
    }

    // execute pre-build commands
    XMakefileConfig config = parser.GetCurrentConfig();
    if (config.PreBuildCommands.size() > 0)
    {
        for (const auto &command : config.PreBuildCommands)
        {
            Logger::LogVerbose("Pre-build command: " + command);

            if (!ExecuteCommand(command))
            {
                Logger::LogError("Pre-build command failed.");
                return false;
            }
        }
    }

    // build all source files in parallel

    std::vector<std::thread> threads;
    std::atomic<int> numberOfBuilds = 0;
    std::atomic<bool> interruptBuild = false;

    // Determine the number of threads to use for parallel builds
    unsigned int numThreads = 0;
    if (cmdLineParser.IsOptionSet("-j"))
    {
        std::string jValue = cmdLineParser.GetOptionValue("-j");

        Logger::LogVerbose("Using -j option with value: " + jValue);

        if (!jValue.empty())
        {
            try
            {
                numThreads = std::stoi(jValue);
            }
            catch (const std::invalid_argument &)
            {
                Logger::LogWarning("Invalid value for -j option. Using hardware concurrency.");
            }
        }
        if (numThreads == 0)
        {
            numThreads = std::thread::hardware_concurrency();
        }
        if (numThreads == 0) // Fallback if hardware_concurrency is not available
        {
            numThreads = 2; // Default to 2 threads
        }
    }
    else
    {
        numThreads = 0;
    }

    if (rebuildScheme == RebuildScheme::Full || rebuildScheme == RebuildScheme::Sources)
    {
        for (const BuildStruct &buildStruct : parser.GetBuildStructures())
        {
            if (interruptBuild)
                break; // Stop building if interrupted

            if (buildStruct.empty())
                continue;

            // get directory of the source file
            std::string sourceDir = buildStruct.objectFile.substr(0, buildStruct.objectFile.find_last_of("/\\"));

            // create the directory if it does not exist
            if (!std::filesystem::exists(sourceDir))
            {
                std::filesystem::create_directories(sourceDir);
            }

            if (rebuildScheme == RebuildScheme::Sources)
            {
                // Check date of the source file and the object file
                std::filesystem::path sourcePath(buildStruct.sourceFile);
                std::filesystem::path objectPath(buildStruct.objectFile);

                if (std::filesystem::exists(objectPath) && std::filesystem::last_write_time(sourcePath) <= std::filesystem::last_write_time(objectPath))
                {
                    Logger::LogVerbose("Skipping: " + buildStruct.sourceFile + " (up to date)");
                    continue;
                }
            }

            if (verbose)
                std::cout << "Building: " << buildStruct.buildString << std::endl;
            else
                std::cout << "Building: " << buildStruct.objectFile << std::endl;

            threads.emplace_back([this, buildStruct = buildStruct, &numberOfBuilds, rebuildScheme = rebuildScheme, &interruptBuild]() -> bool
                                 {
                if (interruptBuild)
                    return false; // Stop building if interrupted

                // Execute the build command
                if (!ExecuteCommand(buildStruct.buildString))
                {
                    interruptBuild = true; // Set interrupt flag
                    Logger::LogError(buildStruct.buildString + " failed.");
                    return false;
                }

                numberOfBuilds++;

                return true; });

            // TODO This is not efficient, as it waits for all threads to finish before starting new ones

            // Limit the number of active threads to numThreads
            if ((numThreads != 0) && (threads.size() >= numThreads))
            {
                for (auto &thread : threads)
                {
                    if (thread.joinable())
                    {
                        thread.join();
                    }
                }
                threads.clear();
            }
        }
    }

    // Wait for all remaining threads to finish
    for (auto &thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    if (interruptBuild)
        return false;

    // After building all source files, link them
    std::string linkString = parser.GetLinkerString();

    if (numberOfBuilds == 0)
    {
        std::cout << "All files are up to date" << std::endl;
        return true;
    }

    if (verbose)
        std::cout << "Linking: " << linkString << std::endl;
    else
        std::cout << "Linking: " << parser.GetOutputFilename() << std::endl;

    // Execute the linker command
    if (!ExecuteCommand(linkString))
    {
        Logger::LogError("Linking failed.");
        return false;
    }

    // execute post-build commands
    if (config.PostBuildCommands.size() > 0)
    {
        for (const auto &command : config.PostBuildCommands)
        {
            Logger::LogVerbose("Post-build command: " + command);

            if (!ExecuteCommand(command))
            {
                Logger::LogError("Post-build command failed.");
                return false;
            }
        }
    }

    std::cout << "Finished building target: " << parser.GetOutputFilename() << std::endl;

    if (rebuildScheme == RebuildScheme::Full || rebuildScheme == RebuildScheme::Sources)
        SaveBuildTimes();

    return true;
}

void XMake::Clean()
{
    std::cout << "Cleaning build files..." << std::endl;

    // run all the clean_commands
    XMakefileConfig config = parser.GetCurrentConfig();
    if (config.CleanCommands.size() > 0)
    {
        for (const auto &command : config.CleanCommands)
        {
            Logger::LogVerbose("Clean command: " + command);

            if (!ExecuteCommand(command))
            {
                Logger::LogError("Clean command failed.");
                return;
            }
        }
    }

    std::cout << "Cleaned build files." << std::endl;
}

void XMake::Run()
{
    XMakefileConfig config = parser.GetCurrentConfig();
    if (config.PreRunCommands.size() > 0)
    {
        for (const auto &command : config.PreRunCommands)
        {
            Logger::LogVerbose("Pre-run command: " + command);

            if (!ExecuteCommand(command))
            {
                Logger::LogError("Pre-run command failed.");
                return;
            }
        }
    }

    // Execute the output file
    std::string runCommand = config.OutputDir + "/" + config.OutputFilename;
    if (!ExecuteCommand(runCommand))
    {
        Logger::LogError("Failed to run the output file.");
        return;
    }

    if (config.PostRunCommands.size() > 0)
    {
        for (const auto &command : config.PostRunCommands)
        {
            Logger::LogVerbose("Post-run command: " + command);

            if (!ExecuteCommand(command))
            {
                Logger::LogError("Post-run command failed.");
                return;
            }
        }
    }
}

void XMake::Install()
{
    std::cout << "Installing..." << std::endl;

    XMakefileConfig config = parser.GetCurrentConfig();
    // run all the install_commands
    if (config.InstallCommands.size() > 0)
    {
        for (const auto &command : config.InstallCommands)
        {
            Logger::LogVerbose("Install command: " + command);

            if (!ExecuteCommand(command))
            {
                Logger::LogError("Install command failed. Need sudo?");
                return;
            }
        }
    }

    std::cout << "Installed successfully." << std::endl;
}

void XMake::Uninstall()
{
    std::cout << "Uninstalling..." << std::endl;

    XMakefileConfig config = parser.GetCurrentConfig();

    // run all the uninstall_commands
    if (config.UninstallCommands.size() > 0)
    {
        for (const auto &command : config.UninstallCommands)
        {
            Logger::LogVerbose("Uninstall command: " + command);

            if (!ExecuteCommand(command))
            {
                Logger::LogError("Uninstall command failed.");
                return;
            }
        }
    }

    std::cout << "Uninstalled successfully." << std::endl;
}

//**************************************************************
// Private functions
//**************************************************************

void XMake::SaveBuildTimes()
{
    parser.SaveBuildTimes();
}