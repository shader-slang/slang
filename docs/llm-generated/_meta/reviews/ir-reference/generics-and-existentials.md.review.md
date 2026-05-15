---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ir-reference/generics-and-existentials.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for ir-reference/generics-and-existentials.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked generic application, witness lookup, existential construction/destructuring, RTTI, AnyValue rows, duplicate rows, and relevant source ranges.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | lines 78-164 | The page omits the type-flow/tagged-union specialization cluster, which is existential/generic machinery. | `source/slang/slang-ir-insts.lua:2893-3121` defines set, tag-translation, tagged-union, and existential-specialization opcodes. | Add a sub-table for type-flow sets/tagged unions or route them to `misc.md` and cross-link explicitly. |
| F-002 | major | lines 126-143 | Structural opcodes such as `key`, `indexedFieldKey`, `witness_table`, and witness-table entries duplicate rows already in `structure.md`. | Duplicate rows are present in `docs/llm-generated/ir-reference/structure.md` and this page. | Choose the owning page for each concrete opcode and replace duplicate rows with cross-links. |
