---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:16:29Z
target_doc: pipeline/02-parse-ast.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 12
actions:
  fixed: 12
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for pipeline/02-parse-ast.md

## Summary

Twelve gaps from two bundles, all fixed; nothing was rejected, deferred,
or escalated, and no gap turned out to be a compiler defect. Every one
of them was a "missing" or "too narrow" complaint rather than a
drift claim, and every hypothesis in the `Suggested addition` column
turned out to be confirmable in `source/slang/slang-parser.cpp` or the
AST headers, so the observations and the source agreed throughout. The
eight gaps from `design/pipeline/02-parse-ast` produced edits in six
sections (`### Two-stage parsing`, `### Error recovery`,
`### The major node families`, `### ASTBuilder`, `## Generics
ambiguity`, `## Modifier parsing`, `## Failure modes`); the four from
`coverage/parser` produced one new `### Angle-bracket annotations`
subsection plus additions to `## Modifier parsing` and
`## Failure modes`. The page grew from 21,328 to 28,630 bytes against a
32,768-byte cap.

Three fixes say more than the gap asked, because the source does.
`84e410a04bb8` asked for the recover-before / recover-after sets in
other contexts; the source has none — every non-block site uses
`TryRecoverBefore` with a single token and no recover-after set — so
the fix states that rule plus the *bail* sets that terminate a matched
region, which is what actually predicts recovery outside a block.
`adb325a74b5a` asked for a literal that trips two float-literal
reports; the parser's shared `diagnosed` flag gives the full precedence
order, so the fix documents the order and then names the one shape that
can overlap. `70faf67147ee` asked for the `z` suffix; the fix
documents the whole width/unsigned suffix scan and the three-way
decimal split, since one is meaningless without the other.

Two operator notes. First, `source/slang/slang-parser.cpp` has drifted
roughly `+8` lines against this page's recorded `source_commit`
(`53b76e6d`) — `TryRecover` is at 483, not 475; `UnwrapDeclarator` at
2752, not 2744; `parseFloatingPointLiteralExpr` at 8715, not 8696. I
re-derived and corrected the citations inside the sentences I rewrote,
but deliberately left the ~10 citations in untouched sections alone
rather than trigger a whole-file digest invalidation; they will need a
regeneration pass. Second, two fixes lean on source outside this page's
`watched_paths`: `7f923f6cd411` (E31002 is emitted by
`source/slang/slang-check-modifier.cpp`) and `adb325a74b5a` (the
lexer-side flag co-occurrence is in
`source/compiler-core/slang-lexer.cpp`, which the page already cites
through `slang-lexer.h`). Both are corroborated by verified test CHECK
lines, but adding `source/compiler-core/slang-lexer.cpp` to
`watched_paths` would make the literal-decoding material
self-supporting.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 7042738f5836 | fixed | `source/slang/core.meta.slang:2594` generates `typedef vector<float,3> float3;` for every scalar type and width, so both spellings name one interned `DeclRefType`; interning table is `m_cachedNodes` at `source/slang/slang-ast-builder.h:223`. Redeclaration outcome is the verified CHECK of the bundle test `astbuilder-hash-consed-type-identity.slang` (E30201). | added a paragraph naming the user-visible consequence: `float3` and `vector<float, 3>` are one signature, so the second `probe` is a redeclaration, not an overload |
| 84e410a04bb8 | fixed | `source/slang/slang-parser.cpp:628-633` is the only `TryRecover` call with both sets; every other site calls `TryRecoverBefore` (`:621`) with one token and `nullptr, 0` — `Parser::readTokenImpl` (`:635`, `:659`) and `AdvanceIfMatch` (`:805`, `:829`). Bail sets are `kMatchedTokenInfos` (`:783`) with `kMatchedToken_BailAtCurlyBraceOrEOF` / `kMatchedToken_BailAtEOF` (`:779-782`), consumed at `:851-870`. | stated that the block pair is the only two-set strategy, described the two `TryRecoverBefore` sites and the per-region bail sets, and said explicitly that parameter/initializer/declaration positions have no sets of their own |
| adb325a74b5a | fixed | `source/slang/slang-parser.cpp:8734-8794`: one `diagnosed` flag, set by the `BadSignificand` / `BadSuffix` arms of a single `switch`, guards `if (isOutOfRange && !diagnosed)` (`:8766`) and `if (precisionLost && !diagnosed)` (`:8786`). The overlap case is real: `source/compiler-core/slang-lexer.cpp:1282-1285` classifies an unknown suffix as `BadSuffix` while `:1360-1364` still sets `isOutOfRange` from `result_out_of_range`; `:1385` already suppresses `precisionLost` when out of range. | replaced "at most one of these per literal" with the precedence order, noted the classification arms are mutually exclusive, and named `1e400q` as the one overlapping shape (suffix report wins) |
| b56e139d0653 | fixed | `source/slang/slang-parser.cpp:2767-2771` raises `Diagnostics::OperatorNameOnNonFunction` from `UnwrapDeclarator` (`:2752`); the only opt-in is `:3697` in the function branch of `ParseDeclaratorDecl` (`:3584`). Code 20020 and message at `source/slang/slang-diagnostics.lua:944-949`. `int operator+ = 3;` drawing E20020 is a verified CHECK line of `operator-name-on-non-function-rejected.slang`; the accepted form matches `operator-name-on-function-accepted.slang`. | added the accepted `V operator+(V a, V b)` versus rejected `int operator+ = 3;` contrast with E20020 |
| bb51b5762505 | fixed | `source/slang/slang-parser.cpp:7255-7286`: `Parser::parseVarDeclrStatement` keeps only `VarDeclBase`, `DeclGroup`, `AggTypeDecl`, `TypeDefDecl`, `UsingDecl` and diagnoses everything else with `Diagnostics::DeclNotAllowed`. `TypeAliasDecl : TypeDefDecl` at `source/slang/slang-ast-decl.h:560`. Code 30102 and message "~declType is not allowed here." at `source/slang/slang-diagnostics.lua:1026-1030`; "namespace is not allowed here." is the verified CHECK of `decl-not-allowed-in-statement.slang`. Distinct from the container-nesting check `isDeclAllowed` (`:5354`), whose `default` arm returns `true` for a `ScopeDecl` parent. | added a `## Failure modes` bullet naming the statement-position whitelist, E30102, and the distinction from `isDeclAllowed` |
| 0188169a6cb8 | fixed | `source/slang/slang-parser.cpp:3030-3053`: the post-speculation `switch` accepts exactly `Scope`, `Dot`, `LParent`, `RParent`, `LBracket`, `RBracket`, `Colon`, `Comma`, `QuestionMark`, `Semicolon`, `OpEql`, `OpNeq`, `OpGreater`, `OpRsh`, `EndOfFile` — identical to the list in `docs/generated/design/syntax-reference/grammar.md:619-629`. | enumerated the FOLLOW set inline, cited the source lines, and cross-linked grammar.md's `<` disambiguation section |
| 7f923f6cd411 | fixed | E31002 is `attribute-not-applicable`, message "attribute '~attrName' is not valid here", at `source/slang/slang-diagnostics.lua:2549-2554`; raised by the checker in `source/slang/slang-check-modifier.cpp:1245` and `:1550` (outside this page's `watched_paths`). That `[unroll]` on a function draws exactly this one diagnostic and no parse-stage report is the exhaustive-mode assertion of the bundle test `modifier-validated-at-check-not-parse.slang`. | named `Diagnostics::AttributeNotApplicable` (E31002) with the `[unroll]`-on-a-function example, anchoring the parse/check boundary |
| 212fa7e5c537 | fixed | `source/slang/slang-parser.cpp:6370-6394`: `Parser::ParseStruct` sees `[` after `struct` and emits `InvalidBracketAttributesPlacement` at `>= SLANG_LANGUAGE_VERSION_2026`, `DeprecatedBracketAttributesPlacement` at `>= 2025`, nothing before, then parses the list anyway. Codes 31204 (warning) / 31205 (error) at `source/slang/slang-diagnostics.lua:3001-3016`; E31205 is the verified CHECK of `struct-bracket-attribute-placement-2026.slang`. | added a paragraph under `## Modifier parsing` with the three-way version gate and both diagnostic names/codes |
| a3a99789ebbb | fixed | Declarator-level skip at `source/slang/slang-parser.cpp:2640-2681`, gated on `parser->options.enableEffectAnnotations` (field at `:99`, set from `CompilerOptionName::EnableEffectAnnotations` at `:9982` and `:10014`), with the `<let` / `< X :` carve-outs at `:2651-2656` and the scratch-`TokenReader` scan for a `;` before the next `>` at `:2663-2678`. Semantic-level skip at `:3966-3978` inside `_parseOptSemantics` (`:3946`), unconditional. Both behaviours match `effect-annotation-angle-bracket-skip.slang` and `semantic-annotation-clause-skipped.slang`. | added a `### Angle-bracket annotations` subsection under `## Parser` covering both skips, which one is flag-gated, the declarator-level disambiguation, and that both discard the clause |
| 70faf67147ee | fixed | Suffix scan at `source/slang/slang-parser.cpp:8630-8687` (`z`/`Z` sets `IntegerLiteralWidthSuffix::Pointer` at `:8665-8674`; a repeated width or `u` sets `unknownSuffix` and draws `InvalidIntegerLiteralSuffix`, `:8683-8688`). Type selection in `_determineIntegerLiteralType` (`:8486`): decimal `Pointer` arm at `:8543-8565` (signed to `INT64_MAX`, `signedMinimumIntException` at `INT64_MAX+1`, `IntegerLiteralTooLarge` at `INT64_MAX+2` and above, `UIntPtr` otherwise); non-decimal `Pointer` arm at `:8589-8592` ignores magnitude. Unary-minus rewrite `UIntPtr` to `IntPtr` at `:9835-9845`. W40004 at `source/slang/slang-diagnostics.lua:4151-4157`. Matches `integer-literal-pointer-width-suffix.slang` and `integer-literal-too-large-width-suffix.slang`. | added a `## Failure modes` bullet documenting the integer suffix scan, the `z` pointer-width type selection, the `signedMinimumIntException` deferral, and W40004 |
| bb1cf0c2a51c | fixed | `Val` base at `source/slang/slang-ast-base.h:380`; `ArrayExpressionType::getElementCount()` returns `IntVal*` at `source/slang/slang-ast-type.h:583` and `VectorExpressionType::getElementCount()` at `:751`; the interned constant itself is `getOrCreate<ConstantIntVal>` at `source/slang/slang-ast-builder.h:438`. | attached the user-level surface to the `Val` bullet: the `3` in `vector<float, 3>` and the `4` in `int a[4]` |
| 91b203c9b706 | fixed | `source/slang/slang-parser.cpp:2960-2996`: after `CheckTerm`, a `DeclRefExpr` whose `declRef` is a `GenericDecl` **or** a `FunctionDeclBase` **or** an `AggTypeDeclBase` sets `BaseGenericKind::Generic` (the comment at `:2969-2975` gives the reason); an `OverloadedExpr` commits if any candidate is one of those three (`:2984-2995`); everything else is `NonGeneric` and returns the base unchanged (`:3013-3014`). The doc's narrower "type-checks as a generic-typed declaration" was wrong; grammar.md:631-638 already had the wider rule. | restated the body-stage rule as generic / function / type commits, including the overloaded-base case, and noted only other resolutions force the comparison reading |
