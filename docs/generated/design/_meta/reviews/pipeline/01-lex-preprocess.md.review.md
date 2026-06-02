---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: pipeline/01-lex-preprocess.md
target_doc_source_commit: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
target_doc_watched_paths_digest: ffca628ccbe37729fbf4b931ff5a00d755e36eb2c4c24e63869e5ee91bba63b3
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 1
  minor: 1
  nit: 0
---

# Review report for pipeline/01-lex-preprocess.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked token model, token definitions, source-location types, lexer skip behavior, `PreprocessorDesc`, include search, invalid token handling, and macro-token replay.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Failure modes` | The page says suppressed lexer diagnostics silently drop invalid characters. Suppression hides diagnostics, but invalid tokens are still returned. | `source/compiler-core/slang-lexer.h:146-150` nulls the diagnostic sink; `source/compiler-core/slang-lexer.cpp:1739` returns `TokenType::Invalid`. | Replace “silently drops them” with “emits `Invalid` tokens without diagnostics”. |
| F-002 | major | `## Source-location preservation` | The page says macro-expanded tokens point at the macro invocation, but raw macro-body tokens are replayed from the macro definition; only constructed tokens such as `__LINE__` / `__FILE__` use invocation locations. | `source/slang/slang-preprocessor.cpp:2435-2444` replays macro tokens; `source/slang/slang-preprocessor.cpp:2332-2334` uses `m_macroInvocationLoc` for constructed source-location builtins. | Distinguish raw body tokens, argument tokens, and constructed builtin/stringized/pasted tokens. |
