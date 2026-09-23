---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:20:00Z
target_doc: name-resolution/scopes.md
review_report: ../../reviews/name-resolution/scopes.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/scopes.md

## Summary
All four findings were re-derived against the source at the recorded
commit and all four were fixed. The major finding was correct: the
representative parser chain uses `PushScope` for declaration scopes and
`pushScopeAndSetParent` only for statement-created `ScopeDecl`s. The
three minor findings corrected an omitted concrete scope-carrying class,
an unsupported `UsingDecl` claim, and a non sequitur in the empty-block
edge case. Breakdown: 4 fixed, 0 rejected-bogus, 0 rejected-out-of-scope,
0 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: `source/slang/slang-parser.cpp:1794` (`parseOptGenericDecl`), `:5099`/`:5112` (`parseFuncDecl`), and `:6286` (`parseDeclBody`) call `PushScope`; only `:7142` (`parseBlockStatement`) calls `pushScopeAndSetParent`, which per `:160-164` merely sets `parentDecl` before delegating to `PushScope`. | `### Parser scope construction`: final sentence replaced with the declaration-parser (`PushScope`) versus statement-`ScopeDecl` (`pushScopeAndSetParent`) distinction, retaining the paired `PopScope`. |
| F-002 | fixed | Confirmed: `source/slang/slang-ast-decl.h:478` declares `ThisTypeDecl : AggTypeDecl`, so it inherits `ContainerDecl::ownedScope` (`:141`); the per-doc prompt lines 46-49 require every such concrete class to be named. Citations kept inside the watched set. | `### Scope-bearing AST nodes`: `ThisTypeDecl` added to the aggregate row with line 478, identified as the synthesized interface `This` member reached via `getThisTypeDecl`, with a note that no parser path pushes its `ownedScope`. |
| F-003 | fixed | Confirmed: `source/slang/slang-parser.cpp:4552` stores `parser->currentScope` in `decl->scope`, and `source/slang/slang-check-decl.cpp:17348-17376` passes that same pointer to `addAllSiblingScopesFromDecl` as the destination; no substitution occurs. | `UsingDecl` edge case: "may differ" sentence replaced with the accurate statement that checking augments the captured scope's `nextSibling` chain. |
| F-004 | fixed | Confirmed: `source/slang/slang-parser.cpp:7217` builds an `EmptyStmt` for a bodyless block, and `source/slang/slang-check-stmt.cpp:106-116` sets `hiddenFromLookup` only for `DeclStmt`s inside a `SeqStmt`, so the flag cannot explain the empty-block case. | `Empty block scope`: split into the unconditional parser scope push and a separate `hiddenFromLookup` explanation scoped to blocks containing declaration statements. |
