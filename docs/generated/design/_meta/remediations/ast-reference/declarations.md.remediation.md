---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:13:04Z
target_doc: ast-reference/declarations.md
review_report: ../../reviews/ast-reference/declarations.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 1, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for ast-reference/declarations.md

## Summary

The review contained one minor finding, which was fixed (fixed=1). No findings were rejected as bogus, rejected as out of scope, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed against source at eb9403ef: `source/slang/slang-parser.cpp:10497` registers `_makeParseDecl("__require_capability", parseRequireCapabilityDecl)`, and `docs/generated/design/syntax-reference/grammar.md:245` defines the production with the `'__require_capability'` token. The row summary's missing leading double underscore is a factual inaccuracy and in-contract for this reference page. | Changed the `RequireCapabilityDecl` row summary from `require_capability` to `__require_capability`. |
