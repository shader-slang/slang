---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:54:00+00:00
target_doc: pipeline/05-ir-passes.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: c7ceb8f7138b0d1f8d9559d2e33d3bfbcf92dade0e6bafe75a6e9dd8ace9f07f
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for pipeline/05-ir-passes.md

## Summary

Overall the page is source-aligned and its pass inventory links resolve at the target source commit. The only issue I found is structural: the `## Pass utilities` section appears before `## Adding a new pass`, while the per-document prompt requires the opposite order.

## Items checked

- Ran `python3 docs/generated/design/_meta/regenerate.py show pipeline/05-ir-passes.md` and reviewed the resolved watched-file scope plus dependency `pipeline/04-ast-to-ir.md`.
- Verified required front matter keys and confirmed `target_doc_source_commit` and `target_doc_watched_paths_digest` match the target document.
- Resolved all 185 relative Markdown links at `52339028a2aa703271533454c6b9528a534bac31` with no missing targets.
- Verified all line-number citations in the body against `source/slang/slang-emit.cpp`, including `linkAndOptimizeIR`, `emitEntryPointsSourceFromIR`, and the cited non-textual emit call sites.
- Spot-checked 18 factual/source-alignment claims covering link/import ordering, target-sensitive pass ordering, pre-link versus post-link separation, representative pass files, coverage metadata finalization, and shared IR utility files.
- Checked required sections, table columns, no-emoji style, workspace-relative links, and source-file existence for representative rows in every pass category.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Pass utilities`, lines 316-329 | The document has `## Pass utilities` before `## Adding a new pass`, but the prompt's required structure lists `## Adding a new pass` first and `## Pass utilities` second. | `docs/generated/design/_meta/prompts/pipeline-05-ir-passes.md` lines 55-60 list `## Adding a new pass` as item 4 and `## Pass utilities` as item 5. | Move `## Pass utilities` below `## Adding a new pass` so the section order matches the prompt contract. |

## No-issues notes

- The generated front matter contains all required keys, and the digest is a valid 64-character hex value.
- Every linked `slang-ir-*.cpp` file sampled from the category tables exists at the target source commit.
- The document correctly distinguishes the unordered category inventory from target-specific ordered pipeline pages.
