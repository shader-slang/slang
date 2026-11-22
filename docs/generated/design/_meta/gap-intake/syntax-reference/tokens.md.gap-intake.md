---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: syntax-reference/tokens.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 10
actions:
  fixed: 10
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for syntax-reference/tokens.md

## Summary

These ten gaps were previously recorded as fixed but are reported again
by a regenerated bundle (REOPENED). Each was re-decided against the
current source. The document already stated every behaviour, so the
remaining defect was stale anchors: citations into
`source/slang/slang-parser.cpp` and `source/slang/hlsl.meta.slang` had
moved, and the `$for` description was out of date. All ten are `fixed`
(none escalated, rejected, or deferred); the lexer files are unchanged
since the previous intake, so those citations were re-checked in place.
The `slangc` binary in the tree is a Linux build and could not be run,
so every worked example is verified by reading source only.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| feab49dbaf9e | fixed | `source/compiler-core/slang-lexer.cpp:561-583` (any alphanumeric run folded as suffix); `:1273-1287` (float suffix set); `source/slang/slang-parser.cpp:9014-9065` (integer suffix loop: `u`, `l`, `ll`, `z`; `ll` letters must match case) | suffix sets stay in the content-token paragraph; parser citation moved from 8614-8719 to 8991-9096 (loop 9014-9065) |
| 76da52623665 | fixed | `source/compiler-core/slang-lexer.cpp:1988-2003` (leading dot), `:665-680` (trailing dot, `x`/`r` carve-out), `:608-624` (`#INF` and exponent), `:651` and `:682` (hex float via `_lexNumber`) | accepted-shapes list in the `FloatingPointLiteral` row re-verified unchanged |
| 4d8e1c71909f | fixed | `source/compiler-core/slang-lexer.cpp:817-820` with `source/compiler-core/slang-lexer-diagnostic-defs.h:52-56` (E10012); `source/slang/slang-parser.cpp:9068` (`InvalidIntegerLiteralSuffix`) | decode-time diagnostics paragraph kept; parser line range refreshed |
| 7f52d6058caa | fixed | `source/compiler-core/slang-lexer.cpp:2292-2311` (nested `>` / `>>` / `>>=` lookahead, `default:` arms return the shorter kind) | maximal-munch preamble re-verified unchanged |
| e508a630fe09 | fixed | `source/compiler-core/slang-lexer.cpp:2418-2430`; `source/slang/slang-parser.cpp:9841` (`Dollar` is a `$foo` value operand) and `:9847` (`DollarDollar` is a `$$foo` type operand) in the `spirv_asm` operand parser; `source/slang/hlsl.meta.slang:1437` (`result:$$float2 = OpImageQueryLod $this $location`); `source/slang/slang-parser.cpp:7219-7228,7268-7269` (`$for` read via `Dollar`, diagnosed as deprecated from 2025 and removed from 202c); no `TokenType::At` consumer in `slang-parser.cpp` | updated the hlsl.meta.slang citation from line 1215 to 1437 and added the current `$for` deprecation/removal status to the `Dollar` row |
| 9861e1c92111 | fixed | `source/slang/slang-preprocessor.cpp:2319-2348` (`_pushStreamForSourceLocBuiltin`, `getHumaneLoc(initiatingLoc)` at 2337); `source/compiler-core/slang-source-loc.h:518` (`SourceLocType::Nominal` default); `source/slang/slang-preprocessor.cpp:4233` (`addLineDirective`) | `__LINE__` paragraph re-verified; line citations remain correct |
| d64b3b8d3f44 | fixed | `source/compiler-core/slang-lexer.cpp:1261-1267` (`#INF` after the digits yields `std::numeric_limits<double>::infinity()`, digits discarded); `:608-624` (`_maybeLexNumberExponent` matches `#INF`) | bullet states `0#INF` and `1#INF` both decode to positive infinity; re-verified unchanged |
| 4b8da54e1594 | fixed | `source/compiler-core/slang-lexer-diagnostic-defs.h:27` (`DIAGNOSTIC(10002, Warning, octalLiteral, ...)`); `source/compiler-core/slang-lexer.cpp:2095-2099` (diagnosed, then `_lexNumber(lexer, 8)`). The source agrees with the observation, so this is a documentation defect, not a compiler bug | octal bullet states warning severity and that lexing proceeds base-8; re-verified unchanged |
| 5daacb34340b | fixed | `source/compiler-core/slang-lexer.cpp:405-421` (`_lexLineComment` ends on `_peek` newline) and `:226-305` (`_peek` looks past a `\`-newline) | `//`-comment continuation example re-verified; example not executed (slangc binary is Linux-only) |
| 90e94bc6b4e2 | fixed | `source/slang/slang-preprocessor.cpp:2700-2712` (`IsEndOfLine` keys off a `NewLine` token) and `:4896` (`Pound` plus `AtStartOfLine` starts a directive) | worked two-line `#define` re-verified; `Pound` citation moved from line 4833 to 4896; example not executed |
