---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:13:07Z
target_doc: ast-reference/base.md
review_report: ../../reviews/ast-reference/base.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 1, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for ast-reference/base.md

## Summary

The review contained one major finding (F-001) and it was fixed. The per-page prompt's `## Support types` contract explicitly requires a `NameLoc` row, and the table omitted it; I added the row. There were no bogus, out-of-scope, deferred, or escalated findings.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed correct and in-contract: `docs/generated/design/_meta/prompts/ast-reference-base.md:50` lists `NameLoc` among the required `## Support types` rows; `source/slang/slang-ast-support-types.h:260` declares `FIDDLE() struct NameLoc` pairing a `Name*` with a `SourceLoc`; and `source/slang/slang-ast-base.h:771` uses `NameLoc nameAndLoc` on `Decl`. The table omitted it. | Added a `NameLoc` row to `## Support types` with `slang-ast-support-types.h` as the header and a purpose describing the `Name*`/`SourceLoc` pair used by `Decl::nameAndLoc`. |
