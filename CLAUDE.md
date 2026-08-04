<!--
SPDX-FileCopyrightText: The Khronos Group, Inc.
SPDX-License-Identifier: CC-BY-4.0
-->

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

### Suppressing Unused Variable Warnings

When a variable declared in an `if` condition is unused inside the body (the condition exists only for its type-check side-effect), use the **C++17 if-init-statement** pattern instead of `SLANG_UNUSED`:

```cpp
// Preferred: C++17 if-init pattern
if (auto foo = as<IRFoo>(inst); foo)
{
    // foo not needed in body — the type check is the point
}

// Avoid: SLANG_UNUSED inside the body
if (auto foo = as<IRFoo>(inst))
{
    SLANG_UNUSED(foo);
}
```

For variables that are set but never read outside an `if` (e.g., a plain local variable), use `SLANG_UNUSED(var)` with a comment explaining why.

### Problem-Solving Methodology

Follow the **principled path**, not the minimal-edit-distance path. The goal is a correct
representation that is robust by construction, even when that means a larger rework.

- **Fix root causes, not symptoms.** When a bug appears in emit/codegen, the cause is usually
  upstream (an IR pass, type legalization, specialization, lowering, or the AST/IR representation
  itself). Trace it there and fix it there.
- **Question every change.** Before keeping a change, answer: _Why is this change necessary? What
  test fails without it? Is this the right fix, or is the problem telling me the
  direction/representation is flawed?_ If you cannot name a test that fails without a change, the
  change probably should not exist.
- **Do not mask.** A guard, null-check, or special case that papers over a malformed
  AST/IR/witness-table is a band-aid that hides a representation bug. A guard that is never hit
  under correct input is dead code. Prefer making the representation correct so consumers stay
  simple.
- **Interrogate the input shape.** Whenever you write or change code that handles a particular
  shape of input — an AST node, IR inst, witness, type, etc. — always ask: _is that input shape
  itself correct and principled, or should the upstream producer of it be fixed instead?_ If the
  shape is wrong or accidental, fix the producer; handle it here only when the shape is genuinely
  valid input. This is the routine double-check that root-causing was done at the right layer, and
  its answer is required in the PR description (see the Process report below).
- **Prefer correct representation over edit distance.** If two surface forms _should_ be
  equivalent, model them identically. If a consumer reads data by position/index/identity when the
  data is conceptually an unordered key→value set (e.g. witness-table / interface requirement
  entries), make the access by role/key, not by position.
- **Keep a working log/report.** Maintain a scratch markdown document throughout the task that
  records: the problem and a motivating example, road-blockers encountered, how issues **cascade**
  (one fix exposing the next), the fix chosen for each and _why it is principled_ (with a concrete
  code trace), and alternatives that were rejected and why. This log is what you distill into the
  PR description below. (Keep the log out of the commit — it feeds the PR body, it is not a repo
  artifact.)

### Self-Review for Unprincipled Changes

Before finalizing a non-trivial compiler change, review the diff for signs that the fix is
compensating for a bad AST/IR/`Val`/witness representation. Treat these patterns as red flags until
you can prove they are the right layer:

- **Custom semantic equivalence.** New recursive helpers over `DeclRef`, `Val`, `Type`, `Witness`,
  or IR shapes (for example `are...Equivalent`, `does...Match`, or `try...Match`) often mean two
  alternative representations were allowed to survive. First ask why `substitute`, `resolve`,
  `getCanonicalType`, `equals`, or an existing canonical builder does not already make the values
  identical.
- **Unaudited helper growth.** Treat every new helper, fallback, and `try...` function as a review
  target, not only large or complicated ones. A helper that exists to make one failing test pass,
  or that reimplements part of substitution, resolution, AST copy, generic solving, lookup, or
  lowering, is often the place where an unprincipled fix hides.
- **Semantic-to-syntax reconstruction.** Code that turns checked semantic data (`Val`, `Type`,
  `DeclRef`, witness, lowered IR value) back into syntax (`Expr`, `TypeExp`, parser-shaped AST) is
  a strong smell. The checked semantic field should usually be the source of truth; rebuilding
  surface syntax, as in a helper that recreates an expression from an `IntVal`, usually means the
  producer/copier/substitution path is preserving the wrong representation.
- **Context rediscovery by graph walking.** Code that walks arbitrary operand graphs, substitution
  chains, witness chains, lookup paths, or IR users to recover generic arguments, requirement keys,
  canonical paths, or parent declarations is usually downstream repair. Prefer storing or building
  the canonical form at the producer.
- **Consumer-side patching.** Lowering, emit, specialization, typeflow, and backend code should not
  patch malformed AST/IR shapes from earlier phases. If these consumers need front-end-specific
  knowledge of an accidental representation, trace and fix the producer instead.
- **Hardcoded representation trivia.** Special cases for particular `DeclRef` subclasses, builtin
  magic type names, generic argument indices, witness-table entry order, or nested-vs-flat
  specialization shape need a strong invariant and usually belong at a canonical construction
  boundary.
- **Silent impossible-shape handling.** A guard that returns a default value for an out-of-contract
  shape hides bugs. Assert impossible shapes; handle a shape only when you can explain why it is
  valid input.

Begin the review with a "suspect helpers" inventory: list every new helper/fallback/special case,
what existing mechanism it overlaps with, the test that fails without it, and whether it survived,
was reverted, or was replaced by a producer-side fix. For every flagged change, run this audit
before keeping it:

1. Name the exact input shape, the producing function, and a concrete source or IR example.
2. Decide whether that shape is canonical and intentionally allowed, or an accidental alternative
   spelling that should be eliminated.
3. If it is accidental, try the producer-side fix first so downstream code can use the normal
   `substitute`/`resolve`/canonicalization path.
4. Name the semantic source of truth that already exists. If the change rebuilds syntax or a
   parallel structural form from that source of truth, justify why the stored representation cannot
   be fixed instead.
5. Name the test that fails without the change and explain why that test proves this layer owns the
   logic. When practical, do the revert drill: remove the helper/special case, run the smallest
   failing test, and use the failure to trace the real producer-consumer break.
6. Prefer replacing the special case with an assertion plus a producer fix, or with an existing
   helper that already encodes the invariant.

Do not keep a flagged change only because it makes tests pass. If it remains necessary, the
`Process report` section of the PR description must justify why the input shape is valid and why
this layer owns the logic, with a code trace from producer to consumer.

### Code Style and Review Conventions

Recurring review feedback distilled into rules — following them avoids review round-trips. (These
govern how code reads and is structured; the Problem-Solving Methodology above governs _what_ to
change.)

- **Write function comments as complete sentences: what, then why.** State what the function does
  first; then, if the reason it exists isn't obvious from that description, add a brief summary of
  why. For non-trivial behavior, include a concrete example. Avoid terse fragment- or bullet-only
  comments on a function. For instance, `substituteElementOfCompositeType` should read like _"Return
  `target` with its element type replaced by `newElementType`, preserving shape: scalar →
  newElementType, `vector<T,N>` → `vector<newElementType,N>`, `matrix<T,R,C>` → `matrix<newElementType,R,C>`."_
  — not _"element coerce target"_.

- **Use conversational examples for code comments and PR explanations.** When explaining a subtle
  compiler path, prefer "Consider this example:" followed by the relevant user code. Do not use
  abstract labels such as "Full source shape", "AST trace", or "IR trace" as a substitute for
  explanation. After the code, describe what happens step by step in natural prose: which parser,
  checker, copier, lowering pass, or IR pass creates the shape; what invariant the local code
  preserves; and which downstream consumer depends on that invariant. Include enough of the user's
  original code for the example to make sense on its own.

- **Reuse before you write; then extract non-trivial logic into a named, documented helper.** Before
  writing a new helper, search for an existing one — what you need is often already provided by a
  shared header (the AST/IR helpers in `slang-ast-type.h`, `slang-ir-util.h`, and the various
  `*-util.h` files). For example, to test whether a type is a `DeclRefType` of a particular
  declaration, use the existing `isDeclRefTypeOf<T>(type)` rather than re-deriving it. When the
  logic genuinely is new, don't bury a multi-step computation in an inline lambda or a long inline
  block: give it an intention-revealing name (`coerceOperandsOfBuiltinBinaryExpr`,
  `substituteElementOfCompositeType`, `unifyBaseType`) and a doc comment, so the caller stays
  readable and the helper is reusable.

- **Keep one source of truth; delete dead code after a refactor.** Map or classify a given thing in
  exactly one place — e.g. the operator-name → operation-kind mapping lives only in
  `getBuiltinOperationKindFromString`, not re-implemented at call sites. When a change makes a
  branch, fallback, or helper unreachable, remove it rather than leaving it as dead code.

- **One canonical representation per value; assert the invariant.** Don't introduce a second
  AST/IR/`Val` representation for something that already has one — multiple forms of the same logical
  value break `equals`/identity checks and deduplication. When an invariant guarantees a
  representation is never produced for certain inputs (e.g. `+`/`-`/`*` are always a
  `PolynomialIntVal`, never a `BuiltinOperationIntVal`), `SLANG_ASSERT` it at the construction site
  so a violation is caught rather than silently producing a divergent form.

- **Fail loudly on out-of-contract input.** When a helper is only valid for a restricted set of
  inputs, `SLANG_RELEASE_ASSERT` on anything outside that set instead of silently returning a default
  — e.g. `substituteElementOfCompositeType` asserts its operand is a builtin scalar/vector/matrix.

### PR Workflow

1. **Format your code**: Run `./extras/formatting.sh` before committing
2. **Label your PR**: Use "pr: non-breaking" (default) or "pr: breaking change" (for ABI/language breaking changes)
3. **Include tests**: Add regression tests as `.slang` files under `tests/`
4. **Write the PR description in this required five-part format:**
   1. **Motivation** — the problem being solved, with a concrete example / motivating test case.
   2. **Proposed solution** — the approach, and why it is the principled one.
   3. **Change summary** — a table or list of the files/areas touched and what each does.
   4. **Concepts and vocabulary** — a short glossary, placed between the change summary and the
      process report. Restate only the _codebase-specific or subtle_ terms the report relies on, as
      a reminder for the reviewer (e.g. witness / `getSub`, facet / `getInheritanceInfo`, the
      fixpoint solver, or a non-obvious distinction the fix hinges on). Do **not** explain basic,
      well-known concepts (e.g. interface, associated type) — assume them.
   5. **Process report** — explain _every_ change with a logical reason. For a change that
      addresses a **cascading** issue, describe the issue (with its motivating test case) and
      justify why the fix is correct with a **code trace** (the exact functions/insts involved),
      not just a description. State explicitly why each change is necessary and principled rather
      than a workaround. For any change that handles, guards, or special-cases a particular input
      shape, the report **must** answer the input-shape check from the methodology — _is that shape
      correct and principled, or should its producer have been fixed instead?_ — so a reviewer can
      confirm the fix sits at the right layer.

   Write for a reviewer who does not have the full context in their head. Use the same
   conversational style required for code comments: start from a concrete user-code example, include
   the full relevant snippet instead of just naming a type or function, and explain the logical
   steps in order. Say what the compiler builds, how that representation flows through named
   functions or IR instructions, and why the chosen fix preserves the invariant. Avoid terse labels
   such as "AST trace"; make the prose read like an explanation to a reviewer who is learning the
   scenario for the first time.

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

Use `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` as the test directive to verify that the compiler emits expected diagnostics. Annotations in comments match against compiler output by message text, severity, or error code. Carets align to columns on the preceding source line:

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

- slangc with `-dump-ast`, `-dump-intermediate-prefix`, `-dump-intermediates`,
  `-dump-ir-ids`, `-serial-ir`, and `-dump-repro`.
- slang-test with `-category` and `-api`

### Repro Tooling

`-load-repro` and `-extract-repro` are specialized repro tools; use them when
working on repro handling. Inputs are validated before use.

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

### Capability Atoms Documentation

**`docs/user-guide/a4-02-reference-capability-atoms.md` is auto-generated — never edit it directly.**

It is produced by `slang-capability-generator` from `source/slang/slang-capabilities.capdef`. To add or update a capability atom's description:

1. Add or update the `///` doc comment immediately before the `def` or `alias` in `slang-capabilities.capdef`:
   ```
   /// My description here.
   alias myatom = ...;
   ```
2. Regenerate the doc:
   ```bash
   cmake --build --preset debug --target slang-capability-generator
   mkdir -p build/capgen-out
   ./build/generators/Debug/bin/slang-capability-generator \
       source/slang/slang-capabilities.capdef \
       --target-directory build/capgen-out \
       --doc docs/user-guide/a4-02-reference-capability-atoms.md
   ```
3. Commit the updated `slang-capabilities.capdef` and the regenerated `.md` together.

Note: the `///` comment must be on the **public alias** (e.g. `alias node = _node;`), not on the internal `def _node : stage;` atom, for the description to appear under the public name.

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

### Rebuilding after `hlsl.meta.slang` / `core.meta.slang` changes

The core module source (`hlsl.meta.slang`, `core.meta.slang`, etc.) is embedded into the `slang-bootstrap` binary at compile time. After modifying these files, force CMake to observe the newer timestamp, regenerate the core-module headers through the build graph, then rebuild `slangc` with the preset/configuration you are using:

```bash
cmake -E touch source/slang/hlsl.meta.slang   # or whichever meta file changed
cmake --build --preset <preset> --target generate_core_module_headers
cmake --build --preset <preset> --target slangc
```

Use the same `<preset>` you use for the build, such as `debug`, `release`, or `releaseWithDebugInfo`. The `generate_core_module_headers` target invokes the correct `slang-bootstrap` binary for that host/configuration, including Windows `.exe` paths and non-Debug output directories.

If you skip the `cmake -E touch` step the cached bootstrap binary may silently embed the OLD source, and diagnostics from the bootstrap step will not match the current source file — a sure sign the binary is stale.

### HLSL named-constant emission rule

**Never emit HLSL enum / named-constant values as hard-coded integers.** DXC maps named constants (attribute strings, flag identifiers, etc.) at parse time; if we bake in a numeric value and DXC later changes the internal mapping the generated HLSL will silently break.

The correct pattern:

1. **Define a Slang enum** (or a set of named intrinsic-backed constants) for each group of conceptual values (e.g. `NodeLaunch` mode, Barrier flag sets).
2. **Store the name, not the integer, in the IR.** Use `IRStringLit` operands (as `NodeLaunchDecoration` does) or a `Ref<T>` / intrinsic-based accessor that preserves the identifier through to emission.
3. **Provide a mapping function** in the HLSL emitter (`slang-emit-hlsl.cpp` or `slang-emit-c-like.cpp`) that converts the stored enum/string value back to the HLSL source name so that emitted code reads e.g. `[NodeLaunch("broadcasting")]` not `[NodeLaunch(0)]`.

Examples of this pattern already in the codebase:
- `NodeLaunchDecoration` stores the mode as `IRStringLit("broadcasting")` and the emitter re-emits the string verbatim.
- Work-graph output record `Get()` returns `Ref<T>` backed by `__intrinsic_asm ".Get"` so the emitted HLSL says `.Get(i)` (an l-value in HLSL) rather than an integer offset.

#### Pattern: emitting enum values as target named constants

Use this when a Slang enum must be emitted as named constants rather than integers (e.g. `UAV_MEMORY` instead of `1`).

1. **Define the C++ enum in `slang-type-system-shared.h`** (inside `namespace Slang`, plain `enum` not `enum class` so values implicitly convert to `int`). This header is transitively included by both the core-module source and the emitters.

2. **Mirror it as a Slang enum in the appropriate `*.meta.slang` file**, pulling the actual values from C++ via `$(...)` splices so the two definitions stay in sync:
   ```slang
   enum MyFlags : uint { FlagA = $(MyFlags::FlagA), FlagB = $(MyFlags::FlagB) }
   ```

3. **Declare a `__intrinsic_op` converter in the `.meta.slang` file** to represent the enum-to-string conversion in the IR:
   ```slang
   __intrinsic_op(getEnumMyFlags)
   int GetEnumMyFlags(MyFlags f);
   ```
   The mnemonic passed to `__intrinsic_op(...)` must exactly match the Lua key in the next step.

4. **Register the new IR op in `slang-ir-insts.lua`** and add a stable ID in `slang-ir-insts-stable-names.lua`.

5. **Emit the named-constant string in the target emitter** (e.g. `tryEmitInstExprImpl` in `slang-emit-hlsl.cpp`): keep the IR operation tied to the symbolic enum or intrinsic value, then map each accepted bit or value to its HLSL named-constant string and write it out with `m_writer->emit(...)`. Do not document or implement examples that recover HLSL source names from raw integer positions.

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

### Release Process

Use the `/slang-release-process` skill to push a new release. See `.claude/skills/slang-release-process/SKILL.md` for the full workflow.

## Additional Documents

- User-facing documentation: `docs/user-guide/`
- Language specification: see below

### Formal Specification

Clone `https://github.com/shader-slang/spec.git` under `external/` if needed. Specification files are in `external/spec/specification/`, feature proposals in `external/spec/proposals/`.
