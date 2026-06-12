---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:04:49+00:00
target_doc: pipeline/05-ir-passes.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 3c3b5585e80344fff65833edfb71495cd9350eddefb45844496574bcd283e01a
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
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

# Review report for pipeline/05-ir-passes.md

## Summary

The IR-pass catalog satisfies the representative inventory prompt and matched sampled source files and orchestrator calls. I found no findings.

## Items checked

- Ran `regenerate.py show pipeline/05-ir-passes.md` and read the target page, `_common.md`, `pipeline-05-ir-passes.md`, and dependency `pipeline/04-ast-to-ir.md`.
- Verified front matter keys, recorded source commit, and 64-character hex watched-path digest.
- Spot-checked more than 25 table rows against actual `source/slang/slang-ir-*.cpp` files, including legalization, autodiff, target-specific, coverage, layout, loop, utility, and validation entries.
- Checked the `linkAndOptimizeIR` ordering overview against `source/slang/slang-emit.cpp`, including the `SLANG_PASS` wrapper, coverage instrumentation/finalization, uniform collection, specialization, optional type/result lowering, target-specific legalization, and emit handoff.
- Confirmed the document treats 04b and per-target pipeline pages as the authoritative ordered references while keeping this page categorical, as required by the prompt.
- Resolved relative links with `regenerate.py lint` for this assigned doc group; lint reported no issues.

## Findings

(no findings)

## No-issues notes

- Representative pass rows point to files that exist in the watched-path expansion.
- The page correctly distinguishes pre-link ordering from post-link target-sensitive ordering.
- The coverage instrumentation row matches the current `instrumentCoverage` option handling in `linkAndOptimizeIR`.
