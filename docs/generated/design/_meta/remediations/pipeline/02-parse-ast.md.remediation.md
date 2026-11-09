---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:21:54Z
target_doc: pipeline/02-parse-ast.md
review_report: ../../reviews/pipeline/02-parse-ast.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/02-parse-ast.md

## Summary

All three findings were verified against the current source and fixed in place: the typedef/canonical-type explanation, the error-recovery inventory, and a stale line citation. Action breakdown: 3 fixed, 0 rejected, 0 deferred, 0 escalated. Net size growth was paid for by trimming wording in the same paragraphs.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | source/slang/slang-syntax.cpp:958-963 interns a NamedExpressionType; source/slang/slang-ast-type.cpp:1400-1405 resolves it to the canonical type | ASTBuilder paragraph: same canonical type via wrapper plus getCanonicalType, not same Type* |
| F-002 | fixed | source/slang/slang-parser.cpp:479-515 has its own recovery loop; line 703 forwards any expected token; TryRecoverBefore has only two callers (703, 871) | Scoped inventory to TryRecoverBefore callers, dropped closing-token-only claim, added ReadToken(const char*) note |
| F-003 | fixed | ParseModifiers is defined at source/slang/slang-parser.cpp:1279; 1237 is a diagnostic branch | Modifier parsing citation: 1237 -> 1279 |
