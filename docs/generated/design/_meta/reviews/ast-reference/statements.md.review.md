---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:27:48+00:00
target_doc: ast-reference/statements.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: ef75dc1de48b06f0b01fd7da196735cab6033928aab034bca2cebfd41fe221d7
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 0
  minor: 4
  nit: 0
---

# Review report for ast-reference/statements.md

## Summary
The statements page covers the concrete classes in `slang-ast-stmt.h` and has valid front matter and links. I found four minor source-alignment problems, mostly around exact parser names or surface syntax that differs from the watched parser.

## Items checked
- Ran `regenerate.py show ast-reference/statements.md` and used only the reported prompt, dependencies, and watched files: `source/slang/slang-ast-stmt.h`, `source/slang/slang-ast-base.h`, and `source/slang/slang-parser.cpp`.
- Read the target document front matter/body, `_common.md`, `ast-reference-statements.md`, dependency docs `ast-reference/base.md` and `syntax-reference/grammar.md`.
- Checked front matter for all required keys, the recorded target source commit, the warning string, and a 64-character hex watched-path digest.
- Verified the required AST-reference sections are present and that every concrete `FIDDLE()` class in `slang-ast-stmt.h` appears in the Nodes table, including helper `UniqueStmtIDNode`.
- Spot-checked more than 10 source-backed claims against the watched files, including `ScopeStmt`, `SeqStmt`, `BlockStmt`, `BreakableStmt`, `ChildStmt`, `TargetCaseStmt`, `ForStmt`, `UnscopedForStmt`, `CompileTimeForStmt`, `ThrowStmt`, `CatchStmt`, and `RequireCapabilityStmt`.
- Resolved the document's workspace-relative source and generated-doc links that were in scope for this page; no broken target paths were found.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Source`, lines 20-23 | The source section names the parser entry point as `parseStatement`, but the watched parser declares and defines it as `ParseStatement` on `Parser`. | `source/slang/slang-parser.cpp:209` declares `Stmt* ParseStatement(Stmt* parentStmt = nullptr);`; `source/slang/slang-parser.cpp:6902` defines `Stmt* Parser::ParseStatement(Stmt* parentStmt)`. | Change `parseStatement` to `Parser::ParseStatement` or ``ParseStatement``. |
| F-002 | minor | `## Nodes`, line 112 | The `ThrowStmt` row summarizes the syntax as `throw e;`, but the watched parser's `ParseThrowStatement` reads `throw` and an expression without consuming a semicolon. | `source/slang/slang-parser.cpp:7568` through `source/slang/slang-parser.cpp:7575` show `ParseThrowStatement` calling `ReadToken("throw")` and `ParseExpression()` and then returning; unlike `ParseExpressionStatement` at `source/slang/slang-parser.cpp:7577`, it has no `ReadToken(TokenType::Semicolon)`. | Remove the semicolon from the `ThrowStmt` summary or explicitly note that the current parser routine does not consume one. |
| F-003 | minor | `## Nodes`, line 115 and `### RequireCapabilityStmt`, lines 220-225 | The page labels the statement as `require_capability`, but the watched parser recognizes the camel-case internal keyword `__requireCapability`. | `source/slang/slang-parser.cpp:6969` dispatches on `LookAheadToken("__requireCapability")`; `source/slang/slang-parser.cpp:7588` documents the production as `__requireCapability '(' identifier (',' identifier)* ')' ';'`; `source/slang/slang-parser.cpp:7593` reads `__requireCapability`. | Use `__requireCapability` in the row summary and notable-node prose, or mark the Grammar cell `(none)` if the grammar page does not expose this internal statement production. |
| F-004 | minor | `### CompileTimeForStmt`, lines 164-171 | The notable-node prose says `CompileTimeForStmt` backs "`[ForceInline]`-style range loops over generic value parameters," but the watched parser shows a `$for (name in Range(...))` syntax and no connection to `ForceInline` in this parse path. | `source/slang/slang-parser.cpp:6888` reads a leading `$`, then `parseCompileTimeForStmt` reads `for`, the loop variable, `in`, and `Range(...)` at `source/slang/slang-parser.cpp:6842` through `source/slang/slang-parser.cpp:6873`. | Replace the `ForceInline` comparison with the actual `$for (... in Range(...))` parser shape, and leave lowering details to the linked pipeline page. |

## No-issues notes
- The `## Nodes` table includes all concrete statement classes declared in `slang-ast-stmt.h`, while keeping abstract intermediates in the hierarchy diagram rather than the table.
- The distinction between `BlockStmt` and `SeqStmt` is supported by `parseBlockStatement`, which creates a `BlockStmt` with `scopeDecl` and wraps multiple body statements in a `SeqStmt`.
- The target-switch rows match the shared parser implementation where `parseTargetSwitchStmt` and `parseStageSwitchStmt` both call `parseTargetSwitchStmtImpl`.
