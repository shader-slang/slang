---
gap_intake_report: true
intake_model: "claude-opus-5[1m]"
intake_at: 2026-08-11T16:42:17Z
target_doc: ast-reference/statements.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 8
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for ast-reference/statements.md

## Summary

Nothing was escalated: every observation in the queue agreed with the
watched source, so all of them were documentation gaps rather than
compiler defects. Seven gaps were fixed by editing six sections of the
target document — five of them turn an internal `Diagnostics::*`
identifier into the user-visible code and message, one restates the
observable `defer` rule, and one records the single-scope consequence of
a `switch` body. One gap was deferred: how a user selects HLSL input is
decided by the driver's file-extension table
(`source/slang/slang-options.cpp:1714`), which is outside this page's
`watched_paths`, and no test in the bundle exercises HLSL input.

The four `missing-example` / `missing-surface` gaps that asked for code
blocks were satisfied with inline spellings instead: the AST-reference
family contract states "Do not include code blocks" for
`## Notable nodes`
(`docs/generated/design/_meta/prompts/_common.md:118`), so each example
is written as an inline `for (...)` / `$for (...)` /
`__requireCapability(...)` form in prose.

Operator follow-ups: (1) diagnostic ids, severities and message text all
live in `source/slang/slang-diagnostics.lua`, which is not in this
page's `watched_paths`, so the five codes now recorded (`E20001`,
`E20004`, `E20101`, `E29110`, `E36105`, `E36109`) are untracked against
that file — adding it would close the loop, and the same argument
applies to `docs/generated/design/pipeline/02-parse-ast.md`, which
already cites codes under the same condition; (2) adding
`source/slang/slang-options.cpp` would unblock the deferred gap;
(3) `docs/generated/design/pipeline/04-ast-to-ir.md`, which this page
points at for `defer` semantics, does not mention `DeferStmt` at all, so
the pointer is currently dangling — worth a gap on that page.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 7050023ee030 | fixed | `source/slang/slang-parser.cpp:6588` parses the whole `switch` body with `parseBlockStatement(AllowCaseDefaultStatements::Allow)`, and `ParseCaseStmt` / `ParseDefaultStmt` (`:6592`, `:6602`) read only `case <expr> :` / `default :`, so a declaration under a label is a sibling inside that one `BlockStmt` scope; bundle test `switchstmt-body-block-scope.slang` pins the observable form (the `default:` read resolves to the `case 1:` declaration, not to the outer variable). | added the user-visible consequence — one scope for the whole body, later labels see earlier declarations, and a case-local declaration shadows a same-named outer one — to the `SwitchStmt` paragraph |
| acf54503f940 | fixed | `source/slang/slang-parser.cpp:6881` reads `Range` with `ReadToken("Range")`, whose failure path is `Unexpected(parser, expected)` at `:349` → `Diagnostics::UnexpectedTokenExpectedTokenName`, error 20004 (`source/slang/slang-diagnostics.lua:853-858`); `:6884-6890` shows the first argument is the *end* and a comma demotes it to `rangeBeginExpr`. Bundle tests `compiletimefor-unrolls.slang` (`Range(4)` sums 0+1+2+3), `compiletimefor-two-argument-range.slang` (`Range(2, 5)` yields 2,3,4) and `compiletimefor-range-keyword-required.slang` ("unexpected identifier, expected 'Range'") pin all three. | gave both spellings inline with their iterated values and the half-open rule, and named `E20004` for any other identifier in that position |
| df583f6f8952 | fixed | `source/slang/slang-parser.cpp:7096-7102` raises `Diagnostics::UnintendedEmptyStatement` only when the parent is an `IfStmt`, then still builds the `EmptyStmt`; the entry is declared with `warning(...)`, id 20101, message "potentially unintended empty statement at this location; use {} instead." (`source/slang/slang-diagnostics.lua:993-998`), and bundle test `emptystmt-after-if-diagnosed.slang` pins that printed text. | added the code, message and the fact that it is a warning that leaves the program compiling |
| 20449a33ee45 | fixed | Lowering is outside `watched_paths`, so the claim is limited to what the bundle observed: `deferstmt-scope-exit-order.slang` (body → deferred → after-block), `deferstmt-at-function-entry.slang` and `deferstmt-runs-on-early-return.slang` (deferred print lands between the callee's last print and the caller's next one). `break` and `throw` are in the suggested wording but are pinned by no test, so they were deliberately not written. | added one observable sentence — the deferred statement runs at exit of the enclosing scope, including an early `return` — ahead of the existing pointer to 04-ast-to-ir.md |
| 81ec1b4a473c | fixed | `source/slang/slang-parser.cpp:7444-7455`: the initial statement is parsed by `ParseStatement`, and anything that is not a `DeclStmt` or `ExpressionStmt` draws `Diagnostics::UnexpectedTokenExpectedTokenType` with `actualToken = "statement"`, `expectedToken = "expression"` — error 20001, "unexpected ~actualToken, expected ~expectedToken" (`source/slang/slang-diagnostics.lua:846-851`). Bundle test `forstmt-init-non-expression-rejected.slang` uses exactly `for ({ int i = 0; } n < 3; n = n + 1)` and pins "unexpected statement, expected expression". | added the rejected inline spelling and the `E20001` message reported on the offending statement |
| 9f664f7cf462 | deferred | Nothing in `watched_paths` decides the input source language: `source/slang/slang-parser.cpp:7419` only reads `getSourceLanguage()`, which the `Parser` receives from its caller (`source/slang/slang-parser.h:14`, itself unwatched). The extension table that answers the gap is `source/slang/slang-options.cpp:1714` (`{".hlsl", SLANG_SOURCE_LANGUAGE_HLSL, SLANG_STAGE_NONE}`), outside `watched_paths`, and no bundle test compiles an HLSL input, so the leaking-loop-variable form has no verified printed evidence either. Needs a `watched_paths` expansion to `source/slang/slang-options.cpp`. | — |
| 98d151850f5e | fixed | `source/slang/slang-parser.cpp:7628-7647`: the names are read in a `while (true)` loop that continues on `AdvanceIf(this, TokenType::Comma)` (`:7642`), so several atoms are accepted, and the statement is closed by `ReadToken(TokenType::RParent)` / `ReadToken(TokenType::Semicolon)` (`:7646-7647`); an unresolved name is dropped and diagnosed at `:7639-7640` — `unknown-capability`, error 36105, "unknown capability name '~capability'." (`source/slang/slang-diagnostics.lua:2436-2440`). Bundle test `requirecapability-in-function.slang` compiles `__requireCapability(hlsl);` in a function body; `requirecapability-unknown-name-rejected.slang` pins the message. | gave the accepted inline spelling, stated that the list is comma-separated and `;`-terminated, and named `E36105` with its message |
| 8d82d193316d | fixed | `source/slang/slang-parser.cpp:6687-6693` diagnoses `Diagnostics::UnknownTargetName` — error 29110, "unknown target name '~name'" (`source/slang/slang-diagnostics.lua:1117-1121`) — and then still stores `capability = (int32_t)CapabilityName::Invalid`; that stored `Invalid` is what makes the checker add `Diagnostics::InvalidTargetSwitchCase` (`source/slang/slang-check-stmt.cpp:484`, outside `watched_paths`) — error 36109, "'~capability' cannot be used as a target_switch case." (`source/slang/slang-diagnostics.lua:2464-2468`). Bundle test `targetcase-unknown-name-rejected.slang` pins both messages on the same label. | named `E29110` with its message and added the follow-on `E36109` error, explaining that it follows from the case still being recorded as `CapabilityName::Invalid` |
