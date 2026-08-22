---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T14:40:00Z
target_doc: pipeline/04-ast-to-ir.md
review_report: ../../reviews/pipeline/04-ast-to-ir.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/04-ast-to-ir.md

## Summary

Three findings were reviewed. The two source-alignment findings were verified against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23` and fixed: the module-output boundary now records the implicit `EntryPointAttribute` that lowering attaches to the checked AST, and the layout-module description no longer claims layout decorations are that module's only contents. The major finding is about the front-matter `watched_paths_digest` and was rejected as out of scope. The document was edited.

## Actions

| Finding ID | Action                | Rationale                                                                                                                                                                                                                                                                                                                 | Fix summary                                                                                                                                                                                                                 |
| ---------- | --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | rejected-out-of-scope | `docs/generated/design/_meta/prompts/_remediate.md` lines 97-100 reserve `generated_at`, `source_commit`, and `watched_paths_digest` for the operator's `regenerate.py mark-fresh` run: "Do not edit those three fields yourself." The digest is refreshed when the operator marks this page fresh after the edits below. | —                                                                                                                                                                                                                           |
| F-002      | fixed                 | Confirmed at HEAD: `source/slang/slang-lower-to-ir.cpp:15215-15224` creates an `EntryPointAttribute`, fills `capabilitySet` from the entry-point profile, and calls `addModifier(entryPointFuncDecl, entryPointAttr)` when no explicit attribute exists.                                                                  | `## Module-level outputs`: replaced "there are no additional side artefacts" with a sentence naming the returned `IRModule` as the only separate output object plus the implicit entry-point attribute added at line 15223. |
| F-003      | fixed                 | Confirmed at HEAD: `createIRModuleForLayout` at `source/slang/slang-lower-to-ir.cpp:16353` decorates the module root at `:16450` and adds `addRequireCapabilityAtomDecoration` to entry-point stubs at `:16491`, alongside the stub globals and their layout instructions.                                                | `## Adjacent pipelines`: 04c bullet now lists stubs, module-root and stub layout decorations with their supporting layout instructions, and SPIR-V / Metal capability decorations.                                          |
