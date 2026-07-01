---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:13:06Z
target_doc: ast-reference/index.md
review_report: ../../reviews/ast-reference/index.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 1, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for ast-reference/index.md

## Summary

The review contained one major finding, which was verified against the source and fixed. The taxonomy mermaid diagram, which the surrounding prose claims mirrors the abstract-root hierarchy declared in `slang-ast-base.h`, incorrectly included `IntVal` and `Witness` as children of `Val`; both classes are declared in `slang-ast-val.h`, not the base header. The two edges were removed; the `values.md` navigation bullet already summarizes `IntVal` and `Witness` as non-Type `Val` leaves, so no coverage is lost.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed at `eb9403ef`: `IntVal` is declared at `source/slang/slang-ast-val.h:144` and `Witness` at `source/slang/slang-ast-val.h:745`, while `slang-ast-base.h` declares only `Type` and `DeclRefBase` as `Val` children. The prompt (ast-reference-index.md:25-32) requires the diagram to show roots exactly as declared in `slang-ast-base.h`, and the doc's own prose says the diagram mirrors that header, so the two extra edges were both factually wrong and a contract violation. | Removed the `Val --> IntVal` and `Val --> Witness` edges from the taxonomy mermaid diagram. |
