---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:58:23Z
target_doc: ast-reference/statements.md
review_report: ../../reviews/ast-reference/statements.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/statements.md

## Summary

All four minor source-alignment findings were verified against the watched parser `source/slang/slang-parser.cpp` at HEAD and fixed in the target document. The fixes correct one parser-method name, one surface-syntax overstatement, one internal-keyword spelling plus its grammar cell, and one unsupported lowering analogy. None were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `slang-parser.cpp:209` declares and `:6902` defines `Parser::ParseStatement` (capital P); doc had `parseStatement`. | `## Source`: `parseStatement` -> `Parser::ParseStatement`. |
| F-002 | fixed | `ParseThrowStatement` (`slang-parser.cpp:7568-7575`) reads `throw` + expression and returns without a `ReadToken(Semicolon)`, unlike `ParseExpressionStatement` (`:7577`). | Nodes ThrowStmt summary: `throw e;` -> `throw e` with a note that the routine consumes no trailing `;`. |
| F-003 | fixed | Statement keyword is `__requireCapability` (`slang-parser.cpp:6969`, `:7593`); `require_capability` was wrong and the grammar page exposes no statement production for it. | Nodes row: keyword corrected to `__requireCapability(...)`, Grammar -> `(none)`; notable prose names `__requireCapability` / `Parser::ParseRequireCapabilityStatement`. |
| F-004 | fixed | `parseCompileTimeForStmt` (`slang-parser.cpp:6842-6885`, leading `$` at `:6888`) parses `$for (name in Range(...))`; no `[ForceInline]` connection exists in this parse path. | `### CompileTimeForStmt`: replaced the `[ForceInline]`-style claim with the actual `$for (... in Range(...))` parser shape (`parseCompileTimeForStmt`). |
