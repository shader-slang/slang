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
under `## Untested claims` rather than tested.


## Functional coverage
| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| Phase B's lowerDefer (B3) emits the defer body at scope exit so downstream passes see straight-line code; no defer opcode survives. | functional | [#lowerdefer](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#lowerdefer) | [`phase-b-lower-defer-inlines-body-at-scope-exit.slang`](phase-b-lower-defer-inlines-body-at-scope-exit.slang) |
| Phase B's lowerDefer emits the defer body at every exit path of the enclosing scope (per the notable-passes section), including the early return. | functional | [#lowerdefer](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#lowerdefer) | [`phase-b-lower-defer-also-fires-on-early-return.slang`](phase-b-lower-defer-also-fires-on-early-return.slang) |
| Phase B's lowerErrorHandling (B2) rewrites throwing functions to return Result<T,E>; after Phase B no tryCall opcode survives in the IR. | functional | [#lowererrorhandling](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#lowererrorhandling) | [`phase-b-lower-error-handling-removes-try-call.slang`](phase-b-lower-error-handling-removes-try-call.slang) |
| Phase B's prelinkIR (B1) pulls in `[unsafeForceInlineEarly]` core-module bodies; the mandatory-early-inlining loop in Phase C then inlines them, so neither the core operator name nor a forwarding call survives. | functional | [#performmandatoryearlyinlining-and-the-surrounding-loop](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#performmandatoryearlyinlining-and-the-surrounding-loop) | [`phase-c-mandatory-early-inlining-pulls-in-core-helper.slang`](phase-c-mandatory-early-inlining-pulls-in-core-helper.slang) |
| Phase A row A6 calls lowerFrontEndEntryPointToIR once per entry point so the entry-point IRFunc skeleton exists before the full declaration walk. | functional | [#phase-a-ast-walk-and-ir-emission](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission) | [`phase-a-multiple-entry-points-each-get-irfunc.slang`](phase-a-multiple-entry-points-each-get-irfunc.slang) |
| Phase A row A8 emits the kIROp_GlobalHashedStringLiterals aggregate that collects every hashed string literal observed during lowering. | functional | [#phase-a-ast-walk-and-ir-emission](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission) | [`phase-a-global-hashed-string-literals-emitted-for-printf.slang`](phase-a-global-hashed-string-literals-emitted-for-printf.slang) |
| Phase A's lowerFrontEndEntryPointToIR (A6) produces an IRFunc with [entryPoint(...)] and [numThreads(...)] decorations visible in the LOWER-TO-IR dump. | functional | [#phase-a-ast-walk-and-ir-emission](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission) | [`phase-a-entry-point-lowered-with-numthreads.slang`](phase-a-entry-point-lowered-with-numthreads.slang) |
| Phase C's applySparseConditionalConstantPropagation (C2) folds `7 + 3` to a single constant 10 stored into the buffer. | functional | [#phase-c-mandatory-optimization-passes](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes) | [`phase-c-sccp-folds-constant-arithmetic.slang`](phase-c-sccp-folds-constant-arithmetic.slang) |
| Phase C's constructSSA (C1) promotes addressable locals to SSA temporaries; trivial `int x = a; x = x + 1;` shows no var/load/store of a local Int. | functional | [#phase-c-mandatory-optimization-passes](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes) | [`phase-c-ssa-construction-eliminates-trivial-local.slang`](phase-c-ssa-construction-eliminates-trivial-local.slang) |
| Phase C's per-function eliminateDeadCode (C5) sweeps unused intra-function instructions; an unread `int dead = a * 999;` is removed before the LOWER-TO-IR dump. | functional | [#phase-c-mandatory-optimization-passes](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes) | [`phase-c-dead-code-eliminates-unused-helper.slang`](phase-c-dead-code-eliminates-unused-helper.slang) |
| Phase C's simplifyCFG (C3) removes empty/unreachable blocks; an `if (true) ... else ...` collapses to the taken branch with no ifElse opcode. | functional | [#phase-c-mandatory-optimization-passes](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes) | [`phase-c-simplify-cfg-removes-dead-branch.slang`](phase-c-simplify-cfg-removes-dead-branch.slang) |
| Phase D's checkForMissingReturns (D5) warns when a non-void function lacks a return on every control-flow path (target=None, target-agnostic pre-link). | negative | [#phase-d-non-essential-validation-stripping-and-finalization](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization) | [`phase-d-missing-return-warns-on-non-void.slang`](phase-d-missing-return-warns-on-non-void.slang) |
| Phase D's checkForRecursiveTypes (D1) rejects a self-referential struct with the cyclic-reference diagnostic. | negative | [#phase-d-non-essential-validation-stripping-and-finalization](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization) | [`phase-d-check-recursion-rejects-recursive-type.slang`](phase-d-check-recursion-rejects-recursive-type.slang) |
| Phase D's final eliminateDeadCode (D12) keeps exports and layouts alive so the post-link linkIR has the mangled-name index built by D15 to resolve against. | functional | [#phase-d-non-essential-validation-stripping-and-finalization](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization) | [`phase-d-mangled-name-export-decoration-survives-strip.slang`](phase-d-mangled-name-export-decoration-survives-strip.slang) |
| Phase D's stripFrontEndOnlyInstructions (D10) removes IRHighLevelDeclDecoration before the LOWER-TO-IR dump. | functional | [#stripfrontendonlyinstructions](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#stripfrontendonlyinstructions) | [`phase-d-strip-front-end-only-removes-high-level-decl.slang`](phase-d-strip-front-end-only-removes-high-level-decl.slang) |
| Phase B's synthesizeBitFieldAccessors (B4) synthesises get/set bodies for bit-field accessors so the early-inlining loop can inline them. | functional | [#synthesizebitfieldaccessors](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#synthesizebitfieldaccessors) | [`phase-b-synthesize-bit-field-accessors-creates-backing.slang`](phase-b-synthesize-bit-field-accessors-creates-backing.slang) |


## Untested claims
| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| `[__unsafeForceInlineEarly]` user-code surface — the attribute is internal-only and must not appear in user code; the loop's fixed-point convergence is tested indirectly via C-12. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| `checkForOperatorShiftOverflow` (D7) — does not reliably diagnose `1 << 40` in the default option set; treat as out-of-scope until a clean surface trigger is documented. | (unclassified) | [#checkforoperatorshiftoverflow](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#checkforoperatorshiftoverflow) | Reason and explanation to be refined by the next regeneration. |
| `insertDebugValueStore` (B6) — requires `-g` and couples observation to SPIR-V debug instructions outside the pre-link scope. | (unclassified) | [#insertdebugvaluestore](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#insertdebugvaluestore) | Reason and explanation to be refined by the next regeneration. |
| `obfuscateModuleLocs` (D13) — requires both `-obfuscate-code` and `-source-map`; runner does not exercise the surface. | (unclassified) | [#obfuscatemodulelocs](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#obfuscatemodulelocs) | Reason and explanation to be refined by the next regeneration. |
| `prelinkIR`'s pull-in of the `externalSymbolsToPrelink` set populated by upstream lowering — the set itself is not user-visible. | (unclassified) | [#prelinkir](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#prelinkir) | Reason and explanation to be refined by the next regeneration. |
| `validateIRModuleIfEnabled` (A10, D14) — no-op unless the IR-validation compiler option is set; not exercised by the agentic bundle. | (unclassified) | [#validateirmoduleifenabled](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#validateirmoduleifenabled) | Reason and explanation to be refined by the next regeneration. |
| The exact ordering of A1…A10, B1…B6, C1…C7, D1…D15 — the `### LOWER-TO-IR:` dump shows only the post-Phase-D state, not intermediate stages, so per-pass ordering is not observable from this bundle. | implementation-detail | (unspecified) | Internal compiler choice (pass ordering, hoistability decisions, deduplication) with no test-directive that reveals it. |


## Doc gaps observed
| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| (unspecified) | undocumented-behavior | The doc states that Phase D's per-function eliminateDeadCode (C5) and the final eliminateDeadCode (D12) both run with `keepExportsAlive = true` and `keepLayoutsAlive = true`, but does not name a user-observable consequence of those flags being set rather than unset. The mangled-name-export test (C-16) anchors on the observed result (export decorations survive); a doc sentence calling that out explicitly would be useful. |  |
| [#lowerexpandtype](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#lowerexpandtype) | undocumented-behavior | The doc names B5 `lowerExpandType` and explains the IR shape change (pattern nested inside `IRExpand`), but does not provide a Slang surface that produces an `IRExpandType` cleanly enough for the pre-link dump to show the rewrite. The variadic-generics surface this depends on is documented only by reference. A doc gap: name a minimal user-code example that produces `IRExpandType` so the pass's effect is testable. |  |
| [#checkautodiffusages](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#checkautodiffusages) | undocumented-behavior | The doc lists D6 `checkAutoDiffUsages`, D8 `addDecorationsForGenericsSpecializedWithExistentials`, and D9 `checkForMeshOutputReads`, but the diagnostic text and severity each pass emits is not stated. | Without verbatim diagnostic text the `DIAGNOSTIC_TEST` anchors are brittle; doc should list at least the error code. |
| [#insertdebugvaluestore](../../../docs/llm-generated/pipeline/04b-pre-link-passes.md#insertdebugvaluestore) | undocumented-behavior | The doc names B6 `insertDebugValueStore` gated on `debugInfoLevel >= Standard`, but does not give the surface-observable inst (`DebugValue`) that distinguishes the `-g` vs no-`-g` IR. A claim sentence pointing at the expected post-pass opcode would unblock a focused test. |  |
