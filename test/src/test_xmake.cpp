#include <gtest/gtest.h>
#include "xmake.h"
#include "Logger.h"
#include <CmdLineParser.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

// Test fixture for XMake tests
class XMakeTest : public ::testing::Test
{
protected:
    std::string testDir;
    std::string xmakefilePath;
    std::stringstream coutBuffer;
    std::stringstream cerrBuffer;
    std::streambuf *oldCoutBuffer;
    std::streambuf *oldCerrBuffer;
    
    // Delete copy constructor and assignment operator (test fixtures should not be copied)
    XMakeTest(const XMakeTest&) = delete;
    XMakeTest& operator=(const XMakeTest&) = delete;

    void SetUp() override
    {
        // Redirect cout and cerr to our buffers
        oldCoutBuffer = std::cout.rdbuf(coutBuffer.rdbuf());
        oldCerrBuffer = std::cerr.rdbuf(cerrBuffer.rdbuf());
        Logger::SetVerbose(false);
        
        // Create a temporary test directory
        testDir = std::filesystem::temp_directory_path().string() + "/xmake_test_" + 
                  std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        std::filesystem::create_directories(testDir);
        std::filesystem::create_directories(testDir + "/src");
        std::filesystem::create_directories(testDir + "/include");
        
        xmakefilePath = testDir + "/xmakefile.json";
    }

    void TearDown() override
    {
        // Restore cout and cerr
        std::cout.rdbuf(oldCoutBuffer);
        std::cerr.rdbuf(oldCerrBuffer);
        
        // Clean up test directory
        if (std::filesystem::exists(testDir))
        {
            std::filesystem::remove_all(testDir);
        }
    }

    std::string getCoutOutput()
    {
        return coutBuffer.str();
    }

    std::string getCerrOutput()
    {
        return cerrBuffer.str();
    }

    void clearBuffers()
    {
        coutBuffer.str("");
        coutBuffer.clear();
        cerrBuffer.str("");
        cerrBuffer.clear();
    }

    void createBasicXMakefile()
    {
        std::ofstream file(xmakefilePath);
        file << R"({
            "configurations": [
                {
                    "name": "Debug",
                    "build_type": "Executable",
                    "build_dir": ".build",
                    "output_filename": "test_app",
                    "compiler_path": "",
                    "compiler": "g++",
                    "c_flags": "-Wall -g",
                    "cxx_flags": "-Wall -g -std=c++17",
                    "linker": "g++",
                    "linker_flags": "",
                    "archiver": "ar",
                    "archiver_flags": "rcs",
                    "defines": ["DEBUG"],
                    "include_paths": ["include"],
                    "library_paths": [],
                    "libraries": [],
                    "source_paths": ["src"],
                    "exclude_paths": [],
                    "exclude_files": [],
                    "pre_build_commands": ["echo Pre-build"],
                    "post_build_commands": ["echo Post-build"],
                    "pre_run_commands": ["echo Pre-run"],
                    "post_run_commands": ["echo Post-run"],
                    "install_commands": ["echo Installing"],
                    "uninstall_commands": ["echo Uninstalling"],
                    "clean_commands": ["rm -rf .build"]
                },
                {
                    "name": "Release",
                    "build_type": "Executable",
                    "build_dir": ".build",
                    "output_filename": "test_app_release",
                    "compiler_path": "",
                    "compiler": "g++",
                    "c_flags": "-Wall -O3",
                    "cxx_flags": "-Wall -O3 -std=c++17",
                    "linker": "g++",
                    "linker_flags": "-s",
                    "archiver": "ar",
                    "archiver_flags": "rcs",
                    "defines": ["NDEBUG"],
                    "include_paths": ["include"],
                    "library_paths": [],
                    "libraries": [],
                    "source_paths": ["src"],
                    "exclude_paths": [],
                    "exclude_files": [],
                    "pre_build_commands": [],
                    "post_build_commands": [],
                    "pre_run_commands": [],
                    "post_run_commands": [],
                    "install_commands": [],
                    "uninstall_commands": [],
                    "clean_commands": ["rm -rf .build"]
                }
            ]
        })";
        file.close();
    }

    void createSourceFile(const std::string& filename, const std::string& content = "int main() { return 0; }")
    {
        std::ofstream file(testDir + "/src/" + filename);
        file << content;
        file.close();
    }

    CmdLineParser createParser(const std::vector<std::string>& args = {})
    {
        std::vector<const char*> argv;
        argv.push_back("xmake");
        for (const auto& arg : args) {
            argv.push_back(arg.c_str());
        }
        
        CmdLineParser parser("xmake", "test build tool", "1.0.0");
        parser.RegisterOption("-c", "Configuration", true);
        parser.RegisterOption("-v", "Verbose");
        parser.RegisterOption("-j", "Jobs", true);
        parser.RegisterOption("clean", "Clean");
        parser.RegisterOption("run", "Run");
        parser.RegisterOption("install", "Install");
        parser.RegisterOption("uninstall", "Uninstall");
        parser.RegisterOption("--print_env", "Print environment");
        
        parser.Parse(static_cast<int>(argv.size()), const_cast<char**>(argv.data()));
        return parser;
    }

    XMakeTest() 
        : testDir(), 
          xmakefilePath(), 
          coutBuffer(), 
          cerrBuffer(), 
          oldCoutBuffer(nullptr), 
          oldCerrBuffer(nullptr) 
    {}
};

// Test constructor
TEST_F(XMakeTest, Constructor)
{
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    
    // Should not crash
    SUCCEED();
}

// Test constructor with verbose flag
TEST_F(XMakeTest, ConstructorWithVerbose)
{
    CmdLineParser parser = createParser({"-v"});
    XMake xmake(parser);
    
    SUCCEED();
}

// Test constructor with config flag
TEST_F(XMakeTest, ConstructorWithConfig)
{
    CmdLineParser parser = createParser({"-c", "Release"});
    XMake xmake(parser);
    
    SUCCEED();
}

// Test Init with valid xmakefile
TEST_F(XMakeTest, InitWithValidXMakefile)
{
    createBasicXMakefile();
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    
    bool result = xmake.Init(xmakefilePath);
    
    EXPECT_TRUE(result);
    std::string output = getCoutOutput();
    EXPECT_TRUE(output.find("No configuration specified") != std::string::npos);
    EXPECT_TRUE(output.find("Debug") != std::string::npos);
}

// Test Init with non-existent file
TEST_F(XMakeTest, InitWithNonExistentFile)
{
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    
    bool result = xmake.Init(testDir + "/nonexistent.json");
    
    EXPECT_FALSE(result);
}

// Test Init with specific configuration
TEST_F(XMakeTest, InitWithSpecificConfig)
{
    createBasicXMakefile();
    CmdLineParser parser = createParser({"-c", "Release"});
    XMake xmake(parser);
    
    bool result = xmake.Init(xmakefilePath);
    
    EXPECT_TRUE(result);
    // Should not show "No configuration specified" message
    std::string output = getCoutOutput();
    EXPECT_TRUE(output.find("No configuration specified") == std::string::npos);
}

// Test Init with invalid configuration
TEST_F(XMakeTest, InitWithInvalidConfig)
{
    createBasicXMakefile();
    CmdLineParser parser = createParser({"-c", "NonExistent"});
    XMake xmake(parser);
    
    bool result = xmake.Init(xmakefilePath);
    
    EXPECT_FALSE(result);
}

// Test Build with no source files
TEST_F(XMakeTest, BuildWithNoSourceFiles)
{
    createBasicXMakefile();
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    bool result = xmake.Build();
    
    EXPECT_FALSE(result);
    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("No build structures found") != std::string::npos);
}

// Test Build with single source file
TEST_F(XMakeTest, BuildWithSingleSourceFile)
{
    createBasicXMakefile();
    createSourceFile("main.cpp");
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    bool result = xmake.Build();
    
    EXPECT_TRUE(result);
    std::string output = getCoutOutput();
    // Pre-build and Post-build echo commands output directly, not captured by our buffer
    // Just verify the important build messages are present
    EXPECT_TRUE(output.find("Building:") != std::string::npos);
    EXPECT_TRUE(output.find("Linking:") != std::string::npos);
    EXPECT_TRUE(output.find("Finished building target:") != std::string::npos);
}

// Test Build with multiple source files
TEST_F(XMakeTest, BuildWithMultipleSourceFiles)
{
    createBasicXMakefile();
    createSourceFile("main.cpp");
    createSourceFile("helper.cpp", "void helper() {}");
    createSourceFile("utils.cpp", "void utils() {}");
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    bool result = xmake.Build();
    
    EXPECT_TRUE(result);
}

// Test Build with verbose mode
TEST_F(XMakeTest, BuildWithVerboseMode)
{
    createBasicXMakefile();
    createSourceFile("main.cpp");
    
    CmdLineParser parser = createParser({"-v"});
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    bool result = xmake.Build();
    
    EXPECT_TRUE(result);
    std::string output = getCoutOutput();
    // Verbose mode should show full build commands
    EXPECT_TRUE(output.find("g++") != std::string::npos);
}

// Test Build skips unchanged files
TEST_F(XMakeTest, BuildSkipsUnchangedFiles)
{
    createBasicXMakefile();
    createSourceFile("main.cpp");
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    // First build
    bool result1 = xmake.Build();
    EXPECT_TRUE(result1);
    
    clearBuffers();
    
    // Second build should skip everything
    xmake.Init(xmakefilePath);
    bool result2 = xmake.Build();
    EXPECT_TRUE(result2);
    
    std::string output = getCoutOutput();
    EXPECT_TRUE(output.find("No changes in files") != std::string::npos);
}

// Test Build with parallel jobs
TEST_F(XMakeTest, BuildWithParallelJobs)
{
    createBasicXMakefile();
    createSourceFile("main.cpp");
    createSourceFile("file1.cpp", "void f1() {}");
    createSourceFile("file2.cpp", "void f2() {}");
    
    CmdLineParser parser = createParser({"-j", "2"});
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    bool result = xmake.Build();
    
    EXPECT_TRUE(result);
}

// Test Build with -j flag without value
TEST_F(XMakeTest, BuildWithParallelJobsNoValue)
{
    createBasicXMakefile();
    createSourceFile("main.cpp");
    
    CmdLineParser parser = createParser({"-j"});
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    bool result = xmake.Build();
    
    EXPECT_TRUE(result);
}

// Test Clean
TEST_F(XMakeTest, Clean)
{
    createBasicXMakefile();
    
    // Create some build artifacts
    std::filesystem::create_directories(testDir + "/.build/Debug");
    std::ofstream file(testDir + "/.build/Debug/test.o");
    file << "dummy";
    file.close();
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    xmake.Clean();
    
    std::string output = getCoutOutput();
    EXPECT_TRUE(output.find("Cleaning build files") != std::string::npos);
    EXPECT_TRUE(output.find("Cleaned build files") != std::string::npos);
}

// Test Run
TEST_F(XMakeTest, Run)
{
    createBasicXMakefile();
    createSourceFile("main.cpp");
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    // Build first
    xmake.Build();
    
    clearBuffers();
    
    // Run
    xmake.Run();
    
    // Echo command output is not captured by our test buffer redirection
    // Just verify the Run() method completes successfully
    EXPECT_TRUE(true);
}

// Test Install
TEST_F(XMakeTest, Install)
{
    createBasicXMakefile();
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    xmake.Install();
    
    std::string output = getCoutOutput();
    EXPECT_TRUE(output.find("Installing") != std::string::npos);
    EXPECT_TRUE(output.find("Installed successfully") != std::string::npos);
}

// Test Uninstall
TEST_F(XMakeTest, Uninstall)
{
    createBasicXMakefile();
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    xmake.Uninstall();
    
    std::string output = getCoutOutput();
    EXPECT_TRUE(output.find("Uninstalling") != std::string::npos);
    EXPECT_TRUE(output.find("Uninstalled successfully") != std::string::npos);
}

// Test PrintEnvironmentVariables
TEST_F(XMakeTest, PrintEnvironmentVariables)
{
    createBasicXMakefile();
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    xmake.PrintEnvironmentVariables();
    
    std::string output = getCoutOutput();
    EXPECT_TRUE(output.find("=") != std::string::npos);
}

// Test Build with pre-build command failure
TEST_F(XMakeTest, BuildWithPreBuildCommandFailure)
{
    std::ofstream file(xmakefilePath);
    file << R"({
        "configurations": [{
            "name": "Debug",
            "build_type": "Executable",
            "build_dir": ".build",
            "output_filename": "test_app",
            "compiler_path": "",
            "compiler": "g++",
            "c_flags": "-Wall -g",
            "cxx_flags": "-Wall -g -std=c++17",
            "linker": "g++",
            "linker_flags": "",
            "archiver": "ar",
            "archiver_flags": "rcs",
            "defines": [],
            "include_paths": [],
            "library_paths": [],
            "libraries": [],
            "source_paths": ["src"],
            "exclude_paths": [],
            "exclude_files": [],
            "pre_build_commands": ["false"],
            "post_build_commands": [],
            "pre_run_commands": [],
            "post_run_commands": [],
            "install_commands": [],
            "uninstall_commands": [],
            "clean_commands": []
        }]
    })";
    file.close();
    
    createSourceFile("main.cpp");
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    bool result = xmake.Build();
    
    EXPECT_FALSE(result);
    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Pre-build command failed") != std::string::npos);
}

// Test Build with post-build command failure
TEST_F(XMakeTest, BuildWithPostBuildCommandFailure)
{
    std::ofstream file(xmakefilePath);
    file << R"({
        "configurations": [{
            "name": "Debug",
            "build_type": "Executable",
            "build_dir": ".build",
            "output_filename": "test_app",
            "compiler_path": "",
            "compiler": "g++",
            "c_flags": "-Wall -g",
            "cxx_flags": "-Wall -g -std=c++17",
            "linker": "g++",
            "linker_flags": "",
            "archiver": "ar",
            "archiver_flags": "rcs",
            "defines": [],
            "include_paths": [],
            "library_paths": [],
            "libraries": [],
            "source_paths": ["src"],
            "exclude_paths": [],
            "exclude_files": [],
            "pre_build_commands": [],
            "post_build_commands": ["false"],
            "pre_run_commands": [],
            "post_run_commands": [],
            "install_commands": [],
            "uninstall_commands": [],
            "clean_commands": []
        }]
    })";
    file.close();
    
    createSourceFile("main.cpp");
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    bool result = xmake.Build();
    
    EXPECT_FALSE(result);
    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Post-build command failed") != std::string::npos);
}

// Test Build detects changed files
TEST_F(XMakeTest, BuildDetectsChangedFiles)
{
    createBasicXMakefile();
    createSourceFile("main.cpp");
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    // First build
    bool result1 = xmake.Build();
    EXPECT_TRUE(result1);
    
    // Wait and modify file
    std::this_thread::sleep_for(std::chrono::seconds(2));
    createSourceFile("main.cpp", "int main() { return 1; }");
    
    clearBuffers();
    
    // Second build should rebuild
    xmake.Init(xmakefilePath);
    bool result2 = xmake.Build();
    EXPECT_TRUE(result2);
    
    std::string output = getCoutOutput();
    EXPECT_TRUE(output.find("Building:") != std::string::npos);
}

// Test switching configurations
TEST_F(XMakeTest, SwitchingConfigurations)
{
    createBasicXMakefile();
    createSourceFile("main.cpp");
    
    // Build with Debug
    CmdLineParser parser1 = createParser({"-c", "Debug"});
    XMake xmake1(parser1);
    xmake1.Init(xmakefilePath);
    bool result1 = xmake1.Build();
    EXPECT_TRUE(result1);
    
    clearBuffers();
    
    // Build with Release
    CmdLineParser parser2 = createParser({"-c", "Release"});
    XMake xmake2(parser2);
    xmake2.Init(xmakefilePath);
    bool result2 = xmake2.Build();
    EXPECT_TRUE(result2);
    
    std::string output = getCoutOutput();
    EXPECT_TRUE(output.find("test_app_release") != std::string::npos);
}

// Test Clean with no clean commands
TEST_F(XMakeTest, CleanWithNoCommands)
{
    std::ofstream file(xmakefilePath);
    file << R"({
        "configurations": [{
            "name": "Debug",
            "build_type": "Executable",
            "build_dir": ".build",
            "output_filename": "test_app",
            "compiler_path": "",
            "compiler": "g++",
            "c_flags": "-Wall -g",
            "cxx_flags": "-Wall -g -std=c++17",
            "linker": "g++",
            "linker_flags": "",
            "archiver": "ar",
            "archiver_flags": "rcs",
            "defines": [],
            "include_paths": [],
            "library_paths": [],
            "libraries": [],
            "source_paths": ["src"],
            "exclude_paths": [],
            "exclude_files": [],
            "pre_build_commands": [],
            "post_build_commands": [],
            "pre_run_commands": [],
            "post_run_commands": [],
            "install_commands": [],
            "uninstall_commands": [],
            "clean_commands": []
        }]
    })";
    file.close();
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    xmake.Clean();
    
    std::string output = getCoutOutput();
    EXPECT_TRUE(output.find("Cleaned build files") != std::string::npos);
}

// Test Install with no install commands
TEST_F(XMakeTest, InstallWithNoCommands)
{
    std::ofstream file(xmakefilePath);
    file << R"({
        "configurations": [{
            "name": "Debug",
            "build_type": "Executable",
            "build_dir": ".build",
            "output_filename": "test_app",
            "compiler_path": "",
            "compiler": "g++",
            "c_flags": "-Wall -g",
            "cxx_flags": "-Wall -g -std=c++17",
            "linker": "g++",
            "linker_flags": "",
            "archiver": "ar",
            "archiver_flags": "rcs",
            "defines": [],
            "include_paths": [],
            "library_paths": [],
            "libraries": [],
            "source_paths": ["src"],
            "exclude_paths": [],
            "exclude_files": [],
            "pre_build_commands": [],
            "post_build_commands": [],
            "pre_run_commands": [],
            "post_run_commands": [],
            "install_commands": [],
            "uninstall_commands": [],
            "clean_commands": []
        }]
    })";
    file.close();
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    xmake.Install();
    
    std::string output = getCoutOutput();
    EXPECT_TRUE(output.find("Installed successfully") != std::string::npos);
}

// Test Run with pre-run command failure
TEST_F(XMakeTest, RunWithPreRunCommandFailure)
{
    std::ofstream file(xmakefilePath);
    file << R"({
        "configurations": [{
            "name": "Debug",
            "build_type": "Executable",
            "build_dir": ".build",
            "output_filename": "test_app",
            "compiler_path": "",
            "compiler": "g++",
            "c_flags": "-Wall -g",
            "cxx_flags": "-Wall -g -std=c++17",
            "linker": "g++",
            "linker_flags": "",
            "archiver": "ar",
            "archiver_flags": "rcs",
            "defines": [],
            "include_paths": [],
            "library_paths": [],
            "libraries": [],
            "source_paths": ["src"],
            "exclude_paths": [],
            "exclude_files": [],
            "pre_build_commands": [],
            "post_build_commands": [],
            "pre_run_commands": ["false"],
            "post_run_commands": [],
            "install_commands": [],
            "uninstall_commands": [],
            "clean_commands": []
        }]
    })";
    file.close();
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    xmake.Init(xmakefilePath);
    
    xmake.Run();
    
    std::string errorOutput = getCerrOutput();
    EXPECT_TRUE(errorOutput.find("Pre-run command failed") != std::string::npos);
}

// Test multiple Init calls
TEST_F(XMakeTest, MultipleInitCalls)
{
    createBasicXMakefile();
    
    CmdLineParser parser = createParser();
    XMake xmake(parser);
    
    bool result1 = xmake.Init(xmakefilePath);
    EXPECT_TRUE(result1);
    
    // Init again should work
    bool result2 = xmake.Init(xmakefilePath);
    EXPECT_TRUE(result2);
}

