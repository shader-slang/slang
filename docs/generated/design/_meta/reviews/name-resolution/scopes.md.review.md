---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:40:55Z
target_doc: name-resolution/scopes.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: f91fa5fc23fe8ba3f3138152490cf51df04153ceec1c03815f28996348937979
source_commit: c63c2fbd5115a98b2b2469f43618671653d40bf1
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 1
  minor: 3
  nit: 0
---

# Review report for name-resolution/scopes.md

## Summary

The document largely matches the implementation, including reopened namespaces, import filtering, and deferred body parsing. The main omission is that the required inventory does not name every concrete scope-bearing class. Two localized behavioral descriptions and the diagnostic declaration citations also need correction.

## Items checked

- Reviewed against the **current working tree**, which the supplied review context identifies as matching the document’s source commit throughout `source/`.
- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show name-resolution/scopes.md`.
- Checked every body line-number citation, including the 158 clauses introduced by “line” or “lines,” against the corresponding source locations. No material line-number drift found.
- Spot-checked more than 20 claims: scope fields, ownership, parser push/pop helpers, generic nesting, namespace reuse and wiring, import filtering, exported imports, using injection, block scopes, HLSL loop scoping, switch cases, lambda parameters, if-let scopes, constraint placement, hidden declarations, lookup stopping, spelling suggestions, deferred parsing, and synthesized constraints.
- Resolved all 102 Markdown links to 17 distinct existing files, including existence at the recorded source commit. All eight generated-document targets appear in the manifest. Consulted dependency documents only for cross-reference checks.
- Checked required sections, class coverage, Mermaid syntax, relative links, Liquid safety, and front-matter fields. The document is 34,078 bytes, below its 49,152-byte cap. No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Rules` → `### Scope-bearing AST nodes`, lines 85–105 | The required concrete-class inventory omits `HLSLClassDecl`, `GetterDecl`, `SetterDecl`, `RefAccessorDecl`, and `SynthesizedFuncDecl`. Naming their base classes does not satisfy the prompt’s requirement that every scope-bearing class be named with its declaring header. | `source/slang/slang-ast-decl.h:465`, `:763`, `:769`, `:775`, and `:811` declare these concrete classes. Their inheritance reaches `ContainerDecl`, whose `ownedScope` field is at line 141. The per-document prompt explicitly requires every concrete class carrying that field. | Add these five names to the appropriate aggregate/callable rows, cite their header locations, and distinguish inheriting `ownedScope` from receiving a fresh parser-created scope. |
| F-002 | minor | `### Scope-bearing AST nodes`, lines 111–113 | The statement that an `IfStmt`’s “branch bodies parse as `BlockStmt`s that own one” is too broad and conflicts with the subsequent explanation of unbraced declarations. Ordinary branches can be arbitrary statements and need not introduce scopes. | `source/slang/slang-parser.cpp:7781` and `:7785` call `ParseStatement(ifStatement)` for the two branches. A fresh block scope is created by `parseBlockStatement` at line 7535. | Qualify this sentence: braced branches parse as `BlockStmt`s and own scopes; unbraced branches do not automatically introduce one. Retain the separate if-let exception. |
| F-003 | minor | `### Sibling scopes`, lines 330–333 | “Once it has searched a `FileDecl`, it does not search the same `FileDecl` again” describes general deduplication, but the implementation remembers only the first encountered file declaration. Repetitions of other file declarations are not skipped by this check. | `source/slang/slang-lookup.cpp:829` initializes one `thisFileDecl` pointer. Lines 849–856 assign it only when empty and skip only when `fileDecl == thisFileDecl`. | Say that lookup remembers the first encountered `FileDecl` and skips later occurrences of that same declaration. Avoid implying a visited set for every file. |
| F-004 | minor | `## Edge cases and failure modes`, lines 449–493 | Diagnostic descriptions cite checker call sites but omit the diagnostic declaration source required by the name-resolution family contract. The placement diagnostic also lacks its `Diagnostics::DeclNotAllowed` identifier. | `_common.md`, “Name-resolution family contract,” item 5 requires relevant diagnostic identifiers with citations to `slang-diagnostics.lua`. The declarations are at `source/slang/slang-diagnostics.lua:1173` (`decl-not-allowed`), `:1635` (`expected-a-namespace`), and `:3851` (`constraint-subject-cannot-be-this-type`). | Add `Diagnostics::DeclNotAllowed` and link these diagnostic descriptions to `slang-diagnostics.lua`, retaining implementation citations. Add that file to the manifest’s watched paths and note the expansion as required by the family scope rule. |