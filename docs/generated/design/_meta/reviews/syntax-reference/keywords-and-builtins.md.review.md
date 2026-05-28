---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: syntax-reference/keywords-and-builtins.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 325e3645f320ff678d4d445cf5017aaf2bafd4200abafa1607fb50209f598653
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 2
  minor: 1
  nit: 0
---

# Review report for syntax-reference/keywords-and-builtins.md

## Summary
The page is structurally lint-clean, but review found 3 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked syntax table entries, statement keyword lookahead, expression keyword registrations, parser special cases for `new`, `struct`, `class`, and `enum`, plus meta-module links.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Decl keywords` | The page says `struct`, `class`, and `enum` are registered through `populateBaseLanguageModule`; source handles them directly by parser lookahead. | `source/slang/slang-parser.cpp:3118-3134` and `source/slang/slang-parser.cpp:10170-10358` show parser special cases. | State that these are parser special cases handled by parser lookahead for `struct`, `class`, and `enum`. |
| F-002 | major | `## Statement keywords` | `catch` is described as paired with `try`, but the parser attaches `catch` after `do`; `try` is parsed as an expression. | `source/slang/slang-parser.cpp:6919-6967` parses `do ... catch`; `source/slang/slang-parser.cpp:7614-7620` handles `try` expression parsing. | Describe `catch` as the `do ... catch` handler form and keep `try` in expression keywords. |
| F-003 | minor | `## Expression keywords` | `new` is omitted even though the prompt calls it out and the parser handles it specially. | `source/slang/slang-parser.cpp:9206-9209` handles `new`; `docs/generated/design/_meta/prompts/syntax-keywords-and-builtins.md` calls it out. | Add `new` to expression or special expression keywords. |
