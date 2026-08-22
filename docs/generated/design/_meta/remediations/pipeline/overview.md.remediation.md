---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T14:20:00Z
target_doc: pipeline/overview.md
review_report: ../../reviews/pipeline/overview.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/overview.md

## Summary

Three minor findings were reviewed. Two were verified against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23` and fixed: the AST-to-IR lowering summary no longer claims that statements uniformly become basic blocks, and both the Emit "Driven by" list and the driver section now name `slang-code-gen.cpp` as the per-target dispatcher with `slang-emit.cpp` correctly cast as the linked-IR orchestrator and source-emitter selector. The third finding concerns the front-matter `watched_paths_digest` and was rejected as out of scope. The document was edited.

## Actions

| Finding ID | Action                | Rationale                                                                                                                                                                                                                                                                                                                                      | Fix summary                                                                                                                                                                                                                    |
| ---------- | --------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| F-001      | rejected-out-of-scope | `docs/generated/design/_meta/prompts/_remediate.md` lines 97-100 reserve `generated_at`, `source_commit`, and `watched_paths_digest` for the operator's `regenerate.py mark-fresh` run: "Do not edit those three fields yourself." The digest is refreshed when the operator marks this page fresh after the edits below.                      | —                                                                                                                                                                                                                              |
| F-002      | fixed                 | Confirmed the three lowering shapes: `visitIfStmt` at `source/slang/slang-lower-to-ir.cpp:8280` creates blocks, `visitSeqStmt` / `visitBlockStmt` at `:8811` / `:8821` recursively lower children, and `visitReturnStmt` at `:8831` emits into the current block.                                                                              | `### AST → IR lowering`: replaced "statements become basic blocks with parameters" with the control-flow-vs-ordinary-statement split, and moved block parameters into the SSA clause.                                          |
| F-003      | fixed                 | `CodeGenContext::_emitEntryPoints` is defined at `source/slang/slang-code-gen.cpp:1114` and `CodeGenContext::emitEntryPoints` at `:1247`; `slang-emit.cpp` defines only `emitEntryPointsSourceFromIR` and its C-like source-emitter switch. `source/slang/slang-code-gen.cpp` is in this page's watched paths, so the new link is in contract. | `### Emit` "Driven by" list gained a `slang-code-gen.cpp` bullet and re-scoped the `slang-emit.cpp` bullet; the `## Driver entry points` `slang-emit.cpp` bullet was rewritten to name `slang-code-gen.cpp` as the dispatcher. |
