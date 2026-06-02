---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T16:30:00+00:00
source_commit: 74db89b9f77cdced9c4d0c47f377b38fffb9180b
watched_paths_digest: 91822d0ba2727d055b7f1218dcd8a2ac05ac7c9441d7fe565ec6c3cc599ea206
source_doc: docs/generated/design/pipeline/overview.md
source_doc_digest: 2e3bb175b1486e8d48ac5bf18b268c81bf3df2a8052f3710c5bcd10f29b748e8
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for pipeline/overview

## Intent

Tests verify the end-to-end pipeline behaviors described in
[`docs/generated/design/pipeline/overview.md`](../../../design/pipeline/overview.md):
that a single source successfully traverses the full
lex/preprocess/parse/check/lower/IR-pass/emit chain to produce
target text on every text backend supported in the no-GPU runner,
that the IR pass list branches on target (per-target shape of
emitted text), that AST decls and statements lower into IR and
re-emerge in the target output, and that the entry-point attribute
flows through the pipeline into the right target-specific marker.

This is a **meta-bundle**: `pipeline/overview.md` is the index to
per-stage docs. Most of its content is pointers to
`01-lex-preprocess.md` … `06-emit.md`, so single-stage claims
(preprocessor diagnostics, parser diagnostics, semantic-check
diagnostics, lowering bugs, individual IR passes, per-target emit
quirks) are intentionally deferred to those bundles. The tests here
exercise only the end-to-end claims that the overview makes itself
and that no per-stage bundle could naturally own.

Coverage strategy: one multi-backend "all-targets" test for the
emit-dispatch claim, one focused multi-target test for the
"target-sensitive IR pass list" claim, two single-target full-flow
tests (SPIR-V text and HLSL text) for the end-to-end-flow diagram,
two multi-target tests for the AST-to-IR-and-emit round trip (one
struct, one if/else control flow), and one multi-target test for the
entry-point-flows-to-target-marker claim. Multi-backend coverage is
the point of this bundle.

## Functional coverage

| Claim                                                                                                                                     | Intent     | Anchor                                                                   | Tests                                                                                  |
| ----------------------------------------------------------------------------------------------------------------------------------------- | ---------- | ------------------------------------------------------------------------ | -------------------------------------------------------------------------------------- |
| Decls in the AST (here a struct) lower into IR types and re-emerge in the emitted target text after IR passes.                            | functional | [#ast-ir-lowering](../../../design/pipeline/overview.md#ast-ir-lowering) | [`ast-to-ir-lowers-struct.slang`](ast-to-ir-lowers-struct.slang)                       |
| Statements in the AST (here an if-statement) become basic blocks in IR and re-emerge as target-appropriate branching in the emitted text. | functional | [#ast-ir-lowering](../../../design/pipeline/overview.md#ast-ir-lowering) | [`ast-to-ir-lowers-control-flow.slang`](ast-to-ir-lowers-control-flow.slang)           |
| The [shader("compute")] entry-point attribute survives every pipeline stage and emerges as the target's compute-kernel marker.            | functional | [#emit](../../../design/pipeline/overview.md#emit)                       | [`entry-point-flows-to-target-marker.slang`](entry-point-flows-to-target-marker.slang) |
| The emit dispatcher selects a backend per TargetRequest and produces target text for HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA, and C++.      | functional | [#emit](../../../design/pipeline/overview.md#emit)                       | [`multi-target-emit-dispatch.slang`](multi-target-emit-dispatch.slang)                 |
| A non-trivial source successfully traverses lex, preprocess, parse, check, lower, IR passes, and emit to produce SPIR-V text.             | functional | [#end-to-end-flow](../../../design/pipeline/overview.md#end-to-end-flow) | [`end-to-end-flow-spirv.slang`](end-to-end-flow-spirv.slang)                           |
| A non-trivial source successfully traverses the full pipeline to produce HLSL text with the entry-point body intact.                      | functional | [#end-to-end-flow](../../../design/pipeline/overview.md#end-to-end-flow) | [`end-to-end-flow-hlsl.slang`](end-to-end-flow-hlsl.slang)                             |
| The IR pass list is target-sensitive; the same source emits target-shaped output (not just textually identical) for each backend.         | functional | [#ir-passes](../../../design/pipeline/overview.md#ir-passes)             | [`pipeline-is-target-sensitive.slang`](pipeline-is-target-sensitive.slang)             |

## Untested claims

| Claim                                                                                                                                                                                                                                                      | Reason         | Anchor                                                         | Why untested                                                                                                           |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------- | -------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------- |
| Targets that require a host compiler or runtime not available in the agentic runner: Torch glue (`-target torch`), LLVM IR / native via `slang-llvm`, DXIL, MSL binary, SPIRV binary (requires `spirv-val` ecosystem). Text-emit forms are tested instead. | out-of-bundle  | [#slang-llvm](../../../design/pipeline/overview.md#slang-llvm) | The pipeline-overview doc is a navigation hub; per-target downstream paths belong to the `target-pipelines/*` bundles. |
| Determinism of the compiler (compile twice → identical text). `slang-test` would need a custom diff harness for this, and the doc does not currently make a determinism claim; recorded as doc-gap-adjacent and skipped here.                              | (unclassified) | [#slang-test](../../../design/pipeline/overview.md#slang-test) | Reason and explanation to be refined by the next regeneration.                                                         |

## Doc gaps observed

| Anchor                                                                                     | Kind                  | Gap                                                                                                                                                                                                                                                                                                                                                                                                                                                                       | Suggested addition |
| ------------------------------------------------------------------------------------------ | --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------ |
| [#end-to-end-flow](../../../design/pipeline/overview.md#end-to-end-flow)                   | undocumented-behavior | The `## End-to-end flow` section says explicitly that the ordering in the diagram "is conceptual — actual control flow weaves checking and parsing together (see [02-parse-ast.md] for the two-stage parser)". That two-stage parsing claim is testable (e.g. generic-vs-comparison disambiguation depends on type info introduced by the checker mid-parse) but is fundamentally a parser-stage claim. Deferred to `pipeline/02-parse-ast`.                              |                    |
| [#include](../../../design/pipeline/overview.md#include)                                   | undocumented-behavior | The `## Lex / Preprocess` subsection states "Lexing, preprocessing, and `#include` resolution all complete before parsing begins". The observable consequences (token list, conditional compilation, include hand-off) are already exercised in `docs/generated/tests/pipeline/01-lex-preprocess`. Not duplicated here.                                                                                                                                                   |                    |
| [#parse-to-ast](../../../design/pipeline/overview.md#parse-to-ast)                         | undocumented-behavior | The `## Parse to AST` subsection's claim that "Recursive-descent parsing produces a strongly-typed AST" is not directly observable from slangc output beyond "syntactic errors fire from the parser stage with E2xxxx codes" — which is already tested in `docs/generated/tests/cross-cutting/diagnostics/parser-error-has-code.slang`. Deferred to that bundle and the future `pipeline/02-parse-ast`.                                                                   |                    |
| [#semantic-check](../../../design/pipeline/overview.md#semantic-check)                     | undocumented-behavior | The `## Semantic check` subsection lists overload resolution, conformance checking, default-witness synthesis, and generated members as the per-concern responsibilities of `SemanticsVisitor` subclasses. Each is a stage-specific claim. The visible "undefined identifier" surface is already covered in `docs/generated/tests/pipeline/03-semantic-check/undefined-identifier-becomes-error.slang`. Deferred to `pipeline/03-semantic-check` and `name-resolution/*`. |                    |
| [#ir-passes](../../../design/pipeline/overview.md#ir-passes)                               | undocumented-behavior | The `## IR passes` subsection mentions "roughly 160 `slang-ir-*.cpp` files implementing analyses, validations, specializations, legalizations, and target-specific lowerings". Individual pass behaviors belong in `pipeline/05-ir-passes` and the per-target bundles. We test only the cross-cutting claim that the pass list is target-sensitive (one shader, multiple targets, divergent emitted shape).                                                               |                    |
| [#driver-entry-points](../../../design/pipeline/overview.md#driver-entry-points)           | undocumented-behavior | The `## Driver entry points` section names the high-level objects (`FrontEndCompileRequest`, `EndToEndCompileRequest`, `Module`, etc.) and the `checkAllTranslationUnits` loop, but these are internal API surfaces with no behavioral assertion accessible through slangc/slangi text. Deferred to `architecture/overview` and `cross-cutting/serialization`.                                                                                                            |                    |
| [#cross-cutting-concerns](../../../design/pipeline/overview.md#cross-cutting-concerns)     | undocumented-behavior | The `## Cross-cutting concerns` section enumerates five cross-cutting topics. Each has its own bundle: diagnostics → `cross-cutting/diagnostics`, IR instructions → `cross-cutting/ir-instructions`, targets and capabilities → `cross-cutting/targets`, core module / preludes → `cross-cutting/core-module`, serialization → `cross-cutting/serialization`. Not duplicated here.                                                                                        |                    |
| [#line-893-at-sourcecommit](../../../design/pipeline/overview.md#line-893-at-sourcecommit) | undocumented-behavior | The doc lists `slang-emit.cpp` line numbers ("line 893 at source_commit", "line 2418 at source_commit") for `linkAndOptimizeIR` and `emitEntryPointsSourceFromIR`. These are navigation aids, not user-facing claims; no test anchors them. The fact that linkAndOptimizeIR runs is observed indirectly by successful end-to-end compile of a shader that requires linking (function calls, struct usage) — covered by the existing tests.                                |                    |
| [#emit](../../../design/pipeline/overview.md#emit)                                         | undocumented-behavior | The `## Emit` subsection lists Torch glue, LLVM IR / native via `slang-llvm`, and VM bytecode as targets in addition to the text emit backends. Torch and LLVM/native require host compilers we cannot assume in the no-GPU runner; VM bytecode is exercised via `INTERPRET` in lower-level bundles. Recorded as out-of-scope here.                                                                                                                                       |                    |
