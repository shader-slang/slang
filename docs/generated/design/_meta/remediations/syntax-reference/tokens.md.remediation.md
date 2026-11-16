---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T12:00:00Z
target_doc: syntax-reference/tokens.md
review_report: ../../reviews/syntax-reference/tokens.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/tokens.md

## Summary

All four findings were verified against the current source and fixed in the document. The raw-string claim, token-text storage description and `Emit` source-map description were corrected, and the suffix inventory and second source-location paragraph were removed per the prompt contract. Breakdown: 4 fixed, 0 rejected, 0 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/compiler-core/slang-lexer.cpp:308-350` `_advance` folds backslash-newline and sets `ScrubbingNeeded`; `_lexRawStringLiteralBody` (1782) uses it | Raw-string bullet now says no escape decoding but backslash-newline is still folded |
| F-002 | fixed | `slang-lexer.cpp:2541-2578` scrubbed text is arena-allocated, otherwise source slice | Token data layout says pointer+length, possibly arena-allocated scrubbed text |
| F-003 | fixed | `source/compiler-core/slang-source-loc.cpp:225-239` `Emit` is refused only for obfuscated source maps | `Emit` described as following non-obfuscated source maps |
| F-004 | fixed | `_meta/prompts/syntax-tokens.md:43-49` limits suffixes to the structural fact and requires one source-location paragraph | Removed suffix/diagnostic bullets; merged Source location into one paragraph (see also F-003) |
