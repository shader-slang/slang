# Neural Module Configuration

This document explains the centralized configuration system for the neural module paths in Slang.

## Overview

The neural module configuration is now centralized to avoid hardcoding paths in multiple locations. This makes it easier to maintain and modify the neural module directory structure in the future.

## Configuration Variables

The following CMake variables control the neural module configuration:

- `SLANG_NEURAL_MODULE_DIR_NAME` (default: "slang-neural-module")
  - The directory name for the neural module relative to libslang location

- `SLANG_NEURAL_MODULE_FILE_NAME` (default: "neural.slang-module") 
  - The file name for the compiled neural module

- `SLANG_NEURAL_MODULE_INSTALL_DIR`
  - **Windows**: `bin/slang-neural-module` (installed next to slang.dll)
  - **Linux/Mac**: `lib/slang-neural-module` (installed next to libslang.so/libslang.dylib)
  - The installation directory for the neural module in release packages

## Files Involved

### Configuration Template
- `source/slang-neural-module/slang-neural-config.h.in` - Template header file with CMake variables

### Generated Files
- `build/source/slang-neural-module/slang-neural-config-header/slang-neural-config.h` - Generated configuration header (internal only)

### CMake Files
- `source/slang-neural-module/CMakeLists.txt` - Defines configuration variables and generates the header
- `source/slang/CMakeLists.txt` - Uses the neural config header internally for the slang library

### C++ Code
- `source/slang/slang-session.cpp` - Uses the configuration constants

## How to Modify

To change the neural module paths:

1. **For development/testing**: Modify the default values in `source/slang-neural-module/CMakeLists.txt`
2. **For users**: Set the CMake cache variables when configuring:
   ```bash
   cmake -DSLANG_NEURAL_MODULE_DIR_NAME="my-neural-module" ...
   ```

## Build Process

1. CMake configures `slang-neural-config.h.in` → `slang-neural-config.h` (internal build directory)
2. The slang library includes the header directory privately (not exposed in public API)
3. C++ code includes `slang-neural-config.h` and uses the constants internally
4. Neural module CMakeLists.txt uses the same variables for consistent paths
5. Neural module is placed in the same directory as libslang.so/slang.dll:
   - **Build - Windows**: `build/Debug/bin/slang-neural-module/`
   - **Build - Linux/Mac**: `build/Debug/lib/slang-neural-module/`
   - **Install - Windows**: `<prefix>/bin/slang-neural-module/`
   - **Install - Linux/Mac**: `<prefix>/lib/slang-neural-module/`

This ensures that both the C++ runtime search logic and the CMake build logic use exactly the same path configuration, while keeping the implementation details internal to the slang library.

The neural module is automatically co-located with the slang library for easy discovery at runtime.
