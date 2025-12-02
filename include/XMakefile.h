#pragma once

//**************************************************************
// Includes
//**************************************************************

#include "XMakefileConfig.h"

//**************************************************************
// Classes
//**************************************************************

class XMakefile
{
public:
    std::vector<XMakefileConfig> configs;

    XMakefile() : configs() {}

    void FromJSON(const JsonVariant &doc, const std::string &basePath)
    {
        // Extract configurations
        JsonArray configsArray = doc["configurations"].as<JsonArray>();
        for (JsonVariant config : configsArray)
        {
            XMakefileConfig cfg;
            cfg.FromJSON(config, basePath);
            configs.push_back(cfg);
        }
    }
};
