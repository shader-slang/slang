---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: cross-cutting/serialization.md
target_doc_source_commit: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
target_doc_watched_paths_digest: 5b25292bf14e2a45191187f721db5ff6afa45b617dfab12381292b32174d4546
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
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

# Review report for cross-cutting/serialization.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked generic serialize pattern, fossil validation comments, RIFF chunk structure, watched serialize files, module-version note, and front matter.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## What is serialized` and `## Source-location serialization` | The page relies on `slang-serialize-ir-types.*` and `slang-serialize-source-loc.*`, but those files are not watched for this page. | The serialization manifest entry includes serialize AST/IR/container/fossil/riff files, but not IR-types or source-loc serialization files. | Add those files to watched paths or mark those details as requiring additional watched paths. |
| F-002 | minor | `## Round-trip and repro files` | The page adds unsupported guidance that `-target slang` plus the test-server framework is the supported path; the prompt only asks for a deprecated repro note. | `docs/generated/design/_meta/prompts/cross-cutting-serialization.md` limits this section to historical repro handling, and watched paths do not establish the alternative workflow. | Remove the alternative-workflow claim unless the manifest is expanded to include supporting files. |
