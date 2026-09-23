---
generated: true
model: claude-opus-5[1m]
generated_at: 2026-09-11T00:00:00Z
source_commit: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
watched_paths_digest: 95ff3cab526c686528a630d76f0bf7e9952fec44677b796b3e89d2d11ae228ae
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
`CMakeLists.txt` that contributes to the same build. Paths in that file
are written relative to `slang_BINARY_DIR` rather than
`CMAKE_BINARY_DIR` — the `copy_slang_headers` target stages
[include/](../../../../include) plus the generated
`slang-tag-version.h` into `${slang_BINARY_DIR}/$<CONFIG>/include`
(lines 225-236) — so that a parent project embedding Slang with
`add_subdirectory` collects the staged headers under Slang's own build
directory instead of the top-level one.

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
  Exports from this shim are not determined by the C++ alone: an entry
  point must also be listed in `slang-glslang.version-script`, from
  which both the ELF and macOS builds derive their export list, or the
  client's symbol lookup returns null and the entry point is silently
  unavailable.
- [source/slang-dispatcher/](../../../../source/slang-dispatcher) —
  shared support for dispatching to downstream tools
  ([main.cpp](../../../../source/slang-dispatcher/main.cpp)).

A downstream compiler is reached through `IDownstreamCompiler`, declared
in
[slang-downstream-compiler.h](../../../../source/compiler-core/slang-downstream-compiler.h).
Capabilities that only some of them have are modelled as separate
interfaces rather than as extra methods on that one: for instance
`IDownstreamCompilerPathProvider` (line 403), whose single `getPath`
returns the on-disk path of the loaded compiler library.
`DownstreamCompilerBase` supplies a default that returns
`SLANG_E_NOT_AVAILABLE`, and only the shared-library-backed compilers
override it — an executable-based command-line compiler found on `PATH`,
or a platform without shared-library introspection, has no such path.
Note that this interface is deliberately *not* `ICastable`-derived and
is handed out only as a borrowed object through `castAs` / `getObject`,
never through `getInterface`: `getInterface` also backs the
ref-counting `queryInterface`, so it must return only releasable
`ISlangUnknown`-derived interfaces, and giving the capability a second
`ICastable` base would leave the concrete compiler class with two
ambiguous `ISlangUnknown` subobjects.
[slang-llvm.cpp](../../../../source/slang-llvm/slang-llvm.cpp)
implements the pattern on `LLVMDownstreamCompiler`.

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
  The Emscripten binding block is the authoritative list of what
  JavaScript can reach: `GlobalSession` currently exposes
  `createSession` and `getBuiltinModuleSource`, the latter handing back
  the source text of a built-in module by name.

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
callers. New API is therefore added by *appending*, and the header shows
four distinct mechanisms for growing without moving anything a compiled
caller already depends on:

- **Appending a method to an interface.** New methods go at the end of
  the vtable (e.g. `IGlobalSession::saveBuiltinModule`).
- **A fresh UUID'd interface.** A capability that not every
  implementation has becomes its own interface, obtained via `castAs` /
  `queryInterface` (e.g. `IBindlessResourceMetadata`,
  `ICoverageTracingMetadata`, `ISyntheticResourceMetadata`), rather
  than a method on an existing one.
- **Appending an enumerator.** New `CompilerOptionName` values are
  appended with an explicit integer before the `CountOf` sentinel, which
  is the one enumerator that deliberately has no explicit value — the
  two most recent are `TraceCoverageBindlessIndex = 158` and
  `GetCompilerPath = 159`.
- **Tail-extending a plain struct behind `structSize`.** A struct passed
  across the boundary carries its own size as its leading field, and new
  trailing members are written only when the caller's `structSize`
  covers them. `SyntheticResourceInfo::bindlessIndex` is the current
  example: it sits past the v1 struct size, so a caller compiled against
  an older header keeps its own layout and never sees the field.

Superseded entry points are normally deprecated rather than deleted.
When a replacement lands, the old declaration stays at its original
vtable / overload position and is annotated deprecated, so existing
callers keep compiling and linking. `VariableReflection` shows the
pattern: `getDefaultValueBlob` returns a variable's default initializer
as a packed byte blob, while the narrower `hasDefaultValue`,
`getDefaultValueInt`, and `getDefaultValueFloat` it replaces remain
declared and marked deprecated. `IGlobalSession::addBuiltins` is
annotated the same way — `[[deprecated]]` on the declaration, which
keeps the vtable slot occupied and so preserves the position of every
method after it.

Two departures from that pattern are worth knowing about, because
reading the header alone would otherwise suggest the rules are absolute.

`IGlobalSession`'s final method was *replaced in place* rather than
appended to: `getDownstreamCompilerVersion(SlangPassThrough, int*, int*)`
became `getDownstreamCompilerPath(SlangPassThrough, ISlangBlob**)` in
commit `b9a17f86b1`, occupying the same last vtable slot with a
different name and signature. The replacement answers a different
question — where the library Slang selected actually lives on disk, so a
client can load it and query capabilities itself — rather than a version
pair that several downstream compilers could only report as `(0, 0)`.
Being the last slot, and having been introduced only shortly before, the
change perturbs no earlier method's position.

Correspondingly, the `CompilerOptionName` enumerator that drove the old
CLI query, `CompilerVersion = 153`, was removed outright rather than
renamed to `REMOVED_CompilerVersion`. The integer `153` is now a hole in
the sequence — `SPIRVUnifiedDescriptorHeapStride = 154` follows the gap —
so the retired value is at least not reused, which is the part of the
rule that protects a caller holding a stale integer.

The versioning of the *language* accepted by the compiler is separate
from this ABI surface and is enumerated by `SlangLanguageVersion`.
Alongside the year-numbered `SLANG_LANGUAGE_VERSION_2025` / `_2026`
there are now letter-suffixed aliases for the same integers
(`_202A = 2025`, `_202B = 2026`) plus `_202C = 2027` for the in-progress
version, whose numeric value the header explicitly warns may change once
that version is given an official name. `SLANG_LANGUAGE_VERSION_LATEST`
names the latest stable version and `_NEXT` the development one.

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
