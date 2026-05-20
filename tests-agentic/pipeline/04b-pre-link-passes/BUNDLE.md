---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T16:50:55+00:00
source_commit: bbd84dc65e58598bfa71fafe72764b4076b0869b
watched_paths_digest: dfd3ca12a15e0fd7d9c109a6b851cf13f5c760f284b7a34d3f7f6c265fbc790f
source_doc: docs/llm-generated/pipeline/04b-pre-link-passes.md
source_doc_digest: 228bb9682f2745b246036d719f177f4cc2baf63ca62090fe5b3bd8328bc2b93f
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for pipeline/04b-pre-link-passes

## Intent

Tests verify the pre-link IR-pass sequence described in
[`docs/llm-generated/pipeline/04b-pre-link-passes.md`](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md):
the ordered Phase A / B / C / D passes that run inside
`generateIRForTranslationUnit` in `slang-lower-to-ir.cpp` before
the per-translation-unit IR module is cached and pulled into
`linkAndOptimizeIR` by `linkIR`.

The `-dump-ir` flag emits a `### LOWER-TO-IR:` block whose body
is the IR snapshot at the end of Phase D (after every pre-link
pass has run, and before any post-link pass runs). That block is
the primary observation surface; the tests in this bundle compile
each shader with
`//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir
-o /dev/null -entry main -stage compute` and FileCheck the block.
Two validation-pass diagnostics are exercised with
`//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` (recursive-types in D1
and missing-return in D5).

The bundle covers all four phases at the granularity the doc
documents. Several internal-only knobs (`[__unsafeForceInlineEarly]`
user attribute, debug-info insertion, obfuscation) are recorded
under `## Out of scope` rather than tested.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                | Claim (one line)                                                                                                                          | Tests                                                                            |
| -------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| C-01     | [#phase-a-ast-walk-and-ir-emission](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission)                     | Phase A row A6 lowers the entry point to an IRFunc carrying `[entryPoint(...)]` and `[numThreads(...)]` decorations.                      | `phase-a-entry-point-lowered-with-numthreads.slang`                              |
| C-02     | [#phase-a-ast-walk-and-ir-emission](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission)                     | Phase A row A8 emits the `kIROp_GlobalHashedStringLiterals` aggregate when any hashed-string-literal source token is observed.            | `phase-a-global-hashed-string-literals-emitted-for-printf.slang`                 |
| C-03     | [#phase-a-ast-walk-and-ir-emission](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission)                     | Phase A row A6 iterates over every entry point so the IR contains an IRFunc for each one.                                                 | `phase-a-multiple-entry-points-each-get-irfunc.slang`                            |
| C-04     | [#lowerdefer](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#lowerdefer)                                                                 | Phase B's lowerDefer (B3) emits the defer body at scope exit and removes the `defer` opcode.                                              | `phase-b-lower-defer-inlines-body-at-scope-exit.slang`                           |
| C-05     | [#lowerdefer](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#lowerdefer)                                                                 | Phase B's lowerDefer emits the deferred body on every exit path, including the early return.                                              | `phase-b-lower-defer-also-fires-on-early-return.slang`                           |
| C-06     | [#lowererrorhandling](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#lowererrorhandling)                                                 | Phase B's lowerErrorHandling (B2) rewrites `try` calls into ordinary call + ifElse; no `tryCall` opcode survives.                         | `phase-b-lower-error-handling-removes-try-call.slang`                            |
| C-07     | [#synthesizebitfieldaccessors](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#synthesizebitfieldaccessors)                               | Phase B's synthesizeBitFieldAccessors (B4) synthesises accessor bodies; the backing field token appears in the lowered emit.              | `phase-b-synthesize-bit-field-accessors-creates-backing.slang`                   |
| C-08     | [#phase-c-mandatory-optimization-passes](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes)           | Phase C's constructSSA (C1) promotes addressable locals to SSA temporaries; a plain `int` local survives without a `var Int`.             | `phase-c-ssa-construction-eliminates-trivial-local.slang`                        |
| C-09     | [#phase-c-mandatory-optimization-passes](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes)           | Phase C's applySparseConditionalConstantPropagation (C2) folds constant arithmetic to a literal in the store.                             | `phase-c-sccp-folds-constant-arithmetic.slang`                                   |
| C-10     | [#phase-c-mandatory-optimization-passes](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes)           | Phase C's simplifyCFG (C3) removes the dead branch when the condition is a compile-time constant.                                         | `phase-c-simplify-cfg-removes-dead-branch.slang`                                 |
| C-11     | [#phase-c-mandatory-optimization-passes](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes)           | Phase C's per-function eliminateDeadCode (C5) drops an intra-function dead computation before the LOWER-TO-IR dump.                       | `phase-c-dead-code-eliminates-unused-helper.slang`                               |
| C-12     | [#performmandatoryearlyinlining-and-the-surrounding-loop](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#performmandatoryearlyinlining-and-the-surrounding-loop) | Phase B's prelinkIR + Phase C's mandatory-early-inlining loop inline `[__unsafeForceInlineEarly]` core helpers so the body appears inline. | `phase-c-mandatory-early-inlining-pulls-in-core-helper.slang`                    |
| C-13     | [#phase-d-non-essential-validation-stripping-and-finalization](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization) | Phase D's checkForRecursiveTypes (D1) rejects a self-referential struct with the cyclic-reference error.                  | `phase-d-check-recursion-rejects-recursive-type.slang`                           |
| C-14     | [#phase-d-non-essential-validation-stripping-and-finalization](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization) | Phase D's checkForMissingReturns (D5) warns when a non-void function lacks a return on every control-flow path.            | `phase-d-missing-return-warns-on-non-void.slang`                                 |
| C-15     | [#stripfrontendonlyinstructions](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#stripfrontendonlyinstructions)                           | Phase D's stripFrontEndOnlyInstructions (D10) removes IRHighLevelDeclDecoration before the LOWER-TO-IR dump.                              | `phase-d-strip-front-end-only-removes-high-level-decl.slang`                     |
| C-16     | [#phase-d-non-essential-validation-stripping-and-finalization](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization) | Phase D's final eliminateDeadCode (D12) keeps export decorations alive so D15 buildMangledNameToGlobalInstMap can index them. | `phase-d-mangled-name-export-decoration-survives-strip.slang`                    |

## Tests in this bundle

| File                                                                  | Intent     | Doc anchor                                                            |
| --------------------------------------------------------------------- | ---------- | --------------------------------------------------------------------- |
| `phase-a-entry-point-lowered-with-numthreads.slang`                   | functional | `#phase-a-ast-walk-and-ir-emission`                                   |
| `phase-a-global-hashed-string-literals-emitted-for-printf.slang`      | functional | `#phase-a-ast-walk-and-ir-emission`                                   |
| `phase-a-multiple-entry-points-each-get-irfunc.slang`                 | functional | `#phase-a-ast-walk-and-ir-emission`                                   |
| `phase-b-lower-defer-inlines-body-at-scope-exit.slang`                | functional | `#lowerdefer`                                                         |
| `phase-b-lower-defer-also-fires-on-early-return.slang`                | functional | `#lowerdefer`                                                         |
| `phase-b-lower-error-handling-removes-try-call.slang`                 | functional | `#lowererrorhandling`                                                 |
| `phase-b-synthesize-bit-field-accessors-creates-backing.slang`        | functional | `#synthesizebitfieldaccessors`                                        |
| `phase-c-ssa-construction-eliminates-trivial-local.slang`             | functional | `#phase-c-mandatory-optimization-passes`                              |
| `phase-c-sccp-folds-constant-arithmetic.slang`                        | functional | `#phase-c-mandatory-optimization-passes`                              |
| `phase-c-simplify-cfg-removes-dead-branch.slang`                      | functional | `#phase-c-mandatory-optimization-passes`                              |
| `phase-c-dead-code-eliminates-unused-helper.slang`                    | functional | `#phase-c-mandatory-optimization-passes`                              |
| `phase-c-mandatory-early-inlining-pulls-in-core-helper.slang`         | functional | `#performmandatoryearlyinlining-and-the-surrounding-loop`             |
| `phase-d-check-recursion-rejects-recursive-type.slang`                | negative   | `#phase-d-non-essential-validation-stripping-and-finalization`        |
| `phase-d-missing-return-warns-on-non-void.slang`                      | negative   | `#phase-d-non-essential-validation-stripping-and-finalization`        |
| `phase-d-strip-front-end-only-removes-high-level-decl.slang`          | functional | `#stripfrontendonlyinstructions`                                      |
| `phase-d-mangled-name-export-decoration-survives-strip.slang`         | functional | `#phase-d-non-essential-validation-stripping-and-finalization`        |

## Doc gaps observed

- The doc states that Phase D's per-function eliminateDeadCode (C5) and the final eliminateDeadCode (D12) both run with `keepExportsAlive = true` and `keepLayoutsAlive = true`, but does not name a user-observable consequence of those flags being set rather than unset. The mangled-name-export test (C-16) anchors on the observed result (export decorations survive); a doc sentence calling that out explicitly would be useful.
- The doc names B5 `lowerExpandType` and explains the IR shape change (pattern nested inside `IRExpand`), but does not provide a Slang surface that produces an `IRExpandType` cleanly enough for the pre-link dump to show the rewrite. The variadic-generics surface this depends on is documented only by reference. A doc gap: name a minimal user-code example that produces `IRExpandType` so the pass's effect is testable.
- The doc lists D6 `checkAutoDiffUsages`, D8 `addDecorationsForGenericsSpecializedWithExistentials`, and D9 `checkForMeshOutputReads`, but the diagnostic text and severity each pass emits is not stated. Without verbatim diagnostic text the `DIAGNOSTIC_TEST` anchors are brittle; doc should list at least the error code.
- The doc names B6 `insertDebugValueStore` gated on `debugInfoLevel >= Standard`, but does not give the surface-observable inst (`DebugValue`) that distinguishes the `-g` vs no-`-g` IR. A claim sentence pointing at the expected post-pass opcode would unblock a focused test.

## Out of scope (no-GPU runner)

- `prelinkIR`'s pull-in of the `externalSymbolsToPrelink` set populated by upstream lowering — the set itself is not user-visible.
- `validateIRModuleIfEnabled` (A10, D14) — no-op unless the IR-validation compiler option is set; not exercised by the agentic bundle.
- `obfuscateModuleLocs` (D13) — requires both `-obfuscate-code` and `-source-map`; runner does not exercise the surface.
- `[__unsafeForceInlineEarly]` user-code surface — the attribute is internal-only and must not appear in user code; the loop's fixed-point convergence is tested indirectly via C-12.
- `checkForOperatorShiftOverflow` (D7) — does not reliably diagnose `1 << 40` in the default option set; treat as out-of-scope until a clean surface trigger is documented.
- The exact ordering of A1…A10, B1…B6, C1…C7, D1…D15 — the `### LOWER-TO-IR:` dump shows only the post-Phase-D state, not intermediate stages, so per-pass ordering is not observable from this bundle.
- `insertDebugValueStore` (B6) — requires `-g` and couples observation to SPIR-V debug instructions outside the pre-link scope.
