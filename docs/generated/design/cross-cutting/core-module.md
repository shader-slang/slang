---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:19:25Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 758f3793c6cde62bb10f3ecad0e65bcabd0d3115b5629165afe8408e4fab2f78
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
   (currently `import slang.neural` and
   `import experimental.workgraph`).

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

| Target            | Emitted form                                         |
| ----------------- | ---------------------------------------------------- |
| HLSL              | `mul(m, v)`                                          |
| GLSL, Metal, WGSL | `(v * m)` — the `*` operator, not a call             |
| SPIR-V            | `OpVectorTimesMatrix`                                |
| CUDA, C++         | a generated `mul_<n>` helper from the `default` body |

`dot` and `length` keep their names on HLSL, GLSL, Metal and WGSL, and
become `OpDot` and the `GLSL.std.450` `Length` extended instruction on
SPIR-V; on CUDA and C++ they too fall through to generated `dot_<n>` /
`length_<n>` helpers.

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
[CLAUDE.md](../../../../CLAUDE.md)), the library carries the embedded
meta-source instead and the module is compiled out-of-line. This is
useful during development because errors in `*.meta.slang` files no
longer break the C++ compile of `slangc` or `slang-test`; they surface
from the separate `generate_core_module` step, which a normal build
still runs — a shared-library build hangs the `ALL` target
`generate_core_module_cache` off it, and the standard-module `ALL`
targets depend on it as well. Compiling the module at runtime is the
fallback used when neither an embedded module nor a valid cache is
available (see
[The runtime core-module cache](#the-runtime-core-module-cache)).

The selection between the two embedding strategies is expressed as a
pair of generator expressions in
[source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt)
(picking either `slang-embedded-core-module` /
`slang-embedded-core-module-source` or their `slang-no-...` siblings).
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

Those three have small surfaces worth naming, since they are usable
with no `import`:

- `Optional<T>` — the `hasValue` and `value` properties, an implicit
  conversion from `T`, and the `none` literal, which is also what
  default initialization produces.
- `Tuple<each T>` — positional members `_0`, `_1`, ..., which also
  compose into swizzles (`t._2_1_0`); constructed with `makeTuple`.
- `IRangedValue` — `static const This maxValue` and `minValue`,
  supplied per scalar type by the extensions that conform each builtin
  numeric type to the interface.

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
`SlangGlobalSessionDesc::enableGLSL` is set (the `if (desc->enableGLSL)`
branch in
[slang-api.cpp](../../../../source/slang/slang-api.cpp)); a later
`import glsl` then retrieves that already-loaded builtin module via the
`glslModuleName` special-case in
[slang-session.cpp](../../../../source/slang/slang-session.cpp).

## Standard modules

Standard modules are independently compiled `.slang-module` files
shipped in a versioned directory next to the `libslang` artefact and
loaded at runtime by an `import` whose qualified name maps to a
subdirectory of that directory. The build infrastructure is in
[source/standard-modules/](../../../../source/standard-modules) and is
described in detail by
[source/standard-modules/README.md](../../../../source/standard-modules/README.md).

Each subdirectory under
[source/standard-modules/](../../../../source/standard-modules) has its
own `CMakeLists.txt`, is pulled in by an `add_subdirectory` call in
[standard-modules/CMakeLists.txt](../../../../source/standard-modules/CMakeLists.txt),
and produces one `.slang-module` artifact. Two exist today:

| Directory                                                         | Entry point       | Module file name variable                                     | Import path                     |
| ----------------------------------------------------------------- | ----------------- | ------------------------------------------------------------- | ------------------------------- |
| [neural/](../../../../source/standard-modules/neural)             | `neural.slang`    | `SLANG_NEURAL_MODULE_FILE_NAME` (`neural.slang-module`)       | `import slang.neural`           |
| [experimental/](../../../../source/standard-modules/experimental) | `workgraph.slang` | `SLANG_WORKGRAPH_MODULE_FILE_NAME` (`workgraph.slang-module`) | `import experimental.workgraph` |

- The **neural** module declares `[ExperimentalModule] module neural;`
  in
  [neural.slang](../../../../source/standard-modules/neural/neural.slang)
  and pulls in the rest of the directory with `__include` directives —
  vector and storage abstractions (`ivector.slang`,
  `inline-vector.slang`, `istorages.slang`, `bindless-storage.slang`),
  matrix-multiply backends (`accelerate-vector-coopmat.slang`,
  `WaveMatrix.slang`, the `mma-tiled-*.slang` family and its layout
  helpers), layer / activation / encoder interfaces and
  implementations (`ilayer.slang`, `iactivation.slang`,
  `iencoder.slang`, `layers.slang`, `activations.slang`,
  `permuto-encoder.slang`), and support code
  (`hash-function.slang`, `shared-memory-pool.slang`,
  `vectorized-reader.slang`,
  `network-parameter-layout-converter.slang`). The
  `unit-test/` subdirectory holds `__include`-able test sources that
  are compiled into the module only when
  `SLANG_STANDARD_MODULE_DEVELOP_BUILD` is set, which adds `-DUNIT_TEST`
  to the compile command; they are resolved through `-I` rather than
  copied to the output directory.
- The **experimental** module declares
  `[ExperimentalModule] module workgraph;` in
  [workgraph.slang](../../../../source/standard-modules/experimental/workgraph.slang)
  and holds the work-graph vocabulary: the node attributes
  (`[NodeLaunch]`, `[NodeMaxDispatchGrid]`, `[MaxRecords]`, `[NodeID]`,
  ...) declared with `attribute_syntax`, the input / output record
  types (`DispatchNodeInputRecord`, `ThreadNodeInputRecord`,
  `GroupNodeOutputRecords`, `NodeOutput`, `NodeOutputArray`, ...), and
  the `BarrierMemoryTypeFlags` / `BarrierSemanticFlags` enums with the
  `Barrier` overloads that consume them.

The `[ExperimentalModule]` attribute both modules carry — declared as
an `attribute_syntax` in
[core.meta.slang](../../../../source/slang/core.meta.slang) — gates the
import rather than merely labelling the module: importing one without
enabling experimental features is an error naming the resolved module
path and the `-experimental-feature` option.

Both directories share the configuration defined in
[standard-modules/CMakeLists.txt](../../../../source/standard-modules/CMakeLists.txt):

- `SLANG_STANDARD_MODULE_DIR_NAME` — the versioned output directory
  name (`slang-standard-module-${SLANG_VERSION_NUMERIC}`).
- `SLANG_STANDARD_MODULE_INSTALL_DIR` — `bin/` on Windows, `lib/`
  elsewhere, joined with the directory name above.
- The per-module file-name variables in the table.

Those values are substituted into
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

The two directories choose that compiler slightly differently. When
`SLANG_GENERATORS_PATH` is set, both use the `slang-bootstrap` binary
from that path instead of one built here, which is how
cross-compilation avoids running a target-platform compiler on the
build host. Otherwise `neural/` always uses the `slang-bootstrap`
target, so it never requires `slangc`; `experimental/` uses
`$<TARGET_FILE:slangc>` when `SLANG_EMBED_CORE_MODULE` and
`SLANG_ENABLE_SLANGC` are both on, and falls back to `slang-bootstrap`
in every other case.

The standard-module mechanism is intended to grow: new modules go
under `source/standard-modules/<name>/` with an `add_subdirectory` in
the parent `CMakeLists.txt`.

## Preludes

Preludes are headers whose text is prepended to textual target output
so that the downstream toolchain can compile what Slang emits. They
are **output-side** rather than input-side: they do not participate in
front-end checking. Each `*-prelude.h` is turned into a C++ string by
`slang-embed` at build time; the global session registers the CUDA,
C++ and HLSL strings as the default language preludes, and emit writes
the selected string directly into the generated source (see
[../pipeline/06-emit.md](../pipeline/06-emit.md)). The headers are
also installed, and a caller may override a language prelude with text
that merely `#include`s one of them. The command-line tools take that
route, so the head of a `-target cpp` emit is the include rather than
the header's text:

```cpp
#include "<install-dir>/slang-cpp-prelude.h"
```

and the generated entry point that follows is marked with a macro the
header defines, `SLANG_PRELUDE_EXPORT`. `-target cuda` is the same
shape with
[slang-cuda-prelude.h](../../../../prelude/slang-cuda-prelude.h).

| Prelude                                                                            | Target                               |
| ---------------------------------------------------------------------------------- | ------------------------------------ |
| [slang-cpp-prelude.h](../../../../prelude/slang-cpp-prelude.h)                     | C++ shader output                    |
| [slang-cpp-types-core.h](../../../../prelude/slang-cpp-types-core.h)               | C++ shared core types                |
| [slang-cpp-types.h](../../../../prelude/slang-cpp-types.h)                         | C++ extended types                   |
| [slang-cpp-scalar-intrinsics.h](../../../../prelude/slang-cpp-scalar-intrinsics.h) | C++ scalar intrinsic implementations |
| [slang-cpp-host-prelude.h](../../../../prelude/slang-cpp-host-prelude.h)           | Host-side C++ runtime                |
| [slang-cuda-prelude.h](../../../../prelude/slang-cuda-prelude.h)                   | CUDA                                 |
| [slang-hlsl-prelude.h](../../../../prelude/slang-hlsl-prelude.h)                   | HLSL                                 |
| [slang-llvm.h](../../../../prelude/slang-llvm.h)                                   | `slang-llvm` integration             |
| [slang-torch-prelude.h](../../../../prelude/slang-torch-prelude.h)                 | PyTorch glue                         |

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
  compilation. Errors in `*.meta.slang` then surface from the separate
  `generate_core_module` step rather than from the C++ compile; a
  normal build still runs that step, because `generate_core_module_cache`
  and the standard-module targets are `ALL` targets that depend on it.

The `SLANG_EMBED_CORE_MODULE_SOURCE` option similarly controls
whether the original Slang source text is embedded alongside the
precompiled bytes (used by `slang-bootstrap` for cross-compilation
scenarios).

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
custom _targets_ rather than on the generated files directly: with the
Visual Studio generator a file-level dependency on a byproduct copies
the producer command into each dependent project, which would run the
core generation more than once.

On Windows, when `SLANG_EMBED_CORE_MODULE` is off and
`SLANG_LIB_TYPE` is `SHARED`, that same custom command first copies
the `slang` shared library next to `slang-bootstrap`, because
`slang-bootstrap` lives in `generators/` while the library is placed
in `bin/` and the Windows loader only searches the executable's own
directory.

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
with three arguments — the linked `slang` library, the
`slang-core-module-without-timestamp.bin` archive from above, and the
output path `slang-core-module.bin` in the library's runtime
(Windows) or library (elsewhere) output directory. The tool reads the
library only to obtain its modification timestamp; it prefixes the
existing archive rather than compiling anything. Note that the two
`.bin` files are therefore different artifacts:
`slang-core-module-without-timestamp.bin` is the build-time archive
passed to `-load-core-module`, while `slang-core-module.bin` is the
timestamp-prefixed runtime cache. Because installation preserves the
library's timestamp, the installed cache stays valid; packaging that
rewrites timestamps causes the runtime to reject and regenerate it.

The three files behind this paragraph — `source/slang/CMakeLists.txt`,
`source/core/slang-builtin-module-cache.{h,cpp}`, and
`tools/slang-core-module-cache/` — are outside this page's
`watched_paths`, which cover only `source/slang-core-module/*`,
`source/slang-glsl-module/*`, `source/standard-modules/**`,
`prelude/*.h`, and the four `*.meta.slang` files. They should be added
to the manifest entry for this page so that changes to the cache
mechanism mark it stale.

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
4. Rebuild. Either way the `*.meta.slang` text must be re-run through
   `slang-generate` and re-embedded in the library, so touch the
   changed meta file and rebuild the `generate_core_module_headers`
   target before rebuilding the tools; the exact command sequence is
   in [CLAUDE.md](../../../../CLAUDE.md). With
   `SLANG_EMBED_CORE_MODULE=ON` the rebuild also reproduces the
   embedded compiled module, so meta-source errors surface during the
   C++ build; with `OFF` they surface from the `generate_core_module`
   step of the same build instead.
5. Add tests under [tests/](../../../../tests).

## What is not in this document

- The full intrinsic list. The authoritative source is the
  `*.meta.slang` files; enumerating them here would replicate a
  generated artefact and drift on every change.
- The user-visible documentation of the standard modules. Per-module
  documentation lives alongside the source (e.g. in
  [source/standard-modules/neural/](../../../../source/standard-modules/neural))
  and in the [user guide](../../../user-guide).
