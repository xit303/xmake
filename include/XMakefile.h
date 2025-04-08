#pragma once

#include "XMakefileConfig.h"

class XMakefile
{
public:
    std::string xmakeFileName;
    std::string xmakeFileVersion;
    std::string xmakeFileDescription;
    std::string xmakeFileAuthor;
    std::string xmakeFileLicense;

    std::vector<XMakefileConfig> configs; // List of configurations

    void FromJSON(const JsonVariant &doc)
    {
        xmakeFileName = doc["name"].as<std::string>();
        xmakeFileVersion = doc["version"].as<std::string>();
        xmakeFileDescription = doc["description"].as<std::string>();
        xmakeFileAuthor = doc["author"].as<std::string>();
        xmakeFileLicense = doc["license"].as<std::string>();

        // Extract configurations
        JsonArray configsArray = doc["configurations"].as<JsonArray>();
        for (JsonVariant config : configsArray)
        {
            XMakefileConfig cfg;
            cfg.FromJSON(config);
            configs.push_back(cfg);
        }
    }
};
