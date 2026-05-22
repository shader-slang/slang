---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:31:34+00:00
source_commit: 330c9a8d807b9f9352e4754f466d1244ae681cff
watched_paths_digest: 1391f257f21ab933b6bb547913982ba82565e5ad8dcdfa4623949fde2837fa52
source_doc: docs/llm-generated/architecture/module-map.md
source_doc_digest: 5c6d48102b0bb57d8eb4ce69c2e86b33e36cddebf9997d84f579cc559a3b2373
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for architecture/module-map

## Intent

Tests verify the slangc-observable consequences of the subsystems
enumerated in
[`docs/llm-generated/architecture/module-map.md`](../../../docs/llm-generated/architecture/module-map.md).
That doc is overwhelmingly a **file-tree lookup table** — it lists
~150 file names under `source/core/`, `source/compiler-core/`,
`source/slang/`, `source/slang-core-module/`,
`source/slang-glsl-module/`, `source/standard-modules/`, `prelude/`,
and seven peer `source/*` subdirectories with a one-line
responsibility note for each. Almost every claim is about *which
directory holds which file*; that is not observable through
`slang-test`, which only runs `slangc` as a command-line program.

The bundle is therefore intentionally small (six tests) and the
`## Untested claims` section is long: it itemises the
file-tree, C++-identity, prefix-convention, IR-pass-enumeration,
Lua-table-generation, and cross-cutting-pointer claims that the doc
makes but that `slangc` cannot expose at its CLI.

The slangc-observable consequences that *are* tested are: (a) a
multi-backend emit dispatcher selects per-target text output; (b)
the core module is embedded into `libslang` and supplies built-in
vector types with no `import`; (c) the GLSL module is embedded and
`import glsl;` resolves with no extra search path; (d) the GLSL
module also backs the GLSL frontend reached via `-allow-glsl`; (e)
the `prelude/` directory ships per-target prelude headers that are
literally `#include`d in the emitted text for CUDA and C++ targets.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| The "Code emission" table lists an emit dispatcher and per-target backends for HLSL, GLSL, SPIR-V, Metal, WGSL, CPP, and CUDA; the same source compiles to each. | functional | [#code-emission](../../../docs/llm-generated/architecture/module-map.md#code-emission) | [`emit-dispatcher-multi-target.slang`](emit-dispatcher-multi-target.slang) |
| The `prelude/` directory ships one prelude header per textual target; CPP emit literally includes `slang-cpp-prelude.h`. | functional | [#prelude-per-target-prelude-headers](../../../docs/llm-generated/architecture/module-map.md#prelude-per-target-prelude-headers) | [`cpp-prelude-include.slang`](cpp-prelude-include.slang) |
| The `prelude/` directory ships one prelude header per textual target; CUDA emit literally includes `slang-cuda-prelude.h`. | functional | [#prelude-per-target-prelude-headers](../../../docs/llm-generated/architecture/module-map.md#prelude-per-target-prelude-headers) | [`cuda-prelude-include.slang`](cuda-prelude-include.slang) |
| The core module is compiled into libslang; built-in vector types like `int4` and `float3` are available without any import. | functional | [#sourceslang-core-module-embedded-core-module](../../../docs/llm-generated/architecture/module-map.md#sourceslang-core-module-embedded-core-module) | [`core-module-builtin-types.slang`](core-module-builtin-types.slang) |
| The GLSL module (glsl.meta.slang) is embedded into libslang; `import glsl;` resolves with no extra search path. | functional | [#sourceslang-glsl-module-embedded-glsl-module](../../../docs/llm-generated/architecture/module-map.md#sourceslang-glsl-module-embedded-glsl-module) | [`glsl-module-import.slang`](glsl-module-import.slang) |
| The GLSL module also backs the GLSL frontend; with `-allow-glsl`, slangc accepts GLSL source syntax and round-trips it to GLSL emit. | functional | [#sourceslang-glsl-module-embedded-glsl-module](../../../docs/llm-generated/architecture/module-map.md#sourceslang-glsl-module-embedded-glsl-module) | [`allow-glsl-frontend.slang`](allow-glsl-frontend.slang) |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#code-emission](../../../docs/llm-generated/architecture/module-map.md#code-emission) | undocumented-behavior | The `## Code emission` table names a "Dispatcher" row (`slang-emit.cpp` "Selects backend per `TargetRequest`") and per-target rows, but does not state the **CLI spelling** of the selector (`-target hlsl\|glsl\|spirv\|spirv-asm\|metal\|wgsl\|cuda\|cpp`). The mapping from logical backend to the `-target` spelling has to be discovered from `slangc -help`; a one-line addition to each row would let future bundles anchor backend-specific tests by anchor. |  |
| [#other-source-subdirectories](../../../docs/llm-generated/architecture/module-map.md#other-source-subdirectories) | undocumented-behavior | The `## Other source/ subdirectories` table lists `slang-llvm`, `slang-glslang`, `slang-dispatcher`, `slang-rt`, `slang-record-replay`, `slang-wasm`, and `slangc`, each with a one-line role. None of these roles are reachable from the agentic runner except `slangc` (trivially: any test runs it). The doc could note which roles are CLI-observable so future bundles can decide whether to skip the row entirely. |  |
| [#prelude](../../../docs/llm-generated/architecture/module-map.md#prelude) | undocumented-behavior | The `## prelude/` table lists `slang-llvm.h` as the "`slang-llvm` integration" prelude, but the LLVM/JIT integration is a downstream-compiler concern, not a textual emit. There is no obvious slangc CLI consequence of that prelude's existence (no `-target llvm-prelude` exists). The doc could mark prelude rows that are inlineable from CLI text-emit vs those that are linked only when the runtime embeds them. |  |
| [#sourcestandard-modules-standard-libraries-row-lists-a](../../../docs/llm-generated/architecture/module-map.md#sourcestandard-modules-standard-libraries-row-lists-a) | undocumented-behavior | The doc's "## source/standard-modules/ — standard libraries" row lists a `neural/` module. Loading it requires `-experimental-feature` and the doc does not commit to a CLI shape; no test is anchored here. If standard modules become a shipped, default-on feature, an `import neural;` test would belong here. |  |
| [#ir-passes](../../../docs/llm-generated/architecture/module-map.md#ir-passes) | undocumented-behavior | The "IR passes" section explicitly defers per-pass enumeration to `pipeline/05-ir-passes.md` but lists six category groupings (cleanup, specialization, differentiation, layout, validation, target-specific lowering) with example file prefixes. None of these have a slangc CLI observable that isn't already covered by general emit tests. Recorded as a no-op gap: the doc is appropriately silent here, but a reader may expect tests anchored on this section to exist and find none. |  |
| [#cross-cutting](../../../docs/llm-generated/architecture/module-map.md#cross-cutting) | undocumented-behavior | The "Cross-cutting" subsection points to dedicated docs for diagnostics, capabilities, profiles, and serialization. Each has its own bundle (or should); the module-map doc's role is pure navigation, so no module-map-anchored tests are written for those subsystems. |  |

## Untested claims

| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| That `source/core/` contains `slang-basic.h`, `slang-array.h`, `slang-string.h`, `slang-list.h`, `slang-dictionary.h`, `slang-hash.h`, `slang-blob.h`, `slang-file-system.h`, `slang-smart-pointer.h`, `slang-allocator.h`, and the rest of the listed utility headers. File-system facts. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| That `source/compiler-core/` contains the lexer pair `slang-lexer.h` / `slang-lexer.cpp`, the source-loc pair, the diagnostic-sink pair, the doc-extractor, the artifact model, the downstream-compiler glue, and the per-vendor bridges (`slang-dxc-compiler.cpp`, `slang-fxc-compiler.cpp`, `slang-glslang-compiler.cpp`, `slang-gcc-compiler-util.cpp`). Pure file inventory. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| That `source/slang/` contains the compile-request files (`slang-compile-request.{h,cpp}`, `slang-end-to-end-request.cpp`), the module pair (`slang-module.{h,cpp}`), the module-library pair, the linkage pair (`slang-session.{h,cpp}`), the linkable pair, and the global-session pair. C++ filenames. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| The semantic-checking file inventory: `slang-check.{h,cpp}`, `slang-check-impl.h`, and the per-concern files (`slang-check-decl.cpp`, `slang-check-expr.cpp`, `slang-check-stmt.cpp`, `slang-check-type.cpp`, `slang-check-overload.cpp`, `slang-check-conformance.cpp`, `slang-check-conversion.cpp`, `slang-check-inheritance.cpp`, `slang-check-modifier.cpp`, `slang-check-constraint.cpp`, `slang-check-resolve-val.cpp`, `slang-check-shader.cpp`, `slang-check-out-of-bound-access.{h,cpp}`). The behaviors these files implement are observable; their *file identity* is not. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| The AST→IR lowering pair (`slang-lower-to-ir.{h,cpp}`) and the IR core pair (`slang-ir.{h,cpp}`, `slang-ir-insts.{h,lua}`). Filename identity. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| The emit-subsystem inventory beyond the per-target backends themselves: `slang-emit-base.{h,cpp}`, `slang-emit-c-like.{h,cpp}`, `slang-emit-source-writer.{h,cpp}`, `slang-emit-precedence.{h,cpp}`, `slang-emit-dependency-file.{h,cpp}`. These are shared helpers internal to the emit step; the dispatcher test exercises the end-to-end shape but not the helper decomposition. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| The cross-cutting file conventions: `slang-diagnostics*`, `source/slang/diagnostics/`, `slang-capability*`, `slang-capabilities.capdef`, `slang-profile.{h,cpp}`, `slang-serialize*`. Each has its own dedicated cross-cutting doc; the module-map row is navigation. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `source/slang-core-module/` contains `slang-embedded-core-module.cpp` and `slang-embedded-core-module-source.cpp`. The fact that the core module *is embedded* is observable (covered by C-02); the filenames behind that embedding are not. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `source/slang-glsl-module/` contains `slang-embedded-glsl-module.cpp`. Same logic as the core module — embedding is observable, filename is not. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| The actual `*.meta.slang` files live in `source/slang/`: `core.meta.slang`, `hlsl.meta.slang`, `diff.meta.slang`, `glsl.meta.slang`. The user-surface consequence of these files (the language they define) is visible everywhere; their file identity is not. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `source/standard-modules/` contains `slang-standard-module-config.h.in` (a CMake template) and the `neural/` standard module. Build-system facts; the neural module is gated on `-experimental-feature` and the doc does not commit to a CLI shape. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `prelude/slang-cpp-types-core.h` and `prelude/slang-cpp-types.h` are sub-headers included by `slang-cpp-prelude.h`. The CLI observable is the top-level `slang-cpp-prelude.h` include (covered by C-06); the inclusion of sub-headers is internal to the emit text. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `prelude/slang-cpp-scalar-intrinsics.h` provides the C++ intrinsic implementations. Reached only through the `slang-cpp-prelude.h` include chain. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `source/slang-llvm/`, `source/slang-glslang/`, `source/slang-dispatcher/`, `source/slang-rt/`, `source/slang-record-replay/`, `source/slang-wasm/`. The fact that slangc *can* delegate to LLVM or glslang is observable through `-target` (LLVM-backed CPU output, SPIR-V via glslang); the *existence* of these bridge directories is a build-system fact and not directly testable. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| That a discrepancy between this doc and the source tree should be resolved by updating `tests-agentic/_meta/manifest.yaml` and regenerating, not by hand-editing. A meta-process rule, not a runtime behavior. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| The "~300 IR-pass files" claim, plus the six category groupings (cleanup, specialization, differentiation, layout, validation, coverage instrumentation, target- specific lowering, shared utilities) with their example file prefixes. The doc explicitly delegates per-pass enumeration to the pipeline doc, and no module-map-anchored CLI observable exists for the count or grouping. | (unclassified) | [#300-ir-pass-files](../../../docs/llm-generated/architecture/module-map.md#300-ir-pass-files) | Reason and explanation to be refined by the next regeneration. |
| The "API surface helpers" rows: `slang-api.cpp`, `slang-artifact-output-util.{h,cpp}`. These are the C++ glue behind the public API and are not directly visible from the CLI. | (unclassified) | [#api-surface-helpers](../../../docs/llm-generated/architecture/module-map.md#api-surface-helpers) | Reason and explanation to be refined by the next regeneration. |
| The preprocessor, parser, and syntax-declaration file pairs in `source/slang/`. The user-visible effects of those subsystems (e.g. `#include` resolution, parse errors) are observable, but the filename / file-pair claim itself is not. Such behavioral claims belong in `pipeline/01-lex-preprocess` and `pipeline/02-parse`. | (unclassified) | [#include](../../../docs/llm-generated/architecture/module-map.md#include) | Reason and explanation to be refined by the next regeneration. |
| `prelude/slang-hlsl-prelude.h` exists, but HLSL emit does not literally `#include` it the way CUDA / CPP do; it is conditionally consumed via `SLANG_HLSL_ENABLE_NVAPI`. The HLSL-prelude file's existence is therefore not observable by a simple FileCheck against HLSL emit. | (unclassified) | [#include](../../../docs/llm-generated/architecture/module-map.md#include) | Reason and explanation to be refined by the next regeneration. |
| `prelude/slang-torch-prelude.h` exists; `-target torch` / `-target torch-binding` emit content from the prelude but do not always `#include` it by name. A FileCheck of the prelude's *contents* (`torch/extension.h`) is more brittle than the CUDA/CPP case and was not adopted. | (unclassified) | [#include](../../../docs/llm-generated/architecture/module-map.md#include) | Reason and explanation to be refined by the next regeneration. |
| The doc's "see X for details" links into `cross-cutting/diagnostics.md`, `cross-cutting/targets.md`, `cross-cutting/serialization.md`, `cross-cutting/core-module.md`, `cross-cutting/ir-instructions.md`, `pipeline/05-ir-passes.md`. Navigation; each linked doc has its own bundle for any behavioral claims it makes. | (unclassified) | [#see-x-for-details](../../../docs/llm-generated/architecture/module-map.md#see-x-for-details) | Reason and explanation to be refined by the next regeneration. |
| `prelude/slang-llvm.h` is for the `slang-llvm` integration; the LLVM/JIT path is engaged by `-target` but the prelude file itself is not in emitted text. | (unclassified) | [#slang-llvm](../../../docs/llm-generated/architecture/module-map.md#slang-llvm) | Reason and explanation to be refined by the next regeneration. |
| `prelude/slang-cpp-host-prelude.h` is the host runtime prelude. It is not inlined into emitted text by `slangc`; it is consumed by `slang-rt` callers. No CLI observable. | (unclassified) | [#slangc](../../../docs/llm-generated/architecture/module-map.md#slangc) | Reason and explanation to be refined by the next regeneration. |
| That the file enumeration corresponds to the recorded `source_commit` in the doc's front matter. A reproducibility contract, not slangc behavior. | (unclassified) | [#sourcecommit](../../../docs/llm-generated/architecture/module-map.md#sourcecommit) | Reason and explanation to be refined by the next regeneration. |
| The AST file inventory: `slang-ast-forward-declarations.h`, `slang-ast-base.{h,cpp}`, `slang-ast-all.h`, `slang-ast-decl.{h,cpp}`, `slang-ast-decl-ref.cpp`, `slang-ast-expr.h`, `slang-ast-stmt.h`, `slang-ast-type.{h,cpp}`, `slang-ast-modifier.{h,cpp}`, `slang-ast-val.{h,cpp}`, `slang-ast-builder.{h,cpp}`, `slang-ast-dispatch.h`, `slang-ast-iterator.h`, `slang-ast-dump.{h,cpp}`, `slang-ast-print.{h,cpp}`, `slang-ast-synthesis.{h,cpp}`, `slang-ast-natural-layout.{h,cpp}`, `slang-ast-support-types.{h,cpp}`, `slang-ast-boilerplate.cpp`. A C++ class hierarchy is not CLI-visible. | needs-unit-test | (unspecified) | No slangc CLI surface reaches this. A C++ unit test in `tools/slang-unit-test/` could exercise the relevant compiler internals directly. |
| That `IRBuilder`, `IRInst`, `IRModule`, `Token`, `TokenKind`, `SourceManager`, `Linkage`, `Session`, `Module`, `IComponentType` exist as C++ classes / types in the cited files. C++ identity is invisible to the CLI; the relevant user-surface behaviors live in their respective subsystem bundles. | needs-unit-test | [#irbuilder](../../../docs/llm-generated/architecture/module-map.md#irbuilder) | No slangc CLI surface reaches this. A C++ unit test in `tools/slang-unit-test/` could exercise the relevant compiler internals directly. |
