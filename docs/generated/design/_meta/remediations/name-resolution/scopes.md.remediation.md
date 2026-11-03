---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T12:00:00Z
target_doc: name-resolution/scopes.md
review_report: ../../reviews/name-resolution/scopes.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for name-resolution/scopes.md

## Summary

Three findings (F-001, F-002, F-003) were verified against the current source and fixed in the doc. F-004 is valid but needs `source/slang/slang-diagnostics.lua` added to the manifest's watched paths, which is outside this cycle, so it is deferred.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: `HLSLClassDecl` (`source/slang/slang-ast-decl.h:465`), `GetterDecl` (:763), `SetterDecl` (:769), `RefAccessorDecl` (:775), `SynthesizedFuncDecl` (:811) were unnamed. | Named all five in the AggTypeDecl and CallableDecl rows with header lines; noted parser-pushed scope (`parseAccessorDecl`, `source/slang/slang-parser.cpp:4999`; `ParseStruct`) versus checker-created `SynthesizedFuncDecl` that only inherits the field. |
| F-002 | fixed | Confirmed: `source/slang/slang-parser.cpp:7781` and `:7785` call `ParseStatement`, so unbraced branches get no scope. | Qualified the IfStmt bullet: braced branch is a scoped `BlockStmt`, unbraced is not. |
| F-003 | fixed | Confirmed: `source/slang/slang-lookup.cpp:829` holds a single `thisFileDecl`, set at :849-856. | Reworded to say lookup remembers the first `FileDecl` and skips later links to it. |
| F-004 | deferred | Valid, but citing `source/slang/slang-diagnostics.lua` (lines 1173, 1635, 3851) requires adding it to the manifest's watched paths, and manifest edits are out of scope here. | — |
