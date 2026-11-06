---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: ast-reference/statements.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 8
actions:
  fixed: 8
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for ast-reference/statements.md

## Summary

Nothing was escalated. Seven gaps were REOPENED, meaning gap-status recorded them as fixed yet the bundle still reports them. The cause is a stale bundle, not a failed fix: the bundle README carries `source_doc_digest` c541a2a8..., which predates the previous intake's edits (the `#deferstmt` gap even quotes the pre-fix sentence). Each gap was re-decided against current source: all eight are `fixed`, including the previously deferred `9f664f7cf462`, now answerable because `source/slang/slang-options.cpp` is watched. Re-checking found that the earlier fix for `9f664f7cf462`-adjacent text was wrong at this commit (see that row), and other cited lines had drifted.

Examples are written inline because `_common.md` (AST reference contract, "Notable nodes") says "Do not include code blocks". Compiler binary is a Linux build and was not run; every example is confirmed by reading source only.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 7050023ee030 | fixed | `source/slang/slang-parser.cpp:6928-6937` parses the switch body with `parseBlockStatement(AllowCaseDefaultStatements::Allow)`; `ParseCaseStmt` / `ParseDefaultStmt` (`:6940`, `:6950`) read only the label, so declarations under labels are siblings in one `BlockStmt` scope; the checker reveals declarations in order (`source/slang/slang-check-stmt.cpp`, `checkDeclAtDeclarationPoint`). | kept the single-scope consequence and added the worked inline example `switch (v) { case 1: int t = 10; break; case 2: t = 20; break; }`; refreshed line numbers; noted a missing `{` yields an `EmptyStmt` body |
| acf54503f940 | fixed | `source/slang/slang-parser.cpp:7210-7265`: `ReadToken("Range")` at `:7240` (failure is `Unexpected(parser, expected)`, `:386`, `unexpected-token-expected-token-name`, E20004, `source/slang/slang-diagnostics.lua:986`); `:7241-7248` shows the first argument is the end and a comma demotes it to the begin. New at this commit: `:7218-7226` report `CompileTimeForIsRemoved` (E30526) from 202c and `CompileTimeForIsDeprecated` (E30525) for 2025/2026 (`source/slang/slang-diagnostics.lua:3659-3670`). | both spellings (`Range(4)`, `Range(2, 5)`) and E20004 were already inline; added the new deprecation warning / removal error and corrected line numbers |
| df583f6f8952 | fixed | `source/slang/slang-parser.cpp:7455-7461`: a `;` whose `parentStmt` is an `IfStmt` raises `UnintendedEmptyStatement` then still builds `EmptyStmt`; declared `warning(...)`, id 20101, "potentially unintended empty statement at this location; use {} instead." (`source/slang/slang-diagnostics.lua:1139-1144`); `parseIfStatement` passes the `IfStmt` for both branches (`:7773-7787`). | code, message and warning-not-error were present; added the `parentStmt` condition (both branches), line 7461, and `if (c);` versus `if (c) {}` |
| 20449a33ee45 | fixed | Observable rule is limited to bundle-observed behaviour (scope exit, early `return`); lowering (`source/slang/slang-lower-to-ir.cpp:8495`) is unwatched. Watched-path evidence for restrictions: `source/slang/slang-check-stmt.cpp:258,280,650,695` diagnose `break`/`continue`/`return`/escaping `throw` inside a defer body (`BreakInsideDefer` E30108, `ContinueInsideDefer` E30109, `ReturnInsideDefer` E30110, `source/slang/slang-diagnostics.lua:2455-2473`). | kept the scope-exit / early-return sentence, added a `{ defer log(1); log(2); }` ordering example and the checker restrictions, and stated that `break`/`throw` exit behaviour is not described here |
| 81ec1b4a473c | fixed | `source/slang/slang-parser.cpp:7826-7840`: initial statement from `ParseStatement`; anything other than `DeclStmt`/`ExpressionStmt` draws `UnexpectedTokenExpectedTokenType` with `"statement"` / `"expression"` at the statement's loc; E20001 "unexpected ~actualToken, expected ~expectedToken" (`source/slang/slang-diagnostics.lua:978-983`). | rejected spelling `for ({ int i = 0; } n < 3; n = n + 1)` and E20001 message already inline; corrected function line (7792) |
| 9f664f7cf462 | fixed | `source/slang/slang-options.cpp:1927-1962` (`OptionsParser::addInputPath`): an explicitly requested language (`-lang`, `:506`) wins; otherwise `findSourceLanguageFromPath` (`:1796`) maps `.hlsl`/`.fx` to HLSL and `.slang` to Slang; `source/slang/slang-parser.cpp:7801-7810` creates `UnscopedForStmt` only for `SourceLanguage::HLSL`. The earlier text said `-lang hlsl` cannot select HLSL for a `.slang` file; at this commit it can, so that claim was removed. | rewrote the HLSL-selection paragraph: `-lang` overrides, else extension; example of the loop variable used after the loop under `.hlsl` but not `.slang` |
| 98d151850f5e | fixed | `source/slang/slang-parser.cpp:8002-8032`: comment `'__requireCapability' '(' identifier (',' identifier)* ')' ';'`; loop continues on `AdvanceIf(Comma)`, unresolved name is diagnosed `UnknownCapability` (`:8020-8022`) and not stored; E36105 "unknown capability name '~capability'." (`source/slang/slang-diagnostics.lua:2692-2697`). | `__requireCapability(hlsl);` and E36105 were present; added the comma-list form for several atoms and corrected line number |
| 8d82d193316d | fixed | `source/slang/slang-parser.cpp:7034-7040` raises `UnknownTargetName` (E29110, `source/slang/slang-diagnostics.lua:1278-1283`) only for non-empty names, then still records `CapabilityName::Invalid`; `source/slang/slang-check-stmt.cpp:516-523` raises `InvalidTargetSwitchCase` (E36109, `:2720-2725`) only in the `__target_switch` branch; the `__stage_switch` branch (`:497-506`) reports only `UnknownStageName` for non-stage atoms, so no E36109 follow-on there. | qualified the follow-on E36109 as `__target_switch` only, cited the checker line, and noted the per-case `ScopeDecl` |
