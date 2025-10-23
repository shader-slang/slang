# Standard Modules Configuration

This document explains the centralized configuration system for Slang's standard modules (currently the neural module).

## Overview

The standard modules configuration is centralized to avoid hardcoding paths in multiple locations. This makes it easier to maintain and add new standard modules in the future. Currently, the only standard module is the neural module, but this structure is designed to accommodate additional modules.

## Configuration Variables

The following CMake variables control the standard modules configuration:

- `SLANG_STANDARD_MODULE_DIR_NAME` (default: "slang-standard-module-${SLANG_VERSION_FULL}")
  - The directory name for all standard modules relative to libslang location
  - Includes the Slang version number for version-specific module isolation

- `SLANG_NEURAL_MODULE_FILE_NAME` (default: "neural.slang-module")
  - The file name for the compiled neural module

- `SLANG_STANDARD_MODULE_INSTALL_DIR`
  - **Windows**: `bin/slang-standard-module-${SLANG_VERSION_FULL}` (installed next to slang.dll)
  - **Linux/Mac**: `lib/slang-standard-module-${SLANG_VERSION_FULL}` (installed next to libslang.so/libslang.dylib)
  - The installation directory for all standard modules in release packages

## Directory Structure

```
source/standard-modules/
├── CMakeLists.txt                          # Central configuration for all modules
├── slang-standard-module-config.h.in      # Configuration template
├── README.md                               # This file
└── neural/                                 # Neural module subdirectory
    ├── CMakeLists.txt                      # Neural module build logic
    ├── neural.slang                        # Neural module entry point
    └── *.slang                             # Other neural module files
```

## Files Involved

### Configuration Template
- `source/standard-modules/slang-standard-module-config.h.in` - Template header file with CMake variables

### Generated Files
- `build/source/standard-modules/slang-standard-module-config-header/slang-standard-module-config.h` - Generated configuration header (internal only)

### CMake Files
- `source/standard-modules/CMakeLists.txt` - Defines configuration variables for all standard modules and generates the header
- `source/standard-modules/neural/CMakeLists.txt` - Neural module specific build logic
- `source/slang/CMakeLists.txt` - Uses the standard module config header internally for the slang library

### C++ Code
- `source/slang/slang-session.cpp` - Uses the configuration constants to locate standard modules at runtime

## How to Modify

To change the standard module paths:

1. **For development/testing**: Modify the default values in `source/standard-modules/CMakeLists.txt`
2. **For users**: Set the CMake cache variables when configuring:
   ```bash
   cmake -DSLANG_STANDARD_MODULE_DIR_NAME="my-custom-path" ...
   ```

## Build Process

1. CMake configures `slang-standard-module-config.h.in` → `slang-standard-module-config.h` (internal build directory)
2. The slang library includes the header directory privately (not exposed in public API)
3. C++ code includes `slang-standard-module-config.h` and uses the constants internally
4. Each module's CMakeLists.txt uses the same variables for consistent paths
5. Standard modules are compiled using `slangc` at build time
6. Standard modules are placed in the same directory as libslang.so/slang.dll:
   - **Build - Windows**: `build/Debug/bin/slang-standard-module-${SLANG_VERSION_FULL}/`
   - **Build - Linux/Mac**: `build/Debug/lib/slang-standard-module-${SLANG_VERSION_FULL}/`
   - **Install - Windows**: `<prefix>/bin/slang-standard-module-${SLANG_VERSION_FULL}/`
   - **Install - Linux/Mac**: `<prefix>/lib/slang-standard-module-${SLANG_VERSION_FULL}/`

This ensures that both the C++ runtime search logic and the CMake build logic use exactly the same path configuration, while keeping the implementation details internal to the slang library.

The standard modules are automatically co-located with the slang library for easy discovery at runtime.

## Cross-Compilation Support

When cross-compiling (e.g., building ARM64 binaries on an x86_64 host), the standard modules need to be compiled using a host-platform compiler, since the target-platform `slangc` cannot run on the build host.

The build system automatically uses `SLANG_GENERATORS_PATH` to locate `slang-bootstrap`, a standalone Slang compiler with no external dependencies:

1. First, build the generators for the host platform and install them:
   ```bash
   cmake --workflow --preset generators --fresh
   cmake --install build --config Release --component generators --prefix build-platform-generators
   ```

   This installs `slang-bootstrap` along with other generator tools.

2. Then, configure the cross-compilation build with `SLANG_GENERATORS_PATH`:
   ```bash
   cmake --preset default --fresh \
     -DSLANG_GENERATORS_PATH=build-platform-generators/bin \
     -DCMAKE_C_COMPILER=your-cross-compiler \
     -DCMAKE_CXX_COMPILER=your-cross-compiler++
   ```

3. Build normally - the standard modules will use the host-platform `slang-bootstrap`:
   ```bash
   cmake --build --preset release
   ```

The build system automatically:
- Detects when `SLANG_GENERATORS_PATH` is set
- Uses `slang-bootstrap` from that path (a standalone tool with no dependencies)
- Falls back to `slangc` for normal (non-cross-compilation) builds

This is the same pattern used in the release workflow for cross-compilation scenarios.

## Adding New Standard Modules

To add a new standard module:

1. Create a new subdirectory under `source/standard-modules/` (e.g., `source/standard-modules/mymodule/`)
2. Add a CMakeLists.txt in the new directory following the pattern in `neural/CMakeLists.txt`
3. Use `${SLANG_STANDARD_MODULE_DIR_NAME}` for output directory consistency
4. Add `add_subdirectory(mymodule)` to `source/standard-modules/CMakeLists.txt`
5. If needed, add module-specific configuration variables to the parent CMakeLists.txt
