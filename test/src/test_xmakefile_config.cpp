#include <gtest/gtest.h>
#include "XMakefileConfig.h"
#include <ArduinoJson.h>
#include <filesystem>
#include <fstream>

// Test fixture for XMakefileConfig tests
class XMakefileConfigTest : public ::testing::Test
{
protected:
    std::string testDir;
    
    // Delete copy constructor and assignment operator (test fixtures should not be copied)
    XMakefileConfigTest(const XMakefileConfigTest&) = delete;
    XMakefileConfigTest& operator=(const XMakefileConfigTest&) = delete;

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

    JsonDocument createBasicConfigJson()
    {
        JsonDocument doc;
        doc["name"] = "TestProject";
        doc["build_type"] = "Debug";
        doc["build_dir"] = ".build";
        doc["output_filename"] = "test_app";
        doc["compiler_path"] = "/usr/bin/g++";
        doc["compiler"] = "g++";
        doc["c_flags"] = "-Wall -O2";
        doc["cxx_flags"] = "-Wall -O2 -std=c++17";
        doc["linker"] = "g++";
        doc["linker_flags"] = "-pthread";
        doc["archiver"] = "ar";
        doc["archiver_flags"] = "rcs";
        
        JsonArray defines = doc["defines"].to<JsonArray>();
        defines.add("DEBUG");
        defines.add("VERSION=1.0");
        
        JsonArray includePaths = doc["include_paths"].to<JsonArray>();
        includePaths.add("include");
        includePaths.add("src");
        
        JsonArray libraryPaths = doc["library_paths"].to<JsonArray>();
        libraryPaths.add("/usr/lib");
        
        JsonArray libraries = doc["libraries"].to<JsonArray>();
        libraries.add("pthread");
        libraries.add("m");
        
        JsonArray sourcePaths = doc["source_paths"].to<JsonArray>();
        sourcePaths.add("src");
        
        JsonArray excludePaths = doc["exclude_paths"].to<JsonArray>();
        excludePaths.add("test");
        
        JsonArray excludeFiles = doc["exclude_files"].to<JsonArray>();
        excludeFiles.add("src/old.cpp");
        
        JsonArray preBuildCommands = doc["pre_build_commands"].to<JsonArray>();
        preBuildCommands.add("echo Building");
        
        JsonArray postBuildCommands = doc["post_build_commands"].to<JsonArray>();
        postBuildCommands.add("echo Build complete");
        
        JsonArray preRunCommands = doc["pre_run_commands"].to<JsonArray>();
        preRunCommands.add("echo Running");
        
        JsonArray postRunCommands = doc["post_run_commands"].to<JsonArray>();
        postRunCommands.add("echo Run complete");
        
        JsonArray installCommands = doc["install_commands"].to<JsonArray>();
        installCommands.add("cp test_app /usr/local/bin");
        
        JsonArray uninstallCommands = doc["uninstall_commands"].to<JsonArray>();
        uninstallCommands.add("rm /usr/local/bin/test_app");
        
        JsonArray cleanCommands = doc["clean_commands"].to<JsonArray>();
        cleanCommands.add("rm -rf .build");
        
        return doc;
    }

    XMakefileConfigTest() : testDir() {}
};

// Test default constructor
TEST_F(XMakefileConfigTest, DefaultConstructor)
{
    XMakefileConfig config;
    
    EXPECT_EQ(config.Name, "");
    EXPECT_EQ(config.BuildType, "");
    EXPECT_EQ(config.BuildDir, "");
    EXPECT_EQ(config.OutputDir, "");
    EXPECT_EQ(config.OutputFilename, "");
    EXPECT_EQ(config.Compiler, "");
    EXPECT_TRUE(config.Defines.empty());
    EXPECT_TRUE(config.IncludePaths.empty());
    EXPECT_TRUE(config.Libraries.empty());
}

// Test FromJSON with basic configuration
TEST_F(XMakefileConfigTest, FromJSONBasicConfig)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.Name, "TestProject");
    EXPECT_EQ(config.BuildType, "Debug");
    EXPECT_EQ(config.BuildDir, ".build");
    EXPECT_EQ(config.OutputFilename, "test_app");
    EXPECT_EQ(config.Compiler, "g++");
    EXPECT_EQ(config.CCompilerFlags, "-Wall -O2");
    EXPECT_EQ(config.CXXCompilerFlags, "-Wall -O2 -std=c++17");
}

// Test defines extraction
TEST_F(XMakefileConfigTest, DefinesExtraction)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.Defines.size(), 2);
    EXPECT_EQ(config.Defines[0], "DEBUG");
    EXPECT_EQ(config.Defines[1], "VERSION=1.0");
}

// Test include paths extraction
TEST_F(XMakefileConfigTest, IncludePathsExtraction)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.IncludePaths.size(), 2);
    EXPECT_TRUE(config.IncludePaths[0].find("include") != std::string::npos);
    EXPECT_TRUE(config.IncludePaths[1].find("src") != std::string::npos);
}

// Test library paths and libraries
TEST_F(XMakefileConfigTest, LibrariesExtraction)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.LibraryPaths.size(), 1);
    EXPECT_EQ(config.LibraryPaths[0], "/usr/lib");
    
    EXPECT_EQ(config.Libraries.size(), 2);
    EXPECT_EQ(config.Libraries[0], "pthread");
    EXPECT_EQ(config.Libraries[1], "m");
}

// Test source paths and exclude paths
TEST_F(XMakefileConfigTest, SourceAndExcludePathsExtraction)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.SourcePaths.size(), 1);
    EXPECT_TRUE(config.SourcePaths[0].find("src") != std::string::npos);
    
    EXPECT_EQ(config.ExcludePaths.size(), 1);
    EXPECT_TRUE(config.ExcludePaths[0].find("test") != std::string::npos);
    
    EXPECT_EQ(config.ExcludeFiles.size(), 1);
    EXPECT_TRUE(config.ExcludeFiles[0].find("old.cpp") != std::string::npos);
}

// Test commands extraction
TEST_F(XMakefileConfigTest, CommandsExtraction)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.PreBuildCommands.size(), 1);
    EXPECT_TRUE(config.PreBuildCommands[0].find("Building") != std::string::npos);
    
    EXPECT_EQ(config.PostBuildCommands.size(), 1);
    EXPECT_TRUE(config.PostBuildCommands[0].find("Build complete") != std::string::npos);
    
    EXPECT_EQ(config.PreRunCommands.size(), 1);
    EXPECT_TRUE(config.PreRunCommands[0].find("Running") != std::string::npos);
    
    EXPECT_EQ(config.PostRunCommands.size(), 1);
    EXPECT_TRUE(config.PostRunCommands[0].find("Run complete") != std::string::npos);
    
    EXPECT_EQ(config.InstallCommands.size(), 1);
    EXPECT_TRUE(config.InstallCommands[0].find("test_app") != std::string::npos);
    
    EXPECT_EQ(config.UninstallCommands.size(), 1);
    EXPECT_TRUE(config.UninstallCommands[0].find("rm") != std::string::npos);
    
    EXPECT_EQ(config.CleanCommands.size(), 1);
    EXPECT_TRUE(config.CleanCommands[0].find(".build") != std::string::npos);
}

// Test IsParentDirectoryTraversalPath
TEST_F(XMakefileConfigTest, IsParentDirectoryTraversalPath)
{
    EXPECT_TRUE(XMakefileConfig::IsParentDirectoryTraversalPath("../path"));
    EXPECT_TRUE(XMakefileConfig::IsParentDirectoryTraversalPath("../../path"));
    EXPECT_FALSE(XMakefileConfig::IsParentDirectoryTraversalPath("./path"));
    EXPECT_FALSE(XMakefileConfig::IsParentDirectoryTraversalPath("/absolute/path"));
    EXPECT_FALSE(XMakefileConfig::IsParentDirectoryTraversalPath("relative/path"));
}

// Test IsRelativePath
TEST_F(XMakefileConfigTest, IsRelativePath)
{
    EXPECT_TRUE(XMakefileConfig::IsRelativePath("relative/path"));
    EXPECT_TRUE(XMakefileConfig::IsRelativePath("../path"));
    EXPECT_TRUE(XMakefileConfig::IsRelativePath("./path"));
    EXPECT_FALSE(XMakefileConfig::IsRelativePath("/absolute/path"));
}

// Test ResolvePath with relative path
TEST_F(XMakefileConfigTest, ResolvePathRelative)
{
    std::string basePath = "/home/user/project";
    std::string relativePath = "src/main.cpp";
    
    std::string resolved = XMakefileConfig::ResolvePath(relativePath, basePath);
    
    EXPECT_TRUE(resolved.find("project") != std::string::npos);
    EXPECT_TRUE(resolved.find("src") != std::string::npos);
    EXPECT_TRUE(resolved.find("main.cpp") != std::string::npos);
}

// Test ResolvePath with parent directory traversal
TEST_F(XMakefileConfigTest, ResolvePathParentDirectory)
{
    std::string basePath = "/home/user/project/build";
    std::string relativePath = "../src/main.cpp";
    
    std::string resolved = XMakefileConfig::ResolvePath(relativePath, basePath);
    
    EXPECT_TRUE(resolved.find("src") != std::string::npos);
    EXPECT_TRUE(resolved.find("main.cpp") != std::string::npos);
}

// Test ResolvePath with absolute path
TEST_F(XMakefileConfigTest, ResolvePathAbsolute)
{
    std::string basePath = "/home/user/project";
    std::string absolutePath = "/usr/include/header.h";
    
    std::string resolved = XMakefileConfig::ResolvePath(absolutePath, basePath);
    
    EXPECT_EQ(resolved, absolutePath);
}

// Test ResolveLibraryPath with just filename
TEST_F(XMakefileConfigTest, ResolveLibraryPathFilename)
{
    std::string basePath = "/home/user/project";
    std::string library = "pthread";
    
    std::string resolved = XMakefileConfig::ResolveLibraryPath(library, basePath);
    
    EXPECT_EQ(resolved, library);
}

// Test ResolveLibraryPath with path
TEST_F(XMakefileConfigTest, ResolveLibraryPathWithPath)
{
    std::string basePath = "/home/user/project";
    std::string library = "lib/libcustom.a";
    
    std::string resolved = XMakefileConfig::ResolveLibraryPath(library, basePath);
    
    EXPECT_TRUE(resolved.find("lib") != std::string::npos);
    EXPECT_TRUE(resolved.find("libcustom.a") != std::string::npos);
}

// Test OutputDir generation
TEST_F(XMakefileConfigTest, OutputDirGeneration)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_TRUE(config.OutputDir.find(".build") != std::string::npos);
    EXPECT_TRUE(config.OutputDir.find("TestProject") != std::string::npos);
}

// Test OutputDir with only BuildDir
TEST_F(XMakefileConfigTest, OutputDirOnlyBuildDir)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    doc["name"] = "";
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_TRUE(config.OutputDir.find(".build") != std::string::npos);
}

// Test OutputDir with only Name
TEST_F(XMakefileConfigTest, OutputDirOnlyName)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    doc["build_dir"] = "";
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_TRUE(config.OutputDir.find("TestProject") != std::string::npos);
}

// Test empty arrays
TEST_F(XMakefileConfigTest, EmptyArrays)
{
    XMakefileConfig config;
    JsonDocument doc;
    doc["name"] = "Test";
    doc["build_type"] = "Debug";
    doc["build_dir"] = "build";
    doc["output_filename"] = "test";
    doc["compiler_path"] = "/usr/bin/g++";
    doc["compiler"] = "g++";
    doc["c_flags"] = "";
    doc["cxx_flags"] = "";
    doc["linker"] = "g++";
    doc["linker_flags"] = "";
    doc["archiver"] = "ar";
    doc["archiver_flags"] = "";
    
    // Create empty arrays
    doc["defines"].to<JsonArray>();
    doc["include_paths"].to<JsonArray>();
    doc["library_paths"].to<JsonArray>();
    doc["libraries"].to<JsonArray>();
    doc["source_paths"].to<JsonArray>();
    doc["exclude_paths"].to<JsonArray>();
    doc["exclude_files"].to<JsonArray>();
    doc["pre_build_commands"].to<JsonArray>();
    doc["post_build_commands"].to<JsonArray>();
    doc["pre_run_commands"].to<JsonArray>();
    doc["post_run_commands"].to<JsonArray>();
    doc["install_commands"].to<JsonArray>();
    doc["uninstall_commands"].to<JsonArray>();
    doc["clean_commands"].to<JsonArray>();
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_TRUE(config.Defines.empty());
    EXPECT_TRUE(config.IncludePaths.empty());
    EXPECT_TRUE(config.Libraries.empty());
    EXPECT_TRUE(config.PreBuildCommands.empty());
}

// Test with multiple defines
TEST_F(XMakefileConfigTest, MultipleDefines)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    JsonArray defines = doc["defines"].as<JsonArray>();
    defines.add("FEATURE_X");
    defines.add("MAX_SIZE=1024");
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.Defines.size(), 4);
    EXPECT_EQ(config.Defines[2], "FEATURE_X");
    EXPECT_EQ(config.Defines[3], "MAX_SIZE=1024");
}

// Test with multiple source paths
TEST_F(XMakefileConfigTest, MultipleSourcePaths)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    JsonArray sourcePaths = doc["source_paths"].as<JsonArray>();
    sourcePaths.add("lib");
    sourcePaths.add("external");
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.SourcePaths.size(), 3);
}

// Test with environment variable substitution in commands
TEST_F(XMakefileConfigTest, EnvironmentVariableSubstitution)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    JsonArray preBuildCommands = doc["pre_build_commands"].as<JsonArray>();
    preBuildCommands.clear();
    preBuildCommands.add("echo ${name}");
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.PreBuildCommands.size(), 1);
    EXPECT_TRUE(config.PreBuildCommands[0].find("TestProject") != std::string::npos);
}

// Test with output_dir variable in commands
TEST_F(XMakefileConfigTest, OutputDirVariableInCommands)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    JsonArray postBuildCommands = doc["post_build_commands"].as<JsonArray>();
    postBuildCommands.clear();
    postBuildCommands.add("cp ${output_file} /tmp");
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.PostBuildCommands.size(), 1);
    EXPECT_TRUE(config.PostBuildCommands[0].find("test_app") != std::string::npos);
}

// Test linker and archiver settings
TEST_F(XMakefileConfigTest, LinkerAndArchiverSettings)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.Linker, "g++");
    EXPECT_EQ(config.LinkerFlags, "-pthread");
    EXPECT_EQ(config.Archiver, "ar");
    EXPECT_EQ(config.ArchiverFlags, "rcs");
}

// Test compiler path
TEST_F(XMakefileConfigTest, CompilerPath)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_EQ(config.CompilerPath, "/usr/bin/g++");
}

// Test with special characters in strings
TEST_F(XMakefileConfigTest, SpecialCharactersInStrings)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    doc["cxx_flags"] = "-Wall -O2 -DSTRING=\"Hello World\"";
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_TRUE(config.CXXCompilerFlags.find("Hello World") != std::string::npos);
}

// Test empty base path
TEST_F(XMakefileConfigTest, EmptyBasePath)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    config.FromJSON(doc.as<JsonVariant>(), "");
    
    EXPECT_EQ(config.Name, "TestProject");
    EXPECT_FALSE(config.OutputDir.empty());
}

// Test path resolution with empty base path
TEST_F(XMakefileConfigTest, PathResolutionEmptyBase)
{
    std::string resolved = XMakefileConfig::ResolvePath("relative/path", "");
    EXPECT_EQ(resolved, "relative/path");
}

// Test with long paths
TEST_F(XMakefileConfigTest, LongPaths)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    std::string longPath = "very/long/path/to/some/deeply/nested/directory/structure";
    JsonArray includePaths = doc["include_paths"].as<JsonArray>();
    includePaths.add(longPath);
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    EXPECT_TRUE(config.IncludePaths[config.IncludePaths.size() - 1].find("deeply") != std::string::npos);
}

// Test with mixed path separators (forward and back slashes)
TEST_F(XMakefileConfigTest, MixedPathSeparators)
{
    std::string path1 = "path/with/forward/slashes";
    std::string path2 = "path\\with\\back\\slashes";
    
    // Both should be recognized as having path components
    std::string resolved1 = XMakefileConfig::ResolveLibraryPath(path1, testDir);
    std::string resolved2 = XMakefileConfig::ResolveLibraryPath(path2, testDir);
    
    EXPECT_TRUE(resolved1.find("slashes") != std::string::npos);
    EXPECT_TRUE(resolved2.find("slashes") != std::string::npos);
}

// Test PrintEnvironmentVariables (just ensure it doesn't crash)
TEST_F(XMakefileConfigTest, PrintEnvironmentVariables)
{
    XMakefileConfig config;
    JsonDocument doc = createBasicConfigJson();
    
    config.FromJSON(doc.as<JsonVariant>(), testDir);
    
    // This should not crash
    testing::internal::CaptureStdout();
    config.PrintEnvironmentVariables();
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_TRUE(output.find("name=") != std::string::npos);
}
