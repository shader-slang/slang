---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T18:00:00+00:00
target_doc: ast-reference/index.md
review_report: ../../reviews/ast-reference/index.md.review.md
target_doc_source_commit_before: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/index.md

## Summary

One minor finding addressed by replacing the pseudo-node
`NonTypeVal` in the taxonomy diagram with the real `Val` children
declared in `slang-ast-base.h` (`DeclRefBase`, `IntVal`, `Witness`).

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ast-base.h` declares the `Val` roots (`Type`, `DeclRefBase`, `IntVal`, `Witness`, ...) directly; `NonTypeVal` was an editorial pseudo-node not present in the source. | Replaced `Val --> NonTypeVal["DeclRefBase, IntVal, Witness, ..."]` with three direct `Val --> DeclRefBase`, `Val --> IntVal`, `Val --> Witness` edges. |
