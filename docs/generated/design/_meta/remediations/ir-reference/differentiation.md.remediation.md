---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T13:45:00Z
target_doc: ir-reference/differentiation.md
review_report: ../../reviews/ir-reference/differentiation.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/differentiation.md

## Summary
Five findings were fixed and one was rejected as out of scope, so the document
was edited and `mark-fresh` is needed. The critical error was real: the
`TrivialForwardDifferentiate` summary said the derivative leaves differentials
alone when the implementation forces them to zero, and that row now states the
zeroing behaviour. The survives-to-emit claim was narrowed, the nullary
`DiffTypeInfo` operand cell now uses `—`, the intended-reader sentence moved
into the opening paragraph, and the `diff.meta.slang` mention became a link.
The stale front-matter digest belongs to the operator's `mark-fresh` step.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed at HEAD. `source/slang/slang-ir-autodiff-fwd.cpp:191-193` documents the `zeroDifferentials` parameter of `emitInOutParamWriteBacks` as "the call contributes no tangent, so every output differential is forced to zero rather than threading the incoming one", and lines 203-205 implement it with `getDifferentialZeroOfType`. `generateTrivialFwdDiffFunc` (line 219) calls it with `zeroDifferentials = true` at line 237 and pairs the primal call with `getDifferentialZeroOfType` at lines 247-249. "Leaves differentials alone" was the reverse of the actual behaviour. | Row summary now reads "Asks for a derivative that runs the primal and returns zero output differentials, ignoring incoming tangents." |
| F-002 | fixed | Confirmed. `source/slang/slang-emit-c-like.cpp:5287-5292` handles `kIROp_BuiltinRequirementKey` with the comment that the key "is hoistable and so may survive as an (unreferenced) global inst after specialization, so skip it explicitly", inside `CLikeSourceEmitter::ensureGlobalInst` (declared at line 5275, not `emitGlobalInst` at 5087). So the opcode can reach the emitter even though it produces no code. | Opening sentence of that `## Source` paragraph changed to "No opcode in this family produces target code", and a two-sentence addition at the end of the paragraph records that `builtinRequirementKey` can survive to `ensureGlobalInst`, which skips it. |
| F-003 | rejected-out-of-scope | Correct but not a remediator edit. `docs/generated/design/_meta/prompts/_remediate.md:97-100` reserves `generated_at`, `source_commit`, and `watched_paths_digest` for the operator's `regenerate.py mark-fresh` run and instructs the remediator not to touch them. This document was edited, so `mark-fresh` will record the current digest. | — |
| F-004 | fixed | Confirmed. `source/slang/slang-ir-insts.lua:1124-1126` declares `DiffTypeInfo` with `hoistable = true` and nothing else — no `operands`, no `min_operands` — so it is genuinely nullary, and `docs/generated/design/_meta/prompts/_common.md:238` requires `—` for nullary opcodes. The `†` marker on this page means "declares only `min_operands`", which does not describe this entry. | Operands cell changed from `† (none declared)` to `—`. |
| F-005 | fixed | Confirmed against `docs/generated/design/_meta/prompts/_common.md:65-66`, which requires the first body paragraph to state both what the document covers and who it is for; the reader sentence was in a separate second paragraph. | Merged the intended-reader sentence into the end of the opening paragraph; wording unchanged apart from the sentence join. |
| F-006 | fixed | Confirmed against `docs/generated/design/_meta/prompts/_common.md:43-45`, which requires source-file citations to be Markdown links with the workspace-relative path. The `BackwardDifferentiate` callout used a bare backticked filename while the top of the page already links the same file. | `` `diff.meta.slang` `` in that callout replaced with a link to `../../../../source/slang/diff.meta.slang`. |
