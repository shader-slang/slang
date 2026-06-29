---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:16:10Z
target_doc: name-resolution/scopes.md
review_report: ../../reviews/name-resolution/scopes.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 5, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for name-resolution/scopes.md

## Summary

All five review findings were verified against the source at commit `eb9403ef` and fixed. F-001 expanded the scope-bearing table to name the previously-implied concrete `ContainerDecl` subclasses (`SemanticDecl`, `AttributeDecl`, `SynthesizedStructDecl`, `GLSLInterfaceBlockDecl`, `AssocTypeDecl`, `GlobalGenericParamDecl`, `FuncAliasDecl`). F-002 and F-003 corrected the intro/rows that wrongly claimed every listed statement gets a parser-populated fresh scope and re-attributed the lambda parameter scope to `LambdaExpr`. F-004 added `using` as a sibling-scope use plus the `ExpectedANamespace` diagnostic, and F-005 refreshed the stale line-number citations.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed in `slang-ast-decl.h`: `SemanticDecl` (732), `AttributeDecl` (1102), `SynthesizedStructDecl` (725), `GLSLInterfaceBlockDecl` (567), `AssocTypeDecl` (420), `GlobalGenericParamDecl` (434), `FuncAliasDecl` (653) are concrete `ContainerDecl`/`AggTypeDecl`/`CallableDecl` subclasses; the prompt's quality checklist requires every scope-bearing class be named with its header. | Replaced ellipsis-laden grouped rows with explicit subclass lists and added a `SemanticDecl`/`AttributeDecl` row, each with header line cites. |
| F-002 | fixed | Verified `ParseWhileStatement`/`ParseDoWhileStatement` (slang-parser.cpp ~7069) never create/assign a `ScopeDecl`, `ParseSwitchStmt` (6192) gives the body a `parseBlockStatement`, and `parseTargetSwitchStmtImpl` (6217) makes per-case `ScopeDecl`s; the table's "introduces a fresh `Scope`" claim was wrong for these. | Rewrote the intro to distinguish field-declaration from parser population and revised the while/do and switch/target/stage-switch rows to describe actual parser behavior. |
| F-003 | fixed | Confirmed `paramScopeDecl` is created/pushed on `lambdaExpr` in `parseLambdaExpr` (slang-parser.cpp 8159-8160) and `LambdaDecl` (slang-ast-decl.h 682) is a `StructDecl` declaring only `funcDecl`; the row conflated the two owners. | Renamed the row to `LambdaExpr` (parameter scope), cited `parseLambdaExpr`, and noted `LambdaDecl` owns a scope only as an aggregate. |
| F-004 | fixed | Verified `SemanticsDeclScopeWiringVisitor::visitUsingDecl` (slang-check-decl.cpp 16419) adds sibling scopes via `addSiblingScopeForContainerDecl` (16449) and emits `Diagnostics::ExpectedANamespace` (16478) for non-namespace args; the prompt's edge-case list requires the `using` scope behavior. | Added `using` as the fourth sibling-scope use and expanded the `UsingDecl` edge case with the check-time injection and the `ExpectedANamespace` diagnostic. |
| F-005 | fixed | Verified `parseIfLetStatement` at 6897 (cited 6820), `ParseForStatement` at 7005 (cited 6928, skip at 7031-7032), `parseUsingDecl` at 4269 (cited 4198), import sibling wiring at 16163 (cited 16061), namespace sibling at 16510 (cited 16408), FileDecl import at 16129 (cited 16027). | Refreshed all the named stale line-number citations and attached function names so the references survive minor drift. |
