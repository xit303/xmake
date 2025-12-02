#include <gtest/gtest.h>
#include "XMakefile.h"
#include <ArduinoJson.h>
#include <filesystem>

// Test fixture for XMakefile tests
class XMakefileTest : public ::testing::Test
{
protected:
    std::string testDir;
    
    // Delete copy constructor and assignment operator (test fixtures should not be copied)
    XMakefileTest(const XMakefileTest&) = delete;
    XMakefileTest& operator=(const XMakefileTest&) = delete;

    void SetUp() override
    {
        // Create a temporary test directory
        testDir = std::filesystem::temp_directory_path().string() + "/xmake_test_" + 
                  std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        std::filesystem::create_directories(testDir);
    }

    void TearDown() override
    {
        // Clean up test directory
        if (std::filesystem::exists(testDir))
        {
            std::filesystem::remove_all(testDir);
        }
    }

    JsonDocument createSingleConfigJson()
    {
        JsonDocument doc;
        JsonArray configs = doc["configurations"].to<JsonArray>();
        
        JsonObject config = configs.add<JsonObject>();
        config["name"] = "Debug";
        config["build_type"] = "Debug";
        config["build_dir"] = ".build";
        config["output_filename"] = "app";
        config["compiler_path"] = "/usr/bin/g++";
        config["compiler"] = "g++";
        config["c_flags"] = "-Wall -g";
        config["cxx_flags"] = "-Wall -g -std=c++17";
        config["linker"] = "g++";
        config["linker_flags"] = "";
        config["archiver"] = "ar";
        config["archiver_flags"] = "rcs";
        
        config["defines"].to<JsonArray>();
        config["include_paths"].to<JsonArray>();
        config["library_paths"].to<JsonArray>();
        config["libraries"].to<JsonArray>();
        config["source_paths"].to<JsonArray>();
        config["exclude_paths"].to<JsonArray>();
        config["exclude_files"].to<JsonArray>();
        config["pre_build_commands"].to<JsonArray>();
        config["post_build_commands"].to<JsonArray>();
        config["pre_run_commands"].to<JsonArray>();
        config["post_run_commands"].to<JsonArray>();
        config["install_commands"].to<JsonArray>();
        config["uninstall_commands"].to<JsonArray>();
        config["clean_commands"].to<JsonArray>();
        
        return doc;
    }

    JsonDocument createMultiConfigJson()
    {
        JsonDocument doc;
        JsonArray configs = doc["configurations"].to<JsonArray>();
        
        // Debug configuration
        JsonObject debugConfig = configs.add<JsonObject>();
        debugConfig["name"] = "Debug";
        debugConfig["build_type"] = "Debug";
        debugConfig["build_dir"] = ".build";
        debugConfig["output_filename"] = "app_debug";
        debugConfig["compiler_path"] = "/usr/bin/g++";
        debugConfig["compiler"] = "g++";
        debugConfig["c_flags"] = "-Wall -g";
        debugConfig["cxx_flags"] = "-Wall -g -std=c++17";
        debugConfig["linker"] = "g++";
        debugConfig["linker_flags"] = "";
        debugConfig["archiver"] = "ar";
        debugConfig["archiver_flags"] = "rcs";
        
        debugConfig["defines"].to<JsonArray>().add("DEBUG");
        debugConfig["include_paths"].to<JsonArray>();
        debugConfig["library_paths"].to<JsonArray>();
        debugConfig["libraries"].to<JsonArray>();
        debugConfig["source_paths"].to<JsonArray>();
        debugConfig["exclude_paths"].to<JsonArray>();
        debugConfig["exclude_files"].to<JsonArray>();
        debugConfig["pre_build_commands"].to<JsonArray>();
        debugConfig["post_build_commands"].to<JsonArray>();
        debugConfig["pre_run_commands"].to<JsonArray>();
        debugConfig["post_run_commands"].to<JsonArray>();
        debugConfig["install_commands"].to<JsonArray>();
        debugConfig["uninstall_commands"].to<JsonArray>();
        debugConfig["clean_commands"].to<JsonArray>();
        
        // Release configuration
        JsonObject releaseConfig = configs.add<JsonObject>();
        releaseConfig["name"] = "Release";
        releaseConfig["build_type"] = "Release";
        releaseConfig["build_dir"] = ".build";
        releaseConfig["output_filename"] = "app_release";
        releaseConfig["compiler_path"] = "/usr/bin/g++";
        releaseConfig["compiler"] = "g++";
        releaseConfig["c_flags"] = "-Wall -O3";
        releaseConfig["cxx_flags"] = "-Wall -O3 -std=c++17";
        releaseConfig["linker"] = "g++";
        releaseConfig["linker_flags"] = "-s";
        releaseConfig["archiver"] = "ar";
        releaseConfig["archiver_flags"] = "rcs";
        
        releaseConfig["defines"].to<JsonArray>().add("NDEBUG");
        releaseConfig["include_paths"].to<JsonArray>();
        releaseConfig["library_paths"].to<JsonArray>();
        releaseConfig["libraries"].to<JsonArray>();
        releaseConfig["source_paths"].to<JsonArray>();
        releaseConfig["exclude_paths"].to<JsonArray>();
        releaseConfig["exclude_files"].to<JsonArray>();
        releaseConfig["pre_build_commands"].to<JsonArray>();
        releaseConfig["post_build_commands"].to<JsonArray>();
        releaseConfig["pre_run_commands"].to<JsonArray>();
        releaseConfig["post_run_commands"].to<JsonArray>();
        releaseConfig["install_commands"].to<JsonArray>();
        releaseConfig["uninstall_commands"].to<JsonArray>();
        releaseConfig["clean_commands"].to<JsonArray>();
        
        return doc;
    }

    XMakefileTest() : testDir() {}
};

// Test default constructor
TEST_F(XMakefileTest, DefaultConstructor)
{
    XMakefile xmakefile;
    
    EXPECT_TRUE(xmakefile.configs.empty());
}

// Test FromJSON with single configuration
TEST_F(XMakefileTest, FromJSONSingleConfig)
{
    XMakefile xmakefile;
    JsonDocument doc = createSingleConfigJson();
    
    xmakefile.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(xmakefile.configs.size(), 1);
    EXPECT_EQ(xmakefile.configs[0].Name, "Debug");
    EXPECT_EQ(xmakefile.configs[0].BuildType, "Debug");
}

// Test FromJSON with multiple configurations
TEST_F(XMakefileTest, FromJSONMultipleConfigs)
{
    XMakefile xmakefile;
    JsonDocument doc = createMultiConfigJson();
    
    xmakefile.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(xmakefile.configs.size(), 2);
    EXPECT_EQ(xmakefile.configs[0].Name, "Debug");
    EXPECT_EQ(xmakefile.configs[1].Name, "Release");
}

// Test configuration details
TEST_F(XMakefileTest, ConfigurationDetails)
{
    XMakefile xmakefile;
    JsonDocument doc = createMultiConfigJson();
    
    xmakefile.FromJSON(doc.as<JsonVariant>(), testDir);
    
    // Check Debug configuration
    EXPECT_EQ(xmakefile.configs[0].OutputFilename, "app_debug");
    EXPECT_TRUE(xmakefile.configs[0].CXXCompilerFlags.find("-g") != std::string::npos);
    EXPECT_EQ(xmakefile.configs[0].Defines.size(), 1);
    EXPECT_EQ(xmakefile.configs[0].Defines[0], "DEBUG");
    
    // Check Release configuration
    EXPECT_EQ(xmakefile.configs[1].OutputFilename, "app_release");
    EXPECT_TRUE(xmakefile.configs[1].CXXCompilerFlags.find("-O3") != std::string::npos);
    EXPECT_EQ(xmakefile.configs[1].Defines.size(), 1);
    EXPECT_EQ(xmakefile.configs[1].Defines[0], "NDEBUG");
}

// Test empty configurations array
TEST_F(XMakefileTest, EmptyConfigurationsArray)
{
    XMakefile xmakefile;
    JsonDocument doc;
    doc["configurations"].to<JsonArray>();
    
    xmakefile.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_TRUE(xmakefile.configs.empty());
}

// Test accessing configs vector
TEST_F(XMakefileTest, AccessConfigsVector)
{
    XMakefile xmakefile;
    JsonDocument doc = createSingleConfigJson();
    
    xmakefile.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_FALSE(xmakefile.configs.empty());
    EXPECT_EQ(xmakefile.configs.size(), 1);
    
    XMakefileConfig& config = xmakefile.configs[0];
    EXPECT_EQ(config.Name, "Debug");
}

// Test with three configurations
TEST_F(XMakefileTest, ThreeConfigurations)
{
    XMakefile xmakefile;
    JsonDocument doc;
    JsonArray configs = doc["configurations"].to<JsonArray>();
    
    // Add three different configurations
    const char* configNames[] = {"Debug", "Release", "Test"};
    for (const char* name : configNames)
    {
        JsonObject config = configs.add<JsonObject>();
        config["name"] = name;
        config["build_type"] = name;
        config["build_dir"] = ".build";
        config["output_filename"] = std::string("app_") + name;
        config["compiler_path"] = "/usr/bin/g++";
        config["compiler"] = "g++";
        config["c_flags"] = "-Wall";
        config["cxx_flags"] = "-Wall -std=c++17";
        config["linker"] = "g++";
        config["linker_flags"] = "";
        config["archiver"] = "ar";
        config["archiver_flags"] = "rcs";
        
        config["defines"].to<JsonArray>();
        config["include_paths"].to<JsonArray>();
        config["library_paths"].to<JsonArray>();
        config["libraries"].to<JsonArray>();
        config["source_paths"].to<JsonArray>();
        config["exclude_paths"].to<JsonArray>();
        config["exclude_files"].to<JsonArray>();
        config["pre_build_commands"].to<JsonArray>();
        config["post_build_commands"].to<JsonArray>();
        config["pre_run_commands"].to<JsonArray>();
        config["post_run_commands"].to<JsonArray>();
        config["install_commands"].to<JsonArray>();
        config["uninstall_commands"].to<JsonArray>();
        config["clean_commands"].to<JsonArray>();
    }
    
    xmakefile.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(xmakefile.configs.size(), 3);
    EXPECT_EQ(xmakefile.configs[0].Name, "Debug");
    EXPECT_EQ(xmakefile.configs[1].Name, "Release");
    EXPECT_EQ(xmakefile.configs[2].Name, "Test");
}

// Test configuration with all features
TEST_F(XMakefileTest, ConfigurationWithAllFeatures)
{
    XMakefile xmakefile;
    JsonDocument doc;
    JsonArray configs = doc["configurations"].to<JsonArray>();
    
    JsonObject config = configs.add<JsonObject>();
    config["name"] = "FullFeature";
    config["build_type"] = "Debug";
    config["build_dir"] = ".build";
    config["output_filename"] = "full_app";
    config["compiler_path"] = "/usr/bin/g++";
    config["compiler"] = "g++";
    config["c_flags"] = "-Wall -g";
    config["cxx_flags"] = "-Wall -g -std=c++17";
    config["linker"] = "g++";
    config["linker_flags"] = "-pthread";
    config["archiver"] = "ar";
    config["archiver_flags"] = "rcs";
    
    JsonArray defines = config["defines"].to<JsonArray>();
    defines.add("DEBUG");
    defines.add("FEATURE_X");
    
    JsonArray includePaths = config["include_paths"].to<JsonArray>();
    includePaths.add("include");
    includePaths.add("src");
    
    JsonArray libraryPaths = config["library_paths"].to<JsonArray>();
    libraryPaths.add("/usr/lib");
    
    JsonArray libraries = config["libraries"].to<JsonArray>();
    libraries.add("pthread");
    libraries.add("m");
    
    JsonArray sourcePaths = config["source_paths"].to<JsonArray>();
    sourcePaths.add("src");
    sourcePaths.add("lib");
    
    JsonArray excludePaths = config["exclude_paths"].to<JsonArray>();
    excludePaths.add("test");
    
    JsonArray excludeFiles = config["exclude_files"].to<JsonArray>();
    excludeFiles.add("src/old.cpp");
    
    JsonArray preBuildCommands = config["pre_build_commands"].to<JsonArray>();
    preBuildCommands.add("echo Pre-build");
    
    JsonArray postBuildCommands = config["post_build_commands"].to<JsonArray>();
    postBuildCommands.add("echo Post-build");
    
    JsonArray preRunCommands = config["pre_run_commands"].to<JsonArray>();
    preRunCommands.add("echo Pre-run");
    
    JsonArray postRunCommands = config["post_run_commands"].to<JsonArray>();
    postRunCommands.add("echo Post-run");
    
    JsonArray installCommands = config["install_commands"].to<JsonArray>();
    installCommands.add("cp full_app /usr/local/bin");
    
    JsonArray uninstallCommands = config["uninstall_commands"].to<JsonArray>();
    uninstallCommands.add("rm /usr/local/bin/full_app");
    
    JsonArray cleanCommands = config["clean_commands"].to<JsonArray>();
    cleanCommands.add("rm -rf .build");
    
    xmakefile.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(xmakefile.configs.size(), 1);
    XMakefileConfig& cfg = xmakefile.configs[0];
    
    EXPECT_EQ(cfg.Name, "FullFeature");
    EXPECT_EQ(cfg.Defines.size(), 2);
    EXPECT_EQ(cfg.IncludePaths.size(), 2);
    EXPECT_EQ(cfg.LibraryPaths.size(), 1);
    EXPECT_EQ(cfg.Libraries.size(), 2);
    EXPECT_EQ(cfg.SourcePaths.size(), 2);
    EXPECT_EQ(cfg.ExcludePaths.size(), 1);
    EXPECT_EQ(cfg.ExcludeFiles.size(), 1);
    EXPECT_EQ(cfg.PreBuildCommands.size(), 1);
    EXPECT_EQ(cfg.PostBuildCommands.size(), 1);
    EXPECT_EQ(cfg.PreRunCommands.size(), 1);
    EXPECT_EQ(cfg.PostRunCommands.size(), 1);
    EXPECT_EQ(cfg.InstallCommands.size(), 1);
    EXPECT_EQ(cfg.UninstallCommands.size(), 1);
    EXPECT_EQ(cfg.CleanCommands.size(), 1);
}

// Test different build types
TEST_F(XMakefileTest, DifferentBuildTypes)
{
    XMakefile xmakefile;
    JsonDocument doc;
    JsonArray configs = doc["configurations"].to<JsonArray>();
    
    const char* buildTypes[] = {"Debug", "Release", "RelWithDebInfo", "MinSizeRel"};
    
    for (const char* buildType : buildTypes)
    {
        JsonObject config = configs.add<JsonObject>();
        config["name"] = buildType;
        config["build_type"] = buildType;
        config["build_dir"] = ".build";
        config["output_filename"] = "app";
        config["compiler_path"] = "/usr/bin/g++";
        config["compiler"] = "g++";
        config["c_flags"] = "-Wall";
        config["cxx_flags"] = "-Wall -std=c++17";
        config["linker"] = "g++";
        config["linker_flags"] = "";
        config["archiver"] = "ar";
        config["archiver_flags"] = "rcs";
        
        config["defines"].to<JsonArray>();
        config["include_paths"].to<JsonArray>();
        config["library_paths"].to<JsonArray>();
        config["libraries"].to<JsonArray>();
        config["source_paths"].to<JsonArray>();
        config["exclude_paths"].to<JsonArray>();
        config["exclude_files"].to<JsonArray>();
        config["pre_build_commands"].to<JsonArray>();
        config["post_build_commands"].to<JsonArray>();
        config["pre_run_commands"].to<JsonArray>();
        config["post_run_commands"].to<JsonArray>();
        config["install_commands"].to<JsonArray>();
        config["uninstall_commands"].to<JsonArray>();
        config["clean_commands"].to<JsonArray>();
    }
    
    xmakefile.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(xmakefile.configs.size(), 4);
    EXPECT_EQ(xmakefile.configs[0].BuildType, "Debug");
    EXPECT_EQ(xmakefile.configs[1].BuildType, "Release");
    EXPECT_EQ(xmakefile.configs[2].BuildType, "RelWithDebInfo");
    EXPECT_EQ(xmakefile.configs[3].BuildType, "MinSizeRel");
}

// Test configuration with different compilers
TEST_F(XMakefileTest, DifferentCompilers)
{
    XMakefile xmakefile;
    JsonDocument doc;
    JsonArray configs = doc["configurations"].to<JsonArray>();
    
    const char* compilers[] = {"g++", "clang++", "c++"};
    
    for (int i = 0; i < 3; i++)
    {
        JsonObject config = configs.add<JsonObject>();
        config["name"] = std::string("Config") + std::to_string(i);
        config["build_type"] = "Debug";
        config["build_dir"] = ".build";
        config["output_filename"] = "app";
        config["compiler_path"] = std::string("/usr/bin/") + compilers[i];
        config["compiler"] = compilers[i];
        config["c_flags"] = "-Wall";
        config["cxx_flags"] = "-Wall -std=c++17";
        config["linker"] = compilers[i];
        config["linker_flags"] = "";
        config["archiver"] = "ar";
        config["archiver_flags"] = "rcs";
        
        config["defines"].to<JsonArray>();
        config["include_paths"].to<JsonArray>();
        config["library_paths"].to<JsonArray>();
        config["libraries"].to<JsonArray>();
        config["source_paths"].to<JsonArray>();
        config["exclude_paths"].to<JsonArray>();
        config["exclude_files"].to<JsonArray>();
        config["pre_build_commands"].to<JsonArray>();
        config["post_build_commands"].to<JsonArray>();
        config["pre_run_commands"].to<JsonArray>();
        config["post_run_commands"].to<JsonArray>();
        config["install_commands"].to<JsonArray>();
        config["uninstall_commands"].to<JsonArray>();
        config["clean_commands"].to<JsonArray>();
    }
    
    xmakefile.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(xmakefile.configs.size(), 3);
    EXPECT_EQ(xmakefile.configs[0].Compiler, "g++");
    EXPECT_EQ(xmakefile.configs[1].Compiler, "clang++");
    EXPECT_EQ(xmakefile.configs[2].Compiler, "c++");
}

// Test base path propagation to configs
TEST_F(XMakefileTest, BasePathPropagation)
{
    XMakefile xmakefile;
    JsonDocument doc = createSingleConfigJson();
    
    JsonArray includePaths = doc["configurations"][0]["include_paths"].as<JsonArray>();
    includePaths.add("include");
    
    xmakefile.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(xmakefile.configs.size(), 1);
    EXPECT_EQ(xmakefile.configs[0].IncludePaths.size(), 1);
    EXPECT_TRUE(xmakefile.configs[0].IncludePaths[0].find(testDir) != std::string::npos);
}

// Test configuration copying
TEST_F(XMakefileTest, ConfigurationCopying)
{
    XMakefile xmakefile;
    JsonDocument doc = createSingleConfigJson();
    
    xmakefile.FromJSON(doc.as<JsonVariant>(), testDir);
    
    // Copy the configs vector
    std::vector<XMakefileConfig> configsCopy = xmakefile.configs;
    
    EXPECT_EQ(configsCopy.size(), 1);
    EXPECT_EQ(configsCopy[0].Name, "Debug");
}

// Test empty XMakefile after construction
TEST_F(XMakefileTest, EmptyAfterConstruction)
{
    XMakefile xmakefile;
    
    EXPECT_EQ(xmakefile.configs.size(), 0);
    EXPECT_TRUE(xmakefile.configs.empty());
}

// Test multiple FromJSON calls (should append or replace?)
TEST_F(XMakefileTest, MultipleFromJSONCalls)
{
    XMakefile xmakefile;
    JsonDocument doc1 = createSingleConfigJson();
    
    xmakefile.FromJSON(doc1.as<JsonVariant>(), testDir);
    EXPECT_EQ(xmakefile.configs.size(), 1);
    
    // Second call
    JsonDocument doc2 = createMultiConfigJson();
    xmakefile.FromJSON(doc2.as<JsonVariant>(), testDir);
    
    // Configs should be appended (3 total: 1 from first + 2 from second)
    EXPECT_EQ(xmakefile.configs.size(), 3);
}
