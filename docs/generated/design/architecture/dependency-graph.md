---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-03T00:00:00Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: c607137db619e7fb3fa941a06e151cddf9fdb533d8005452d73cf7e068cf805a
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Dependency Graph

This document captures the static link dependencies among the major
subsystems of the Slang source tree, derived from the
`slang_add_target(... LINK_WITH_PRIVATE ...)` and
`LINK_WITH_PUBLIC` clauses in the per-directory
[CMakeLists.txt](../../../../source) files. The granularity is
**subsystem level** (one node per `source/<subsystem>/`), not file
level; for the file-level inventory consult
[module-map.md](module-map.md).

The intended reader wants to predict what is at risk when changing a
specific subsystem.

## Edges (intra-project only)

External dependencies (`miniz`, `lz4_static`, `Threads::Threads`,
`unordered_dense`, `fast_float`, `SPIRV-Headers`, `SPIRV-Tools-opt`,
`SPIRV-Tools-link`, `SPIRV`, `glslang`, `libcmark-gfm`, `${CMAKE_DL_LIBS}`) are omitted
from the diagram
to keep it focused on internal structure. The most significant ones
are summarized in the notes per node below; the per-node notes are
not an exhaustive list of every external link dependency.

```mermaid
flowchart TB
    coreLib[core]
    compilerCore[compiler-core]
    prelude[prelude]
    coreModule[slang-core-module]
    glslModule[slang-glsl-module]
    standardModules["standard-modules<br/>(no observed link edge)"]
    slangLib[slang]
    slangRecordReplay["slang-record-replay<br/>(folded into slang sources)"]
    slangc[slangc]
    slangDispatcher[slang-dispatcher]
    slangRt[slang-rt]
    slangLlvm["slang-llvm<br/>(no observed link edge)"]
    slangGlslang[slang-glslang]
    slangWasm[slang-wasm]

    compilerCore --> coreLib
    coreModule --> coreLib
    coreModule -->|generated targets| slangLib
    glslModule --> coreLib

    slangLib --> coreLib
    slangLib --> prelude
    slangLib --> compilerCore
    slangLib --> coreModule
    slangLib -.->|source include| slangRecordReplay

    slangc --> coreLib
    slangc --> slangLib
    slangc -.->|REQUIRES, build order| glslModule

    slangDispatcher --> coreLib

    slangWasm --> slangLib
    slangWasm --> coreLib
    slangWasm --> compilerCore
```

Inside `source/slang/` the build also defines four generated-code
targets that the diagram deliberately does not show as separate
subsystems: `slang-fiddle-output` (FIDDLE-generated AST/IR support),
`slang-capability-defs` and `slang-capability-lookup` (the generated
capability tables), and `slang-lookup-tables` (generated SPIR-V and
similar lookups). They are declared in
[source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt)
and are consumed by `slang` and `slang-wasm`. They also explain the
`slang-core-module → slang` edge above: `source/slang-core-module/`
links `slang-capability-defs` and `slang-fiddle-output`, so it depends
on generated artefacts owned by `source/slang/` without linking the
compiler library itself.

Three subsystems present in [module-map.md](module-map.md) appear
above without ordinary `LINK_WITH_*` edges:

- **`source/standard-modules/`** — its
  [CMakeLists.txt](../../../../source/standard-modules/CMakeLists.txt)
  only `configure_file`s a config header and `add_subdirectory`s the
  `neural`, `experimental`, `numerics` and `functional` modules; it
  does not declare a link target of its own. The module products are shipped as standalone
  `.slang-module` files.
- **`source/slang-record-replay/`** — has no `CMakeLists.txt` of
  its own; the sources are pulled directly into `slang` (and into
  `slang-common-objects`) through `EXTRA_SOURCE_DIRS
  ${SLANG_RECORD_REPLAY_SYSTEM}`, set up in
  [source/slang/CMakeLists.txt lines
  184-200](../../../../source/slang/CMakeLists.txt) (dashed edge
  above). The inclusion is conditional on the `SLANG_ENABLE_RECORD_REPLAY`
  option (default `ON`, declared in the root
  [CMakeLists.txt](../../../../CMakeLists.txt) lines 235-239): when it
  is `OFF` the sources are omitted. `slang` also receives a
  `SLANG_ENABLE_RECORD_REPLAY=<0|1>` private compile definition
  (line 292).
- **`source/slang-llvm/`** — has no `CMakeLists.txt` of its own.
  `slang-llvm` is produced out-of-tree (or downloaded as a prebuilt
  binary controlled by `SLANG_SLANG_LLVM_FLAVOR` in the root
  [CMakeLists.txt](../../../../CMakeLists.txt), declared at line 471 and
  handled from line 488); no
  in-source target links against it directly.

## Edge citations

Every solid edge in the diagram is justified by a `LINK_WITH_PUBLIC`
or `LINK_WITH_PRIVATE` clause in the cited file.

| Edge | Cited CMakeLists.txt | Clause |
| --- | --- | --- |
| `compiler-core → core` | [source/compiler-core/CMakeLists.txt](../../../../source/compiler-core/CMakeLists.txt) | `LINK_WITH_PRIVATE core` |
| `core-module → core`, `core-module → slang` (generated targets) | [source/slang-core-module/CMakeLists.txt](../../../../source/slang-core-module/CMakeLists.txt) | `LINK_WITH_PRIVATE core slang-capability-defs slang-fiddle-output` |
| `glsl-module → core` | [source/slang-glsl-module/CMakeLists.txt](../../../../source/slang-glsl-module/CMakeLists.txt) | `LINK_WITH_PRIVATE core` |
| `slang → {core, prelude, compiler-core, core-module}`, plus the generated targets in `source/slang/` | [source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt) | The `slang_add_target(slang ... LINK_WITH_*)` clause near the bottom of the file; `prelude` is an object library of generated embeddings of the prelude headers ([prelude/CMakeLists.txt](../../../../prelude/CMakeLists.txt) lines 9-29), linked privately into the compiler |
| `slangc → core`, `slangc → slang` | [source/slangc/CMakeLists.txt](../../../../source/slangc/CMakeLists.txt) | `LINK_WITH_PRIVATE core slang Threads::Threads` plus the optional `REQUIRES` entries below |
| `slang-dispatcher → core` | [source/slang-dispatcher/CMakeLists.txt](../../../../source/slang-dispatcher/CMakeLists.txt) | `LINK_WITH_PRIVATE core` |
| `slang-wasm → {slang, core, compiler-core}`, plus the generated targets in `source/slang/` | [source/slang-wasm/CMakeLists.txt](../../../../source/slang-wasm/CMakeLists.txt) | `LINK_WITH_PRIVATE miniz lz4_static slang core compiler-core slang-capability-defs slang-capability-lookup slang-fiddle-output slang-lookup-tables` on the wasm target |

The dashed edge `slang -.-> slang-record-replay` is justified by the
source-list inclusion at
[source/slang/CMakeLists.txt lines
184-200](../../../../source/slang/CMakeLists.txt), not by a
`LINK_WITH_*` clause.

The dashed edge `slangc -.-> slang-glsl-module` is a `REQUIRES
slang-glsl-module` build-order dependency, present only when
`TARGET slang-glsl-module` exists; `slangc` likewise adds `REQUIRES
generate_core_module_cache` when that target exists
([source/slangc/CMakeLists.txt](../../../../source/slangc/CMakeLists.txt)
lines 1-12 and 14-24). Neither is a link edge.

External dependencies (visible in
[CMakeLists.txt](../../../../source/core/CMakeLists.txt) and friends but
not shown in the diagram):

- `coreLib` (`core`): `miniz`, `lz4_static`, `Threads::Threads`,
  `unordered_dense`, `${CMAKE_DL_LIBS}`, and — only when
  `SLANG_ENABLE_MIMALLOC` is set — `mimalloc-static`, linked `PUBLIC`
  together with a `PUBLIC SLANG_ENABLE_MIMALLOC=1` compile definition
  so dependents see the same allocator choice
  ([source/core/CMakeLists.txt](../../../../source/core/CMakeLists.txt)).
  The configure step hard-fails if the `mimalloc-static` target is not
  available.
- `compilerCore` (`compiler-core`): `fast_float` (fast floating-point
  parsing), in addition to its internal `core` link; see the
  `LINK_WITH_PRIVATE core fast_float` clause in
  [source/compiler-core/CMakeLists.txt](../../../../source/compiler-core/CMakeLists.txt).
- `slangLib` (`slang`): `SPIRV-Headers::SPIRV-Headers` and
  `libcmark-gfm`; `slang-wasm`: `miniz` / `lz4_static`.
- `slang-rt`: `miniz`, `lz4_static`, `Threads`, `unordered_dense`,
  `${CMAKE_DL_LIBS}` — note the absence of any internal Slang library
  dependency. `slang-rt` is shipped alongside the compiler and links
  none of the compiler's libraries, but it is not wholly independent of
  the compiler's source: its
  [CMakeLists.txt](../../../../source/slang-rt/CMakeLists.txt) passes
  `EXTRA_SOURCE_DIRS ${slang_SOURCE_DIR}/source/core`, which recompiles
  the `source/core/` sources into the runtime with
  `SLANG_RT_DYNAMIC_EXPORT` defined, and
  `INCLUDE_DIRECTORIES_PRIVATE ${slang_SOURCE_DIR}/source` so those
  sources resolve the project's direct-path includes such as
  `#include "core/slang-basic.h"`.
- `slang-glslang`: `glslang`, `SPIRV`, `SPIRV-Tools-opt`,
  `SPIRV-Tools-link`, and, only when `SLANG_GLSLANG_USES_MIMALLOC` is
  set (computed in the root
  [CMakeLists.txt](../../../../CMakeLists.txt) at lines 726-738, ON only on
  Windows when `SPIRV-Tools-opt` links `mimalloc-static`), `mimalloc-static`
  with a `SLANG_GLSLANG_USES_MIMALLOC=1` definition. It also adds
  `${slang_SOURCE_DIR}/source` to its private include directories,
  includes `unordered_dense::unordered_dense` (`INCLUDE_FROM_PRIVATE`),
  and uses `SKIP_SANITIZERS` rather than `SKIP_ASAN`. With
  `SLANG_USE_SYSTEM_GLSLANG` it locates `SPIRV/GlslangToSpv.h` with
  `find_path` (cite
  [source/slang-glslang/CMakeLists.txt](../../../../source/slang-glslang/CMakeLists.txt)).
- `slang-lookup-tables`: `SPIRV-Headers`.

## Notable invariants

The layering above implies several invariants. Each is justified by a
specific build file.

- **`source/core/` does not depend on any other internal subsystem.**
  Its `slang_add_target(... LINK_WITH_PRIVATE miniz lz4_static
  Threads::Threads ...)` block in
  [source/core/CMakeLists.txt](../../../../source/core/CMakeLists.txt)
  lists only external libraries.
- **`source/compiler-core/` may depend on `source/core/` but not on
  `source/slang/`.** The corresponding block in
  [source/compiler-core/CMakeLists.txt](../../../../source/compiler-core/CMakeLists.txt)
  contains `LINK_WITH_PRIVATE core` only.
- **`source/slang/` is the only subsystem that pulls in the
  AST/IR/emit/check sources.** The concrete source-owning target is
  `slang` itself in a non-embedded build, or `slang-common-objects` when
  `SLANG_EMBED_CORE_MODULE` is on and both library targets are declared
  `NO_SOURCE`
  ([source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt)).
  Every other binary that
  needs compilation services (such as `slangc`,
  [source/slangc/CMakeLists.txt](../../../../source/slangc/CMakeLists.txt))
  links against `slang` rather than reaching into individual files.
- **The capability subsystem is split into two libraries.**
  `slang-capability-defs` is the generated header library and
  `slang-capability-lookup` is the generated source library; the main
  `slang` target consumes both
  ([source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt)).
- **The core module is linked optionally.** The choice between
  `slang-embedded-core-module` and `slang-no-embedded-core-module` is
  controlled by the CMake option `SLANG_EMBED_CORE_MODULE` and
  expressed as a generator expression in
  [source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt).
  When the option is off *and* `SLANG_LIB_TYPE` is `SHARED`, the same
  file adds a `generate_core_module_cache` target that runs the
  `slang-core-module-cache` tool over the freshly linked library and the
  archive produced by `generate_core_module`
  (`core_module_archive_without_timestamp` in
  [source/slang-core-module/CMakeLists.txt](../../../../source/slang-core-module/CMakeLists.txt)),
  writing a `slang-core-module.bin` next to the library. This is a
  build-order dependency on a target under
  [tools/](../../../../tools), not a link edge, and it makes the
  library's on-disk timestamp part of the cache's validity.
- **`slang-rt` does not depend on the compiler.** The runtime is
  shipped alongside emitted CPU-target output, and its
  `LINK_WITH_PRIVATE` list contains no compiler internals.
- **`slang-glslang`'s exported surface is bounded by a file, not by
  the compiler's visibility settings.** `CXX_VISIBILITY_PRESET hidden`
  and `-Wl,--exclude-libs,ALL` hide the shim's own non-exported symbols
  and the statically linked dependencies' symbols respectively, but
  neither bounds the final export list irrespective of where a symbol
  came from. `slang-glslang.version-script` does, and is the single
  source of truth for the exported names
  ([source/slang-glslang/CMakeLists.txt](../../../../source/slang-glslang/CMakeLists.txt)).
  ELF consumes that file directly. Mach-O has no version-script
  concept, so on Apple the same CMake file parses the script's
  `global:` block at configure time and derives an
  `-exported_symbols_list` — a bare list of names each carrying the
  leading underscore ld64 prefixes to C symbols, so `glslang_compile`
  becomes `_glslang_compile`. Deriving rather than hand-maintaining a
  second list is what keeps the two formats from drifting, and the
  parse is written to fail loudly instead of silently under-exporting:
  it `message(FATAL_ERROR)`s both when it extracts no names at all and
  when anything other than whitespace survives removing the `name;`
  entries, since a residue means some entry was not in the expected
  form and would have been quietly dropped. The practical consequence
  for a contributor is the one recorded in the shim's own header
  comment — a new exported entry point must be added to the version
  script as well as to the C++, or it will not be exported on either
  ELF or macOS.
- **Public headers in [include/](../../../../include) must not include
  private headers from [source/](../../../../source).** The build
  exposes `${slang_SOURCE_DIR}/include` as the public include directory
  ([source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt)
  line 315), which is the observed public include boundary; the build
  does not enforce the no-private-includes rule itself.

## Cycles and known irregularities

No link-level cycles are observed in the per-directory CMake files.

Two irregularities are worth knowing about. The first is that the
`slang` library reaches *upward* into the tools tree for headers:
[source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt)
adds `${slang_SOURCE_DIR}/tools` to `INCLUDE_DIRECTORIES_PRIVATE`, which
is what lets `slang-language-server.cpp` compile
`#include "platform/performance-counter.h"` from
[tools/platform/](../../../../tools/platform). No link edge accompanies
it — the header is used for its inline definitions — but it does mean
`tools/platform/` is not freely movable without touching the library.

The second is the `slang-common-objects`
indirection in
[source/slang/CMakeLists.txt](../../../../source/slang/CMakeLists.txt):
when configured in some modes the same source files are compiled into
an object library and then re-linked into both
`slang-without-embedded-core-module` and the main `slang` library,
which is a build-system convenience for shipping a "compiler with no
embedded core module" generator alongside the user-facing `slang`.

## Where to go next

- For the file-level breakdown of each subsystem, see
  [module-map.md](module-map.md).
- For runtime data flow rather than build dependencies, follow the
  pipeline starting at [../pipeline/overview.md](../pipeline/overview.md).
