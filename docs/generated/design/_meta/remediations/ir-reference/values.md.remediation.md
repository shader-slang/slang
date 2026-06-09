---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ir-reference/values.md
review_report: ../../reviews/ir-reference/values.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/values.md

## Summary

Of two findings, one was fixed and one was rejected as out-of-scope.
The `logicalAnd` / `logicalOr` short-circuit wording was corrected to
describe ordinary two-operand boolean ops. The missing-opcodes finding
was rejected because all four opcodes are already owned by
more-specific sibling pages; duplicating them here would violate the
coverage rule.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-out-of-scope | `_common.md:259-264` coverage rule lists a straddling opcode in the more-specific family only. `packAnyValue`/`unpackAnyValue` are owned by `ir-reference/generics-and-existentials.md` (and already cross-linked from values.md `## See also`), `makeValuePack` by `ir-reference/misc.md`, and `makeCombinedTextureSampler` by `ir-reference/resources-and-atomics.md`. Adding rows here would duplicate; the alternative (prompt/manifest revision) is outside the editable target doc. | — |
| F-002 | fixed | `source/slang/slang-ir-insts.lua:1465-1471` defines `logicalAnd`/`logicalOr` as two-operand (`left, right`) ops; `source/slang/slang-ir.cpp:6730-6739` emits `kIROp_And`/`kIROp_Or` from already-supplied operands. IR has no short-circuit. | Rewrote both row summaries to describe boolean AND/OR over already-evaluated operands and removed "Short-circuit". |
