---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:24:00+00:00
target_doc: ir-reference/control-flow.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: e27926ca78614bca20d3b57a5268d5884f642e04074ed66afbbed157eadbfdd7
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 1
  minor: 3
  nit: 0
---

# Review report for ir-reference/control-flow.md

## Summary

The page has the required IR-reference structure, the front matter is valid, and the opcode coverage matches the `TerminatorInst` Lua group plus `block` and `param`. The main issue is that the `C++ wrapper` column does not consistently name the actual `IR*` wrapper structs from `slang-ir-insts.h`, and a few AST-origin cells describe source-produced opcodes as synthesized.

## Items checked

- Verified the front-matter fields and copied `source_commit` / `watched_paths_digest` into this report.
- Checked the manifest prompt, dependencies, and resolved watched paths from `regenerate.py show ir-reference/control-flow.md`.
- Verified all relative links in the page resolve locally, including peer generated docs and `docs/design/ir.md`.
- Checked the Lua declarations for `block`, `param`, every `TerminatorInst` child, `discard`, `Require*`, `StaticAssert`, `Printf`, `Abort`, and `gpuForeach`.
- Checked wrapper declarations for `IRReturn`, `IRYield`, `IRLoop`, `IRConditionalBranch`, `IRIfElse`, `IRSwitch`, `IRTargetSwitch`, `IRThrow`, `IRTryCall`, `IRDefer`, `IRGenericAsm`, and the `IRRequire*` wrappers.
- Checked lowering origins for `IfStmt`, `ForStmt`, `WhileStmt`, `DoWhileStmt`, `SwitchStmt`, `TargetSwitchStmt`, `ReturnStmt`, `ThrowStmt`, `DeferStmt`, `DiscardStmt`, `BreakStmt`, `ContinueStmt`, `GpuForeachStmt`, `TryExpr`, and `IntrinsicAsmStmt`.
- Spot-checked block-parameter placement and phi-replacement claims against `IRBlock` / `IRParam` helpers.
- Spot-checked branch operand claims against `IRBuilder::emitBranch`, `emitLoop`, `emitIfElse`, `emitSwitch`, and `emitTryCallInst`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Opcodes`, `C++ wrapper` column | The wrapper column does not list the exact C++ wrapper struct names required by the IR-reference contract. It uses values such as `Return`, `Loop`, and `IfElse` instead of the actual `IRReturn`, `IRLoop`, and `IRIfElse`, and it uses `—` for opcodes that do have wrappers, including `yield`, `targetSwitch`, `throw`, `tryCall`, `defer`, `GenericAsm`, and the `Require*` opcodes. | `source/slang/slang-ir-insts.h:1935` declares `struct IRReturn`; `source/slang/slang-ir-insts.h:1941` declares `struct IRYield`; `source/slang/slang-ir-insts.h:2069` declares `struct IRTargetSwitch`; `source/slang/slang-ir-insts.h:2085` declares `struct IRTryCall`; `source/slang/slang-ir-insts.h:2813` declares `struct IRGenericAsm`; `source/slang/slang-ir-insts.h:2820` declares `struct IRRequirePrelude`. | Replace the column values with the exact wrapper names from `slang-ir-insts.h`; keep `—` only for opcodes with no corresponding wrapper struct. |
| F-002 | minor | `## Opcodes`, `targetSwitch` row | The `targetSwitch` row says the AST origin is `(synthesized)`, but `slang-lower-to-ir.cpp` has a statement visitor that lowers `TargetSwitchStmt` directly to `kIROp_TargetSwitch`. | `source/slang/slang-lower-to-ir.cpp:9229` defines `visitTargetSwitchStmt`, and `source/slang/slang-lower-to-ir.cpp:9263` emits `kIROp_TargetSwitch`. | Change the AST origin to `TargetSwitchStmt` / `StageSwitchStmt` in `slang-lower-to-ir.cpp`, or otherwise mention that target-switch statements produce this opcode. |
| F-003 | minor | `## Opcodes`, `GenericAsm` row | The `GenericAsm` row says the AST origin is `(synthesized)`, but lowering emits `kIROp_GenericAsm` from `IntrinsicAsmStmt`. | `source/slang/slang-lower-to-ir.cpp:9274` defines `visitIntrinsicAsmStmt`, and `source/slang/slang-lower-to-ir.cpp:9292` emits `kIROp_GenericAsm`. | Change the AST origin to `IntrinsicAsmStmt` in `slang-lower-to-ir.cpp`. |
| F-004 | minor | `### Abort` | The `Abort` notable-opcode callout goes beyond the per-opcode shape/origin role and gives backend legalization and emission behavior: `slang-ir-spirv-legalize.cpp rewrites it`, `OpAbortKHR is a SPIR-V block terminator`, and `The GLSL emitter maps it to abortEXT`. That is pass/backend behavior, which the IR-reference family contract says belongs in pipeline or target-emission docs rather than the opcode catalog. | `docs/generated/design/_meta/prompts/_common.md:266` forbids pass-by-pass behavior descriptions in IR-reference pages and points readers to `../pipeline/05-ir-passes.md`. | Trim the `Abort` callout to opcode shape and direct source/origin facts, and move or link the legalization/emission details to the appropriate pipeline or target page. |

## No-issues notes

- The Lua opcode coverage for `TerminatorInst` plus `block` and `param` is complete.
- The front matter includes all required generated-document keys and uses a hex-looking watched-path digest.
- The control-flow links resolve to existing generated docs or source/design files.
