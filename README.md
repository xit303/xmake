# xmake

[![CI](https://github.com/xit303/xmake/actions/workflows/ci.yml/badge.svg)](https://github.com/xit303/xmake/actions/workflows/ci.yml)

This repository contains a small C/C++ build tool named `xmake` (not the popular xmake.io project). It reads `xmakefile.json` and builds targets via a simple Makefile. This README explains how to download, build, install, and use it on Linux.

## Overview

- Purpose: Provide a lightweight alternative to hand-written Makefiles for C/C++ projects.
- Key files: `makefile`, `xmakefile.json`, sources under `src/` and headers under `include/`.
- Outputs: A CLI binary `xmake` that parses `xmakefile.json` and drives builds.

A detailed description of the `xmakefile.json` format and available fields can be found in the `xmakefile.json` documentation [here](XMAKEFILE.md).

## Requirements

- Linux with `bash` (default shell).
- Build tools: `g++`, `make`.
- Git for cloning the repository.

Install typical prerequisites on Debian/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y build-essential git
```

Fedora/RHEL:

```bash
sudo dnf install -y @development-tools git
```

Arch:

```bash
sudo pacman -S --needed base-devel git
```

## Download

Clone the repository recursive to get submodules:

```bash
git clone --recursive https://github.com/xit303/xmake.git
cd xmake
```

## Build using the makefile

This project (the build tool itself) must be compiled with the provided `makefile` first. There is no self-hosted build before the binary exists. The default build is a debug build, there is no release build in the makefile. To create a release build, follow the instructions below.

Primary targets defined in `makefile`:

- (default): builds `./.bin/xmake` using C++23 with `-DDEBUG` and optimization `-Os`.
- `info`: prints discovered source, object, and dependency lists.
- `install`: copies the built binary to `/usr/local/bin` (uses `sudo`).
- `clean`: removes the `.bin` directory and dependency files.

Pattern/object rules populate `./.bin/Debug/` with object and dependency files; the linked binary ends up at `./.bin/xmake`.

Notes:
- Invoking `make` directly builds the debug binary. There is no release version yet.
- Set an alternate compiler via `CC=clang CXX=clang++ make` if desired.

Build the tool in parallel:

```bash
make -s -j
```

This creates the binary at `./.bin/xmake`.

## Install

To install system-wide:

```bash
# Install into /usr/local/bin (requires sudo)
sudo make install
```

The `install` rule is hardcoded to `/usr/local/bin`. If you want to install to a different location, you will need to modify the `makefile`.

After installing, you can run `xmake` from any directory.

If no `install` rule exists in your environment, you can copy the built binary manually:

```bash
# Example if the binary is at ./.bin/xmake
install -Dm755 ./.bin/xmake "$HOME/.local/bin/xmake"
```

And add the destination to your `PATH` if needed:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

## Usage

After building (and optionally installing) `xmake`, you can use it to build C/C++ projects defined by `xmakefile.json`. To get you started, try the following command in this repository:

With the built binary at `./.bin/xmake`, run:

```bash
./.bin/xmake
```

This will read `xmakefile.json` in the current directory and build the defined targets.

If installed system-wide, simply run:

```bash
xmake
```

If no specific `xmakefile.json` is provided, it looks for one in the current working directory and the default configuration (first one in the `xmakefile.json`) is used unless overridden by the options below.

### Options

To see available options:

```bash
xmake -h
```

- `-h`: Show help message.
- `--version`: Show version information.
- `-c <config>`: Specify which configuration to use (default: first one in `xmakefile.json`).
- `-v`: Enable verbose output.
- `-j <num>`: Number of jobs to run simultaneously.
- `--print_env`: Print environment variables.
- `clean`: Clean all build files (requires `clean_commands` in `xmakefile.json`).
- `run`: Run the output file after building.
- `install`: Install the output file.
- `uninstall`: Uninstall the output file.

### Clean build files

To clean all build files as defined in `xmakefile.json`, use:

```bash
./.bin/xmake clean
```

or

```bash
xmake clean
```

This will delete the build directory and execute the `clean_commands` specified in the `xmakefile.json`. More on the `clean_commands` field can be found in the `xmakefile.json` documentation [here](XMAKEFILE.md).

### Using the verbose option

To enable verbose output during the build process, use the `-v` option:

```bash
xmake -v
```

This will print detailed information about the build steps being executed, as well as any commands run by `xmake`.

### Specifying number of jobs

To specify the number of jobs to run simultaneously during the build process, use the `-j` option followed by the desired number:

```bash
xmake -j 4
```

This will allow `xmake` to run up to 4 jobs in parallel, speeding up the build process on multi-core systems. By default, `xmake` will use all available CPU cores. In my opinion, it's better to always compile with all cores unless you have a specific reason not to (e.g., system load management or many compilation errors).

### Using a specific xmakefile

To use a specific `xmakefile.json`, provide its path as an argument:

```bash
xmake path/to/xmakefile.json
```

This will build the targets defined in that file. The build will occur in the directory containing the specified `xmakefile.json`.

### Using configurations

You can define multiple configurations in `xmakefile.json` (e.g., Debug, Release). Use the `-c` option to select one:

```bash
xmake -c Release
```

or

```bash
xmake -c Test
```

if you have a `Test` configuration defined.

### Running the built output

To run the output file after building, use the `run` option:

```bash
xmake run
```

This will execute the built binary defined in the `xmakefile.json`. In case of running tests, it will execute the test binary.

### Installing and uninstalling the built output

This basically runs the commands defined in the `install_commands` and `uninstall_commands` fields of the `xmakefile.json`. To install the built output, use:

```bash
xmake install
```

To uninstall the built output, use:

```bash
xmake uninstall
```

Please note that these commands may require elevated permissions depending on where the output is being installed. If necessary, run them with `sudo`.

### Printing environment variables

To print the environment variables used during the build process, use the `--print_env` option:

```bash
xmake --print_env
```

This will display all relevant environment variables that `xmake` uses for compiling and linking.

```bash
No configuration specified, using [Debug]
build_dir=/home/xit/workspace/xmake/.bin
name=Debug
output_dir=/home/xit/workspace/xmake/.bin/Debug
output_file=/home/xit/workspace/xmake/.bin/Debug/xmake
output_filename=/home/xit/workspace/xmake/xmake
```

## Testing

This repository includes a `test/` directory with its own `xmakefile.json`. To run the tests, you can use xmake directly after building it like so:

```bash
.bin/xmake test/xmakefile.json && .bin/xmake test/xmakefile.json run
```

or

```bash
xmake test/xmakefile.json && xmake test/xmakefile.json run
```

This will invoke `xmake` within the `test/` directory.

If you add unit tests, prefer Google Test as per project standards. Write tests under `test/src/`. There is no need to modify the `xmakefile.json` when adding tests, as the test `xmakefile.json` will read all source files under `test/src/` automatically.

## Troubleshooting

- Missing headers or libraries: verify `includeDirs` and `ldflags` in `xmakefile.json`.
- Compiler not found: ensure `g++` is installed and on `PATH`.
- Permission issues on install: use `sudo`.
- Wrong binary path: check where the `makefile` places outputs; adjust commands accordingly.

## Contributing

- Follow the coding standards in `.github/copilot-instructions.md` (naming, error handling, testing).
- Use conventional commits (e.g., `feat(core): add new parser option`).
- Add unit tests for new components and utilities.

## License

See `LICENSE` for details.
