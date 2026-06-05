---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: name-resolution/scopes.md
review_report: ../../reviews/name-resolution/scopes.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/scopes.md

## Summary

The review reported three findings, all fixed in the target doc. F-001: `WhileStmt`/`DoWhileStmt` are `LoopStmt` -> `BreakableStmt` -> `ScopeStmt` subclasses (`source/slang/slang-ast-stmt.h:100,209,234,242`) and therefore own scopes; they were added to the scope-bearing table and removed from the "do not own a scope" bullet. F-002: the `addSiblingScopeForContainerDecl` description claimed parser usage, but the helper has no parser call site (only `slang-check-expr.cpp`, `slang-check-decl.cpp`, `slang-session.cpp`); the sentence now credits semantic-checking and session/module setup. F-003: the unwatched `slang-lookup.h` citation was replaced with a `lookup.md` link.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ast-stmt.h:234,242` declare `WhileStmt`/`DoWhileStmt` as `LoopStmt`; `LoopStmt:209` is `BreakableStmt`, `:100` is `ScopeStmt`. | Added a `WhileStmt`/`DoWhileStmt` row to the scope-bearing table and dropped them from the "do not own a scope" bullet (kept `IfStmt`). |
| F-002 | fixed | `addSiblingScopeForContainerDecl` has no `slang-parser.cpp` call site; uses are in `slang-check-expr.cpp` (def), `slang-check-decl.cpp`, `slang-session.cpp`. | Changed "used by both the parser and the checker" to "used by semantic-checking and session/module setup code". |
| F-003 | fixed | `slang-lookup.h` is not in this doc's watched paths; manifest edits are out of remediation scope, so the reviewer's link alternative was used. | Replaced the `slang-lookup.h` citation in the scope-walking section with a link to `lookup.md`. |
