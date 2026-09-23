---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:38:52Z
target_doc: syntax-reference/tokens.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
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

All ten open gaps were confirmed against the lexer source and fixed in
the document; none was escalated, deferred, or rejected. Six sections
were touched: `#content-tokens` (one consolidated edit covering the
suffix ellipses, the accepted float shapes, and the decode-time
diagnostics), `#operators` (a maximal-munch preamble), the punctuation
table's `At` / `Dollar` / `DollarDollar` rows, `#token-flags` (a
worked `#define` continuation), `#special-case-lexing-rules` (a
consolidated edit covering the `//`-comment continuation example, the
octal warning's severity, and the `#INF` decoded value), and
`#source-location` (the `__LINE__` surface). The `drift-from-source`
gap on the octal diagnostic was a documentation defect, not a compiler
one: `slang-lexer-diagnostic-defs.h` declares `octalLiteral` as
`Warning`, which is exactly what the test observed, so the doc's
severity-neutral wording was the thing at fault. Two claims rest on
`source/slang/slang-parser.cpp`, which is not in this page's
`watched_paths` — see the note at the end of the Actions table.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| feab49dbaf9e | fixed | `source/compiler-core/slang-lexer.cpp:561-583` (`_maybeLexNumberSuffix` accepts any alphanumeric/underscore run, so no fixed suffix set exists at lex time); `source/compiler-core/slang-lexer.cpp:1273-1287` (float set: empty or `f`/`F`, then `h`/`H`/`hf`/`HF`/`fh`/`FH`, then `l`/`L`/`lf`/`LF`/`fl`/`FL`, with both letters of a two-letter form required to match in case); `source/slang/slang-parser.cpp:8608-8688` (integer set `u`/`l`/`ll`/`z`, any order) | replaced both `...` ellipses with the lexer's no-fixed-set rule plus the per-kind decode sets and the case rule, in one consolidated paragraph after the content-token table (also covers 4d8e1c71909f) |
| 76da52623665 | fixed | `source/compiler-core/slang-lexer.cpp:1988-2003` (leading dot), `:665-680` (trailing dot, plus the `x`/`r` swizzle carve-out), `:598-602` and `:656-688` (hex `0x…p…` via `_lexNumber(16)`), `:608-624` (`#INF` and decimal exponent) | added an "accepted shapes" list to the `FloatingPointLiteral` Notes cell, including the `.x`/`.r` carve-out the source states |
| 4d8e1c71909f | fixed | `source/compiler-core/slang-lexer.cpp:817-820` (`integerLiteralTooLargeForAnyType`) with `source/compiler-core/slang-lexer-diagnostic-defs.h:52-56` (`DIAGNOSTIC(10012, Error, ...)`); `source/slang/slang-parser.cpp:8685` (`Diagnostics::InvalidIntegerLiteralSuffix`); both CHECK-verified by `negative-integer-literal-too-large.slang` and `negative-invalid-int-suffix.slang` in the reporting bundle | named the two decode-time diagnostics in the same consolidated content-token paragraph as feab49dbaf9e, attributing each to the helper that raises it |
| 7f52d6058caa | fixed | `source/compiler-core/slang-lexer.cpp:2292-2311` (nested `>` / `>>` / `>>=` lookahead; `default:` arms return the shorter kind) | added a maximal-munch preamble to the operator table with the `>`-prefixed family as the worked example |
| e508a630fe09 | fixed | `source/compiler-core/slang-lexer.cpp:2418-2420` (`At`) and `:2421-2430` (`Dollar` / `DollarDollar`); `source/slang/hlsl.meta.slang:1215` (`result:$$float2 = OpImageQueryLod $this $location` — a `spirv_asm` body using both); no `TokenType::At` reference anywhere in `source/slang/` | filled the three Notes cells: `$`/`$$` as `spirv_asm` value / type operand prefixes, `@` as reserved with no parser consumer (matching the `DotDot` row's wording) |
| 9861e1c92111 | fixed | `source/slang/slang-preprocessor.cpp:2318-2348` (`_pushStreamForSourceLocBuiltin` calls `getHumaneLoc(initiatingLoc)`) with `source/compiler-core/slang-source-loc.h:518` (`SourceLocType type = SourceLocType::Nominal` default); `source/slang/slang-preprocessor.cpp:4233` (`addLineDirective`); `source-location-line-directive.slang` | added a paragraph naming `__LINE__` as the user-level read of the `Nominal` interpretation and `#line` as the way to change it |
| d64b3b8d3f44 | fixed | `source/compiler-core/slang-lexer.cpp:1262-1267` (`getFloatingPointLiteralValue` sets `std::numeric_limits<double>::infinity()` as soon as the text after the digits starts with `#INF`; the preceding digits are never parsed); `:610-624` (`_maybeLexNumberExponent` matches `#INF` before any base check) | stated in the leading-zero bullet that `0#INF` and `1#INF` both decode to positive infinity and the leading digits are discarded (consolidated with 4b8da54e1594) |
| 4b8da54e1594 | fixed | `source/compiler-core/slang-lexer-diagnostic-defs.h:27` — `DIAGNOSTIC(10002, Warning, octalLiteral, ...)`; emitted at `source/compiler-core/slang-lexer.cpp:2095-2099` immediately before `_lexNumber(lexer, 8)`, so lexing is not gated on it. Source agrees with the observation, so this is a doc defect, not a compiler one | added the severity (W10002) and the explicit statement that the literal is still lexed and decoded base-8 (consolidated with d64b3b8d3f44) |
| 5daacb34340b | fixed | `source/compiler-core/slang-lexer.cpp:405-421` (`_lexLineComment` ends only when `_peek` reports a newline) and `:226-305` (`_peek` looks past a `\`-newline before returning); `line-comment-backslash-extends.slang` | added a two-line worked example to the backslash bullet showing the following physical line being swallowed by the comment |
| 90e94bc6b4e2 | fixed | `source/slang/slang-preprocessor.cpp:2700-2712` (`IsEndOfLine` keys off a `NewLine` *token*, which a folded continuation never produces) and `:4833` (`Pound` + `AtStartOfLine` is what starts a directive); `line-continuation-in-macro-definition.slang` | added a worked two-physical-line `#define` after the token-flag table, with the mechanism attributed to the absent `NewLine` token |

Note for the operator: two claims are anchored outside this page's
`watched_paths`. The complete integer-suffix set and the
`InvalidIntegerLiteralSuffix` diagnostic live in
`source/slang/slang-parser.cpp` (`parseIntegerLiteralExpr`, lines
8608-8688), and the `SourceLocType` default that makes `__LINE__` read
the `Nominal` interpretation is in
`source/compiler-core/slang-source-loc.h:518`. Both are cited in the
page (the latter file was already linked by the pre-existing
`## Source location` text), but drift in either will not be caught by
this page's `watched_paths_digest`. Adding both files to the manifest
entry for `syntax-reference/tokens.md` would close that hole. A third
fact reachable only from `slang-parser.cpp` was deliberately left
undocumented: the parser splits a lexed `OpRsh` back apart when it
closes a nested generic argument list (`slang-parser.cpp:2900-2916`),
which is the natural companion to the new maximal-munch paragraph.
