# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

**Repository**: shader-slang/slang - A shading language for GPU programming
**Primary Language**: C++ with custom Slang language
**MCP Tool Available**: `mcp__deepwiki__ask_question` with repoName: "shader-slang/slang"

Reference other instruction files as well:

- @.github/copilot-instructions.md (shares formatting/testing/debugging info; this CLAUDE.md is the canonical source)

User-specific instructions for Slang (optional, may not exist):

- @~/.claude/slang-instructions.md

## Build System and Common Commands

### Building the Project

If you are running in a Windows sandbox, run extras\win-sandbox-build.bat to produce a build in
debug configuration. This script discovers Visual Studio, runs vcvarsall.bat, configures with the
`vs2022-dev` preset, prefers locally cached dependencies instead of fetching them over the network,
and defaults to building `slangc`, `slang-test`, and `slangi`. Pass extra target names if you need
something other than that default target set.

On non-Windows platforms (Linux/macOS), run cmake directly to build:

```bash
# Configure with default settings (Ninja Multi-Config)
cmake --preset default

# Configure with visual studio 2022 settings (Preferred on Windows)
# On Windows, include -DSLANG_IGNORE_ABORT_MSG=ON to suppress
# modal abort dialogs during unattended/LLM-driven builds.
# Use -DSLANG_EMBED_CORE_MODULE=OFF to keep core module compilation separate
# from C++ source compilation. This way errors in *.meta.slang files (e.g.
# hlsl.meta.slang) do not break the C++ build — slangc and slang-test still
# compile successfully and the module errors surface at runtime instead.
cmake.exe --preset vs2022 -DSLANG_IGNORE_ABORT_MSG=ON -DSLANG_EMBED_CORE_MODULE=OFF

# Build Release/Debug binaries.
# It can take from 5 minutes to 20 minutes depending on the machine.
cmake --build --preset debug # Debug binary
cmake --build --preset release # Release binary

# Alternative: use workflow preset (configure + build in one step)
cmake --workflow --preset debug

# Build specific targets
cmake --build --preset debug --target slangc
cmake --build --preset debug --target slang-test
```

**sccache**: Pass `-DSLANG_USE_SCCACHE=ON` at configure time (or set `SLANG_USE_SCCACHE=1` env var) to use sccache as the compiler launcher for faster rebuilds. This automatically disables precompiled headers due to a known incompatibility. Requires `sccache` in PATH.

When building with `cmake --build`, redirect all of outputs to null-device.
When the build failed, then, re-run the same command without the redirections.
It is to avoid wasting the token usage of LLM.

Example,

```
# Print the build logs only when the initial attempt failed.
cmake --build --preset debug >/dev/null 2>&1 || cmake --build --preset debug
```

### Formatting

**Run `./extras/formatting.sh` before committing changes.** PRs must conform to the project's coding style. Use `./extras/formatting.sh --check-only` to verify without modifying files.

### PR Workflow

1. **Format your code**: Run `./extras/formatting.sh` before committing
2. **Label your PR**: Use "pr: non-breaking" (default) or "pr: breaking" (for ABI/language breaking changes)
3. **Include tests**: Add regression tests as `.slang` files under `tests/`

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

**Writing Tests Without GPU**:

- Use CPU compute: `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type`
- Use interpreter: `//TEST:INTERPRET(filecheck=CHECK):`
- Example test structure in `tests/language-feature/lambda/lambda-0.slang`

**Diagnostic Tests** (see `docs/diagnostics.md` for full details):

Use `// DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` as the test directive to verify that the compiler emits expected diagnostics. Annotations in comments match against compiler output by message text, severity, or error code. Carets align to columns on the preceding source line:

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
int foo = undefined;
//CHECK: E01234
//CHECK:  ^^^^^^^^^ error
```

**SPIRV Validation**:

- Set `SLANG_RUN_SPIRV_VALIDATION=1` when using `slangc -target spirv`
- Don't use system's `spirv-val` tool (may be outdated)

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
- `source/slang-core-module/`, `source/slang-glsl-module/`, `source/standard-modules/`: Standard library modules
- `source/slang-wasm/`: WebAssembly bindings
- `source/slang-record-replay/`: API call record/replay
- `source/slang-rt/`: Runtime library
- `tools/`: Development and testing tools
- `include/`: Public API headers (`slang.h`)
- `external/`: Third-party dependencies and submodules
- `prelude/`: Built-in language definitions and standard library
- `tests/`: Comprehensive test suite
- `docs/`: Project documentation (user guide in `docs/user-guide/`)
- `build/source/slang/fiddle/`: Generated code from FIDDLE macros (created during build)

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

### Adding New Language Features

1. Update lexer for new tokens (`source/compiler-core/slang-lexer.cpp`)
2. Extend parser for new syntax (`source/slang/slang-parser.cpp`)
3. Add semantic analysis (`source/slang/slang-check-*.cpp`)
4. Implement IR generation (`source/slang/slang-ir-*.cpp`)
5. Add code generation for each target backend (`source/slang/slang-emit-*.cpp`)
6. Write comprehensive tests under `tests/`

### Common Development Tasks

- **Adding an IR instruction**: Update the Lua definition files in `source/slang/slang-ir-insts.lua`, then regenerate
- **Adding a built-in function**: Add to appropriate module in `prelude/`
- **Adding a new target**: Implement new emitter in `source/slang/slang-emit-*.cpp`

### Modifying Public Headers (`include/`)

All files under `include/` are public API. Changes must preserve binary (ABI) and source
compatibility for callers compiled against older versions of the header.

#### Enums

- **Never insert a new enumerator in the middle of an existing enum.** Insertion shifts all
  subsequent integer values, silently breaking any caller that stores or compares the value.
- **Always append** new enumerators immediately before the terminal count/sentinel member
  (e.g. `CountOf`, `Count`, `NUM_*`), assigning an explicit integer value (the next sequential
  integer after the preceding enumerator).
- **Removed enumerators**: rename to `REMOVED_<Name>` and keep the original integer value.
  Never reuse or reclaim a retired integer.

#### Virtual tables (COM interfaces)

Slang's public interfaces (`ISession`, `IModule`, `IComponentType`, etc.) are COM-style
vtables declared with `virtual` methods in `include/slang.h`. The vtable layout is fixed by
declaration order. Violating these rules corrupts the vtable and causes silent crashes or
wrong-method dispatch for any caller compiled against an older header.

- **Never reorder virtual methods** within an interface.
- **Never change a virtual method's signature** (return type, parameter types, calling
  convention, or `SLANG_MCALL` decoration).
- **Never insert a new virtual method** in the middle of an interface — append only, at the
  end of the interface before the closing brace.
- **Never remove a virtual method** — replace its body with a stub that returns
  `SLANG_E_NOT_IMPLEMENTED` and keep the declaration in place.
- Avoid extending an existing public COM interface in place when clients may implement or
  query it by UUID. Prefer adding a new derived/versioned interface with its own UUID, while
  keeping the original interface declaration and UUID supported for existing callers.

### Debugging tools

#### IR Dump (`-dump-ir`)

```bash
# Dump IR at every pass (use with -target and -o to avoid mixing output)
slangc -dump-ir -target spirv-asm -o tmp.spv test.slang | python extras/split-ir-dump.py

# Dump IR before/after a specific pass
slangc -dump-ir-before lowerGenerics -dump-ir-after lowerGenerics -target spirv-asm -o tmp.spv test.slang > pass.dump
```

- Always combine `-dump-ir` with `-target` (otherwise compilation stops early) and `-o <file>` (otherwise target code mixes with IR on stdout)
- Use `extras/split-ir-dump.py` to split large dumps into per-pass files. See `extras/split-ir-dump.md` for details.
- You can insert `dumpIRToString()` in C++ code and write to a file with `File::writeAllText()` for ad-hoc inspection.
- When debugging, focus on root causes in IR passes (specialization, inlining, type legalization, buffer lowering) rather than band-aid fixes in emit logic. The compiler philosophy is to keep emission simple and do heavy transforms in IR passes.

#### InstTrace

Trace where a problematic IR instruction was created:

```bash
python3 ./extras/insttrace.py <debugUID> ./build/Debug/bin/slangc tests/my-test.slang -target spirv
```

#### SPIRV Tools

- `slangc -target spirv-asm` — compile to SPIRV assembly
- Set `SLANG_RUN_SPIRV_VALIDATION=1` for static validation; use `-skip-spirv-validation` to see SPIRV output even when validation fails
- `slangc -target spirv-asm -emit-spirv-via-glsl` — generate reference SPIRV via GLSL for comparison

#### Assertion Behavior (`SLANG_ASSERT`)

On Windows, assertion failures normally open a modal dialog that blocks execution. Set the `SLANG_ASSERT` environment variable to control this:

| Value                 | Behavior                                                                                                                       |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| `system`              | Use the system `assert()`, which shows a modal dialog and allows the developers to attach the debugger                         |
| `debugbreak`          | When a debugger is already attached, it will hit a debug-break; fall back to `system` behavior when a debugger is not attached |
| `release-assert-only` | Skip debug-only assertions (`SLANG_ASSERT`, `SLANG_ASSERT_FAILURE`) and continue; `SLANG_RELEASE_ASSERT` still fires           |
| _(unset)_             | Throws an exception                                                                                                            |

The behavior on Windows after an exception is thrown is controlled by the CMake option `SLANG_IGNORE_ABORT_MSG`.
This option is highly recommended for unattended automation with LLM workflow; it bakes the behavior into all built executables at compile time.

#### RTX Remix Testing

Use the `/repro-remix` skill or see `extras/repro-remix.md`.

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
- `FIDDLE()` and `FIDDLE(...)` statements in AST node declarations indicate that additional source is generated and included from `build/source/slang/fiddle`, providing static type system and reflection metadata, visitor support, and serialization support.

### Git commit message

- Don't mention Claude on the commit message

### Debugging with slangpy

Use the `/slangpy-debug` skill to build slangpy from source with your local Slang build for compatibility testing.

## Cross-Platform Considerations

**Supported Platforms**:
Windows (x64/ARM64), Linux (x64/ARM64), macOS (x64/ARM64), WebAssembly

**Platform Abstractions**:
Use utilities in `source/core/` for file system, process management, platform detection

**Graphics APIs**:
Code generation supports all major APIs but runtime testing requires appropriate drivers/SDKs

**WSL on Windows**:
When running under WSL environment, try to append `.exe` to the executables to avoid using Linux binaries

- Use `cmake.exe` instead of `cmake`,
- Use `python.exe` instead of `python`,
- Use `gh.exe` instead of `gh` and so on.

## Additional Documents

- User-facing documentation: `docs/user-guide/`
- Language specification: see below

### Formal Specification

Clone `https://github.com/shader-slang/spec.git` under `external/` if needed. Specification files are in `external/spec/specification/`, feature proposals in `external/spec/proposals/`.
