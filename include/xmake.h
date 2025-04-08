#pragma once

#include "XMakefileParser.h"
#include <iostream>
#include <filesystem>

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

        while (true)
        {
            const BuildStruct &buildStruct = parser.GetNextBuildStruct();

            // Check if the build structure is empty
            if (buildStruct.empty())
            {
                // No more build structures to process
                break;
            }

            std::cout << "Building: " << buildStruct.objectFile << " ... ";

            // get directory of the source file
            std::string sourceDir = buildStruct.objectFile.substr(0, buildStruct.objectFile.find_last_of("/\\"));

            // create the directory if it does not exist
            if (!std::filesystem::exists(sourceDir))
            {
                std::filesystem::create_directories(sourceDir);
            }
            // get the output file name

            // Execute the build command
            int result = system(buildStruct.buildString.c_str());
            if (result != 0)
            {
                std::cerr << "failed" << std::endl;
                return false;
            }

            std::cout << "done" << std::endl;
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