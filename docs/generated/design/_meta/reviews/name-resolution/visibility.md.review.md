---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: name-resolution/visibility.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 7334450582d01cf1d273acf69603e1a99298f518a80c10f85a058fb506f75ff0
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 3
  minor: 0
  nit: 0
---

# Review report for name-resolution/visibility.md

## Summary
The page is structurally lint-clean, but review found 3 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked visibility modifiers, `DeclVisibility`, default visibility logic, lookup/overload visibility filters, diagnostics, and edge cases.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Source` / visibility filtering | The page’s core visibility filter evidence lives in `slang-check-expr.cpp`, but that file is not watched for this page. | The `visibility.md` manifest entry excludes `source/slang/slang-check-expr.cpp`; the filter is in `source/slang/slang-check-expr.cpp:1079-1244`. | Add `slang-check-expr.cpp` to watched paths or restrict the page to watched sources and call out the missing path. |
| F-002 | major | `## Concepts` | The page says the default language version is `SLANG_LANGUAGE_VERSION_2025` for sessions that do not override it, but public defaults still define `SLANG_LANGUAGE_VERSION_DEFAULT = SLANG_LANGUAGE_VERSION_LEGACY`; `SessionDesc::minLanguageVersion` is 2025. | `include/slang.h:5403-5405` defines the language default; `include/slang.h:5420-5421` defines `minLanguageVersion`. | Distinguish the language default from the minimum supported language version. |
| F-003 | major | multiple sections | The page cites several non-watched paths. | The `visibility.md` manifest entry excludes cited files such as `slang-ast-support-types.h`, `include/slang.h`, `slang-check-expr.cpp`, `slang-lookup.cpp`, and `slang-diagnostics.lua`. | Expand watched paths or remove unsupported claims. |
