---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T19:30:00+00:00
target_doc: pipeline/01-lex-preprocess.md
review_report: ../../reviews/pipeline/01-lex-preprocess.md.review.md
target_doc_source_commit_before: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/01-lex-preprocess.md

## Summary

Both findings addressed by rewriting the affected passages to match
the actual code behavior: the "silently drops" claim and the
oversimplified macro-token source-location story.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The reviewer is correct: with `kLexerFlag_SuppressDiagnostics` the lexer still emits `Invalid` tokens; only the diagnostics are suppressed. | Rewrote the "Failure modes" bullet to say the lexer "still emits `TokenType::Invalid` tokens for these inputs but suppresses the diagnostics" and explained why this matters inside skipped `#if` blocks. |
| F-002 | fixed | The page conflated the three macro-token categories; the source distinguishes raw body, argument, and constructed tokens with different `SourceLoc` choices. | Rewrote `## Source-location preservation` to enumerate the three categories (raw body tokens, argument tokens, constructed builtins/stringized/pasted) and cite the relevant `slang-preprocessor.cpp` lines (2435-2444 for replay, 2332-2334 for `m_macroInvocationLoc`). |
