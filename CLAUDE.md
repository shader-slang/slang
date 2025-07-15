+# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

Reference other instruction files as well:
- `.github/copilot-instructions.md`
- `.github/workflows/claude.yml`

## Build System and Common Commands

**IMPORTANT:** On Windows, always use `cmake.exe` (not `cmake`) to ensure proper GPU test execution. Using `cmake` without the `.exe` extension may invoke WSL's cmake, which cannot run GPU tests.

### Building the Project
```bash
# Configure with default settings (Ninja Multi-Config)
cmake --preset default

# Configure with visual studio 2022 settings (Preferred on Windows)
cmake.exe --preset vs2022

# Build Release/Debug binaries.
# It can take from 5 minutes to 20 minutes depending on the machine.
cmake --build --preset debug # Debug binary
cmake --build --preset release # Release binary
```

### Testing
slang-test must run from repository root

```bash
# Run all tests with multiple servers (takes from 10 to 30 minutes)
./build/Release/bin/slang-test -use-test-server -server-count 8

# Run specific test
# The test file must be placed under "tests/" directory
./build/Release/bin/slang-test tests/path/to/test.slang

# Run unit tests
./build/Release/bin/slang-test slang-unit-test-tool/
```

Many of the Slang tests requires a GPU to run. However your local environment may not have a GPU. Prefer crafting the
test that runs on the CPU with `//TEST:COMPARE_COMPUTE(filecheck=CHECK): -cpu -output-using-type`, or with
`//TEST:INTERPRET(filecheck=CHECK):`. Prefer creating executable tests over compile-only tests, unless the test is to
check for a specific form of downstream code or diagnostics output.

When using `slangc` to produce SPIRV, you should set the `SLANG_RUN_SPIRV_VALIDATION` environment variable to `1` to ensure
that slangc runs SPIRV validation on the generated SPIRV. If you don't see any errors you are good.
Don't use the system's `spirv-val` tool to validate the SPIRV because it is not up-to-date.


### Slang Command Line Usage
**IMPORTANT:** Slang uses single dashes for multi-character options (not double dashes like most tools):
- Use `-help` (not `--help`)
- Use `-target spirv` (not `--target spirv`)
- Use `-dump-ir` (not `--dump-ir`)
- Use `-stage compute` (not `--stage compute`)

### AVOID These Debugging Options
**DO NOT USE** these options as they are unmaintained, unreliable or unnecessary:
- slangc with `-dump-ast`, `-dump-intermediate-prefix`, `-dump-intermediates`, `-dump-ir-ids`, `-serial-ir`, `-dump-repro`, `-load-repro` and `-extract-repro`.
- slang-test with `-category` and `-api`

### Code Formatting
```bash
# Format code before submitting PR
./extras/formatting.sh --no-version-check --cpp --since HEAD
```

## Architecture Overview

### Core Components

**Compiler Pipeline**:
- **Lexer** (`source/compiler-core/slang-lexer.cpp`): Tokenizes source code
- **Preprocessor** (`source/slang/slang-preprocessor.cpp`): Handles #include, macros, conditionals
- **Parser** (`source/slang/slang-parser.cpp`): Recursive descent parser producing AST
- **Semantic Checker** (`source/slang/slang-check.cpp`): Type checking, name resolution, validation
- **IR Generation** (`source/slang/slang-lower-to-ir.cpp`): Converts AST to Slang IR
- **IR Passes** (`source/slang/slang-ir-*.cpp`): Optimization and lowering passes
- **Code Emission** (`source/slang/slang-emit-*.cpp`): Target-specific code generation

**Key Directories**:
- `source/core/`: Core utilities (strings, containers, file system, platform abstractions)
- `source/compiler-core/`: Compiler infrastructure (diagnostics, downstream compilers)
- `source/slang/`: Main compiler implementation (frontend, IR, backend)
- `source/slangc/`: Command-line compiler tool
- `tools/`: Development and testing tools
- `include/`: Public API headers (`slang.h`, `slang-gfx.h`)
- `external/`: - Third-party dependencies and submodules
- `prelude/`: - Built-in language definitions and standard library
- `examples/`: - Sample programs demonstrating Slang usage
- `tests/`: - Comprehensive test suite
- `docs/`: - Project documentation

### Compilation Model

**Key Concepts**:
- **CompileRequest**: Bundles options, input files, and code generation requests
- **TranslationUnit**: Collection of source files (HLSL: one per file, Slang: all files together)
- **EntryPoint**: Function name + pipeline stage to compile
- **Target**: Output format (DXIL, SPIR-V, etc.) + capability profile

**Supported Targets**:
- Direct3D 11/12 (HLSL output)
- Vulkan (SPIR-V, GLSL output)
- Metal (MSL output) - experimental
- WebGPU (WGSL output) - experimental
- CUDA/OptiX (C++ output)
- CPU (C++ output, executables, libraries)

## Development Workflow

### Slang internal steps, as an example
1. Update lexer for new tokens (slang-lexer.cpp)
2. Extend parser for new syntax (slang-parser.cpp)
3. Add semantic analysis (slang-check-*.cpp)
4. Implement IR generation (slang-ir-*.cpp)
5. Add code generation for each target backend
6. Write comprehensive tests

### Debugging tools

#### slangc with `-dump-ir`
slangc with `-dump-ir` option is most efficient way to investigate problems that can be observed at IR level.

It will often require a use of `-target` and the most common combination is `-dump-ir -target spirv-asm`.
When `-dump-ir` is used without `-target`, the compilation process may stop earlier than it should be.

Since it dumps many lines, it will be good to store the result into a file for a further investigation.
The dump prints multiple sections which of each is separated by `### ` header.
Each section visualizes the IR state on multiple steps during the compilation.
It is necessary to differentiate the information on one section from one section, because the issue might be observed at a specific section.

You can also modify Slang code and insert a call to `dumpIRToString()` at any point of interest to dump any part of the IR
to string. You can then write that string to a temp file with `File::writeAllText()` to analyze what is going on.

When checking the IR dump, look for type consistency or logical errors in the IR to locate the potential transformation
pass at fault. Focus on passes that makes significant and systematic changes to the IR, such as specialization, inlining,
type legalization, and buffer lowering passes. You may iterate this process multiple times to narrow down the issue.


#### slangc with `-target spirv-asm`
slangc with `-target spirv-asm` is the most common way to see how the given slang shader is compiled into spirv code.

When an environment variable, `SLANG_RUN_SPIRV_VALIDATION=1`, is set, it will also run a static SPIRV valdiation.

You can skip the validation, if needed, with a command-line argument, `-skip-spirv-validation`.
When SPIRV validation fails, the actual spirv code is not printed.
You can skip the validation with the option and print the spirv code even when it fails the validation.

#### slangc with `-target spirv-asm -emit-spirv-via-glsl`
By default, slang uses `-emit-spirv-directly` and slang emits from slang shader to spirv directly.
When `-emit-spirv-via-glsl` is used, slang will translate the input slang shader to glsl and let glslang to generate spirv code.
This can be useful when we want to generate a reference spirv code for a comparison.

### IR System
- Slang uses a custom SSA-based IR (not LLVM)
- IR instructions defined in `slang-ir-insts.h` (generated from Lua)
- Extensive IR pass framework for optimization and lowering
- Target-specific legalization passes before code emission

### Language Server
- Language Server Protocol implementation in `source/slang/slang-language-server.cpp`
- Supports IntelliSense, completion, diagnostics, formatting
- Used by VS Code and Visual Studio extensions

### Module System
- Slang supports separate compilation via modules
- Modules can be compiled to IR and linked at runtime
- Optional obfuscation for distributed modules
- Core language features defined as modules in `prelude/`

### Generated files
- The enum values starting with `kIROp_` are defined in a generated file, `build/source/slang/fiddle/slang-ir-insts-enum.h.fiddle`

### Git commit message
- Don't mention Claude on the commit message

## Cross-Platform Considerations

**Supported Platforms**: Windows (x64/ARM64), Linux (x64/ARM64), macOS (x64/ARM64), WebAssembly

**Platform Abstractions**: Use utilities in `source/core/` for file system, process management, platform detection

**Graphics APIs**: Code generation supports all major APIs but runtime testing requires appropriate drivers/SDKs

## Additional documents

The most important documents are the end-user facing user-guide documents.
And it can be found under `docs/user-guide/`.

There is a dedicated repo, https://github.com/shader-slang/spec.git
If needed, you should clone the repo under `external/` directory.

### Formal Specification (`external/spec/specification/`)
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

### Feature Proposals (`external/spec/proposals/`)
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

