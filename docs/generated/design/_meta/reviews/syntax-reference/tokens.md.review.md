---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:37:15Z
target_doc: syntax-reference/tokens.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 420d22e36762196fb735538454cabb173bfdf2b017241535d0a3ffa6e9e4a75d
source_commit: c63c2fbd5115a98b2b2469f43618671653d40bf1
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 1
  major: 0
  minor: 3
  nit: 0
---

# Review report for syntax-reference/tokens.md

## Summary

The catalog covers every token kind, and its links and line-number citations resolve correctly. Corrections are needed for raw-string continuation handling, token-text storage, and source-map interpretation. The most important finding is that raw strings still undergo backslash-newline folding despite the document claiming literal preservation.

## Items checked

- Read `_review.md`, `_common.md`, and `syntax-tokens.md`; ran `regenerate.py show syntax-reference/tokens.md`. No dependency documents are declared.
- Verified against the **current working tree**; `git diff` against the document’s source commit showed no differences under `source/`.
- Compared all 67 token-table rows with `slang-token-defs.h`: no missing, extra, or duplicate kinds.
- Checked every explicit line-number citation, including the nested integer-suffix-loop range. No material citation drift found.
- Spot-checked more than 10 behavioral claims: identifier classification, numeric bases, exponent recognition, suffix scanning, scalar swizzles, longest-match operators, invalid-token diagnostics, trivia filtering, comment termination, raw strings, character decoding, token flags, include-path assembly, `$for`, and source-location decoding.
- Resolved all 30 Markdown links to 14 distinct targets, including existence at the document’s recorded commit; confirmed all three referenced generated pages are manifest entries.
- Checked required sections, table columns, front-matter keys and formats, and the size limit: 20,676 bytes versus 24,576 allowed. No raw Liquid openers, emojis, or editorial commentary found.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Special-case lexing rules`, lines 289–296 | The assertion that raw-string `newlines and backslashes are taken literally` omits an implemented transformation: backslash followed by a physical newline is folded out. A raw string containing that sequence therefore does not preserve its original contents. | `source/compiler-core/slang-lexer.cpp:1788` and `:1825`: `_lexRawStringLiteralBody` uses `_peek` and `_advance`; `:341–350` folds escaped newlines and sets `ScrubbingNeeded`; `:2541–2574` removes them from stored token text without excluding raw strings. | Distinguish ordinary escape decoding from line continuation: raw strings bypass ordinary escape decoding, but backslash-newline sequences are still folded and scrubbed by the shared lexer machinery. |
| F-002 | minor | `## Token data layout`, lines 225–228 | The statement that a token without the `Name` flag points `into the original source buffer` is too restrictive. Scrubbed token text instead resides in memory allocated by the lexer. This distinction matters for tooling that interprets token pointers as source-buffer offsets. | `source/compiler-core/slang-lexer.cpp:2544` allocates replacement storage through `m_memoryArena`; `:2574` installs that slice with `token.setContent(...)`. Only the alternative at `:2578` retains the original slice. | Describe the non-name representation as a pointer and length to token text, which may reference the source buffer or arena-allocated scrubbed content. |
| F-003 | minor | `## Source location`, lines 361–364 | `Emit ... ignores source maps` repeats the header’s simplified description but contradicts the implementation: `Emit` suppresses **obfuscated** source maps and can follow other source maps. | `source/compiler-core/slang-source-loc.cpp:225–239`: `_canFollowSourceMap` rejects `Emit` only when `getSourceMapKind() == SourceMapKind::Obfuscated`, then returns `_isNominalLike(type)`, which includes `Emit`. `:362–368` uses this lookup when decoding locations. | Qualify the description: `Emit` honors line directives and follows non-obfuscated source maps, while ignoring obfuscated maps. |
| F-004 | minor | Content-token discussion, lines 81–98; `## Source location`, lines 356–376 | The document exceeds two explicit prompt constraints: it catalogs accepted numeric suffixes and decoding diagnostics despite the structural-only suffix scope, and its source-location section contains two paragraphs instead of the required one. | `docs/generated/design/_meta/prompts/syntax-tokens.md:43–49` limits suffix handling to “only the structural fact that suffixes are part of the token text and parsed later” and specifies `Source location — one paragraph`. | Remove the detailed suffix acceptance/diagnostic inventory while retaining the structural explanation. Condense source-location coverage into one paragraph, incorporating F-003’s correction. |