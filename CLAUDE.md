# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Essential Build Commands

**Configure and build (recommended):**
```bash
cmake --preset default
cmake --build --preset releaseWithDebugInfo  # or --preset debug, or --preset release
```

**Alternative Visual Studio workflow:**
```bash
cmake --preset vs2022  # or vs2019
cmake --build --preset releaseWithDebugInfo
```

**Quick release build and package:**
```bash
cmake --workflow --preset release
```

**Build only generators for cross-compilation:**
```bash
cmake --workflow --preset generators --fresh
```

## Testing Commands

**Run all tests:**
```bash
build/Release/bin/slang-test
# or for debug builds:
build/Debug/bin/slang-test
```

**Run specific test categories:**
```bash
slang-test -category smoke    # Basic smoke tests
slang-test -category quick    # Quick tests  
slang-test -category full     # Full test suite
slang-test -category compute  # Compute shader tests
```

**Run tests with parallel execution:**
```bash
slang-test -use-test-server -server-count 8
```

**Run specific API tests:**
```bash
slang-test -api vk           # Vulkan only
slang-test -api dx12         # DirectX 12 only
```

**Full CI-style test run:**
```bash
slang-test -use-test-server -server-count 8 -category full -expected-failure-list tests/expected-failure-github.txt
```

**Run single test:**
```bash
slang-test tests/compute/array-param
```

## Core Architecture Overview

### Main Compilation Pipeline
- **Front-end**: Source → AST → IR (with semantic checking and lowering)
- **Back-end**: IR → Optimized IR → Target Code Generation

### Key Source Directories

**`source/core/`** - Foundation utilities, platform abstraction, collections, memory management

**`source/compiler-core/`** - Shared compiler infrastructure
- Lexical analysis, diagnostics, source locations
- Downstream compiler integration (DXC, FXC, GLSLANG)
- Artifact management and JSON utilities for tooling

**`source/slang/`** - Main compiler implementation
- `slang-parser.cpp` - Recursive descent parser
- `slang-ast-*.cpp` - Abstract Syntax Tree and type system
- `slang-check-*.cpp` - Semantic checking and type checking
- `slang-ir*.cpp` - Intermediate representation and optimization passes
- `slang-emit-*.cpp` - Target-specific code generators (HLSL, GLSL, SPIR-V, Metal, etc.)
- `slang-lower-to-ir.cpp` - AST to IR transformation

### IR System Characteristics
- **SSA Form**: Single Static Assignment with block parameters
- **Instruction-based**: Everything is an instruction (types, constants, operations)
- **Typed IR**: Every instruction has a type
- **Global Deduplication**: Types and constants automatically deduplicated

### Code Generation Targets
- `slang-emit-hlsl.cpp` - HLSL generation
- `slang-emit-glsl.cpp` - GLSL generation  
- `slang-emit-spirv.cpp` - Direct SPIR-V bytecode
- `slang-emit-metal.cpp` - Metal Shading Language
- `slang-emit-wgsl.cpp` - WebGPU Shading Language
- `slang-emit-cuda.cpp` - CUDA C++ generation
- `slang-emit-cpp.cpp` - Host C++ code generation

## Key Build Configuration

**CMake presets available:**
- `default` - Ninja Multi-Config (recommended)
- `vs2022` / `vs2019` - Visual Studio generators
- `emscripten` - WebAssembly build
- `generators` - Build only code generators

**Important CMake options:**
- `SLANG_ENABLE_TESTS=TRUE` - Enable test targets (default)
- `SLANG_ENABLE_GFX=TRUE` - Enable graphics abstraction layer
- `SLANG_ENABLE_SLANGC=TRUE` - Enable standalone compiler
- `SLANG_LIB_TYPE=SHARED` - Build shared library (default)

## Module and Generic System

**Module compilation:**
- Independent compilation with cross-module linking
- Serialization support for caching and distribution
- Built-in core modules for language features

**Generics and interfaces:**
- First-class generic types and functions
- Interface-based programming with witness tables
- Compile-time specialization and monomorphization

## Development Notes

**Language features:**
- HLSL compatibility with Slang extensions
- Automatic differentiation for neural graphics
- Multi-target compilation (DX11/12, Vulkan, Metal, CUDA, CPU)
- Capability system for platform feature management

**Testing structure:**
- Main tests in `tests/` directory
- Categories: compute, autodiff, bugs, bindings, etc.
- Expected failure lists for CI (`tests/expected-failure-*.txt`)

**Cross-compilation:**
First build generators for build platform, then configure with `SLANG_GENERATORS_PATH` pointing to them.

## Debugging and Development Workflow

**Using slangc for debugging:**
```bash
# Basic compilation with target-specific output
./build/Debug/bin/slangc -target cuda file.slang

# Debug IR stages and optimization passes
./build/Debug/bin/slangc -dump-ir -target cuda file.slang

# HLSL output for debugging
./build/Debug/bin/slangc -target hlsl file.slang -o output.hlsl
```

**Interactive debugging with slangi (interpreter):**
```bash
# Run bytecode interpreter for testing without GPU
./build/Debug/bin/slangi file.slang
```

**Test-driven development:**
```bash
# CPU-based testing (no GPU required)
slang-test tests/your-test.slang -cpu

# Interpreter-based testing 
slang-test tests/your-test.slang -slangi

# Test with specific backend
slang-test tests/your-test.slang -api dx12  # or vk, metal, etc.
```

## Code Formatting and Style

**Before submitting changes:**
```bash
# Format code according to project standards
./extras/formatting.sh
```

**PR requirements:**
- All PRs must be labeled "pr: non-breaking" or "pr: breaking"
- Include regression tests under `tests/` directory
- Breaking changes are rare and only for API/language compatibility issues

## Test Structure and Patterns

**Test file headers for different execution modes:**
```bash
# CPU execution (no GPU required)
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

# Interpreter execution
//TEST:INTERPRET(filecheck=CHECK):

# HLSL compilation test
//TEST(hlsl):COMPARE_HLSL: -entry main -target hlsl
```

**Common test categories:**
- `tests/compute/` - Compute shader tests
- `tests/autodiff/` - Automatic differentiation tests  
- `tests/bugs/` - Bug reproduction and regression tests
- `tests/bindings/` - Resource binding tests
