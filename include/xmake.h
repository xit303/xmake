#pragma once

#include "XMakefileParser.h"
#include "CmdLineParser.h"
#include <iostream>
#include <filesystem>
#include <thread>
#include <vector>

class XMake
{
private:
    XMakefileParser parser;
    bool verbose = false;

public:
    XMake() = default;

    bool Init(const std::string &makefileName)
    {
        return parser.Parse(makefileName);
    }

    bool Build()
    {
        parser.CreateBuildList();

        // build all source files in parallel

        std::vector<std::thread> threads;

        std::atomic<int> numberOfBuilds = 0;

        for (const BuildStruct &buildStruct : parser.GetBuildStructures())
        {
            threads.emplace_back([this, buildStruct = buildStruct, &numberOfBuilds]() -> bool
                                 {
                if (buildStruct.empty())
                {
                    return false;
                }

                // get directory of the source file
                std::string sourceDir = buildStruct.objectFile.substr(0, buildStruct.objectFile.find_last_of("/\\"));

                // create the directory if it does not exist
                if (!std::filesystem::exists(sourceDir))
                {
                    std::filesystem::create_directories(sourceDir);
                }

                // Execute the build command
                int result = system(buildStruct.buildString.c_str());
                if (result != 0)
                    return false;

                numberOfBuilds++;

                return true; });
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
        int result = system(linkString.c_str());

        if (result != 0)
        {
            std::cerr << "Error: Linking failed." << std::endl;
            return false;
        }

        std::cout << "Finished building target: " << parser.GetOutputFilename() << std::endl;
        return true;
    }

    void Clean()
    {
        std::cout << "Cleaning build files..." << std::endl;

        parser.CreateBuildList();

        for (const BuildStruct &buildStruct : parser.GetBuildStructures())
        {
            if (!buildStruct.empty())
            {
                if (!std::filesystem::remove(buildStruct.objectFile))
                {
                    std::cerr << "Error: Failed to remove " << buildStruct.objectFile << std::endl;
                }
            }
        }

        std::cout << "Cleaned build files." << std::endl;
    }

    void CleanAll()
    {
        std::cout << "Cleaning all files..." << std::endl;

        parser.CreateBuildList();

        for (const BuildStruct &buildStruct : parser.GetBuildStructures())
        {
            if (!buildStruct.empty())
            {
                if (!std::filesystem::remove(buildStruct.objectFile))
                {
                    std::cerr << "Error: Failed to remove " << buildStruct.objectFile << std::endl;
                }
            }
        }

        std::filesystem::remove(parser.GetOutputFilename());

        std::cout << "Cleaned all files." << std::endl;
    }
};