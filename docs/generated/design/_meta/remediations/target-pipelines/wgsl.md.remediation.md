---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:07:14Z
target_doc: target-pipelines/wgsl.md
review_report: ../../reviews/target-pipelines/wgsl.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/wgsl.md

## Summary
All four findings were verified against source and fixed in the target document. The HEAD-committed regeneration still carried the stale wording (`push_constant`, "element-wise select", no glslang downstream node, no user-guide link); the working tree now contains the corrected text. Edits completed the `WGSLSPIRVAssembly` downstream chain (F-001), removed the non-emitted `push_constant` WGSL address space (F-002), reworded the `legalizeLogicalAndOr` callout to match the cast/rebuild behavior (F-003), and added the user-guide See also link (F-004). Nothing was rejected, deferred, or escalated. The source commit is unchanged (watched source files did not change); the operator runs `mark-fresh` to refresh front-matter.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-code-gen.cpp:1059-1060` maps `WGSLSPIRVAssembly` to the `WGSLSPIRV` intermediate and `_emitEntryPoints` disassembles it; `source/slang/slang-global-session.cpp:218,222-225` registers `WGSL -> WGSLSPIRV` via Tint and `WGSLSPIRV -> WGSLSPIRVAssembly` via glslang. In-scope downstream-chain accuracy. | Phase D diagram: added `asmGate` + glslang `(downstream)` node and edges; added table row 9 (glslang disassembly); expanded the runtime-predicate row and Downstream Tint callout to cover the second transition. |
| F-002 | fixed | `source/slang/slang-ir-wgsl-legalize.cpp:243-328` assigns only Uniform/Global/Function/ThreadLocal/GroupShared; `source/slang/slang-emit-wgsl.cpp:331-356` emits uniform/storage/function/private/workgroup with no `PushConstant` case. | `specializeAddressSpaceForWGSL` callout: address-space list now `function`, `private`, `storage`, `uniform`, `workgroup` (removed `push_constant`). |
| F-003 | fixed | `source/slang/slang-ir-legalize-binary-operator.cpp:179-310` (`kIROp_And`/`kIROp_Or`) casts operands/results to bool-vector via `emitCast`, rebuilds with `emitAnd`/`emitOr`, and loops per-element with `emitMakeArray` for array-lowered matrices; no select is emitted. | `legalizeLogicalAndOr` callout: removed "element-wise selects"; now describes the cast/rebuild path and per-element array handling. |
| F-004 | fixed | Contract requires a user-facing target-doc link when present (`docs/generated/design/_meta/prompts/_common.md:352-359`); `docs/user-guide/a2-03-wgsl-target-specific.md` exists. | See also: added bullet linking `../../../user-guide/a2-03-wgsl-target-specific.md`. |
