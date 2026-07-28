---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:01:07Z
target_doc: ir-reference/control-flow.md
review_report: ../../reviews/ir-reference/control-flow.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/control-flow.md

## Summary

All four findings were verified against source and fixed in the target
document. F-001 (major) replaced the `C++ wrapper` column with the exact
`IRFoo` struct names from `slang-ir-insts.h`, keeping `—` only for the six
opcodes (`missingReturn`, `unreachable`, `discard`, `gpuForeach`, `Printf`,
`Abort`) that have no wrapper struct. F-002 and F-003 corrected two AST-origin
cells from `(synthesized)` to the statements that lower to them. F-004 trimmed
the `### Abort` callout of pass/backend legalization and emission behavior the
IR-reference contract forbids. Breakdown: 4 fixed, 0 rejected-bogus, 0
rejected-out-of-scope, 0 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Wrappers confirmed in `source/slang/slang-ir-insts.h` (`IRReturn` 1935, `IRYield` 1941, `IRLoop` 1999, `IRConditionalBranch` 2017, `IRIfElse` 2037, `IRSwitch` 2047, `IRTargetSwitch` 2069, `IRThrow` 2079, `IRTryCall` 2085, `IRDefer` 2095, `IRGenericAsm` 2813, `IRRequire*`/`IRStaticAssert` 2820-2855) and `IRUnconditionalBranch` 1981; no struct for missingReturn/unreachable/discard/gpuForeach/Printf/Abort. | Wrapper cells set to exact `IRFoo` names; `—` kept for the six wrapperless opcodes |
| F-002 | fixed | `visitTargetSwitchStmt` (`source/slang/slang-lower-to-ir.cpp:9229`) emits `kIROp_TargetSwitch` at 9265. | `targetSwitch` AST origin `(synthesized)` -> `TargetSwitchStmt` (same row edit as F-001) |
| F-003 | fixed | `visitIntrinsicAsmStmt` (`source/slang/slang-lower-to-ir.cpp:9274`) emits `kIROp_GenericAsm` at 9294. | `GenericAsm` AST origin `(synthesized)` -> `IntrinsicAsmStmt` (same row edit as F-001) |
| F-004 | fixed | `_meta/prompts/_common.md:266` forbids pass-by-pass behavior in IR-reference pages; callout had described spirv-legalize rewriting, OpAbortKHR terminator handling, and GLSL `abortEXT`. | Removed legalization/emission prose from `### Abort` and the SPIR-V/GLSL clause in its table row; opcode shape and origin kept |
