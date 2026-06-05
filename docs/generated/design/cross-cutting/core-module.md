---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-05T09:24:37Z
source_commit: 52339028a2aa703271533454c6b9528a534bac31
watched_paths_digest: 19ad329c51b4e53e37b131c94d49631623fa525a7de092b35d5852c27a4bca02
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Core Module and Preludes

This document describes the bundled standard libraries (the core
module, the GLSL module, and the standard modules) and the per-target
preludes shipped under [prelude/](../../../../prelude). The intended
reader is a developer adding a built-in function, intrinsic, or per-
target prelude entry.

## What ships with the compiler

Three families of "shipped Slang code" exist, with distinct build-
time treatments:

1. **The core module** — a set of Slang `*.meta.slang` source files
   in [source/slang/](../../../../source/slang), embedded directly
   into `libslang` so the compiler can use them at compile time. They
   define built-in types, conversion rules, intrinsics, and per-target
   spellings.
2. **The GLSL module** — analogous embedded module that ships GLSL-
   flavoured names.
3. **The standard modules** — separately compiled `.slang-module`
   files installed alongside the compiler binary and loaded on demand
   via `import slang.<name>` (currently the `neural` module).

Per-target preludes ([prelude/](../../../../prelude)) are a separate
notion: those are C / C++ / CUDA headers shipped alongside emitted
text targets so that the downstream toolchain can compile what Slang
emits. They are not Slang source.

## Core module

The Slang sources for the core module are:

- [core.meta.slang](../../../../source/slang/core.meta.slang) — base
  types (`int8_t`, `int32_t`, `int64_t`, `float`, `half`, `double`,
  pointer / size types) and type aliases. From the file's preamble
  ("public module core;"), this is a Slang module declared with the
  `core` name.
- [hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang) — HLSL-
  compatibility names (`Texture2D`, `RWTexture2D`,
  `StructuredBuffer`, intrinsics like `mul`, `dot`, `length`, ...).
- [diff.meta.slang](../../../../source/slang/diff.meta.slang) —
  differentiable-pair types and helpers used by the autodiff machinery
  ([../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)).

The embedding glue lives in
[source/slang-core-module/](../../../../source/slang-core-module):

- [slang-embedded-core-module.cpp](../../../../source/slang-core-module/slang-embedded-core-module.cpp)
  — links the precompiled core module bytes into `libslang`. Used
  when the CMake option `SLANG_EMBED_CORE_MODULE` is on (the default).
- [slang-embedded-core-module-source.cpp](../../../../source/slang-core-module/slang-embedded-core-module-source.cpp)
  — a variant that embeds the original `.slang` source rather than
  the precompiled bytes (used by the bootstrap build flow when
  generating the embedded artefact in the first place).

When `SLANG_EMBED_CORE_MODULE=OFF` (see
[CLAUDE.md](../../../../CLAUDE.md)), the core module is compiled
out-of-line. This is useful during development because errors in
`*.meta.slang` files no longer break the C++ compile of `slangc` or
`slang-test`; they only surface at runtime when those binaries try
to load the module.

The selection between the two embedding strategies is expressed as a
generator expression in
[source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt)
(picking either `slang-embedded-core-module` /
`slang-embedded-core-module-source` or their `slang-no-...` siblings).

### What the core module provides

The core module file sets up the language vocabulary that user code
(and the meta-modules themselves) rely on. From a freshly-checked
[core.meta.slang](../../../../source/slang/core.meta.slang) it begins
with:

```
public module core;

// Slang `core` library

typedef half float16_t;
typedef float float32_t;
typedef double float64_t;
typedef int int32_t;
typedef uint uint32_t;
typedef uintptr_t size_t;
// ...
```

The full set of declarations covers scalar / vector / matrix types,
operator overloads (mapped to IR opcodes via
`__intrinsic_op` / `__target_intrinsic` modifiers — see
[../syntax-reference/keywords-and-builtins.md](../syntax-reference/keywords-and-builtins.md)),
implicit conversions, ranges and iterators, and the `Optional`,
`Result`, `Tuple` types.

The HLSL meta-module layers in HLSL-named texture / sampler / buffer
types and the corresponding intrinsics so that HLSL code compiles
unchanged.

The diff meta-module declares the `IDifferentiable` interface and the
differentiable-pair types consumed by the autodiff IR passes.

## GLSL module

[glsl.meta.slang](../../../../source/slang/glsl.meta.slang) provides
GLSL-flavored aliases (`vec3`, `mat4`, `gl_*` system values) and is
embedded by
[source/slang-glsl-module/](../../../../source/slang-glsl-module) via
[slang-embedded-glsl-module.cpp](../../../../source/slang-glsl-module/slang-embedded-glsl-module.cpp).
Loading it is target-conditional: the compiler pulls it in
when the user is compiling GLSL or asks for GLSL-flavoured names from
Slang code.

## Standard modules

Standard modules are independently compiled `.slang-module` files
shipped in a versioned directory next to the `libslang` artefact and
loaded at runtime via `import slang.<name>`. The build infrastructure
is in
[source/standard-modules/](../../../../source/standard-modules) and is
described in detail by
[source/standard-modules/README.md](../../../../source/standard-modules/README.md).

The single standard module shipping today is the **neural** module:

- Source: [source/standard-modules/neural/](../../../../source/standard-modules/neural)
  containing files such as
  `neural.slang` (entry point),
  `iactivation.slang`, `iencoder.slang`, `ilayer.slang`,
  `ivector.slang`, `istorages.slang`,
  `accelerate-vector-coopmat.slang`,
  `activations.slang`,
  `bindless-storage.slang`,
  `hash-function.slang`,
  `inline-vector.slang`,
  `layers.slang`,
  `mma-linear-layout-help.slang`,
  `mma-tiled-cuda.slang`,
  `mma-tiled-layout-helper.slang`,
  `mma-tiled-metal.slang`,
  `mma-tiled-vulkan.slang`,
  `network-parameter-layout-converter.slang`,
  `permuto-encoder.slang`,
  `shared-memory-pool.slang`,
  `vectorized-reader.slang`,
  `WaveMatrix.slang`.
- Configuration template:
  [slang-standard-module-config.h.in](../../../../source/standard-modules/slang-standard-module-config.h.in)
  is processed by CMake into the internal header
  `slang-standard-module-config.h`, which carries the
  versioned install-directory name and the per-module file names.
- Runtime lookup: `slang-session.cpp` consumes the generated
  configuration header to locate the standard modules at
  `${SLANG_STANDARD_MODULE_DIR_NAME}/slang/` next to the loaded
  `libslang` (per
  [source/standard-modules/README.md](../../../../source/standard-modules/README.md)).

The standard-module mechanism is intended to grow: new modules go
under `source/standard-modules/<name>/` with an `add_subdirectory` in
the parent `CMakeLists.txt`.

## Preludes

Preludes are headers emitted alongside textual target output so that
the downstream toolchain can compile what Slang emits. They are
**output-side** rather than input-side: they do not participate in
front-end checking; they are referenced from emitted code via
`#include "<prelude>"`-style mechanisms in the relevant emit
backend (see [../pipeline/06-emit.md](../pipeline/06-emit.md)).

| Prelude | Target |
| --- | --- |
| [slang-cpp-prelude.h](../../../../prelude/slang-cpp-prelude.h) | C++ shader output |
| [slang-cpp-types-core.h](../../../../prelude/slang-cpp-types-core.h) | C++ shared core types |
| [slang-cpp-types.h](../../../../prelude/slang-cpp-types.h) | C++ extended types |
| [slang-cpp-scalar-intrinsics.h](../../../../prelude/slang-cpp-scalar-intrinsics.h) | C++ scalar intrinsic implementations |
| [slang-cpp-host-prelude.h](../../../../prelude/slang-cpp-host-prelude.h) | Host-side C++ runtime |
| [slang-cuda-prelude.h](../../../../prelude/slang-cuda-prelude.h) | CUDA |
| [slang-hlsl-prelude.h](../../../../prelude/slang-hlsl-prelude.h) | HLSL |
| [slang-llvm.h](../../../../prelude/slang-llvm.h) | `slang-llvm` integration |
| [slang-torch-prelude.h](../../../../prelude/slang-torch-prelude.h) | PyTorch glue |

GLSL, Metal, WGSL, and SPIR-V do not use a `prelude/` header in the
same way; their built-in vocabularies are emitted directly from the
backends or handled by the downstream toolchain.

## Building the core module

From [CLAUDE.md](../../../../CLAUDE.md) and
[source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt):

- `cmake -DSLANG_EMBED_CORE_MODULE=ON` (the default) bakes the
  precompiled core module into `libslang`. Errors in
  `*.meta.slang` show up at C++ build time because the embedded
  artefact is a build product of the `*.meta.slang` sources.
- `cmake -DSLANG_EMBED_CORE_MODULE=OFF` keeps the C++ build of
  `slangc` and `slang-test` independent of the core-module
  compilation. Errors in `*.meta.slang` then surface only at runtime
  when those tools try to use the missing or broken module.

The `SLANG_EMBED_CORE_MODULE_SOURCE` option similarly controls
whether the original Slang source text is embedded alongside the
precompiled bytes (used by `slang-bootstrap` for cross-compilation
scenarios).

## Adding a new built-in

To add an intrinsic visible to user code:

1. Decide the home: the core module
   ([core.meta.slang](../../../../source/slang/core.meta.slang)) for
   universal language additions; the HLSL or GLSL meta-module for
   dialect-specific names; the diff meta-module for differentiation
   support.
2. Declare the function or type. Use modifiers such as
   `__intrinsic_op(<IROp>)` or `__target_intrinsic(<target>, <text>)`
   to map it onto the IR or per-target spelling — see
   [../syntax-reference/keywords-and-builtins.md](../syntax-reference/keywords-and-builtins.md)
   for the registered modifier vocabulary.
3. If the new intrinsic needs a runtime helper in emitted code, add
   the corresponding entry in the appropriate prelude under
   [prelude/](../../../../prelude) and arrange for the emit backend
   to bring it into scope (see
   [../pipeline/06-emit.md](../pipeline/06-emit.md)).
4. Rebuild. With `SLANG_EMBED_CORE_MODULE=ON` the rebuild
   reproduces the embedded artefact; with `OFF` the runtime simply
   picks up the new sources on the next compile.
5. Add tests under [tests/](../../../../tests).

## What is not in this document

- The full intrinsic list. The authoritative source is the
  `*.meta.slang` files; enumerating them here would replicate a
  generated artefact and drift on every change.
- The user-visible documentation of the standard modules. Per-module
  documentation lives alongside the source (e.g. in
  [source/standard-modules/neural/](../../../../source/standard-modules/neural))
  and in the [user guide](../../../user-guide).
