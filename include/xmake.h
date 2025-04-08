#pragma once

#include "XMakefileParser.h"
#include <iostream>
#include <filesystem>
#include <thread>
#include <vector>

class XMake
{
private:
    XMakefileParser parser;

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

        for (const BuildStruct &buildStruct : parser.GetBuildStructures())
        {
            threads.emplace_back([this, buildStruct]() -> bool
            {
                if (buildStruct.empty())
                {
                    return false;
                }

                std::cout << "Building: " << buildStruct.objectFile << " ... ";

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
                {
                    std::cerr << "failed" << std::endl;
                    return false;
                }

                std::cout << "done" << std::endl;
                return true;
            });
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

        std::cout << "Linking: " << linkString << " ... ";
        // Execute the linker command
        int result = system(linkString.c_str());

        if (result != 0)
        {
            std::cerr << "failed" << std::endl;
            return false;
        }

        std::cout << "done" << std::endl;
        std::cout << "Finished building target: " << parser.GetOutputFilename() << std::endl;
        return true;
    }
};