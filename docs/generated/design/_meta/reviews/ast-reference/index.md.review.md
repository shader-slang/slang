---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:20+00:00
target_doc: ast-reference/index.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: d8e32ce634cb5c690185a7348f23f158bf8feb8f41b3a73c73eb75ceb79f8bd5
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for ast-reference/index.md

## Summary
The index is useful as a navigation page, and its page links and approximate class counts are within the prompt tolerance. The main issue is that the taxonomy diagram claims to mirror `slang-ast-base.h` while including abstract families that are declared in `slang-ast-val.h` instead.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ast-reference/index.md` and used the listed prompt, dependency docs, and watched paths at `eb9403ef595a99c2ff6def1d538dbd7a792d9371`.
- Verified the required intro, `## Family taxonomy`, `## Pages`, `## Cross-cutting topics`, and `## How to navigate` sections.
- Checked the taxonomy diagram against `source/slang/slang-ast-base.h` and checked the Val-family entries against `source/slang/slang-ast-val.h`.
- Checked approximate concrete FIDDLE class counts: declarations 60, expressions 94, statements 30, types 119, values 60, modifiers 254.
- Resolved the relative document links and anchors; the body has no source line-number citations.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Family taxonomy` | The paragraph says the diagram mirrors the abstract-root hierarchy declared in `slang-ast-base.h`, but the diagram includes `IntVal` and `Witness`, which are not declared in that header. This also violates the index prompt's requirement that the diagram show the roots exactly as declared in `slang-ast-base.h`. | `docs/generated/design/_meta/prompts/ast-reference-index.md:25-32` requires the base-header roots; `source/slang/slang-ast-val.h:143-145` declares `IntVal`; `source/slang/slang-ast-val.h:744-746` declares `Witness`. | Remove `IntVal` and `Witness` from the base-header taxonomy diagram, and keep them in the `values.md` navigation bullet where the non-Type `Val` leaves are already summarized. |
