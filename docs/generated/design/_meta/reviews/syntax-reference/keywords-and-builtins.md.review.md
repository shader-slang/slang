---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-28T09:03:07+00:00
target_doc: syntax-reference/keywords-and-builtins.md
target_doc_source_commit: 9cc1ac7cb67ffc5d742af5e8ded1381487ab6109
target_doc_watched_paths_digest: c14a0f34a0411eb654010a94f1366a4724d201d82e68f7e9547e3782269541fd
source_commit: 9cc1ac7cb67ffc5d742af5e8ded1381487ab6109
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 0
severity_breakdown:
  critical: 0
  major: 0
  minor: 0
  nit: 0
---

# Review report for syntax-reference/keywords-and-builtins.md

## Summary
No findings were identified in this follow-up review. The prior front-matter / freshness-ledger findings have been remediated, and the sampled source claims checked in this pass are supported by the source tree.

## Items checked
- Checked `__func_extension`, `__apply`, and wave builtin descriptions against parser registration and `hlsl.meta.slang` source.
- Verified the target document front matter against `docs/generated/design/_meta/freshness.json` and the current `regenerate.py digest` result.
- Ran the generated-doc linter before and after updating the review records.

## Findings

(no findings)

## No-issues notes
- The new keyword/builtin entries are supported by parser registration and `hlsl.meta.slang`.
- The front matter timestamp, source commit, and watched digest are aligned with the freshness ledger.
