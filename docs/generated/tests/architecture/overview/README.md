---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T15:00:00+00:00
source_commit: 74db89b9f77cdced9c4d0c47f377b38fffb9180b
watched_paths_digest: b5bae7d719ddfca09ffeb7d96497c5c488f8241c98977291cb22b1aef784eac1
source_doc: docs/generated/design/architecture/overview.md
source_doc_digest: 3895e3132d3762339ddfb3fedad04d48dc40bec19e81e36ba65ac96233bbe55f
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for architecture/overview

## Intent
Tests verify the architectural-overview claims that have an
observable consequence at the `slangc` command line. The source
document
[`docs/generated/design/architecture/overview.md`](../../../docs/generated/design/architecture/overview.md)
is overwhelmingly an introduction to the **public C++ / COM API**
(`ISession`, `IGlobalSession`, `IModule`, `IComponentType`,
`Linkage`, `EndToEndCompileRequest`, ...) and to the source-tree
layout — neither of which is exercised by the agentic test runner,
which drives the command-line `slangc` only. The bundle is
therefore intentionally compact (5 tests) and the
`## Untested claims` section is long: it itemises the
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


## Functional coverage
| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| A `TranslationUnitRequest` is "a collection of source files that share a namespace"; multiple entry points in one Slang translation unit each pick up their own emit when `-entry` selects them. | functional | [#compilation-request-lifecycle](../../../docs/generated/design/architecture/overview.md#compilation-request-lifecycle) | [`translation-unit-multiple-entries.slang`](translation-unit-multiple-entries.slang) |
| An `EntryPointRequest` bundles a function name with a pipeline stage; `-stage vertex` produces vertex-shader emit and the stage-specific outputs are visible. | functional | [#compilation-request-lifecycle](../../../docs/generated/design/architecture/overview.md#compilation-request-lifecycle) | [`stage-selects-pipeline-stage.slang`](stage-selects-pipeline-stage.slang) |
| An `EntryPointRequest` is "a function name plus a pipeline stage"; `-entry foo` on the command line selects the function named `foo` as the entry. | functional | [#compilation-request-lifecycle](../../../docs/generated/design/architecture/overview.md#compilation-request-lifecycle) | [`entry-point-by-name.slang`](entry-point-by-name.slang) |
| One Slang source compiles to every text-emit target the doc lists (HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA, C++). | functional | [#purpose](../../../docs/generated/design/architecture/overview.md#purpose) | [`multi-target-emit.slang`](multi-target-emit.slang) |
| Slang accepts HLSL-compatible source ("Slang (and HLSL-compatible) source code"); a `cbuffer` block is parsed and lowered to the chosen target. | functional | [#purpose](../../../docs/generated/design/architecture/overview.md#purpose) | [`hlsl-compatible-syntax.slang`](hlsl-compatible-syntax.slang) |


## Untested claims
| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| The `.slang-module` serialization format — slangc can emit it (`-emit-ir` / `-o file.slang-module`) but the doc does not state a CLI behavior, only the existence of the format. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `include/slang.h` declares the COM-style interfaces; together with `include/slang-com-helper.h` and `include/slang-com-ptr.h` it is the binary-stable surface. The *existence* of these headers is a build artefact; their *use* is API. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| The full build configuration for the core library lives in `source/slang/CMakeLists.txt`; each peer subdirectory has its own. CMake topology. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| Source-tree layout: `source/core/`, `source/compiler-core/`, `source/slang/`, `source/slangc/`, `prelude/`, `tools/`, `external/`, `extras/`, `source/slang-core-module/`, `source/slang-glsl-module/`, `source/standard-modules/`, `source/slang-llvm/`, `source/slang-glslang/`, `source/slang-dispatcher/`, `source/slang-rt/`, `source/slang-record-replay/`, `source/slang-wasm/`, `tests/`. File-system facts; no slangc-CLI observable. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| File-name prefix conventions inside `source/slang/` (`slang-ast-*`, `slang-parser*`, `slang-preprocessor*`, `slang-check*`, `slang-lower-to-ir*`, `slang-ir.*`, `slang-ir-insts.*`, `slang-ir-*.cpp`, `slang-emit*`, `slang-serialize*`, `slang-capability*`, `slang-diagnostics*`). Pure file-system convention. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `source/slang-glsl-module/` plus `glsl.meta.slang` form the GLSL module. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| One prelude per textual target family under `prelude/`. (Slangc *does* inject these into emitted code, which is observable, but the per-target inventory itself is not.) | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| The `FIDDLE(...)` macro expands to additional members / visitors / serialization tables produced under `build/source/slang/fiddle/`. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `slang-ir-insts-enum.h.fiddle` enumerates IR opcodes from `slang-ir-insts.lua`. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| Diagnostic catalogs are generated from `slang-diagnostics.lua` and the per-area Lua tables under `source/slang/diagnostics/`. Build-system facts; the *content* leaks through into emitted diagnostics, but the generation pipeline itself does not. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `source/slang-llvm/` is LLVM JIT / static glue. The fact that slangc *can* delegate to LLVM is observable via `-target` (e.g. CPU-target compilation); the *existence* of this shim directory is not a slangc CLI claim. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `source/slang-dispatcher/`, `source/slang-rt/`, `source/slang-record-replay/`, `source/slang-wasm/` — all file-inventory facts. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `tests/` is the project's test corpus; this bundle lives under `docs/generated/tests/` instead. File-system fact. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `extras/` exists for developer tools; not shipped to end users. Repo-policy fact. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `external/` holds vendored dependencies (spirv-headers, glslang, lz4, miniz, …) as submodules. Build dependency. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| The doc ends with links to `module-map.md`, `dependency-graph.md`, `pipeline/overview.md`, `cross-cutting/*`, and `syntax-reference/*`. These are navigation, not behavior; each linked doc has its own bundle for any behavioral claims it makes. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `source/slang-glslang/` bridges to Khronos `glslang` for SPIR-V via GLSL. Engaged by `-emit-spirv-via-glsl` (debugging flag), but the doc makes no CLI claim. | (unclassified) | [#glslang](../../../docs/generated/design/architecture/overview.md#glslang) | Reason and explanation to be refined by the next regeneration. |
| `Linkage` bundles search paths, preprocessor macros, target settings, and a source manager. Some bits leak through to the CLI (`-I`, `-D`, `-target`) but the `Linkage` *object* and its field layout are not. | (unclassified) | [#linkage](../../../docs/generated/design/architecture/overview.md#linkage) | Reason and explanation to be refined by the next regeneration. |
| `Module` implements the public `slang::IModule`. Verifiable only by linking against `slang.h` and querying COM interfaces. | (unclassified) | [#module](../../../docs/generated/design/architecture/overview.md#module) | Reason and explanation to be refined by the next regeneration. |
| `source/standard-modules/` ships extra modules (e.g. `neural`) not embedded the same way. | (unclassified) | [#neural](../../../docs/generated/design/architecture/overview.md#neural) | Reason and explanation to be refined by the next regeneration. |
| "No enum re-ordering, no virtual-method changes mid-vtable, no removal" — ABI rules from `CLAUDE.md`. Enforced by code review, not by `slangc` behavior. | (unclassified) | [#slangc](../../../docs/generated/design/architecture/overview.md#slangc) | Reason and explanation to be refined by the next regeneration. |
| The build produces three primary artefacts: `slangc`, `libslang`, `slang-rt`. A build-system fact, not a slangc behavior. | (unclassified) | [#slangc](../../../docs/generated/design/architecture/overview.md#slangc) | Reason and explanation to be refined by the next regeneration. |
| `IComponentType` is the linkable-program abstraction; a `Module`, an entry-point binding, or a composite of these can all be presented as an `IComponentType`. The composite is built up through the API, not the CLI. | needs-unit-test | [#icomponenttype](../../../docs/generated/design/architecture/overview.md#icomponenttype) | No slangc CLI surface reaches this. A C++ unit test in `tools/slang-unit-test/` could exercise the relevant compiler internals directly. |
| `IGlobalSession` is the process-wide singleton; `ISession` (implemented by `Linkage`) is the per-compile-session interface. Observable only through code that links against `slang.h`. | needs-unit-test | [#iglobalsession](../../../docs/generated/design/architecture/overview.md#iglobalsession) | No slangc CLI surface reaches this. A C++ unit test in `tools/slang-unit-test/` could exercise the relevant compiler internals directly. |
| The "session means two different things" naming-confusion observation (`IGlobalSession` vs `ISession`) — pedagogical, not a runtime behavior. | needs-unit-test | [#iglobalsession](../../../docs/generated/design/architecture/overview.md#iglobalsession) | No slangc CLI surface reaches this. A C++ unit test in `tools/slang-unit-test/` could exercise the relevant compiler internals directly. |
| `Session` (the C++ class in `slang-global-session.h`) owns built-in modules, the AST builder, and the global type-checking environment. Internal C++ identity; not surface-visible. | needs-unit-test | [#session](../../../docs/generated/design/architecture/overview.md#session) | No slangc CLI surface reaches this. A C++ unit test in `tools/slang-unit-test/` could exercise the relevant compiler internals directly. |
| `TranslationUnitRequest`, `EntryPointRequest`, `TargetRequest`, `FrontEndCompileRequest`, `BackEndCompileRequest`, and `EndToEndCompileRequest` exist as C++ classes with the relationships the doc names. Their existence as classes is not observable; only the CLI shape of each request is. | needs-unit-test | [#translationunitrequest](../../../docs/generated/design/architecture/overview.md#translationunitrequest) | No slangc CLI surface reaches this. A C++ unit test in `tools/slang-unit-test/` could exercise the relevant compiler internals directly. |
| `source/slang-core-module/` plus `*.meta.slang` files (`core.meta.slang`, `hlsl.meta.slang`, `diff.meta.slang`) form the core module. Embedded into `libslang` at build time. | compile-time-toggle | [#libslang](../../../docs/generated/design/architecture/overview.md#libslang) | Preprocessor define or build-time flag baked into the binary; not observable at runtime. |


## Doc gaps observed
| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#compilation-request-lifecycle](../../../docs/generated/design/architecture/overview.md#compilation-request-lifecycle) | undocumented-behavior | The `## Compilation request lifecycle` section describes the `EntryPointRequest` / `TargetRequest` / `TranslationUnitRequest` objects without naming the **command-line spellings** that drive them (`-entry`, `-stage`, `-target`, `-profile`). The mapping from C++ object to CLI flag is implicit; an agent has to read `slangc -help` or `slang-compile-request.cpp` to anchor a user-surface test. | A one-line cross-link from each bullet to the flag would let this bundle anchor more tests directly. |
| [#an-output-format-combined-with-a-profile-eg-spir-v-at-glsl450](../../../docs/generated/design/architecture/overview.md#an-output-format-combined-with-a-profile-eg-spir-v-at-glsl450) | undocumented-behavior | The doc lists `TargetRequest` as "an output format combined with a profile (e.g. SPIR-V at `glsl_450`)" but does not name an observable consequence of `-profile` distinct from `-target`. A `-profile`-only test would have to pick a profile-gated feature, which is not described in this doc. No test anchored here; profile semantics belong in a future capabilities or targets doc. |  |
| [#dxc](../../../docs/generated/design/architecture/overview.md#dxc) | undocumented-behavior | The doc lists DXIL as a target output but DXIL is a binary format consumed by `dxc`, not text Slang can emit directly without a downstream toolchain. The textual proxy is HLSL; `multi-target-emit.slang` covers the HLSL path. A future doc revision could clarify which targets are reachable as text and which require a downstream compiler invocation. |  |
| [#purpose](../../../docs/generated/design/architecture/overview.md#purpose) | undocumented-behavior | The `## Purpose` paragraph mentions reflection / layout information as a primary output alongside target code, but the doc does not state how it surfaces at the command line. Slangc has reflection emission but the doc does not commit to a CLI observable; no test anchored here. |  |
| [#compilation-request-lifecycle](../../../docs/generated/design/architecture/overview.md#compilation-request-lifecycle) | undocumented-behavior | The `## Compilation request lifecycle` section notes that "HLSL inputs go one-per-unit; Slang inputs all together" — a `TranslationUnitRequest` packing rule. Verifying this requires passing multiple source files on one command line; the doc does not state a user-surface consequence ("declarations in distinct Slang files in the same compile see each other") and a test of that would belong in a pipeline-stage bundle, not the architectural overview. Recorded as a doc-gap for a future cross-link. |  |
