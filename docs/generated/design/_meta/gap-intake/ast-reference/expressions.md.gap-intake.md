---
gap_intake_report: true
intake_model: "claude-opus-5[1m]"
intake_at: 2026-08-11T16:39:23Z
target_doc: ast-reference/expressions.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 8
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 2
---

# Gap-intake report for ast-reference/expressions.md

## Summary

Two gaps were escalated: both `drift-from-source` rows turned out to be
compiler defects, because the watched source agrees with the document in
each case and an existing finding already records the misbehaviour, so
neither produced a document edit. The remaining six were fixed by
editing five sections — three rows of `## Nodes` plus a new note under
the table, and the `LiteralExpr family`, `CastOptionalExpr`,
`LambdaExpr` and type-expression callouts. Nothing was rejected or
deferred.

The most substantive finding of this cycle was not in a gap's suggested
addition but next to it: `TupleTypeExpr` has no surface syntax at all —
`parseTupleTypeExpr` sits inside `#if 0` and its type-specifier call
site is commented out — while `FuncTypeExpr` is reachable only through
the `functype` keyword, which the table previously omitted from the
spelling. That is why every position the reporting agent tried was
rejected, and it is now stated in the document rather than left as a
missing example.

Operator follow-ups: two claims rest on source outside this document's
`watched_paths`. The `Optional<T>` to `Optional<U>` trigger rule lives
only in `source/slang/slang-check-conversion.cpp:2090-2170`, and the
scalar default for an omitted `sizeof` data-layout argument only in
`source/slang/slang-check-expr.cpp:6650-6653`; both are corroborated by
checked-in tests (see the Actions table), but adding
`source/slang/slang-check-expr.cpp` and
`source/slang/slang-check-conversion.cpp` to `watched_paths` would put
them under drift tracking. The diagnostic severities cited in the
`LiteralExpr family` edit come from `source/slang/slang-diagnostics.lua`,
also unwatched, though the parser call sites that select them are
watched.

## Escalated gaps

- `73ce79da8acb` — `countof` on an array. The source intends the
  element-count reading the document states:
  `_isTypeOrValValidForCountOf` (`source/slang/slang-check-expr.cpp:6329-6357`)
  admits `ArrayExpressionType` alongside packs and tuples, so an array
  operand is deliberately accepted rather than diagnosed. What is
  missing is the fold: `CountOfIntVal::tryFoldOrNull`
  (`source/slang/slang-ast-val.cpp:2676-2703`) handles `ConcreteTypePack`,
  `TupleType`, and `ConcreteIntValPack` and has no array case, so the
  array operand falls through to an unfolded `CountOfIntVal` and the
  compiler emits the element type's byte size (4 for `int a[9]`, 8 for
  `double b[6]`) instead of the length. Document unchanged. Existing
  finding: `docs/generated/tests/_meta/findings/countof-on-array-returns-element-size.yaml`.
- `f1ba993d9756` — `new T(args)`. The parser explicitly supports the
  argument-list form: `parsePrefixExpr`
  (`source/slang/slang-parser.cpp:9705-9725`) parses the operand of
  `new` as a postfix expression and, when that is an `InvokeExpr`,
  copies its `arguments` and `argumentDelimeterLocs` onto the `NewExpr`.
  A form the parser builds a node for is not an unsupported spelling, so
  aborting with `E99997` "could not resolve target declaration for call"
  is a compiler defect and the `new T(...)` row stays as written.
  Existing finding:
  `docs/generated/tests/_meta/findings/new-expr-with-constructor-args-internal-error.yaml`.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| f04541338585 | fixed | `source/slang/slang-check-conversion.cpp:2090-2098` enters the branch for any `Optional<T>` to `Optional<U>` with `T != U`, `:2115-2125` probes the inner `T`-to-`U` conversion with the ordinary `_coerce`, and `:2160-2167` builds the node only when that probe succeeded — so the qualifying set is exactly the checker's ordinary implicit conversions. Regression tests `tests/language-feature/optional/optional-cast-numeric.slang` (`Optional<int>` to `Optional<float>`) and `optional-cast-interface.slang` (`Optional<Square>` to `Optional<IShape>`) pin both kinds; the gap's claim that both spellings are rejected is contradicted by them and by the feature commit `b264071b8e`, which predates the observation's `source_commit`. The confirming file is outside `watched_paths` — see the follow-up note in `## Summary`. | added the triggering shape and both qualifying conversion kinds (numeric, interface up-cast) to the `CastOptionalExpr` callout |
| 25c0599b8438 | fixed | `source/slang/slang-parser.cpp:8456-8466` takes the block body when the next token is `{` and otherwise wraps `ParseArgExpr` in a synthesized `ReturnStmt`; `:8444-8453` reads the parameter list with `ReadToken(LParent)` plus `ParseParameter`; `:8820-8829` only calls `parseLambdaExpr` when a balanced parenthesized group is followed by `=>`, which is why a bare identifier never reaches it. Bundle test `lambda-expr-call.slang` compiles `(int x, int y) => x + y`. | put the two body spellings side by side and stated the parse rule that makes `x => x + 1` a parse error; the diagnostic id is deliberately not named, as neither the watched parser nor any bundle test pins it |
| 6d040fda2fc4 | fixed | `source/slang/slang-parser.cpp:8767-8786` chooses `FloatLiteralTooSmall` for a finite out-of-range value and `FloatLiteralUnrepresentable` otherwise, `:8788-8795` `FloatHexLiteralPrecisionLost`, and `:8797` then assigns the already-converted value to `constExpr->value`; `source/compiler-core/slang-lexer.cpp:1322-1362` shows `fast_float` returning `inf` with `result_out_of_range` for `1e50f`. All three are declared `warning(...)` — 40009, 40010, 40019 — in `source/slang/slang-diagnostics.lua:4172-4191`, and bundle test `literal-float-out-of-range-diagnosed.slang` pins the 40009 message and caret. | named the three diagnostics and their codes, stated that they are warnings so the compile continues, and corrected the tail of the sentence: the converted value (`inf`) does land in `value` |
| 73ce79da8acb | escalated-to-finding | Source agrees with the document — see `## Escalated gaps`. Finding `docs/generated/tests/_meta/findings/countof-on-array-returns-element-size.yaml`. | — |
| f1ba993d9756 | escalated-to-finding | Source agrees with the document — see `## Escalated gaps`. Finding `docs/generated/tests/_meta/findings/new-expr-with-constructor-args-internal-error.yaml`. | — |
| 1e07ed9da30d | fixed | `source/slang/slang-parser.cpp:8865-8880` builds the empty `TupleExpr` only when `currentModule->languageVersion >= SLANG_LANGUAGE_VERSION_2026` and otherwise diagnoses `InvalidEmptyParenthesisExpr`; `:8884-8896` keeps `Precedence::Comma` (the comma operator) below 2026 and drops to `Precedence::Assignment` at 2026 so `,` becomes an element separator, with `:8911-8931` assembling the multi-element node. Bundle tests `tuple-expr-pair-construction.slang` and `tuple-expr-empty.slang` pass `-std 2026` on every target line. | replaced "(Slang 2026 and later)" in the `TupleExpr` row with the required `-std 2026` and the comma-operator reading below it |
| d75ffe6d4085 | fixed | `source/slang/hlsl.meta.slang:23-71` declares `IBufferDataLayout` and its six implementations (`DefaultDataLayout`, `DefaultPushConstantDataLayout`, `Std140DataLayout`, `Std430DataLayout`, `ScalarDataLayout`, `CDataLayout`); `source/slang/slang-parser.cpp:8042-8046` and `:8064-8068` accept the comma-separated second argument for `sizeof` and `alignof` while `:8085` shows `countof` parsing a single operand. The omitted-argument default is `getScalarLayoutType()` at `source/slang/slang-check-expr.cpp:6650-6653` (outside `watched_paths`), corroborated by bundle test `sizeof-expr-data-layout-argument.slang`, whose CHECKs give `scalar=24` for the bare form against `std140=32` / `std430=28`. | added a note under the `## Nodes` table listing the accepted layout type names, the scalar default, and the fact that `countof` takes no such argument; pointed the `SizeOfExpr` row at it |
| d546ac334a6f | fixed | `source/slang/slang-parser.cpp:3470-3474` reaches `parseFuncTypeExpr` only after `AdvanceIf(parser, "functype")`, and `hlsl.meta.slang:28642` uses that spelling in a real declaration (`functype(T, T) -> T combineOp`); `slang-parser.cpp:3238-3253` wraps `parseTupleTypeExpr` in `#if 0` and `:3464-3469` leaves its call site commented out, so `(T1, T2)` has no type position. The other four nodes are reachable: `:3127-3132` (`T*`), `:7713-7721` (`T & U`), `:3272-3284` (modifier prefixes), `:8157-8168` (`__packBranch`). `docs/generated/design/syntax-reference/grammar.md:773` already spells the `functype` production and has no tuple-type production. | added a paragraph naming the accepted spelling of each node and stating that `TupleTypeExpr` has none; corrected the `FuncTypeExpr` row to include the `functype` keyword and set the `TupleTypeExpr` row's Grammar cell to `(none)` |
