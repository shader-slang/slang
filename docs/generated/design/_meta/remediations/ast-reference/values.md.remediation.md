---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:01:29Z
target_doc: ast-reference/values.md
review_report: ../../reviews/ast-reference/values.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 1
---

# Remediation report for ast-reference/values.md

## Summary

Three findings were fixed in the document (F-001, F-002, F-004) after verifying each against the current source. F-003 was escalated: its small half (missing witness types on the `DiffTypeInfoWitness` row) was applied, but the single-table consolidation would remove headings that peer docs link to.

## Escalated findings

- F-003: consolidating `## Nodes` into one table deletes the `#declref-family`, `#intval-family`, `#witness-family`, `#modifier-values`, `#differentiation-values` and `#polynomial-helpers` anchors that `docs/generated/design/ast-reference/index.md` (lines 91-97) and `docs/generated/design/name-resolution/lookup.md` (line 522) link to; a human must decide whether to restructure and update those peers.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ast-val.cpp:312-319` builds the witness via `getOrCreate` in substitution, so "every construction site" was too strong. | Scoped the claim to semantic-checking sites and added the substitution exception with citation. |
| F-002 | fixed | `c` and `a` both used `2*N+3`, so the example did not exercise canonicalization; `b` is the differently spelled bound. | Changed `c` to `3+2*N`, noted the compared spellings, dropped the unverifiable "observed passing, total=165" claim. |
| F-003 | escalated | Row types verified at `source/slang/slang-ast-val.h:1018-1024` (all `SubtypeWitness`) and added to the `DiffTypeInfoWitness` row; the single-table requirement (`_common.md` AST contract item 4) conflicts with inbound anchors in peer docs, which this remediation may not edit. | — |
| F-004 | fixed | The emitter and SPIR-V image-format walkthrough is IR-level, forbidden by the AST reference contract in `_meta/prompts/_common.md`. | Replaced the backend bullet list with a short per-target statement and the `../pipeline/06-emit.md` link; kept the front-end WGSL check paragraph. |
