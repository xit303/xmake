# XMakefile JSON Format Documentation

## Overview

The `xmakefile.json` is a JSON-based configuration file that defines how to build C/C++ projects using the `xmake` build tool. It provides a declarative way to specify compiler settings, source paths, dependencies, and build commands without writing traditional Makefiles.

## File Structure

The xmakefile.json contains a root object with a `configurations` array. Each configuration represents a different build variant (e.g., Debug, Release, Test) with its own settings.

```json
{
    "configurations": [
        {
            "name": "Debug",
            ...
        },
        {
            "name": "Release",
            ...
        }
    ]
}
```

## Configuration Fields

Each configuration object supports the following fields:

### Basic Configuration

#### `name` (string, required)
The name of the build configuration. Used with the `-c` flag to select which configuration to build.

**Example:**
```json
"name": "Debug"
```

#### `build_type` (string, required)
Specifies the type of output to produce. Valid values:
- `"Executable"` - Creates a standalone executable binary
- `"StaticLibrary"` - Creates a static library (.a file)
- `"SharedLibrary"` - Creates a shared/dynamic library (.so/.dll file)

**Example:**
```json
"build_type": "Executable"
```

#### `build_dir` (string, required)
The directory where build artifacts (object files, dependency files) will be placed. Relative paths are resolved from the xmakefile.json location.

**Example:**
```json
"build_dir": ".bin"
```

#### `output_filename` (string, required)
The name of the final output binary/library file (without path). An extension will be added based on the `build_type` and platform. Therefore this should not include an extension.

**Example:**
```json
"output_filename": "xmake"
```

### Compiler Configuration

#### `compiler_path` (string, optional)
Path to the compiler directory. If empty, the compiler will be searched in the system PATH.

**Example:**
```json
"compiler_path": "/usr/bin"
```
or leave empty:
```json
"compiler_path": ""
```

#### `compiler` (string, required)
The compiler executable to use for C++ files.

**Example:**
```json
"compiler": "g++"
```

#### `c_flags` (string, optional)
Compiler flags specifically for C source files (.c).

**Example:**
```json
"c_flags": "-std=c11 -Wall -Wextra -pedantic"
```

#### `cxx_flags` (string, optional)
Compiler flags for C++ source files (.cpp, .cc, .cxx, .m, .mm).

**Example:**
```json
"cxx_flags": "-g -std=c++23 -Wall -Wextra -pedantic -ffunction-sections -fdata-sections -MMD -O0"
```

Common flags:
- `-g` - Include debug symbols
- `-std=c++23` - Set C++ standard version
- `-Wall -Wextra -pedantic` - Enable warnings
- `-ffunction-sections -fdata-sections` - Place each function/data in separate sections for better linking
- `-MMD` - Generate dependency files
- `-O0` / `-Os` / `-O3` - Optimization levels

### Linker Configuration

#### `linker` (string, required)
The linker executable to use. Typically the same as the compiler for executables.

**Example:**
```json
"linker": "g++"
```

#### `linker_flags` (string, optional)
Additional flags passed to the linker.

**Example:**
```json
"linker_flags": "-lm -lstdc++"
```

#### `archiver` (string, required for StaticLibrary)
The archiver tool for creating static libraries. Only used when `build_type` is `"StaticLibrary"`.

**Example:**
```json
"archiver": "ar"
```

#### `archiver_flags` (string, optional)
Flags for the archiver tool. Common value is `"rcs"` (replace, create, index).

**Example:**
```json
"archiver_flags": "rcs"
```

### Preprocessor Defines

#### `defines` (array of strings, optional)
Preprocessor definitions to pass to the compiler. Must be specified without the `-D` prefix.

**Example:**
```json
"defines": [
    "DEBUG",
    "VERSION=1.0"
]
```

### Paths Configuration

#### `include_paths` (array of strings, required)
List of directories to search for header files. These are passed to the compiler as `-I` flags. The tool recursively searches these directories for headers and creates a list of all included headers for dependency tracking. If you want to exclude certain headers from being tracked, use the `exclude_paths` and `exclude_files` options.

**Example:**
```json
"include_paths": [
    "include",
    "lib/CommandLineParser/include",
    "lib/ArduinoJson/src"
]
```

#### `source_paths` (array of strings, required)
List of directories containing source files to compile. The tool recursively searches these directories for `.c`, `.cpp`, `.cc`, `.cxx`, `.m`, and `.mm` files. If you want to exclude certain source files or directories from being compiled, use the `exclude_paths` and `exclude_files` options.

**Example:**
```json
"source_paths": [
    "src/",
    "lib/CommandLineParser/src"
]
```

#### `library_paths` (array of strings, optional)
List of directories to search for libraries. These are passed to the linker as `-L` flags.

**Example:**
```json
"library_paths": [
    "/usr/local/lib"
]
```

#### `libraries` (array of strings, optional)
List of libraries to link against. Can be specified as:
- Library names (without `lib` prefix or extension) - will be passed as `-l<name>`
- Absolute paths to library files - will be passed as-is

**Example:**
```json
"libraries": [
    "pthread",
    "m",
    "/usr/local/lib/libcustom.a"
]
```

### Exclusion Filters

#### `exclude_paths` (array of strings, optional)
List of directories to exclude from the build.

**Example:**
```json
"exclude_paths": [
    "test/",
    "deprecated/"
]
```

#### `exclude_files` (array of strings, optional)
List of specific filenames to exclude from the build (matched by filename or full path).

**Example:**
```json
"exclude_files": [
    "old_main.cpp",
    "test_util.c"
]
```

### Build Commands

#### `pre_build_commands` (array of strings, optional)
Shell commands to execute before the build starts.

**Example:**
```json
"pre_build_commands": [
    "echo 'Starting build...'",
    "mkdir -p logs"
]
```

#### `post_build_commands` (array of strings, optional)
Shell commands to execute after the build completes successfully.

**Example:**
```json
"post_build_commands": [
    "echo 'Build completed!'",
    "strip ${output_file}"
]
```

#### `pre_run_commands` (array of strings, optional)
Shell commands to execute before running the built executable (when using `xmake run`).

**Example:**
```json
"pre_run_commands": [
    "echo 'Running...'"
]
```

#### `post_run_commands` (array of strings, optional)
Shell commands to execute after running the built executable.

**Example:**
```json
"post_run_commands": [
    "echo 'Run completed!'"
]
```

#### `install_commands` (array of strings, required for install)
Shell commands to execute when running `xmake install`. Typically copies the built binary and headers to system locations.

**Example:**
```json
"install_commands": [
    "mkdir -p /usr/local/bin",
    "cp ${output_file} /usr/local/bin/",
    "mkdir -p /usr/include/xit",
    "cp -r include/ /usr/include/xit/"
]
```

#### `uninstall_commands` (array of strings, required for uninstall)
Shell commands to execute when running `xmake uninstall`.

**Example:**
```json
"uninstall_commands": [
    "rm -f /usr/local/bin/${output_filename}"
]
```

#### `clean_commands` (array of strings, required for clean)
Shell commands to execute when running `xmake clean`. Typically removes the build directory.

**Example:**
```json
"clean_commands": [
    "rm -rf ${build_dir}"
]
```

## Variable Substitution

Commands support variable substitution for the following variables:
- `${build_dir}` - The build directory path
- `${output_dir}` - The output directory path (typically build_dir/config_name)
- `${output_file}` - Full path to the output binary/library
- `${output_filename}` - Just the filename of the output

**Example:**
```json
"post_build_commands": [
    "ls -lh ${output_file}",
    "cp ${output_file} backups/"
]
```

The environment variables can be printed using `xmake --print_env` to see their resolved values.

## Complete Example

Here's a complete example with both Debug and Release configurations:

```json
{
    "configurations": [
        {
            "name": "Debug",
            "build_type": "Executable",
            "build_dir": ".bin",
            "output_filename": "myapp",
            "compiler_path": "",
            "compiler": "g++",
            "linker": "g++",
            "archiver": "ar",
            "archiver_flags": "rcs",
            "c_flags": "-std=c11 -Wall -Wextra -pedantic",
            "cxx_flags": "-g -std=c++23 -Wall -Wextra -pedantic -ffunction-sections -fdata-sections -MMD -O0",
            "linker_flags": "-lm -lstdc++",
            "defines": [
                "DEBUG",
                "ENABLE_LOGGING"
            ],
            "include_paths": [
                "include",
                "lib/external/include"
            ],
            "source_paths": [
                "src/"
            ],
            "exclude_paths": [
                "*test*"
            ],
            "exclude_files": [],
            "library_paths": [],
            "libraries": [],
            "pre_build_commands": [],
            "post_build_commands": [],
            "pre_run_commands": [
                "echo 'Starting application...'"
            ],
            "post_run_commands": [
                "echo 'Application finished.'"
            ],
            "install_commands": [
                "mkdir -p /usr/local/bin",
                "cp ${output_file} /usr/local/bin/"
            ],
            "uninstall_commands": [
                "rm -f /usr/local/bin/${output_filename}"
            ],
            "clean_commands": [
                "rm -rf ${build_dir}"
            ]
        },
        {
            "name": "Release",
            "build_type": "Executable",
            "build_dir": ".bin",
            "output_filename": "myapp",
            "compiler_path": "",
            "compiler": "g++",
            "linker": "g++",
            "archiver": "ar",
            "archiver_flags": "rcs",
            "c_flags": "-std=c11 -Wall -Wextra -pedantic",
            "cxx_flags": "-std=c++23 -Wall -Wextra -pedantic -ffunction-sections -fdata-sections -MMD -Os",
            "linker_flags": "-lm -lstdc++",
            "defines": [],
            "include_paths": [
                "include",
                "lib/external/include"
            ],
            "source_paths": [
                "src/"
            ],
            "exclude_paths": [
                "*test*"
            ],
            "exclude_files": [],
            "library_paths": [],
            "libraries": [],
            "pre_build_commands": [],
            "post_build_commands": [
                "strip ${output_file}"
            ],
            "pre_run_commands": [],
            "post_run_commands": [],
            "install_commands": [
                "mkdir -p /usr/local/bin",
                "cp ${output_file} /usr/local/bin/"
            ],
            "uninstall_commands": [
                "rm -f /usr/local/bin/${output_filename}"
            ],
            "clean_commands": [
                "rm -rf ${build_dir}"
            ]
        }
    ]
}
```

## How xmake Processes the Configuration

1. **Parsing**: xmake loads and parses the xmakefile.json file
2. **Configuration Selection**: Selects the first configuration by default, or the one specified with `-c`
3. **File Discovery**: Recursively scans `source_paths` for source files, applying exclusion filters
4. **Dependency Checking**: Compares file modification times with previous build to determine what needs rebuilding
5. **Build String Generation**: Creates compiler command lines for each source file with appropriate flags and includes
6. **Parallel Compilation**: Compiles source files in parallel (controlled by `-j` flag)
7. **Linking**: Links object files into the final executable/library using the linker configuration
8. **Commands Execution**: Runs any post-build, install, or other commands as requested

## Build Optimization

xmake implements intelligent incremental builds:

- **Full Rebuild**: Triggered when header files change
- **Source Rebuild**: Triggered when source files change
- **Link Only**: Triggered when libraries change but source files are unchanged
- **No Rebuild**: When no files have been modified since last build

Build times are tracked in `${build_dir}/build_times.txt` and compared on subsequent builds.

## Best Practices

1. **Use Separate Configurations**: Create distinct Debug and Release configurations with appropriate optimization levels
2. **Organize Includes**: Keep project headers in `include/` and third-party headers in `lib/*/include/`
3. **Set Proper Flags**: Use `-MMD` for automatic dependency tracking
4. **Exclude Tests**: Use `exclude_paths` to keep test code out of production builds
5. **Variable Substitution**: Use `${variable}` syntax in commands for portability
6. **Relative Paths**: Prefer relative paths to make the build portable across systems
7. **Clean Commands**: Always provide clean commands to fully remove build artifacts

## Differences from Traditional Makefiles

- **Declarative**: Specify what to build, not how
- **Automatic Discovery**: Source files are discovered automatically from specified paths
- **JSON Format**: Easy to parse and generate programmatically
- **Cross-Platform Ready**: Path handling works on different operating systems
- **Incremental Builds**: Built-in dependency tracking and intelligent rebuild detection
- **Multiple Configurations**: Easy to switch between Debug/Release/Test builds 