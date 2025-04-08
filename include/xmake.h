#pragma once

#include "XMakefileParser.h"
#include <iostream>

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

        std::string buildString;
        while (!(buildString = parser.GetNextBuildString()).empty())
        {
            // Execute the build command
            int result = system(buildString.c_str());
            if (result != 0)
            {
                std::cerr << "Error: Build failed with command: " << buildString << std::endl;
                return false;
            }
        }

        // Execute the linker command
        std::string linkString = parser.GetLinkerString();
        int result = system(linkString.c_str());

        if (result != 0)
        {
            std::cerr << "Error: Linking failed with command: " << linkString << std::endl;
            return false;
        }

        std::cout << "Build and linking completed successfully." << std::endl;
        return true;
    }
};