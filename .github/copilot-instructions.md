This file provides guidance to LLM when working with code in this repository.

## Project Overview

Slang is a shading language compiler that generates code for multiple graphics APIs including D3D12, Vulkan, Metal, D3D11, OpenGL, CUDA, and CPU. The project is based on C++ and uses CMake for build configuration.

### Slang Command Line Usage
**IMPORTANT:** Slang uses single dashes for multi-character options (not double dashes like most tools):
- Use `-help` (not `--help`)
- Use `-target spirv` (not `--target spirv`)
- Use `-dump-ir` (not `--dump-ir`)
- Use `-stage compute` (not `--stage compute`)

Currently most of the additional outputs are printed to stderr not stdout.
When store the output, use "... > filename 2>&1" to include both stdout and stderr.

## Build Commands

### Quick Start
```bash
# Configure with default settings
cmake --preset default

# Build debug version
cmake --build --preset debug

# Build release version  
cmake --build --preset release
```

### Platform-Specific Build Commands

#### Windows
**IMPORTANT:** On Windows, always use `cmake.exe` (not `cmake`) to ensure proper GPU test execution. Using `cmake` without the `.exe` extension may invoke WSL's cmake, which cannot run GPU tests.

```bash
# Configure for Visual Studio 2022
cmake.exe --preset vs2022

# Build from command line
cmake.exe --build --preset release
```

#### Linux
```bash
# Install dependencies
sudo apt-get install cmake ninja-build

# Configure and build
cmake --preset default
cmake --build --preset release
```

#### macOS
```bash
# Install dependencies (using Homebrew)
brew install ninja cmake

# Configure and build
cmake --preset default
cmake --build --preset release
```

### Testing
- `slang-test` must run from repository root.
- The test file must be placed under "tests/" directory.
- When `slang-test` run without any command-line argument describing which test to run, it will run a full test, which can take for a few minutes if not an hour.
- When running a full test with `slang-test`, LLM must use `-v failure` to reduce the verbosity.
- When the given issue is resolved, a full regression test with `slang-test` must pass all tests; otherwise, the fix will cause a regression.

#### Windows
```bash
# Install VulkanSDK from LunarG, then set environment variable
set SLANG_RUN_SPIRV_VALIDATION=1

# Run tests with multiple processes
build/Release/bin/slang-test -use-test-server -server-count 8

# Run specific test
build/Release/bin/slang-test tests/path/to/test.slang
```

#### Linux/macOS
```bash
# Install VulkanSDK (Linux)
sudo apt update && sudo apt install vulkan-sdk

# Run tests from repository root
build/Release/bin/slang-test -use-test-server -server-count 8
```

#### Test Types
Slang uses a LLVM tool, "filecheck".
For more details, refer https://llvm.org/docs/CommandGuide/FileCheck.html

```bash
# Add this line to test files:
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type

# Interpreter test
//TEST:INTERPRET(filecheck=CHECK):

# API filtering (exclude CPU tests)
slang-test -api all-cpu

# API filtering (include specific APIs)
slang-test -api gl+dx11
```

### Code Formatting
```bash
# Format code before submitting PR
./extras/formatting.sh

# Check formatting without modifying files
./extras/formatting.sh --check-only

# Skip version compatibility checks (if tools are newer than expected)
./extras/formatting.sh --no-version-check

# Format specific file types only
./extras/formatting.sh --cpp        # C++ files only
./extras/formatting.sh --cmake      # CMake files only
./extras/formatting.sh --yaml       # YAML/JSON files only
./extras/formatting.sh --md         # Markdown files only
./extras/formatting.sh --sh         # Shell script files only

# Format files changed since specific git revision
./extras/formatting.sh --since HEAD~3
```

### Labeling your PR

All PRs needs to be labeled as either "pr: non-breaking" or "pr: breaking".
Label your PR as "pr: breaking" if you are introducing public API changes that breaks ABI compabibility,
or you are introducing changes to the Slang language that will cause the compiler to error out on existing Slang code.
It is rare for a PR to be a breaking change.

## Architecture Overview

### Core Components

- **source/core/** - Core utilities and data structures used throughout the project
- **source/compiler-core/** - Shared compiler infrastructure including lexer, parser, and diagnostic systems
- **source/slang/** - Main Slang language compiler implementation
- **source/slangc/** - Command-line compiler tool
- **source/slangd/** - Language server implementation
- **source/slang-rt/** - Runtime library
- **tools/** - Various utility tools and executables
- **tools/gfx** - Old rendering system that will be deprecated
- **external/slang-rhi** - New rendering system that replaces tools/gfx

### Key Directories

- **external/** - Third-party dependencies and submodules
- **prelude/** - Built-in language definitions and standard library
- **examples/** - Sample programs demonstrating Slang usage
- **tests/** - Comprehensive test suite
- **docs/** - Project documentation

### Code Generation Pipeline

1. **Lexing & Parsing** - Convert source text to AST (source/slang/slang-lexer.cpp, parsing logic)
2. **Semantic Analysis** - Type checking, symbol resolution (source/slang/slang-check-*.cpp)
3. **IR Generation** - Convert AST to intermediate representation (source/slang/slang-ir-*.cpp)
4. **Optimization** - Various IR transformation passes
5. **Code Emission** - Generate target-specific code (source/slang/slang-emit-*.cpp)

### Target Backends

- **HLSL** - Direct3D shader generation (slang-emit-hlsl.cpp)
- **GLSL** - OpenGL shader generation (slang-emit-glsl.cpp)
- **SPIRV** - Vulkan/OpenGL binary format (slang-emit-spirv.cpp)
- **Metal** - Apple Metal shading language (slang-emit-metal.cpp)
- **CUDA** - NVIDIA CUDA C++ (slang-emit-cuda.cpp)
- **CPU** - Host CPU code generation (slang-emit-cpp.cpp)

## Development Guidelines

### Contribution Workflow

#### Git repo setup
If you are a github copilot, you don't need to setup git repo, because it is handled by .github/workflows/copilot-setup-steps.yml .

```bash
# 1. Fork the repository on GitHub and clone your fork
git clone --recursive --tags https://github.com/YOUR-USERNAME/slang.git
cd slang

# 2. Add upstream remote and fetch tags (required for build process)
git remote add upstream https://github.com/shader-slang/slang.git
git fetch --tags upstream
git push --tags origin

# 3. Create a feature branch
git checkout -b feature/your-feature-name
```

#### Making Changes
```bash
# Make your changes, then commit with descriptive message
git commit -m "Add feature description

Fixes #issue-number

Detailed explanation of what this change does and why."

# Push to your fork
git push origin feature/your-feature-name
```

#### Syncing with Upstream
```bash
# Keep your branch in sync during review process
git fetch upstream master
git merge upstream/master  # resolve conflicts if needed
git submodule update --recursive
```

### PR Requirements
- Code must be formatted with `./extras/formatting.sh` or use `/format` bot command
- Include regression tests for bug fixes in `tests/` directory as `.slang` files
- Follow descriptive commit message guidelines
- Enable Actions in your forked repository for CI to work

### Cross-Platform Considerations
- Primary development platforms: Windows, Linux, macOS
- Build system supports both Ninja and Visual Studio generators
- WebAssembly builds require Emscripten SDK

### GitHub API Rate Limits
If you encounter GitHub API rate limit warnings during `cmake --preset`:
```bash
# Use personal access token to increase rate limit
cmake --preset default -DSLANG_GITHUB_TOKEN=your_token_here
```
Generate a personal access token at: https://github.com/settings/tokens

## Common Development Tasks

### Slang internal steps, as an example
1. Update lexer for new tokens (slang-lexer.cpp)
2. Extend parser for new syntax (slang-check-*.cpp)
3. Add semantic analysis (slang-check-*.cpp)
4. Implement IR generation (slang-ir-*.cpp)
5. Add code generation for each target backend
6. Write comprehensive tests

### Debugging
- Use `SLANG_ENABLE_IR_BREAK_ALLOC=TRUE` for IR debugging
- `slangc` has a debugging feature to dump the IR information for multiple steps; `-dump-ir`.
- Use `printf` as a debugging tool extensively.
- When debugging, especially after a build, the first compilation of a shader will also trigger the compilation of Slang's core modules. This can produce a large amount of debug output that is not relevant to the specific shader being tested.
- To isolate the debug output for a particular shader, it is recommended to compile it twice. The first compilation will handle the core modules, and the second run will show only the debug information for the shader under investigation.
- Confirm the working theory about the given issue before start applying any code fixes.

### Working with Tests
- Most tests are `.slang` files with embedded test directives
- Use `//TEST:` comments to specify test behavior
- CPU-only tests use `-cpu` flag for environments without GPU
- Interpreter tests use `//TEST:INTERPRET` for bytecode execution

#### Advanced Testing Options
```bash
# Test categories
slang-test -category smoke          # Quick smoke tests
slang-test -category render         # Rendering tests
slang-test -category compute        # Compute shader tests
slang-test -category vulkan         # Vulkan-specific tests

# API filtering (+ includes, - excludes)
slang-test -api vk+dx12            # Only Vulkan and D3D12
slang-test -api all-cpu            # All APIs except CPU
slang-test -api gl+dx11            # Only OpenGL and D3D11

# Parallel execution
slang-test -use-test-server -server-count 8

# Unit tests
# Due to the backward compatibility the use of a forward-slash is required
slang-test slang-unit-test-tool/    # Core unit tests
slang-test gfx-unit-test-tool/      # Graphics unit tests
```

## Advanced Development Features

### CMake Configuration Options

Key options for development:
- `SLANG_ENABLE_TESTS=ON` - Enable test targets
- `SLANG_ENABLE_GFX=ON` - Enable graphics abstraction layer  
- `SLANG_ENABLE_SLANGD=ON` - Enable language server
- `SLANG_ENABLE_SLANGC=ON` - Enable standalone compiler
- `SLANG_ENABLE_SLANGI=ON` - Enable Slang interpreter
- `SLANG_SLANG_LLVM_FLAVOR` - Configure LLVM backend support:
  - `FETCH_BINARY_IF_POSSIBLE` (default) - Download prebuilt binary, fallback to disable
  - `FETCH_BINARY` - Download prebuilt binary, fail if unavailable  
  - `USE_SYSTEM_LLVM` - Use system LLVM installation
  - `DISABLE` - Build without LLVM support
- `SLANG_LIB_TYPE` - SHARED or STATIC library build
- `SLANG_ENABLE_FULL_IR_VALIDATION=ON` - Enable thorough IR validation (slow)
- `SLANG_ENABLE_IR_BREAK_ALLOC=ON` - Enable IR debugging features

### WebAssembly Build
```bash
# 1. Install and activate Emscripten SDK
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest

# 2. Build generators for build platform first
cmake --workflow --preset generators --fresh
mkdir generators && cmake --install build --prefix generators --component generators

# 3. Configure with emcmake for target platform  
source ../emsdk/emsdk_env  # or emsdk_env.bat on Windows
emcmake cmake -DSLANG_GENERATORS_PATH=generators/bin --preset emscripten -G "Ninja"

# 4. Build WebAssembly target
cmake --build --preset emscripten --target slang-wasm
```

### Cross-Compilation Workflow
```bash
# 1. Build generators for build platform
cmake --workflow --preset generators --fresh
mkdir build-platform-generators
cmake --install build --prefix build-platform-generators --component generators

# 2. Configure for target platform
cmake --preset default --fresh \
  -DSLANG_GENERATORS_PATH=build-platform-generators/bin \
  -DCMAKE_C_COMPILER=target-gcc \
  -DCMAKE_CXX_COMPILER=target-g++

# 3. Build for target
cmake --workflow --preset release
```

### Debugging and Reproduction System

#### Reliable Debugging Options
```bash
# Dump IR at various stages
# `-target` is often required to move into the rest steps of compilation
slangc -dump-ir -target spirv shader.slang

# Enable IR validation
slangc -validate-ir shader.slang

# Generate reproduction data during compilation
slangc -dump-repro repro.slang-repro shader.slang
```

#### AVOID These Debugging Options
**DO NOT USE** these options as they are unmaintained and unreliable:
- `-dump-ast` - AST dumping is not maintained
- `-dump-intermediate-prefix` - Intermediate dumping is unreliable  
- `-dump-intermediates` - Not actively maintained
- `-dump-ir-ids` - ID dumping is not reliable
- `-serial-ir` - Serial IR output is not maintained

**Always use `-dump-ir` as the primary debugging tool** - it's actively maintained and reliable.

#### Repro System for Bug Reproduction
**Note** The options, `-dump-repro`, `-load-repro`, and `-extract-repro`, are not actively maintained.

```bash
# Generate reproduction data during compilation
slangc -dump-repro repro.slang-repro shader.slang

# Load and replay compilation from repro data
slangc -load-repro repro.slang-repro

# Extract files from repro archive
slangc -extract-repro repro.slang-repro extracted/
```

### Systematic IR Investigation Methodology

When debugging IR-related issues, follow this structured approach:

#### 1. Create Minimal Reproduction
- Isolate the issue to the smallest possible test case
- Place test files under `tests/` directory for easy execution
- Use `-dump-ir` to capture the problematic IR transformation

#### 2. Use Efficient Debugging Tools
- **When the issue is observable with `-dump-ir`**: Stay with `-dump-ir` and avoid running `slang-test`
- `slang-test` is slower and less efficient than direct `-dump-ir` for IR investigation
- slangi could be a quicker alternative to slang-test if it can reproduce the problem
- **Note**: Not all IR issues can be observed with `-dump-ir` alone - some require full test execution

#### 3. Compare IR Across Commits (When Regression is Known)
- If you know which commit caused the issue, take IR dumps before and after:
```bash
# Before offending commit
git checkout <good_commit>
slangc -dump-ir -target spirv-asm test.slang > before.ir 2>&1

# After offending commit (or current state)
git checkout <bad_commit_or_current>
slangc -dump-ir -target spirv-asm test.slang > after.ir 2>&1

# Compare the dumps to identify the problematic transformation
diff before.ir after.ir
```

#### 4. Understand IR Design Principles
- Review `docs/design/ir.md` for fundamental IR concepts
- Understand hoistable values, witness tables, and specialization
- Check if the issue involves global/hoistable value deduplication

#### 5. Identify the Transformation Stage
- Use `-dump-ir` to trace where unexpected changes occur
- Look for optimization passes that might violate IR constraints
- Check witness table integrity for interface-related issues

#### 6. Common Issue Patterns
- **Interface/Generic Issues**: Often involve witness table or specialization problems
- **Optimization Issues**: May not respect constraints on hoistable operations
- **Specialization Issues**: Can break when optimizations modify shared structures

#### 7. Before Writing Fixes
- Understand the exact IR transformation causing the issue
- Verify the fix doesn't break other IR invariants described in `docs/design/ir.md`
- Test with multiple specialization scenarios and both direct calls and interface dispatch
- Run comprehensive tests to ensure no regressions

### Comprehensive Documentation Reference

For detailed understanding of Slang's design and implementation, consult the extensive documentation available:

#### Core Design Documents (`docs/design/`)
- `design/README.md` - Overview of all design documents
- `design/overview.md` - Overall compilation pipeline and architecture
- `design/ir.md` - **IR design, hoistable values, witness tables, deduplication** 
- `design/semantic-checking.md` - Type checking and symbol resolution
- `design/parsing.md` - Parser implementation details
- `design/interfaces.md` - Interface and generic system design
- `design/capabilities.md` - Capability system architecture
- `design/existential-types.md` - Existential type implementation
- `design/autodiff.md` - Automatic differentiation system
- `design/decl-refs.md` - Declaration reference system (DeclRef type)
- `design/casting.md` - Casting mechanisms in compiler codebase
- `design/coding-conventions.md` - Detailed coding standards
- `design/experimental.md` - Experimental API deployment process
- `design/serialization.md` - IR and module serialization
- `design/stdlib-intrinsics.md` - Standard library intrinsic functions

#### Build and Development (`docs/`)
- `building.md` - Comprehensive build instructions
- `ci.md` - Continuous integration setup
- `repro.md` - Reproduction system for bug reports
- `command-line-slangc-reference.md` - Complete slangc command reference

#### Target-Specific Documentation  
- `cpu-target.md` - CPU code generation and C++ interop
- `cuda-target.md` - CUDA backend implementation
- `target-compatibility.md` - Cross-target compatibility matrix
- `nvapi-support.md` - NVIDIA API integration

#### Language and Features
- `language-guide.md` - Language overview and examples
- `faq.md` - Frequently asked questions
- `64bit-type-support.md` - 64-bit integer and pointer support
- `layout.md` - Memory layout and data structure design
- `wave-intrinsics.md` - GPU wave/warp intrinsic functions
- `shader-execution-reordering.md` - SER (Shader Execution Reordering)
- `shader-playground.md` - Online shader playground documentation

#### Standard Library and Tools
- `stdlib-doc.md` - Standard library documentation
- `stdlib-docgen.md` - Documentation generation for stdlib
- `doc-system.md` - Documentation system architecture
- `update_spirv.md` - SPIR-V integration and updates

#### User-Facing Documentation (`docs/user-guide/`)
- `user-guide/00-introduction.md` - Introduction to Slang
- `user-guide/01-get-started.md` - Getting started guide
- `user-guide/02-conventional-features.md` - Conventional language features
- `user-guide/03-convenience-features.md` - Convenience features
- `user-guide/04-modules-and-access-control.md` - Module system and access control
- `user-guide/05-capabilities.md` - Capability system usage
- `user-guide/06-interfaces-generics.md` - Interfaces and generics
- `user-guide/07-autodiff.md` - Automatic differentiation
- `user-guide/08-compiling.md` - Compilation guide
- `user-guide/09-reflection.md` - Reflection API
- `user-guide/09-targets.md` - Target platform support
- `user-guide/10-link-time-specialization.md` - Link-time specialization
- `user-guide/11-language-version.md` - Language version control
- `user-guide/a1-01-matrix-layout.md` - Matrix layout appendix
- `user-guide/a1-03-obfuscation.md` - Code obfuscation appendix
- `user-guide/a1-04-interop.md` - Interoperability appendix
- `user-guide/a1-05-uniformity.md` - Uniformity analysis appendix
- `user-guide/a1-special-topics.md` - Special topics appendix
- `user-guide/a2-01-spirv-target-specific.md` - SPIR-V target specifics
- `user-guide/a2-02-metal-target-specific.md` - Metal target specifics
- `user-guide/a2-03-wgsl-target-specific.md` - WGSL target specifics
- `user-guide/a2-04-glsl-target-specific.md` - GLSL target specifics
- `user-guide/a2-target-specific-features.md` - Target-specific features
- `user-guide/a3-01-reference-capability-profiles.md` - Capability profiles reference
- `user-guide/a3-02-reference-capability-atoms.md` - Capability atoms reference
- `user-guide/a3-reference.md` - Reference documentation
- `user-guide/index.md` - User guide index

#### Language Reference (`docs/language-reference/`)
Multiple files covering language features, compilation, reflection API, and platform-specific details.

#### Formal Specification (`external/spec/specification/`)
- `specification/index.bs` - Main specification document
- `specification/types.md` - Type system specification
- `specification/generics.md` - Generics and templates specification
- `specification/interfaces.md` - Interface system specification
- `specification/expressions.md` - Expression evaluation rules
- `specification/declarations.md` - Declaration syntax and semantics
- `specification/statements.md` - Statement execution semantics
- `specification/checking.md` - Type checking rules
- `specification/conversion.md` - Type conversion rules
- `specification/overloading.md` - Overload resolution
- `specification/lookup.md` - Name lookup rules
- `specification/modules.md` - Module system
- `specification/capabilities.md` - Capability system
- `specification/autodiff.md` - Automatic differentiation
- `specification/attributes.md` - Attribute system
- `specification/lexical.md` - Lexical analysis
- `specification/parsing.md` - Parsing rules
- `specification/preprocessor.md` - Preprocessor behavior
- `specification/execution.md` - Execution model
- `specification/extensions.md` - Language extensions
- `specification/subtyping.md` - Subtyping relationships
- `specification/visibility.md` - Visibility and access control

#### Feature Proposals (`external/spec/proposals/`)
**Template and Active Proposals:**
- `000-template.md` - Template for new proposals
- `001-where-clauses.md` - `where` clauses for generic constraints (Partially implemented)
- `002-type-equality-constraints.md` - Type equality constraints in generics
- `003-atomic-t.md` - Atomic operations and types
- `004-initialization.md` - Initialization syntax improvements
- `005-write-only-textures.md` - Write-only texture support
- `007-variadic-generics.md` - Variadic generic types and functions (Implemented)
- `008-tuples.md` - Tuple types built on variadic generics (Implemented)
- `009-ifunc.md` - Function interface types for callbacks
- `010-new-diff-type-system.md` - New automatic differentiation type system
- `011-structured-binding.md` - Structured binding declarations
- `012-language-version-directive.md` - Language version directives
- `013-aligned-load-store.md` - Aligned memory load/store operations
- `014-extended-length-vectors.md` - Extended vector length support
- `015-descriptor-handle.md` - Descriptor handle types
- `016-slangpy.md` - Python binding for Slang
- `017-shader-record.md` - Shader record data structures
- `018-packed-data-intrinsics.md` - Packed data manipulation intrinsics
- `019-cooperative-vector.md` - Cooperative vector operations
- `020-stage-switch.md` - Stage-specific code switching
- `022-C++20-migration.md` - C++20 feature migration
- `023-cooperative-matrix.md` - Cooperative matrix operations
- `024-any-dyn-types.md` - Any and dynamic types
- `025-lambda-1.md` - Lambda expressions with immutable capture (In Implementation)
- `026-error-handling.md` - Error handling mechanisms
- `027-tuple-syntax.md` - Tuple syntax improvements
- `028-cooperative-matrix-2.md` - Extended cooperative matrix support
- `029-conditional.md` - Conditional compilation features
- `030-interface-method-default-impl.md` - Default interface method implementations (In Experiment)

**Legacy and Implementation Proposals:**
- `legacy/001-basic-interfaces.md` - Basic interface system (legacy)
- `legacy/002-api-headers.md` - API header generation (legacy)
- `legacy/004-com-support.md` - COM support (legacy)
- `legacy/005-components.md` - Component system (legacy)
- `legacy/006-artifact-container-format.md` - Artifact container format (legacy)
- `implementation/ast-ir-serialization.md` - AST/IR serialization implementation

#### Project Documentation (`external/spec/`)
- `README.md` - Project overview and goals
- `CONTRIBUTING.md` - Contribution guidelines
- `CODE_OF_CONDUCT.md` - Community standards

### Coding Standards (Key Points)

#### C++ Subset Restrictions
- **No STL containers** - Use Slang's own List<>, Dictionary<>, etc.
- **No iostreams** - Use Slang's StringBuilder, FileStream
- **No exceptions** - Use Result<> types for error handling
- **No built-in RTTI** - Use Slang's RTTI system

#### Naming Conventions
- **Types:** `UpperCamelCase` (e.g., `IRInst`, `CompileRequest`)
- **Values:** `lowerCamelCase` (e.g., `fileName`, `moduleCount`)
- **Macros:** `SLANG_SCREAMING_SNAKE_CASE` (e.g., `SLANG_ASSERT`)
- **Global variables:** `g` prefix (e.g., `gSharedContext`)
- **Static members:** `s` prefix (e.g., `sInstanceCount`)
- **Constants:** `k` prefix (e.g., `kMaxIterations`)
- **Member variables:** `m_` prefix allowed (e.g., `m_name`)

#### Function Parameters
Use prefixes to indicate parameter intent:
- `in` - Input parameter (default, can be omitted)
- `out` - Output parameter
- `io` - Input/output parameter

```cpp
void processData(
    InputData       inputData,      // or inInputData
    OutputResult*   outResult,
    IOBuffer*       ioBuffer)
```

### Target-Specific Development

#### CPU Target Features
```bash
# Compile for CPU execution
slangc -target cpp -line-directive-mode none shader.slang

# Host-callable compilation (direct execution)
slangc -target host-cpp shader.slang

# Global variable support
// Use __global modifier for shared state
__global int globalCounter = 0;
```

#### NVAPI Integration
```bash
# Enable NVAPI in build
cmake --preset default -DSLANG_ENABLE_NVAPI=ON

# Use in shaders with prelude inclusion
// Automatically includes NVAPI headers
#include "nvapi_wrapper.slang"
```

### Static Linking Requirements
When linking against static `libslang.a`, include these dependencies:
```cmake
${SLANG_DIR}/build/Release/lib/libslang.a
${SLANG_DIR}/build/Release/lib/libcompiler-core.a  
${SLANG_DIR}/build/Release/lib/libcore.a
${SLANG_DIR}/build/external/miniz/libminiz.a
${SLANG_DIR}/build/external/lz4/build/cmake/liblz4.a
```

### IR Development Best Practices

#### Safe IR Modification
```cpp
// Safe iteration when modifying IR
List<IRInst*> instsToRemove;
for (auto inst : block->getChildren()) {
    if (shouldRemove(inst))
        instsToRemove.add(inst);
}
for (auto inst : instsToRemove) {
    inst->removeAndDeallocate();
}

// Use IRBuilder for safe operand replacement
builder.replaceOperand(inst->getOperand(0), newValue);
// NOT: inst->setOperand(0, newValue);  // Can break use-def chains
```

#### IR Design Principles
- Everything is an instruction (types, constants, functions)
- SSA form with block parameters instead of phi nodes
- Global/hoistable values are automatically deduplicated
- Critical edges are eliminated during SSA construction

This comprehensive update adds crucial information from the docs directory that wasn't in the original CLAUDE.md, including advanced testing, debugging tools, coding standards, target-specific development, and IR modification best practices.

