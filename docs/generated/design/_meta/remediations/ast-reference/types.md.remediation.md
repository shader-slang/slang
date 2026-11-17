---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:55:59Z
target_doc: ast-reference/types.md
review_report: ../../reviews/ast-reference/types.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/types.md

## Summary

All three minor findings were verified against current source and fixed with small edits. The doc shrank slightly, so the size cap is unaffected.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | source/slang/core.meta.slang:1187 registers scalars via `__builtin_type`, so the exclusive magic-type claim was too strong | DeclRefType callout: noted scalar builtins use `__builtin_type(...)` |
| F-002 | fixed | source/slang/slang-ast-type.h:488 names the lowerUntypedResourceHandleToUInt IR pass | Handle callout: "during IR lowering" replaced by the named pass before emission |
| F-003 | fixed | Summary cell violated the one-short-clause contract in _common.md | NamedExpressionType row shortened; explanation moved to a new Notable nodes subsection |
