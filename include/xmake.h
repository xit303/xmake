#pragma once

#include "XMakefileParser.h"

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
        return false;
    }
};