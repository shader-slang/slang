---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: cross-cutting/ir-instructions.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: a7b1c184243cc33ab7365f1e766ae76123f4e9039f529babd0a030cb03949933
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for cross-cutting/ir-instructions.md

## Summary
The page is structurally lint-clean, but review found 1 finding; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked Lua schema description, `parent` / `hoistable` / flag bits, module-version warning, IR-reference links, and add-opcode workflow.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Instruction families` | The prompt requires family tables with opcode, struct name, operands, and notes; the page only links to `ir-reference/*` pages and provides no opcode tables. | `docs/generated/design/_meta/prompts/cross-cutting-ir-instructions.md` requires per-family tables. | Add summary-level tables for the required families, or update the prompt if this page is intended to be conventions-only. |
