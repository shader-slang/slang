---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:08:24Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 696515884e297ba8af6c8c7765c2f7fdc0119fd11e6bc325c1982bfa8a8062d2
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Architectural Overview

This document is the entry point for the LLM-generated architectural
documentation under [docs/generated/design/](..). It introduces Slang as a
codebase: the public artefacts it produces, the major subsystems that
make up the source tree, and the central data objects that flow through
a compilation. After reading it you should know which subdirectory under
[source/](../../../../source) implements any given concern, and where to
go next for detail. It is written for a competent C++ engineer who has
not yet opened the Slang source tree.

## Purpose

Slang is a shading-language compiler. Its job is to take Slang (and
HLSL-compatible) source code and produce target code for a graphics or
compute toolchain — DXIL, SPIR-V, GLSL, Metal Shading Language, WGSL,
C++, CUDA, or PyTorch glue — together with reflection / layout
information about the shader parameters.

The build produces four primary public artefacts:

- `slangc`: the command-line compiler driver, built from
  [source/slangc/](../../../../source/slangc).
- `slang-compiler` (a shared library): the embeddable compiler; its
  public C API and COM-style interfaces are declared in
  [include/slang.h](../../../../include/slang.h). Under
  `SLANG_ENABLE_SLANG_PROXY` the historical names are still produced as
  backward-compatibility aliases: a `libslang` symlink on Unix and a
  forwarding `slang.dll` on Windows.
- the WebAssembly bindings built from
  [source/slang-wasm/](../../../../source/slang-wasm), which expose the
  compiler to JavaScript together with a generated `interface.d.ts`.
- `slang-rt`: optional runtime support used by code emitted to non-GPU
  targets, built from [source/slang-rt/](../../../../source/slang-rt).

The compiler is built with CMake. The full build configuration for the
core library lives in
[source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt);
each peer subdirectory under [source/](../../../../source) has its own
`CMakeLists.txt` that contributes to the same build.

## Top-level decomposition

The source tree is layered. Lower layers do not depend on upper layers,
and the public API in [include/](../../../../include) sits above
everything else as an immutable boundary.

### Foundational layers

- [source/core/](../../../../source/core) — platform-agnostic C++
  utilities: containers, strings, smart pointers, file-system
  abstractions, hashing, allocation. Nothing else in the project would
  link without it. Representative file:
  [slang-basic.h](../../../../source/core/slang-basic.h). The allocator
  can optionally be backed by mimalloc: setting `SLANG_ENABLE_MIMALLOC`
  makes
  [source/core/CMakeLists.txt](../../../../source/core/CMakeLists.txt)
  link `mimalloc-static` into `core` and define
  `SLANG_ENABLE_MIMALLOC=1` for every dependent target.
- [source/compiler-core/](../../../../source/compiler-core) — language-
  agnostic compiler infrastructure that could in principle be reused by
  another language: lexer
  ([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)),
  diagnostic sink
  ([slang-diagnostic-sink.h](../../../../source/compiler-core/slang-diagnostic-sink.h)),
  source-location encoding, downstream-compiler glue (DXC, FXC, GCC,
  glslang), and the artifact / blob model used to carry compiled
  outputs.

### The compiler proper

- [source/slang/](../../../../source/slang) — the Slang frontend, AST,
  intermediate representation, IR passes, and emit backends. This is
  the bulk of the compiler. Subgroups (file-name prefix conventions):
  `slang-ast-*` (AST), `slang-parser*` and `slang-preprocessor*`
  (frontend), `slang-check*` (semantic checking), `slang-lower-to-ir*`
  (AST→IR lowering), `slang-ir.*` and `slang-ir-insts.*` (IR core),
  `slang-ir-*.cpp` (IR passes), `slang-emit*` (code emission),
  `slang-serialize*` (AST/IR/RIFF serialization),
  `slang-capability*` (the capability lattice),
  `slang-diagnostics*` (Slang-specific diagnostic catalog).
  See [module-map.md](module-map.md) for the file-level inventory.
- [prelude/](../../../../prelude) — per-target prelude headers that the
  compiler ships alongside emitted text targets so the downstream
  toolchain can compile the result. One prelude per textual target
  family (HLSL, CUDA, C++, Torch).

### Standard libraries

- [source/slang-core-module/](../../../../source/slang-core-module) and
  the `*.meta.slang` files inside [source/slang/](../../../../source/slang)
  ([core.meta.slang](../../../../source/slang/core.meta.slang),
  [hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang),
  [diff.meta.slang](../../../../source/slang/diff.meta.slang)) —
  the core module that defines built-in types, intrinsics, and operator
  mappings. Embedded into `slang-compiler` at build time. When
  `SLANG_EMBED_CORE_MODULE` is off and `slang-compiler` is built as a shared
  library,
  [source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt)
  instead adds a `generate_core_module_cache` target that writes a
  `slang-core-module.bin` cache next to the library, so the first
  global-session request loads that archive rather than compiling the
  core module from source.
- [source/slang-glsl-module/](../../../../source/slang-glsl-module) and
  [source/slang/glsl.meta.slang](../../../../source/slang/glsl.meta.slang)
  — analogous module for GLSL-flavoured intrinsics.
- [source/standard-modules/](../../../../source/standard-modules) —
  standard-module sources that are shipped but not embedded in the
  same way. Each subdirectory builds a separate `.slang-module`
  artifact: `neural`
  ([source/standard-modules/neural/](../../../../source/standard-modules/neural))
  and `experimental`
  ([source/standard-modules/experimental/](../../../../source/standard-modules/experimental)),
  which currently holds the work-graph module.

### Downstream-compiler shims

- [source/slang-llvm/](../../../../source/slang-llvm) — LLVM-based JIT /
  static compilation glue
  ([slang-llvm.cpp](../../../../source/slang-llvm/slang-llvm.cpp)).
- [source/slang-glslang/](../../../../source/slang-glslang) — bridge to
  Khronos `glslang` for SPIR-V generation via GLSL
  ([slang-glslang.cpp](../../../../source/slang-glslang/slang-glslang.cpp)).
- [source/slang-dispatcher/](../../../../source/slang-dispatcher) —
  shared support for dispatching to downstream tools
  ([main.cpp](../../../../source/slang-dispatcher/main.cpp)).

### Runtime and bindings

- [source/slang-rt/](../../../../source/slang-rt)
  ([CMakeLists.txt](../../../../source/slang-rt/CMakeLists.txt)) —
  runtime library linked into host-style CPU outputs, including the
  PyTorch binding output; CUDA source is kernel-style and does not
  take this dependency.
- [source/slang-record-replay/](../../../../source/slang-record-replay)
  — recorder/replayer for the public Slang API
  ([replay-context.cpp](../../../../source/slang-record-replay/replay-context.cpp)).
- [source/slang-wasm/](../../../../source/slang-wasm) — WebAssembly
  bindings
  ([slang-wasm-bindings.cpp](../../../../source/slang-wasm/slang-wasm-bindings.cpp)).

### Driver and tooling

- [source/slangc/](../../../../source/slangc) — the `slangc` command-line
  driver ([main.cpp](../../../../source/slangc/main.cpp)).
- [tools/](../../../../tools) — auxiliary developer tools (testing,
  reflection, code generation, fiddle, embed, profiling).

### Auxiliary trees (outside `source/`)

- [tests/](../../../../tests) — the project's test corpus. Most files
  are `.slang` inputs driven by `slang-test` (see
  [tools/slang-test/](../../../../tools/slang-test)). New regression
  tests live here; the directory structure mirrors the area being
  tested (e.g. `tests/language-feature/`, `tests/diagnostics/`,
  `tests/spirv/`).
- [extras/](../../../../extras) — developer scripts and small helpers
  that are not built into any binary: formatting
  ([extras/formatting.sh](../../../../extras/formatting.sh)), IR-dump
  splitting ([extras/split-ir-dump.py](../../../../extras/split-ir-dump.py)),
  Windows sandbox build helpers, etc. Anything in this tree exists
  only to support developers and is not shipped to end users.
- [external/](../../../../external) — third-party dependencies and git
  submodules (spirv-headers, glslang, lz4, miniz, …). Code here is
  vendored, not modified.

### Build-time generated code

Slang relies heavily on build-time code generation. The macro
`FIDDLE(...)` in AST and IR headers expands to additional members /
visitors / serialization tables produced under
`build/source/slang/fiddle/` (e.g.
`slang-ir-insts-enum.h.fiddle` enumerates IR opcodes from the Lua table
in [slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua)).
Diagnostic catalogs are similarly generated from
[slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua)
and the per-area Lua tables under
[source/slang/diagnostics/](../../../../source/slang/diagnostics).

## Compilation request lifecycle

A Slang compilation flows through a small set of central objects whose
declarations are spread across
[include/slang.h](../../../../include/slang.h) and several headers under
[source/slang/](../../../../source/slang) — including
`slang-translation-unit.h`, `slang-session.h`, `slang-target.h`,
`slang-compile-request.h`, `slang-end-to-end-request.h`, and
`slang-module.h`.

- `Session` — global-session-scoped compiler state. Owns built-in modules, the
  AST builder, and the global type-checking environment. The COM-style
  public interface is `slang::IGlobalSession`
  ([include/slang.h](../../../../include/slang.h)); the implementation
  class is `Session` in
  [slang-global-session.h](../../../../source/slang/slang-global-session.h).
- `Linkage` — a configuration scope that bundles search paths,
  preprocessor macros, target settings, and a source manager. Multiple
  modules share a `Linkage` so they can resolve `import`s against each
  other consistently. `Linkage` is what the public `slang::ISession`
  interface ([include/slang.h](../../../../include/slang.h)) actually
  implements — see
  [slang-session.h](../../../../source/slang/slang-session.h). In this
  codebase "session" therefore means *two different things*:
  `IGlobalSession` holds the global-session-scoped state that
  applications usually create once and reuse to amortize startup cost
  (though distinct global sessions may coexist), and `ISession` is what
  most callers think of as a "compile session".
- `TranslationUnitRequest` — a collection of source files that share a
  namespace. By default, all Slang source files passed to `slangc` go
  into a single `TranslationUnitRequest`; HLSL inputs go one-per-unit.
- `FrontEndEntryPointRequest` — a function name plus the profile whose
  stage the entry point is compiled for (e.g. `main` as `compute`).
  Declared in
  [slang-compile-request.h](../../../../source/slang/slang-compile-request.h).
- `TargetRequest` — an output format combined with a profile
  (e.g. SPIR-V at `glsl_450`).
- `FrontEndCompileRequest` — drives the front-end (parse, check, lower
  to IR) for a set of translation units. Declared in
  [slang-compile-request.h](../../../../source/slang/slang-compile-request.h).
- `CodeGenContext` — drives the back-end (IR passes, emit) for a set of
  entry points and targets. Declared in
  [slang-code-gen.h](../../../../source/slang/slang-code-gen.h); it
  replaces the historical `BackEndCompileRequest` object that earlier
  revisions named here.
- `EndToEndCompileRequest` — the umbrella object behind a single
  `slangc` invocation, declared separately in
  `slang-end-to-end-request.h`.
- `Module` — the front-end output for a translation unit. Implements
  the public `slang::IModule`
  ([include/slang.h](../../../../include/slang.h)) and contains both the
  checked AST and the lowered IR; it is also the unit that the
  `.slang-module` serialization format describes. Declared in
  [slang-module.h](../../../../source/slang/slang-module.h).
- `IComponentType` — the linkable-program abstraction. A `Module`,
  an entry-point binding, or a composite of these can all be presented
  as an `IComponentType`; the back-end consumes a single composite
  component for code generation.

The objects above explain why concepts like "translation unit" and
"target" are first-class: Slang's pipeline is parameterized so that
front-end work is cleanly separated from back-end work. Parsing and
reusable module checking are largely target-independent, but
entry-point validation does consult the requested targets — it compares
inferred capability requirements and profile stages against every
`TargetRequest` in
[slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp)
— while all code generation happens in the back end.

## Where the public API lives

[include/slang.h](../../../../include/slang.h) is the canonical public
header. It declares the COM-style interfaces (`ISession`, `IModule`,
`IComponentType`, ...) and the small handful of free functions used to
create a session. Together with
[include/slang-com-helper.h](../../../../include/slang-com-helper.h) and
[include/slang-com-ptr.h](../../../../include/slang-com-ptr.h), this is
the binary-stable surface that downstream applications link against.

Anything under [source/](../../../../source) is implementation. The
public-header rules in [CLAUDE.md](../../../../CLAUDE.md) (no enum
re-ordering, no virtual-method changes mid-vtable, no removal) reflect
the fact that this surface must keep ABI compatibility with older
callers. New API is therefore added by *appending*: methods are
appended to the end of an interface (e.g.
`IGlobalSession::getDownstreamCompilerVersion`), new capabilities arrive
as fresh UUID'd interfaces obtained via `castAs` /
`queryInterface` (e.g. `IBindlessResourceMetadata`), and new
`CompilerOptionName` enumerators are appended with an explicit integer
value before the `CountOf` sentinel, which is the one enumerator that
deliberately has no explicit value — all visible in
[include/slang.h](../../../../include/slang.h).

Superseded entry points are deprecated rather than deleted. When a
replacement lands, the old declaration stays at its original vtable /
overload position and is annotated `SLANG_DEPRECATED`, so existing
callers keep compiling and linking. `VariableReflection` shows the
pattern: `getDefaultValueBlob` returns a variable's default initializer
as a packed byte blob, while the narrower `hasDefaultValue`,
`getDefaultValueInt`, and `getDefaultValueFloat` it replaces remain
declared and marked deprecated.

## Reading guide

To go deeper, follow one of these paths:

- For an exhaustive file inventory grouped by subsystem, read
  [module-map.md](module-map.md).
- For inter-subsystem dependencies, read
  [dependency-graph.md](dependency-graph.md).
- For the end-to-end compilation flow with one document per stage,
  start at [../pipeline/overview.md](../pipeline/overview.md).
- For concerns that span every stage, see the
  [../cross-cutting/](../cross-cutting) tree:
  [diagnostics](../cross-cutting/diagnostics.md),
  [IR instructions](../cross-cutting/ir-instructions.md),
  [targets](../cross-cutting/targets.md),
  [core module](../cross-cutting/core-module.md), and
  [serialization](../cross-cutting/serialization.md).
- For surface syntax, see [../syntax-reference/](../syntax-reference).

The handwritten developer notes in [docs/design/](../../../design)
overlap with this tree and are also useful, particularly
[overview.md](../../../design/overview.md),
[ir.md](../../../design/ir.md), and
[parsing.md](../../../design/parsing.md).
