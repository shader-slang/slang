---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-03T00:00:00Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: d4e2be8995c8c95a45df9e148d2a3e7eca71adcea84c5108fd81038c3a296350
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
   by an `import` whose qualified name maps to a subdirectory
   (currently `import slang.neural`, `import experimental.workgraph`,
   `import slang.numerics` and `import slang.functional`).

Per-target preludes ([prelude/](../../../../prelude)) are a separate
notion: C / C++ / CUDA headers shipped alongside emitted text targets.
They are not Slang source.

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
- [diff.meta.slang](../../../../source/slang/diff.meta.slang) — the
  autodiff surface consumed by the differentiation IR passes
  ([../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)): the
  `attribute_syntax` declarations for `[ForwardDerivative]`,
  `[BackwardDerivative]`, `[PrimalSubstitute]`, their `...Of` variants,
  `[DerivativeMember]`, and `[NoDiffThis]`; the `NullDifferential`
  type; `IDifferentiable` / `IDifferentiablePtrType` conformances for
  `Array`, `Optional`, and `Tuple`; and the PyTorch-facing
  `TensorView`, `DiffTensorView`, and `TorchTensor` types.

An HLSL-compatibility name is not a call on every target. These
intrinsics are written as a `__target_switch` whose cases give the
per-target form, with the ordinary Slang body under `default` used
where no case matches — that body is emitted as a generated helper
function. `mul` is the sharpest case; for `mul(m, v)` on a `float4x4`
and a `float4`:

| Target | Emitted form |
| --- | --- |
| HLSL | `mul(m, v)` |
| GLSL, Metal, WGSL | `(v * m)` — the `*` operator, not a call |
| SPIR-V | `OpVectorTimesMatrix` |
| CUDA, C++ | a generated `mul_<n>` helper from the `default` body |

`dot` and `length` keep their names on HLSL, GLSL, Metal and WGSL,
become `OpDot` and `GLSL.std.450` `Length` on SPIR-V, and fall through
to generated `dot_<n>` / `length_<n>` helpers on CUDA and C++.

The embedding glue lives in
[source/slang-core-module/](../../../../source/slang-core-module):

- [slang-embedded-core-module.cpp](../../../../source/slang-core-module/slang-embedded-core-module.cpp)
  — links the precompiled core module bytes into `libslang` when
  `SLANG_EMBED_CORE_MODULE` is on (the default).
- [slang-embedded-core-module-source.cpp](../../../../source/slang-core-module/slang-embedded-core-module-source.cpp)
  — a variant that embeds the original `.slang` source rather than
  the precompiled bytes (used by the bootstrap build flow).

When `SLANG_EMBED_CORE_MODULE=OFF` (see
[CLAUDE.md](../../../../CLAUDE.md)), the library carries the embedded
meta-source instead and the module is compiled out-of-line (see
[Building the core module](#building-the-core-module)). Compiling the module at runtime is the
fallback used when neither an embedded module nor a valid cache is
available (see
[The runtime core-module cache](#the-runtime-core-module-cache)).

The top-level [CMakeLists.txt](../../../../CMakeLists.txt) requires at
least one of `SLANG_EMBED_CORE_MODULE` and
`SLANG_EMBED_CORE_MODULE_SOURCE` to be enabled.

### What the core module provides

The core module file sets up the language vocabulary that user code
(and the meta-modules themselves) rely on. It opens with
`public module core;` and a block of scalar aliases (`float16_t`,
`float32_t`, `float64_t`, `int32_t`, `uint32_t`, `size_t`, `usize_t`,
`ssize_t`), then declares modifier syntax (`constexpr`,
`globallycoherent`, `pervertex`, ...) via `syntax` declarations. These
are real modifiers rather than merely accepted spellings, and some of
them survive lowering into the emitted code: a `globallycoherent`
buffer emits `globallycoherent` on HLSL, `coherent` on GLSL, and a
`Coherent` decoration on SPIR-V.

`matrix` shows how a C++-side constant is given a Slang-side type.
Its fourth generic parameter is
`let L : MatrixLayoutMode = MatrixLayoutMode.Unknown`
([core.meta.slang](../../../../source/slang/core.meta.slang) line 2372),
and `MatrixLayoutMode` is a `//@hidden:` enum immediately above it
(line 2362) whose cases splice their values out of the C++ macros
(`Unknown = $(SLANG_MATRIX_LAYOUT_MODE_UNKNOWN)`, `RowMajor`,
`ColumnMajor`) so the two definitions cannot drift apart.

The full set of declarations covers scalar / vector / matrix types,
operator overloads mapped onto IR opcodes with the `__intrinsic_op`
modifier (see
[../syntax-reference/keywords-and-builtins.md](../syntax-reference/keywords-and-builtins.md)),
implicit-conversion costs declared with `__implicit_conversion`, the
`IRangedValue` interface and its per-scalar extensions, `Optional`
and `Tuple`, and the autodiff vocabulary itself — `IDifferentiable`
and the `DifferentialPair` magic type are declared here in
[core.meta.slang](../../../../source/slang/core.meta.slang), not in
the diff meta-module.

The HLSL meta-module layers in HLSL-named texture / sampler / buffer
types and the corresponding intrinsics so that HLSL code compiles
unchanged. It is also where the `__target_intrinsic(<target>, <text>)`
modifier is used to give a declaration a per-target spelling (for
example the `hlsl` and `cuda` spellings of `RayDesc` and its fields).

## GLSL module

[glsl.meta.slang](../../../../source/slang/glsl.meta.slang) provides
GLSL-flavored aliases (`vec3`, `mat4`, `gl_*` system values) and is
embedded by
[source/slang-glsl-module/](../../../../source/slang-glsl-module) via
[slang-embedded-glsl-module.cpp](../../../../source/slang-glsl-module/slang-embedded-glsl-module.cpp).
The global session loads the GLSL builtin module at creation time when
`SlangGlobalSessionDesc::enableGLSL` is set (see
[slang-api.cpp](../../../../source/slang/slang-api.cpp)); a later
`import glsl` then retrieves that already-loaded builtin module via the
`glslModuleName` special-case in
[slang-session.cpp](../../../../source/slang/slang-session.cpp).

## Standard modules

Standard modules are independently compiled `.slang-module` files
shipped in a versioned directory next to the compiler shared library
(`libslang.so` / `slang.dll`, as the comment on `findStandardModulePath`
in
[slang-session.cpp](../../../../source/slang/slang-session.cpp) line 43
puts it) and loaded at runtime by an `import` whose qualified name maps
to a subdirectory of that directory. The build infrastructure is in
[source/standard-modules/](../../../../source/standard-modules) and is
described in detail by
[source/standard-modules/README.md](../../../../source/standard-modules/README.md).

Each subdirectory under
[source/standard-modules/](../../../../source/standard-modules) has its
own `CMakeLists.txt`, is pulled in by an `add_subdirectory` call in
[standard-modules/CMakeLists.txt](../../../../source/standard-modules/CMakeLists.txt),
and produces one or more `.slang-module` artifacts. Four exist today:

| Directory | Entry point | Module file name variable | Import path |
| --- | --- | --- | --- |
| [neural/](../../../../source/standard-modules/neural) | `neural.slang` | `SLANG_NEURAL_MODULE_FILE_NAME` (`neural.slang-module`) | `import slang.neural` |
| [experimental/](../../../../source/standard-modules/experimental) | `workgraph.slang` | `SLANG_WORKGRAPH_MODULE_FILE_NAME` (`workgraph.slang-module`) | `import experimental.workgraph` |
| [numerics/](../../../../source/standard-modules/numerics) | `numerics.slang` | none — paths are built directly in its own `CMakeLists.txt` | `import slang.numerics` |
| [functional/](../../../../source/standard-modules/functional) | `functional.slang` | none — the path is built directly in its own `CMakeLists.txt` | `import slang.functional` |

The `one subdirectory, one artifact` shape holds for all but `numerics/`
(`functional/` emits `functional.slang-module` into `slang/`). Runtime
lookup is purely path-based, so nothing limits a directory to one
module: `numerics/` emits four artifacts, two public entry points and
two implementation adapters. `numerics.slang-module` lands in the
`slang/` output directory and is what `import slang.numerics` resolves
to; the other three — `__builtin.slang-module`,
`__builtin_differentiable.slang-module` and `differentiable.slang-module`
— land one level deeper in `slang/numerics/`. That nesting does not
imply privacy: `differentiable.slang-module` is a client-facing entry
point, imported as `slang.numerics.differentiable`.

- The **neural** module declares `[ExperimentalModule] module neural;`
  in
  [neural.slang](../../../../source/standard-modules/neural/neural.slang)
  and pulls in the rest of the directory with `__include` directives
  (vector / storage abstractions, matrix-multiply backends, layers,
  activations, encoders). The
  `unit-test/` subdirectory holds `__include`-able test sources that
  are compiled into the module only when
  `SLANG_STANDARD_MODULE_DEVELOP_BUILD` is set, which adds `-DUNIT_TEST`
  to the compile command; they are resolved through `-I` rather than
  copied to the output directory.
- The **experimental** module declares
  `[ExperimentalModule] module workgraph;` in
  [workgraph.slang](../../../../source/standard-modules/experimental/workgraph.slang)
  and holds the work-graph vocabulary: the node attributes
  declared with `attribute_syntax`, the input / output record types,
  and the barrier flag enums with the `Barrier` overloads that consume
  them.
- The **numerics** module declares
  `[ExperimentalModule] module numerics;` in
  [numerics.slang](../../../../source/standard-modules/numerics/numerics.slang)
  and supplies representation-independent numeric interfaces plus the
  generic operations written against them: the interface hierarchy in
  `interfaces.slang` (numeric, additive, integer and comparison
  interfaces and their componentwise forms), conformances that bind those interfaces to concrete
  types (`builtin-conformances.slang`,
  `floating-point-conformances.slang`,
  `cooperative-vector-conformances.slang`), and `wrappers.slang`.
  A parallel `differentiable-*` set builds the annex module
  `differentiable`, which `__exported import`s `slang.numerics` and
  refines it with differentiable requirements and explicit derivative
  rules. The two `__builtin*.slang` adapters are implementation
  modules that the public modules import; because the adapters do not
  import the public numerics modules, their lookup environment cannot contain the public
  interface-generic overloads, so their calls resolve to the
  established core-module operations instead of recursively selecting a
  witness-backed overload.

- The **functional** module declares `[ExperimentalModule] module functional;`
  in
  [functional.slang](../../../../source/standard-modules/functional/functional.slang)
  and holds higher-order element operations as public extensions on
  `Array<T, N>`, `vector<T, N>` and `matrix<T, R, C, L>`: `map`, a
  static `generate`, and a `__init` that builds a value from an
  element-index function.

The `[ExperimentalModule]` attribute all four modules carry — declared
as an `attribute_syntax` in
[core.meta.slang](../../../../source/slang/core.meta.slang) — gates the
import: importing one without
enabling experimental features fails with **E00104**, *"'<module>' is an
experimental module, need to enable '-experimental-feature' to load this
module"*, which names the resolved module path and the option that
unblocks it. The check is in
[slang-session.cpp](../../../../source/slang/slang-session.cpp) around
line 1651 — the checked `ModuleDecl` carries an
`ExperimentalModuleAttribute` and `CompilerOptionName::ExperimentalFeature`
is unset — and the diagnostic is `need-to-enable-experiment-feature`
(`104`) in
[slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua)
line 603.

All of the directories share the configuration defined in
[standard-modules/CMakeLists.txt](../../../../source/standard-modules/CMakeLists.txt):

- `SLANG_STANDARD_MODULE_DIR_NAME` — the versioned output directory
  name (`slang-standard-module-${SLANG_VERSION_NUMERIC}`).
- `SLANG_STANDARD_MODULE_INSTALL_DIR` — `bin/` on Windows, `lib/`
  elsewhere, joined with that name.
- The per-module file-name variables in the table.

Those values, together with `SLANG_DISABLE_NOTE_IDS` (note IDs
disabled by default, applied as the `DisableNotes` option in
[slang-session.cpp](../../../../source/slang/slang-session.cpp) line 73),
are substituted into
[slang-standard-module-config.h.in](../../../../source/standard-modules/slang-standard-module-config.h.in)
by `configure_file`, producing the internal header
`slang-standard-module-config.h` behind the
`generate_standard_module_config_header` target. At runtime
[slang-session.cpp](../../../../source/slang/slang-session.cpp)
consumes `SLANG_STANDARD_MODULE_DIR_NAME` to locate the directory next
to the loaded `libslang`, and `findStandardModulePath` appends the
imported module's qualified name plus `.slang-module`, so
`slang.neural` resolves to `slang/neural.slang-module` and
`experimental.workgraph` to `experimental/workgraph.slang-module`.

Each standard module is compiled at build time by a Slang compiler
already available to the build (see
[neural/CMakeLists.txt](../../../../source/standard-modules/neural/CMakeLists.txt)
and
[experimental/CMakeLists.txt](../../../../source/standard-modules/experimental/CMakeLists.txt)):
the `add_custom_command` invokes the compiler with `-load-core-module`
pointing at `${core_module_archive_without_timestamp}`, the
`slang-core-module-without-timestamp.bin` archive produced by the
core-module build (see
[Building the core module](#building-the-core-module)), and depends on
the `generate_core_module` target. Loading the prebuilt core archive
means the standard-module step does not recompile the core module.

The directories choose that compiler slightly differently. When
`SLANG_GENERATORS_PATH` is set, all four use the `slang-bootstrap`
binary from that path (the cross-compilation case). Otherwise
`neural/`, `numerics/` and `functional/` always use the
`slang-bootstrap` target; `experimental/` uses `$<TARGET_FILE:slangc>` when
`SLANG_EMBED_CORE_MODULE` and `SLANG_ENABLE_SLANGC` are both on, else
`slang-bootstrap`.

`numerics/` also orders its artifacts against each other: the public
modules import the adapters, so each adapter's serialized
`.slang-module` is a build dependency of the public artifact that
imports it. Its `CMakeLists.txt` also stages the base and adapter
sources under its binary directory, passed with `-I`, because a
bootstrap compiler given the core archive cannot load a second freshly
serialized module that records a dependency on that core module.

New modules go under `source/standard-modules/<name>/` with an
`add_subdirectory` in the parent `CMakeLists.txt`.

## Preludes

Preludes are headers whose text is placed at the head of textual target
output so that the downstream toolchain can compile what Slang emits. They
are **output-side** rather than input-side: they do not participate in
front-end checking. Each `*-prelude.h` is turned into a C++ string by
`slang-embed` at build time; the global session registers the CUDA,
C++ and HLSL strings as the default language preludes, and emit writes
the selected string directly into the generated source (see
[../pipeline/06-emit.md](../pipeline/06-emit.md)). The headers are
also installed, and a caller may override a language prelude with text
that merely `#include`s one of them. `slangc` takes that route unless
`-embed-prelude` appears on its command line (the scan is
`shouldEmbedPrelude` in
[main.cpp](../../../../source/slangc/main.cpp)): otherwise it calls
`TestToolUtil::setSessionDefaultPreludeFromExePath`, defined in
[slang-test-tool-util.cpp](../../../../source/core/slang-test-tool-util.cpp),
which sets the C++ and CUDA language preludes to an `#include` of the
header found relative to the executable, so a `-target cpp` emit begins
with `#include "<install-dir>/slang-cpp-prelude.h"` rather than the
header's text. `-target cuda` is the same shape with
[slang-cuda-prelude.h](../../../../prelude/slang-cuda-prelude.h).

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
same way; the backends emit their vocabularies directly.

## Building the core module

From [CLAUDE.md](../../../../CLAUDE.md) and
[source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt):

- `cmake -DSLANG_EMBED_CORE_MODULE=ON` (the default) bakes the
  precompiled core module into `libslang`, so errors in
  `*.meta.slang` show up at C++ build time.
- `cmake -DSLANG_EMBED_CORE_MODULE=OFF` keeps the C++ build of
  `slangc` and `slang-test` independent of the core-module
  compilation; `*.meta.slang` errors surface from `generate_core_module`,
  which a normal build still runs (the cache and standard-module `ALL`
  targets depend on it).

The `SLANG_EMBED_CORE_MODULE_SOURCE` option controls whether the
original source text is embedded alongside the precompiled bytes (used
by `slang-bootstrap`).

A single `slang-bootstrap` invocation in
[source/slang-core-module/CMakeLists.txt](../../../../source/slang-core-module/CMakeLists.txt)
produces three build products with one `-compile-core-module` run:

- `slang-core-module-without-timestamp.bin` — a standalone RIFF/LZ4
  archive of the compiled core module (written via `-save-core-module`,
  named by the CMake variable
  `core_module_archive_without_timestamp`). This archive is fed to the
  standard-module build through `-load-core-module` so that the modules
  above are compiled against the same core module without recompiling
  it. The variable is exported to the parent scope so the
  `standard-modules/` subdirectories can name it.
- the embeddable core-module header (`-save-core-module-bin-source`),
  consumed by `slang-embedded-core-module`.
- the embeddable GLSL-module header (`-save-glsl-module-bin-source`),
  consumed by `slang-embedded-glsl-module`.

These outputs are wired through the custom targets
`generate_core_module`, `generate_glsl_module_header`, and the umbrella
`generate_core_module_headers`. Downstream targets depend on the
custom *targets* rather than on the generated files: with the Visual
Studio generator a file-level dependency would copy the producer
command into each dependent project and run core generation repeatedly.

### The runtime core-module cache

When `SLANG_EMBED_CORE_MODULE` is off there is no core module inside
the library, so a global session would have to compile the core module
from the source text embedded in the library on first use. To avoid
that, a non-embedded shared build produces a cache file beside the
library.

The cache format is implemented by `BuiltinModuleCache::read` /
`BuiltinModuleCache::write` in
[slang-builtin-module-cache.h](../../../../source/core/slang-builtin-module-cache.h):
the file is `[uint64_t library timestamp][serialized module bytes]`,
with the timestamp written in host byte order because the cache is
tied to the one shared-library build that produced it. A zero
timestamp is the failure sentinel and is rejected by `write`.

The build writes that file eagerly. When `SLANG_EMBED_CORE_MODULE` is
off and `SLANG_LIB_TYPE` is `SHARED`,
[source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt)
adds a `generate_core_module_cache` target that runs the
`slang-core-module-cache` tool
([tools/slang-core-module-cache/](../../../../tools/slang-core-module-cache))
on the linked `slang` library and the
`slang-core-module-without-timestamp.bin` archive, writing
`slang-core-module.bin` beside the library. The tool reads the
library only for its modification timestamp and prefixes the existing
archive; nothing is recompiled. Packaging that rewrites timestamps
makes the runtime reject and regenerate the cache.

These cache files are outside this page's `watched_paths`, so changes
to the mechanism do not mark it stale.

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
4. Rebuild. Touch the changed meta file and rebuild the
   `generate_core_module_headers` target before rebuilding the tools;
   the exact command sequence is in
   [CLAUDE.md](../../../../CLAUDE.md). Meta-source errors surface
   during the C++ build with `SLANG_EMBED_CORE_MODULE=ON`, or from the
   `generate_core_module` step with `OFF`.
5. Add tests under [tests/](../../../../tests).

## What is not in this document

- The full intrinsic list; the `*.meta.slang` files are authoritative.
- The user-visible documentation of the standard modules. Per-module
  documentation lives alongside the source (e.g. in
  [source/standard-modules/neural/](../../../../source/standard-modules/neural))
  and in the [user guide](../../../user-guide).
