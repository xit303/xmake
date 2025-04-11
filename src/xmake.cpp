#include "xmake.h"
#include "SecurityHelper.h"

XMake::XMake(const CmdLineParser &cmdLineParser) : cmdLineParser(cmdLineParser)
{
    // Check if verbose option is set
    if (cmdLineParser.IsOptionSet("-v"))
    {
        verbose = true;
    }
}

bool XMake::Init(const std::string &makefileName)
{
    parser.SetVerbose(verbose);
    return parser.Parse(makefileName);
}

bool XMake::Build()
{
    parser.CreateBuildList();

    // check if dependencies are up to date
    if (parser.GetBuildStructures().empty())
    {
        std::cout << "No build structures found." << std::endl;
        return false;
    }

    RebuildScheme rebuildScheme = parser.CheckRebuild();

    if (rebuildScheme == RebuildScheme::None)
    {
        std::cout << "No changes in files." << std::endl;
        return false;
    }

    // execute pre-build commands
    XMakefileConfig config = parser.GetCurrentConfig();
    if (config.PreBuildCommands.size() > 0)
    {
        for (const auto &command : config.PreBuildCommands)
        {
            if (verbose)
                std::cout << "Pre-build command: " << command << std::endl;

            if (!ExecuteCommand(command))
            {
                std::cerr << "Error: Pre-build command failed." << std::endl;
                return false;
            }
        }
    }

    // build all source files in parallel

    std::vector<std::thread> threads;
    std::atomic<int> numberOfBuilds = 0;
    std::atomic<bool> interruptBuild = false;

    if (rebuildScheme == RebuildScheme::Full || rebuildScheme == RebuildScheme::Sources)
    {
        for (const BuildStruct &buildStruct : parser.GetBuildStructures())
        {
            if (interruptBuild)
                break; // Stop building if interrupted

            if (buildStruct.empty())
                continue;

            threads.emplace_back([this, buildStruct = buildStruct, &numberOfBuilds, rebuildScheme = rebuildScheme, &interruptBuild]() -> bool
                                 {
                if (interruptBuild)
                    return false; // Stop building if interrupted
                    
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
                        if (verbose)
                            std::cout << "Skipping: " << buildStruct.sourceFile << " (up to date)" << std::endl;

                        return true;
                    }
                }

                if (verbose)
                    std::cout << "Building: " << buildStruct.buildString << std::endl;
                else
                    std::cout << "Building: " << buildStruct.objectFile << std::endl;

                // Execute the build command
                if (!ExecuteCommand(buildStruct.buildString))
                {
                    interruptBuild = true; // Set interrupt flag
                    std::cerr << buildStruct.buildString << " failed" << std::endl;
                    return false;
                }

                numberOfBuilds++;

                return true; });
        }
    }

    // Wait for all threads to finish
    for (auto &thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    // After building all source files, link them
    std::string linkString = parser.GetLinkerString();

    if (numberOfBuilds == 0)
    {
        std::cout << "All files are up to date" << std::endl;
        return false;
    }

    if (verbose)
        std::cout << "Linking: " << linkString << std::endl;
    else
        std::cout << "Linking: " << parser.GetOutputFilename() << std::endl;

    // Execute the linker command
    if (!ExecuteCommand(linkString))
    {
        std::cerr << "Error: Linking failed." << std::endl;
        return false;
    }

    // execute post-build commands
    if (config.PostBuildCommands.size() > 0)
    {
        for (const auto &command : config.PostBuildCommands)
        {
            if (verbose)
                std::cout << "Post-build command: " << command << std::endl;

            if (!ExecuteCommand(command))
            {
                std::cerr << "Error: Post-build command failed." << std::endl;
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
            if (verbose)
                std::cout << "Clean command: " << command << std::endl;

            if (!ExecuteCommand(command))
            {
                std::cerr << "Error: Clean command failed." << std::endl;
                return;
            }
        }
    }

    // parser.CreateBuildList();

    // for (const BuildStruct &buildStruct : parser.GetBuildStructures())
    // {
    //     if (!buildStruct.empty())
    //     {
    //         if (!std::filesystem::remove(buildStruct.objectFile))
    //         {
    //             std::cerr << "Error: Failed to remove " << buildStruct.objectFile << std::endl;
    //         }
    //     }
    // }

    std::cout << "Cleaned build files." << std::endl;
}

void XMake::Run()
{
    XMakefileConfig config = parser.GetCurrentConfig();
    if (config.PreRunCommands.size() > 0)
    {
        for (const auto &command : config.PreRunCommands)
        {
            if (verbose)
                std::cout << "Pre-run command: " << command << std::endl;

            if (!ExecuteCommand(command))
            {
                std::cerr << "Error: Pre-run command failed." << std::endl;
                return;
            }
        }
    }

    // Execute the output file
    std::string runCommand = "./" + config.BuildDir + "/" + config.BuildType + "/" + config.OutputFilename;
    if (!ExecuteCommand(runCommand))
    {
        std::cerr << "Error: Failed to run the output file." << std::endl;
        return;
    }

    if (config.PostRunCommands.size() > 0)
    {
        for (const auto &command : config.PostRunCommands)
        {
            if (verbose)
                std::cout << "Post-run command: " << command << std::endl;

            if (!ExecuteCommand(command))
            {
                std::cerr << "Error: Post-run command failed." << std::endl;
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
            if (verbose)
                std::cout << "Install command: " << command << std::endl;

            if (!ExecuteCommand(command))
            {
                std::cerr << "Error: Install command failed." << std::endl;
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
            if (verbose)
                std::cout << "Uninstall command: " << command << std::endl;

            if (!ExecuteCommand(command))
            {
                std::cerr << "Error: Uninstall command failed." << std::endl;
                return;
            }
        }
    }

    std::cout << "Uninstalled successfully." << std::endl;
}

void XMake::SaveBuildTimes()
{
    parser.SaveBuildTimes();
}