---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:17:46+00:00
target_doc: ast-reference/statements.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 070b3ccec0f278478300fb9f59f4c0f312a12c4abadffcc0764197842e97ad2b
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 0
  major: 1
  minor: 4
  nit: 0
---

# Review report for ast-reference/statements.md

## Summary

The page is structurally complete, covers every concrete class in the watched statement header, and has valid links and front matter. Five findings remain. The most important is that it equates `ScopeStmt` inheritance with opening a lexical scope, although the parser creates no statement-owned scope for `WhileStmt`, `DoWhileStmt`, or `SwitchStmt`.

## Items checked

- Verified the document against all three resolved watched files at `53b76e6d3009b8e6434d41573524c7ce5c499d23`, which is also the review-time `HEAD`.
- Checked all 30 concrete classes and their table fields against `slang-ast-stmt.h`, plus every abstract intermediate shown in the hierarchy.
- Re-derived all 24 line-number citation occurrences (22 distinct ranges or lines) in `slang-parser.cpp`; each cited line identifies the stated enum, call site, or function definition.
- Verified more than 30 factual claims, including parser dispatch, deferred body parsing, block sequencing, loop construction, target-switch cases, catch chaining, labels, and capability-name validation.
- Resolved all 41 relative links, including anchors, and checked both dependency documents: `ast-reference/base.md` and `syntax-reference/grammar.md`.
- Checked the mandatory sections, front-matter keys, warning text, 64-character hexadecimal digest shape, and the 49,152-byte size cap.

## Findings

| ID    | Severity | Location                                                                                                         | Description                                                                                                                                                                                                                                                                                                                                                                                         | Evidence                                                                                                                                                                                                                                                                                     | Recommendation                                                                                                                                                                                                                                                                                                                           |
| ----- | -------- | ---------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | major    | `## Family hierarchy`, lines 42-46; `### BlockStmt and SeqStmt`, lines 134-139; `### Loop family`, lines 203-205 | The page presents `ScopeStmt` inheritance as the lexical-scope axis and concludes that every loop is “scope-introducing.” That is not how the watched parser constructs these nodes: only `ForStmt` receives and conditionally pushes its own `ScopeDecl`; `WhileStmt` and `DoWhileStmt` receive none, and `SwitchStmt` relies on its parsed `BlockStmt` for a scope rather than owning one itself. | `source/slang/slang-parser.cpp:6572-6581` creates `SwitchStmt` without assigning `scopeDecl`; `source/slang/slang-parser.cpp:7394-7420` creates and pushes the `ForStmt` scope; `source/slang/slang-parser.cpp:7457-7479` creates `WhileStmt` and `DoWhileStmt` without any scope operation. | Describe `ScopeStmt` as a structural base used by several control-flow groupings, then state precisely which parser routines create a statement-owned `ScopeDecl`: block, scoped `for`, GPU foreach, and compile-time `for`. Explain that a switch's block supplies its lexical scope and that while/do-while do not add a separate one. |
| F-002 | minor    | `### Loop family`, lines 206-209                                                                                 | The page says `ParseForStatement` “rejects” an initializer that is not a `DeclStmt` or `ExpressionStmt`, but the function emits a diagnostic and continues constructing and returning the `ForStmt`.                                                                                                                                                                                                | `source/slang/slang-parser.cpp:7424-7437` diagnoses the unexpected statement without returning or clearing it; the function returns `stmt` at `source/slang/slang-parser.cpp:7454`.                                                                                                          | Replace “rejects” with “diagnoses” and, if useful, note that parser recovery retains the constructed loop node.                                                                                                                                                                                                                          |
| F-003 | minor    | `## Nodes`, `DeferStmt` row, line 123                                                                            | The summary spells the form as `defer S;`, implying that `defer` itself requires a trailing semicolon. The parser consumes `defer` followed by an arbitrary `Stmt`, so a deferred block has no extra semicolon.                                                                                                                                                                                     | `source/slang/slang-parser.cpp:7571-7577` calls `ParseStatement()` immediately after `ReadToken("defer")`; `docs/generated/design/syntax-reference/grammar.md:439` specifies `DeferStmt ::= 'defer' Stmt`.                                                                                   | Change the generic spelling to `defer S`; optionally use `defer f();` and `defer { ... }` as examples if both forms need illustration.                                                                                                                                                                                                   |
| F-004 | minor    | `## Nodes`, `DeferStmt` row, line 123; `### DeferStmt`, lines 264-268                                            | The page describes IR lowering in detail (“scope-exit handlers”), even though the per-document prompt explicitly forbids IR-level statement lowering and asks this page to stop at the AST node and link to the lowering page.                                                                                                                                                                      | `docs/generated/design/_meta/prompts/ast-reference-statements.md:39-42` asks for the AST shape and a pipeline citation; lines 50-55 forbid IR-level lowering.                                                                                                                                | Retain that `DeferStmt::statement` stores the deferred statement and link to `pipeline/04-ast-to-ir.md`; remove the claims about enqueueing and scope-exit-handler materialization.                                                                                                                                                      |
| F-005 | minor    | Intro, lines 12-16                                                                                               | The first body paragraph says what the page covers but not who it is for; the audience appears in a separate paragraph. The universal contract requires both in the first paragraph.                                                                                                                                                                                                                | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first paragraph to state the coverage and intended reader.                                                                                                                                                               | Merge the audience sentence into the opening paragraph.                                                                                                                                                                                                                                                                                  |

## No-issues notes

- Every concrete `FIDDLE()` class in `slang-ast-stmt.h`, including `UniqueStmtIDNode`, appears exactly once in the Nodes table; abstract intermediates remain in the hierarchy.
- The two-stage `UnparsedStmt` account and all associated parser citations match `parseOptBody` and `parseUnparsedStmt`.
- The target-switch explanation correctly captures shared parsing, immediate capability-name validation, default-label encoding, and shared bodies for stacked labels.
