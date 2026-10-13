---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:08:35+00:00
target_doc: syntax-reference/tokens.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 1a02145d6df9d35fc274dac0e859c24bfa85f644da98d8b22f9096154e3f16ae
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: partial
finding_count: 6
severity_breakdown:
  critical: 0
  major: 0
  minor: 6
  nit: 0
---

# Review report for syntax-reference/tokens.md

## Summary
The catalog exhaustively covers all 67 `TokenType` values, all links resolve, and every line-number citation is accurate at the recorded commit. Six minor findings remain: one watched source is absent from the source inventory, three statements exceed or misstate the source and prompt contract, the source-location model is described incorrectly, and the front-matter digest no longer matches the resolved watched paths.

## Items checked
- Read `_review.md`, `_common.md`, `syntax-tokens.md`, the target, and all six files resolved by `regenerate.py show`; the manifest declares no dependencies.
- Compared all 67 taxonomy rows with `source/compiler-core/slang-token-defs.h` and spot-checked more than 20 behavioral, parser-use, preprocessor, data-layout, and source-location claims at `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Re-derived all 13 unique line-number citations in `source/compiler-core/slang-lexer.cpp`; every cited function, branch, and range is accurate.
- Resolved all 20 markdown-link occurrences at the recorded commit and confirmed all three linked generated peer pages are manifest entries.
- Swept 121 backticked identifier candidates and all eight source filenames; the sole text-search miss, `Lexer::getDiagnosticSink`, was manually resolved to `source/compiler-core/slang-lexer.h:147-150`.
- Confirmed the required sections and universal style rules, and measured 14,552 bytes against the 24,576-byte cap.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Source`, lines 16-31 | The source inventory says the catalog is reverse-engineered from five linked files but omits the sixth resolved watched file, `source/slang/slang-preprocessor.cpp`. | `docs/generated/design/_meta/manifest.yaml:229-240` includes `source/slang/slang-preprocessor.cpp`; it supplies the documented include-path behavior in `source/slang/slang-preprocessor.cpp:3605-3635`. | Add `slang-preprocessor.cpp` to `## Source` and identify `HandleIncludeDirective` as the owner of angle-bracket include-path assembly. |
| F-002 | minor | `## Token-kind taxonomy`, lines 90-93 | The page calls `_lexTokenImpl` a punctuation “dispatch table,” but the implementation is a `switch` with per-character arms; no punctuation table is present. | `source/compiler-core/slang-lexer.cpp:1966-1974,1987-2023,2181-2430`. | Replace “punctuation dispatch table” with “per-character switch in `_lexTokenImpl`.” |
| F-003 | minor | `## Token flags`, line 194 | `AfterWhitespace` is labeled “relevant to macro pasting,” but token-paste handling does not consult this flag. The preprocessor uses it to preserve whitespace during stringization and to distinguish function-like macro definitions. | `source/slang/slang-preprocessor.cpp:2103-2241` handles `TokenPaste` without reading the flag; actual reads are at `source/slang/slang-preprocessor.cpp:2531-2539` and `3897-3902`. | Replace the parenthetical with the verified uses: stringization/spacing preservation and function-like macro recognition. |
| F-004 | minor | `## Special-case lexing rules`, lines 246-256 | The numeric-suffix bullet goes beyond the per-document prompt's explicit “only the structural fact” scope by documenting suffix classification, rounding, error enum values, range, and precision flags. | `docs/generated/design/_meta/prompts/syntax-tokens.md:42-46` limits this item to suffixes remaining in token text and being parsed later. | End the bullet after stating that suffixes remain in raw token text and consumers decode them later; remove the decoder-specific details. |
| F-005 | minor | `## Source location`, lines 276-284 | The page says the `SourceLoc` encoding itself distinguishes “spelling” and “expansion” locations. The source defines one `uint32_t` value mapped to one `SourceView`; its selectable interpretations are `Nominal`, `Actual`, and `Emit`, while an initiating location is a separate `SourceView` field. | `source/compiler-core/slang-source-loc.h:130-170,388-396,527-532`; `source/compiler-core/slang-source-loc.cpp:1022-1045`. | Describe `SourceLoc` as a 32-bit key resolved through `SourceManager`/`SourceView`; remove the spelling/expansion claim or separately explain `getInitiatingSourceLoc` without attributing it to the integer encoding. |
| F-006 | minor | Front matter, line 6 | `watched_paths_digest` is valid hexadecimal but does not match the six currently resolved watched files at the recorded source commit: the document records `1a02145d...f16ae`, while `regenerate.py digest syntax-reference/tokens.md` returns `6e0a427e...8b39`. | `docs/generated/design/_meta/manifest.yaml:229-240` defines the current watched set; the target front matter records the older digest at `docs/generated/design/syntax-reference/tokens.md:6`. | Refresh the document front matter through the generation workflow so `watched_paths_digest` records `6e0a427eba39e35678e64610833cfef0257a86ba98ba6a3bac59ba622cdb8b39`. |
