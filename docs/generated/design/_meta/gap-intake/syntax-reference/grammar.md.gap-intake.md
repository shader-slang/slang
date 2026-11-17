---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: syntax-reference/grammar.md
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

# Gap-intake report for syntax-reference/grammar.md

## Summary

Eight gaps recorded as fixed were still reported by a regenerated bundle,
so each was re-decided against the source at 2518a942d59b. All eight are
again `fixed`: no source contradicts the document, so nothing is
escalated. The earlier edits were kept but re-verified (line citations
had drifted by several hundred lines and were re-derived), and the
passages the gaps point at were made more explicit: the diagnostic code
and message for the reversed `countof` form, the check-time rule for
`optional` constraints, the exact CLI flag and warning for
`__func_extension`, a lambda example that is a real lambda expression,
and the three modifiers added since the last refresh. The `-experimental-feature`,
lambda and `where optional` examples were verified by reading source and the
repository tests only; `build/Release/bin/slangc` is a Linux binary and was not run.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 2570d305e7ce | fixed | `source/slang/slang-parser.cpp:8824-8851` (`parseLambdaExpr`: traditional `ParseParameter` list, `=>`, block or `ParseArgExpr`) and `9205-9212` (a parenthesized group followed by `DoubleRightArrow` selects it); `source/slang/core.meta.slang:2101-2105` declares `IFunc<TR, each TP>` with `TR operator()(expand each TP p)`; the consuming shape matches `tests/language-feature/lambda/lambda-0.slang`. Not executed. | replaced the comment-only example with a compiled-looking `apply((float v) => v * 2.0, x)` call inside a function, stated the `=>` lookahead rule, and cited the `IFunc` line |
| dd12ac189d86 | fixed | `source/slang/slang-parser.cpp:2053-2067` diagnoses the reversed form with `VariadicPackCountConstraintRequiresCountofOnLeft` and then calls `_skipRestOfGenericWhereClause`; `source/slang/slang-diagnostics.lua:2269-2274` gives code 30437 and the message text; the non-`==` operator case is `source/slang/slang-parser.cpp:1945-1962` with code 30436 (`slang-diagnostics.lua:2262-2267`). | named E30437 with its message text, the rewrite it asks for, and the separate E30436 for a non-`==` comparison |
| 202fbec095aa | fixed | `source/slang/slang-parser.cpp:2016` consumes `optional`; `2029-2047` and `2090-2121` attach `OptionalConstraintModifier` to the nonempty, countof, conformance and equality terms, and `2078-2082` diagnoses it on `__hasDiffTypeInfo`; the checker rejects `optional nonempty` / `optional countof` at `source/slang/slang-check-decl.cpp:4607` and `4689`; the unguarded-use error is `RequiredConstraintIsNotChecked` (E30403, `source/slang/slang-diagnostics.lua:3815-3819`); examples `tests/language-feature/generics/where-optional-1.slang` and `where-optional-2.slang`. | kept the minimal `where optional T : IThing` example with the `is` guard, added the E30403 text, and listed which terms the parser versus the checker rejects with `optional` |
| c60c5946d167 | fixed | `source/slang/slang-parser.cpp:11105-11243` (`g_parseSyntaxEntries[]`): `_makeParseModifier(name, getSyntaxClass<...>)` entries are bare; callback entries are tail modifiers, with mandatory parentheses in `parseGLSLExtensionModifier` (10566), `parseGLSLVersionModifier` (10577), `parseWGSLExtensionModifier` (10588), `parseSemanticVersion` (10599), `parseBuiltinTypeModifier` (10939), `parseBuiltinRequirementModifier` (10950), `parseMagicTypeModifierImpl` (10967), `parseIntrinsicTypeModifier` (11008), `parseAttributeTargetModifier` (11055), `parseLayoutModifier` (10764), and optional ones in `parseIntrinsicOpModifier` (10484), `parseTargetIntrinsicModifier` (10513), `parseSpecializedForTargetModifier` (10555), `parseImplicitConversionModifier` (11023); `parseHitAttributeEXTModifier` (10932) and the `shared`/`volatile`/`coherent`/`restrict`/`readonly`/`writeonly` callbacks read no tail. A script comparing the doc's keyword lists to the `_makeParseModifier` entries found three missing bare keywords. | kept the Bare/Tail split and added `__ref_readonly`, `__ref_writeonly` and `__borrow` to `BareModifierKeyword` |
| 574379db1ee4 | fixed | `source/slang/slang-parser.cpp:1453-1484` (`parseModuleDeclarationDecl`) reads one `Identifier` or one `StringLiteral` (or nothing) then `';'`; the dotted loop lives only in `parseFileReferenceDeclBase` (1366), called from the import (1406) and include/implementing (1440) parsers. | kept `ModuleHeader` with a single-token name and the `ModuleName` comment that only `implementing` and `ImportPath` reach the dotted form |
| 8bf2be1914f4 | fixed | `source/slang/slang-parser.cpp:4494-4546` (`parseFuncExtensionDecl`: head via `tryParseUsingSyntaxDecl`, `parseModernParamList`, optional `throws` / `->` / `where`, `parseOptBody`); `source/slang/slang-check-decl.cpp:17462-17471` rejects user code without the option unless `isFromCoreModule`; `source/slang/slang-options.cpp:1282` spells the flag `-experimental-feature`; `source/slang/slang-diagnostics.lua:2394-2399` makes it warning 30131; example matches `tests/autodiff/func-extension/fwd-diff-basic.slang`. Not executed. | kept the `__func_extension fwd_diff(cube)(...)` example and added the exact flag, warning 30131 with early return, core-module exemption, and the unsupported-head diagnostics |
| 77a45d0801b2 | fixed | `source/slang/slang-parser.cpp:4906-4926` (`parseRequireCapabilityDecl`): the `AdvanceIf(parser, "+") \|\| AdvanceIf(parser, ",")` continuation routes to `AdvanceIf(Parser*, char const*)` (787), which calls `LookAheadToken(const char*)` (759), requiring `TokenType::Identifier`; `+` and `,` are operator tokens, so the loop always ends after one atom and `ReadToken(TokenType::Semicolon)` fails on the separator. The document, not the compiler, was wrong (the compiler does what the source says). | kept the production narrowed to one `CapabilityName` with the dead-branch explanation and the `__requireCapability(a, b)` statement alternative |
| 22e335a751f6 | fixed | `source/slang/slang-parser.cpp:5497` and `5874` resolve the `':'` name through `astBuilder->findSyntaxClass`; worked examples `syntax constexpr : ConstExprModifier;` (`source/slang/core.meta.slang:28`) and `attribute_syntax [vk_binding(binding: int, set: int = 0)] : GLSLBindingAttribute;` (`core.meta.slang:4470`). Nothing gates either form to the core module (`g_parseSyntaxEntries[]` at 11125-11126), so the "core-module-only" suggestion was not written. | kept the paragraph on what the `':'` clause may name and the two core-module examples; made the optional `':'` of `AttributeSyntaxDecl` explicit in the production (parse-time TODO at 5876-5883) |

## Notes for the operator

- Two previously deferred gaps (`115c5727e059`, `1b1ee9e6546d`) became answerable
  because `slang-check-decl.cpp` and `slang-check-stmt.cpp` are now watched. The
  document now states both answers (untyped modern parameter: E30621 unless an
  initializer is given, `slang-check-decl.cpp:15084-15094`; `$for` variable `int`
  and const, bounds integer constants, `slang-check-stmt.cpp:361-390`), plus
  the `$for` deprecation (E30525) and removal (E30526). Their ledger rows are
  unchanged by this report.
- Suspected compiler defect, no finding opened: the `+` / `,` continuation in
  `parseRequireCapabilityDecl` is dead code (see gap 77a45d0801b2).
