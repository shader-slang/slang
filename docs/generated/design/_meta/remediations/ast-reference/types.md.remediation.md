---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T18:00:00+00:00
target_doc: ast-reference/types.md
review_report: ../../reviews/ast-reference/types.md.review.md
target_doc_source_commit_before: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/types.md

## Summary

One major finding addressed by removing the `Fp8Type` row from the
Nodes table. The abstract-intermediates paragraph at line 103
already lists `Fp8Type` and the hierarchy diagram still shows
`DeclRefType --> Fp8Type` so the parent role remains visible.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ast-type.h:113-114` declares `class Fp8Type : public DeclRefType` with `FIDDLE(abstract)`; per the page's stated rule abstract intermediates do not appear as table rows. | Removed the `Fp8Type` row from `## Nodes`. The abstract-intermediates listing at line 103 already calls it out and the hierarchy diagram still shows `DeclRefType --> Fp8Type --> FloatE4M3Type / FloatE5M2Type`. |
