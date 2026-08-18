---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:35:44Z
target_doc: cross-cutting/ir-instructions.md
review_report: ../../reviews/cross-cutting/ir-instructions.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/ir-instructions.md

## Summary

All seven findings were checked against `slang-ir-insts.lua`, the HLSL
emitter, the IR module headers and the CI script, and all seven held
up, so all seven were fixed. The two major fixes correct the decoration
table's Opcode column and scope the deduplication guarantee to a single
`IRModule`. Nothing was rejected, deferred, or escalated. The new
`docs/design/ir.md` link added for F-007 uses the `../../../` prefix the
page already uses elsewhere, so it resolves.

## Actions

| Finding ID | Action | Rationale                                                                                                                                                                                                                                                     | Fix summary                                                                                                                                                           |
| ---------- | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | fixed  | `source/slang/slang-ir-insts.lua:1793,1827,2087,2157` declare keys `targetIntrinsic`, `nameHint`, `entryPoint`, `keepAlive` with unprefixed `struct_name` values and operands `nameOperand`, `target`/`definitionOperand`, `profileInst`/`name`/`moduleName`. | Four decoration rows now use the Lua opcodes, `struct_name` values and source operand names; `## Decorations` prose calls `IR*Decoration` the generated C++ wrappers. |
| F-002      | fixed  | `slang-ir-insts.lua:3408-3437`: every `constexpr*` opcode has an explicit 1-3 operand list.                                                                                                                                                                   | Line 192: `(variadic)` -> `1-3 fixed operands; see Lua entries`.                                                                                                      |
| F-003      | fixed  | `slang-ir-insts.lua:1631-1633`: `waveGetActiveMask = {}` is nullary, `waveMaskBallot` takes `mask, condition`.                                                                                                                                                | Wave row: `(variadic)` -> `none / mask, condition`, noting shape varies by opcode.                                                                                    |
| F-004      | fixed  | `source/slang/slang-ir.h:2303` makes `IRDeduplicationContext` an `IRModule` member (accessor `2160`) and `slang-ir.cpp:2869` numbers insts in that per-module map, so "regardless of which module asks" overstated identity.                                  | Scoped the `getBuiltinRequirementKey` guarantee to one destination `IRModule`; noted imports dedupe on entry, not by cross-module pointer identity.                   |
| F-005      | fixed  | `source/slang/slang-emit-hlsl.cpp:589-591` emits `[NodeLaunch("`, the mode string, `")]`.                                                                                                                                                                     | "an HLSL named constant" -> "a quoted HLSL attribute string".                                                                                                         |
| F-006      | fixed  | `extras/check-inst-version-changes.sh:19-21` states it makes no GitHub API call; `156-166` writes `pr-number.txt`/`comment-body.txt` for a `workflow_run` job.                                                                                                | Step 3 now attributes the artifact to the script and the comment to the consuming workflow.                                                                           |
| F-007      | fixed  | `_meta/prompts/cross-cutting-ir-instructions.md:57-59` requires the migration note; `docs/design/ir.md:61` still calls it future work while `slang-ir-insts.h:34-39` already defines `IRDecoration : IRInst`.                                                 | Added one sentence to `## Decorations` marking that design-note wording as history, not an active roadmap.                                                            |
