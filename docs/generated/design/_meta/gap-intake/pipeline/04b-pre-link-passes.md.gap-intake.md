---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:45:32Z
target_doc: pipeline/04b-pre-link-passes.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 7
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for pipeline/04b-pre-link-passes.md

## Summary

Seven gaps from the `design/pipeline/04b-pre-link-passes` bundle, all
confirmed in source and all fixed; nothing was escalated and no gap
turned out to be a compiler defect. Five sections were touched — the
Phase A prose (A3/A9 observability), the Phase D table plus a new
diagnostics table, the option-set toggles, the per-translation-unit
predicates (D8), and the auto-diff pruning subsection — which is
exactly the set the gaps anchored to. Two of the observations were
richer in source than in the gap: the D7 shift validator has a second
diagnostic (`W41034`, the narrow-integer-promotion case) that the gap
did not mention, and the reason `E33180` survives
`-disable-non-essential-validations` is a documented body-scan
fallback in the typeflow pass, not merely "another pass also
diagnoses". The page is 44,470 bytes against a 65,536-byte cap.

Unwatched-source caveat: this page's `watched_paths` cover the
orchestrator (`slang-lower-to-ir.cpp`), `slang-ir-link.cpp`, the
option header, the request/session files, and the two `*.meta.slang`
modules, but not the ~30 pass implementations the page necessarily
names. Five of the seven fixes therefore rest partly on files outside
`watched_paths`; each is named in the Evidence cell below. The three
worth adding to the manifest, in priority order, are
`source/slang/slang-ir-operator-shift-overflow.cpp`,
`source/slang/slang-ir-constexpr.cpp`, and
`source/slang/slang-compile-request.h` (the header, alongside the
already-watched `.cpp`, is where the `NV_SHADER_EXTN_SLOT` handling
that gates A9 lives).

## Actions

| Gap ID       | Action | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                    | Fix summary                                                                                                                       |
| ------------ | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| 6dc294a9d9e1 | fixed  | All three emit sites are in watched `source/slang/slang-lower-to-ir.cpp`: `emitIncrementCoverageCounter` at `:9999` under `context->traceCoverage` (`:9995`), `emitIncrementFunctionCoverageCounter` at `:14321` under `traceFunctionCoverage` (`:14311`), `emitIncrementBranchCoverageCounter` at `:8146` under `traceBranchCoverage` (`:8142`); options read at `:15443-15447`. CLI spellings from `source/slang/slang-options.cpp:622-640` and the verified directives/CHECKs of the bundle test `phase-a-trace-coverage-toggles-add-counters-in-decl-lowering.slang`. Post-link lowering is `SLANG_PASS(instrumentCoverage, ...)` at `source/slang/slang-emit.cpp:1216`. | added the CLI spelling to each of the three toggle rows and a paragraph naming the three counter opcodes, their emit sites, and the post-link `instrumentCoverage` pass |
| b2cfe19ee365 | fixed  | Watched `source/slang/slang-lower-to-ir.cpp:15449-15451` and `:15529-15534` attach both decorations to `module->getModuleInst()`. `dumpIRModule` walks `module->getGlobalInsts()` (`source/slang/slang-ir.cpp:8400-8406`) and `IRInst::getFirstChild` skips past the last decoration (`:8872-8885`), so module-inst decorations are never printed. A3's surface is watched `source/slang/core.meta.slang:127-128` (`attribute_syntax [ExperimentalModule]`), used at `source/standard-modules/experimental/workgraph.slang:9`. A9's surface is `NV_SHADER_EXTN_SLOT` / `NV_SHADER_EXTN_REGISTER_SPACE` in unwatched `source/slang/slang-compile-request.h:283-338`. | added a Phase A paragraph stating the decorations land on the module instruction (hence absent from `-dump-ir`) and naming the user-level trigger for each |
| 0a08762a3dce | fixed  | Codes read from `source/slang/slang-diagnostics.lua`: `41001` `:4909-4913`, `41010` `:4937-4942`, `41016` `:4972-4977`, `41022` `:5097-5102`, `41030` `:5125-5130`, `54005` `:5539-5544`, plus D3's `40012` `:4865-4869` and `40013` `:4871-4876`. Each printed form is a verified CHECK line of the bundle tests `phase-d-check-recursion-rejects-recursive-type.slang`, `phase-d-missing-return-warns-on-non-void.slang`, `phase-d-uninitialized-value-warns-on-unassigned-local.slang`, `phase-d-autodiff-usage-rejects-non-differentiable-call.slang`, `phase-d-shift-overflow-warns-on-oversized-shift-amount.slang`, `phase-d-mesh-output-read-rejected.slang`, whose shaders are the minimal triggers used. | added a diagnostics table after the Phase D pass table (code, title, minimal trigger for D1/D3/D4/D5/D6/D7/D9) instead of widening the 5-column pass table |
| d72afa09f9f3 | fixed  | Unwatched `source/slang/slang-ir-constexpr.cpp`: `markConstExpr` at `:300-302` delegates to `source/slang/slang-ir.cpp:10050-10060`, which retypes the value to `@ConstExpr T` via `getRateQualifiedType(getConstExprRate(), ...)`; the two diagnostics are `Diagnostics::ArgIsNotConstexpr` at `:552` and `Diagnostics::NeedCompileTimeConstant` at `:671`. Entry point `propagateConstExpr` at `:699`. | extended the D3 row with the `@ConstExpr T` retyping it performs and pointed at the new diagnostics table for `E40012` / `E40013`               |
| 972347a246df | fixed  | Unwatched `source/slang/slang-ir-operator-shift-overflow.cpp:27-88`: the pass only inspects `kIROp_Lsh` insts, warns when the *literal* right operand meets or exceeds the left operand's element width (`:57`), and has a second arm `OperatorShiftOnNarrowType` (`W41034`, `:82`) for non-literal amounts on a sub-32-bit left operand. Folding is confirmed: SCCP evaluates `kIROp_Lsh` via `evalLsh` (`source/slang/slang-ir-sccp.cpp:776`, dispatched at `:1190-1196`) and C2 runs at watched `source/slang/slang-lower-to-ir.cpp:15606`, before D7 at `:15711`. | added the "only shifts that survive Phase C" caveat with the `evalLsh` citation, and documented the second warning `W41034`                     |
| 57115a6bf165 | fixed  | Unwatched `source/slang/slang-ir-check-specialize-generic-with-existential.cpp:40-41` says in-source "The actual diagnostic is emitted later by the typeflow or specialize passes"; the pass only adds `kIROp_DisallowSpecializationWithExistentialsDecoration` (`:67-69`). The consumer is `isInvalidExistentialSpecialization` (`source/slang/slang-ir-typeflow-specialize.cpp:228-240`), whose comment documents the body-scan fallback for the undecorated case — the reason the error survives `-disable-non-essential-validations` — and `emitExistentialSpecializationDiagnostic` (`:8293-8312`) emits `CannotSpecializeGenericWithExistential` = `E33180` (`source/slang/slang-diagnostics.lua:1547-1552`). | added a note under the per-translation-unit predicates that D8 only marks the `specialize`, that `E33180` is raised post-link, and that the body-scan fallback keeps it firing with D8 off |
| eea2fd2864c6 | fixed  | Watched `source/slang/slang-ir-link.cpp:69-84`: the `canPruneAutodiffLinkArtifacts` comment states the artifacts are removed by DCE either way and that the cost is compile time (issue #11781), so emitted code is not the observable; the artifacts are named there and at `cloneAnnotations` (`:228-250`) and `shouldDeepCloneWitnessTable` (`:729`). No dedicated observation point: `linkIR` is called bare at `source/slang/slang-emit.cpp:1004` (the "dump it here" comment at `:1041-1043` has no call under it), while `SLANG_PASS`-wrapped passes emit `### AFTER <pass>:` under `-dump-ir` (`source/slang/slang-pass-wrapper.cpp:80-83`). | added a paragraph naming what to look for (module-scope `IRAnnotation`s, mangled-name-keyed witness entries), and stating that the difference is not in emitted code and has no isolating dump |
