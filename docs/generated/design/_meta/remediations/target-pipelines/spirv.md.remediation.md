---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:17:03Z
target_doc: target-pipelines/spirv.md
review_report: ../../reviews/target-pipelines/spirv.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 4, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for target-pipelines/spirv.md

## Summary

All four review findings were verified against the source at the reviewed
commit and fixed. Each was both factually correct and within the
target-pipeline page contract, so no findings were rejected, deferred, or
escalated. The edits correct the `simplifyIRForSpirvLegalization` loop
description (the bound counters are never incremented), the `spirv-val`
validation trigger, the intro's `CodeGenTarget` claim, and the
`optimizeSPIRV` / downstream-`compile` table rows.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed at `source/slang/slang-ir-spirv-legalize.cpp:2878-2916`: `iterationCounter` and `funcIterationCount` are declared and tested but never incremented in the loop bodies, so the `< 8` / `< 16` guards are inert and the documented bound-based termination and `8*16=128` worst case are wrong. | Rewrote the Loops prose, Phase D diagram diamond labels and exit edges, the Phase D table row 5 note, and the Source-section bullet to state the constants exist but the counters are not advanced, so termination is fixed-point-only. |
| F-002 | fixed | Confirmed at `source/slang/slang-emit.cpp:3045-3067`: `shouldRunSPIRVValidation` returns false when `SkipSPIRVValidation` or `IncompleteLibrary` is set and returns true only when `SLANG_RUN_SPIRV_VALIDATION == "1"`; no `-validate-spirv` option exists in `source/slang/slang-options.cpp` (only `-skip-spirv-validation` at line 685). | Replaced the "`-validate-spirv` or non-empty env var" claim in Phase D table row 18 and the downstream-chain prose with the actual gate (env var exactly `"1"`, with `SkipSPIRVValidation` and `IncompleteLibrary` off; no `-validate-spirv` flag). |
| F-003 | fixed | Confirmed at `source/slang/slang-code-gen.cpp:1154-1158`: `emitSPIRVForEntryPointsDirectly` is called only from the `CodeGenTarget::SPIRV` case under `shouldEmitSPIRVDirectly()`, while `SPIRVAssembly` (lines 1089-1111) compiles an intermediate `CodeGenTarget::SPIRV` artifact and disassembles it. | Narrowed the intro to `CodeGenTarget::SPIRV` with `shouldEmitSPIRVDirectly()` as the direct-emit trigger and added a note that `SPIRVAssembly` reaches the pipeline only indirectly via the intermediate SPIR-V compile-then-disassemble path. |
| F-004 | fixed | Confirmed `optimizeSPIRV` `#if 0` block is at `source/slang/slang-emit.cpp:3092-3099` (no `optimizeSPIRV` symbol in `slang-emit-spirv.cpp`), and the downstream `compiler->compile` at line 3210 is inside the `if (compiler)` guard at line 3106, not always invoked. | Moved the Phase D row 16 `optimizeSPIRV` citation to `slang-emit.cpp` (lines 3092-3099) and changed row 19's gate from "always invoked" to `compiler != nullptr` plus the optimization-level options, citing the `if (compiler)` guard. |
