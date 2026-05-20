---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T15:00:00+00:00
source_commit: 74db89b9f77cdced9c4d0c47f377b38fffb9180b
watched_paths_digest: b5bae7d719ddfca09ffeb7d96497c5c488f8241c98977291cb22b1aef784eac1
source_doc: docs/llm-generated/architecture/overview.md
source_doc_digest: 3895e3132d3762339ddfb3fedad04d48dc40bec19e81e36ba65ac96233bbe55f
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for architecture/overview

## Intent

Tests verify the architectural-overview claims that have an
observable consequence at the `slangc` command line. The source
document
[`docs/llm-generated/architecture/overview.md`](../../../docs/llm-generated/architecture/overview.md)
is overwhelmingly an introduction to the **public C++ / COM API**
(`ISession`, `IGlobalSession`, `IModule`, `IComponentType`,
`Linkage`, `EndToEndCompileRequest`, ...) and to the source-tree
layout — neither of which is exercised by the agentic test runner,
which drives the command-line `slangc` only. The bundle is
therefore intentionally compact (5 tests) and the
`## Out of scope (no-GPU runner)` section is long: it itemises the
API-shaped, file-tree, and ABI-policy claims that the doc makes but
that `slang-test` cannot probe.

The slangc-observable claims that *are* tested are: (a) Slang is a
multi-target compiler that compiles one source to every text-emit
target named in `## Purpose`; (b) `-entry` selects an
`EntryPointRequest` by function name; (c) `-stage` selects the
pipeline stage that `EntryPointRequest` bundles with the name;
(d) the frontend accepts HLSL-compatible syntax as `## Purpose`
states; (e) a single `TranslationUnitRequest` can hold multiple
entry points and shared file-scope declarations.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                       | Claim (one line)                                                                                                                                                | Tests                                              |
| -------- | -------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------- |
| C-01     | [#purpose](../../../docs/llm-generated/architecture/overview.md#purpose)                                                                     | Slang compiles one source to many target languages — DXIL, SPIR-V, GLSL, Metal, WGSL, C++, CUDA, PyTorch glue. The text-emit subset is observable from slangc. | `multi-target-emit.slang`                          |
| C-02     | [#purpose](../../../docs/llm-generated/architecture/overview.md#purpose)                                                                     | Slang accepts HLSL-compatible source code; HLSL-flavoured constructs (`cbuffer`) are parsed and lowered to the chosen target.                                  | `hlsl-compatible-syntax.slang`                     |
| C-03     | [#compilation-request-lifecycle](../../../docs/llm-generated/architecture/overview.md#compilation-request-lifecycle)                         | An `EntryPointRequest` is "a function name plus a pipeline stage"; `-entry foo` selects the function named `foo` from the translation unit.                    | `entry-point-by-name.slang`                        |
| C-04     | [#compilation-request-lifecycle](../../../docs/llm-generated/architecture/overview.md#compilation-request-lifecycle)                         | An `EntryPointRequest` bundles a pipeline stage with the function name; `-stage vertex` produces vertex-shader emit with stage-specific outputs.               | `stage-selects-pipeline-stage.slang`                |
| C-05     | [#compilation-request-lifecycle](../../../docs/llm-generated/architecture/overview.md#compilation-request-lifecycle)                         | A `TranslationUnitRequest` is "a collection of source files that share a namespace"; multiple entry points in one Slang file each compile from the same unit.   | `translation-unit-multiple-entries.slang`          |

## Tests in this bundle

| File                                         | Intent     | Doc anchor                          |
| -------------------------------------------- | ---------- | ----------------------------------- |
| `multi-target-emit.slang`                    | functional | `#purpose`                          |
| `hlsl-compatible-syntax.slang`               | functional | `#purpose`                          |
| `entry-point-by-name.slang`                  | functional | `#compilation-request-lifecycle`    |
| `stage-selects-pipeline-stage.slang`         | functional | `#compilation-request-lifecycle`    |
| `translation-unit-multiple-entries.slang`    | functional | `#compilation-request-lifecycle`    |

## Doc gaps observed

- The `## Compilation request lifecycle` section describes the
  `EntryPointRequest` / `TargetRequest` / `TranslationUnitRequest`
  objects without naming the **command-line spellings** that drive
  them (`-entry`, `-stage`, `-target`, `-profile`). The mapping
  from C++ object to CLI flag is implicit; an agent has to read
  `slangc -help` or `slang-compile-request.cpp` to anchor a
  user-surface test. A one-line cross-link from each bullet to the
  flag would let this bundle anchor more tests directly.
- The doc lists `TargetRequest` as "an output format combined with
  a profile (e.g. SPIR-V at `glsl_450`)" but does not name an
  observable consequence of `-profile` distinct from `-target`.
  A `-profile`-only test would have to pick a profile-gated
  feature, which is not described in this doc. No test anchored
  here; profile semantics belong in a future capabilities or
  targets doc.
- The doc lists DXIL as a target output but DXIL is a binary
  format consumed by `dxc`, not text Slang can emit directly
  without a downstream toolchain. The textual proxy is HLSL;
  `multi-target-emit.slang` covers the HLSL path. A future doc
  revision could clarify which targets are reachable as text and
  which require a downstream compiler invocation.
- The `## Purpose` paragraph mentions reflection / layout
  information as a primary output alongside target code, but the
  doc does not state how it surfaces at the command line. Slangc
  has reflection emission but the doc does not commit to a CLI
  observable; no test anchored here.
- The `## Compilation request lifecycle` section notes that "HLSL
  inputs go one-per-unit; Slang inputs all together" — a
  `TranslationUnitRequest` packing rule. Verifying this requires
  passing multiple source files on one command line; the doc does
  not state a user-surface consequence ("declarations in distinct
  Slang files in the same compile see each other") and a test of
  that would belong in a pipeline-stage bundle, not the
  architectural overview. Recorded as a doc-gap for a future
  cross-link.

## Out of scope (no-GPU runner)

(In this bundle the section heading is used for "claims
unobservable through any allowed test directive" — most are not
literally GPU-bound, but the runner cannot reach them. The doc is
overwhelmingly about API surface, file-tree layout, and ABI policy
rather than about behaviors observable at `slangc`'s command-line
interface.)

### API / COM-interface claims (not reachable via slangc CLI)

- `IGlobalSession` is the process-wide singleton; `ISession`
  (implemented by `Linkage`) is the per-compile-session interface.
  Observable only through code that links against `slang.h`.
- `Session` (the C++ class in `slang-global-session.h`) owns
  built-in modules, the AST builder, and the global type-checking
  environment. Internal C++ identity; not surface-visible.
- `Linkage` bundles search paths, preprocessor macros, target
  settings, and a source manager. Some bits leak through to the
  CLI (`-I`, `-D`, `-target`) but the `Linkage` *object* and its
  field layout are not.
- `TranslationUnitRequest`, `EntryPointRequest`, `TargetRequest`,
  `FrontEndCompileRequest`, `BackEndCompileRequest`, and
  `EndToEndCompileRequest` exist as C++ classes with the
  relationships the doc names. Their existence as classes is not
  observable; only the CLI shape of each request is.
- `Module` implements the public `slang::IModule`. Verifiable
  only by linking against `slang.h` and querying COM interfaces.
- `IComponentType` is the linkable-program abstraction; a
  `Module`, an entry-point binding, or a composite of these can
  all be presented as an `IComponentType`. The composite is
  built up through the API, not the CLI.
- The "session means two different things" naming-confusion
  observation (`IGlobalSession` vs `ISession`) — pedagogical,
  not a runtime behavior.
- The `.slang-module` serialization format — slangc can emit it
  (`-emit-ir` / `-o file.slang-module`) but the doc does not
  state a CLI behavior, only the existence of the format.

### Public-header / ABI rules (repo policy, not runtime behavior)

- "No enum re-ordering, no virtual-method changes mid-vtable, no
  removal" — ABI rules from `CLAUDE.md`. Enforced by code review,
  not by `slangc` behavior.
- `include/slang.h` declares the COM-style interfaces; together
  with `include/slang-com-helper.h` and `include/slang-com-ptr.h`
  it is the binary-stable surface. The *existence* of these
  headers is a build artefact; their *use* is API.

### Build-system / file-tree claims

- The build produces three primary artefacts: `slangc`, `libslang`,
  `slang-rt`. A build-system fact, not a slangc behavior.
- The full build configuration for the core library lives in
  `source/slang/CMakeLists.txt`; each peer subdirectory has its
  own. CMake topology.
- Source-tree layout: `source/core/`, `source/compiler-core/`,
  `source/slang/`, `source/slangc/`, `prelude/`, `tools/`,
  `external/`, `extras/`, `source/slang-core-module/`,
  `source/slang-glsl-module/`, `source/standard-modules/`,
  `source/slang-llvm/`, `source/slang-glslang/`,
  `source/slang-dispatcher/`, `source/slang-rt/`,
  `source/slang-record-replay/`, `source/slang-wasm/`,
  `tests/`. File-system facts; no slangc-CLI observable.
- File-name prefix conventions inside `source/slang/`
  (`slang-ast-*`, `slang-parser*`, `slang-preprocessor*`,
  `slang-check*`, `slang-lower-to-ir*`, `slang-ir.*`,
  `slang-ir-insts.*`, `slang-ir-*.cpp`, `slang-emit*`,
  `slang-serialize*`, `slang-capability*`,
  `slang-diagnostics*`). Pure file-system convention.

### Standard-library and prelude claims (file-inventory)

- `source/slang-core-module/` plus `*.meta.slang` files
  (`core.meta.slang`, `hlsl.meta.slang`, `diff.meta.slang`) form
  the core module. Embedded into `libslang` at build time.
- `source/slang-glsl-module/` plus `glsl.meta.slang` form the GLSL
  module.
- `source/standard-modules/` ships extra modules (e.g. `neural`)
  not embedded the same way.
- One prelude per textual target family under `prelude/`. (Slangc
  *does* inject these into emitted code, which is observable, but
  the per-target inventory itself is not.)

### Build-time generated code

- The `FIDDLE(...)` macro expands to additional members /
  visitors / serialization tables produced under
  `build/source/slang/fiddle/`.
- `slang-ir-insts-enum.h.fiddle` enumerates IR opcodes from
  `slang-ir-insts.lua`.
- Diagnostic catalogs are generated from `slang-diagnostics.lua`
  and the per-area Lua tables under `source/slang/diagnostics/`.
  Build-system facts; the *content* leaks through into emitted
  diagnostics, but the generation pipeline itself does not.

### Downstream-compiler and runtime tree

- `source/slang-llvm/` is LLVM JIT / static glue. The fact that
  slangc *can* delegate to LLVM is observable via `-target` (e.g.
  CPU-target compilation); the *existence* of this shim
  directory is not a slangc CLI claim.
- `source/slang-glslang/` bridges to Khronos `glslang` for SPIR-V
  via GLSL. Engaged by `-emit-spirv-via-glsl` (debugging flag),
  but the doc makes no CLI claim.
- `source/slang-dispatcher/`, `source/slang-rt/`,
  `source/slang-record-replay/`, `source/slang-wasm/` — all
  file-inventory facts.

### Auxiliary tree facts (outside `source/`)

- `tests/` is the project's test corpus; this bundle lives under
  `tests-agentic/` instead. File-system fact.
- `extras/` exists for developer tools; not shipped to end
  users. Repo-policy fact.
- `external/` holds vendored dependencies (spirv-headers,
  glslang, lz4, miniz, …) as submodules. Build dependency.

### Reading-guide pointers

- The doc ends with links to `module-map.md`,
  `dependency-graph.md`, `pipeline/overview.md`,
  `cross-cutting/*`, and `syntax-reference/*`. These are
  navigation, not behavior; each linked doc has its own bundle
  for any behavioral claims it makes.
