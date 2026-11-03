---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:10:36Z
target_doc: ir-reference/misc.md
review_report: ../../reviews/ir-reference/misc.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for ir-reference/misc.md

## Summary

Six findings were verified against the current source and fixed in the doc (F-001, F-002, F-004, F-005, F-006, F-007). One finding (F-003) is deferred because assigning ownership of `reportOptiXIntersection` requires edits to a peer page and prompts; the doc's exhaustive-coverage claim was corrected meanwhile.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `core.meta.slang` line 2330 declares the intrinsic op; `slang-lower-to-ir.cpp` 991-1019 emits it; peephole 411-416 folds it later. | GetArrayLength row: producer is the intrinsic call path; distinguished from the `visitGetArrayLengthExpr` shortcut. |
| F-002 | fixed | `slang-ir-lower-expand-type.cpp` 86-108 builds `IRExpand`; `slang-ir-specialize.cpp` 3317 expands per slot. | Callout reattributed; Expand row names `visitExpand*` producers and the `ExpandTypeOrVal` conversion. |
| F-003 | deferred | Valid, but ownership needs a row in `resources-and-atomics.md` and prompt routing updates, outside this doc. | Follow-up: assign the opcode to the ray-tracing section of that page. (Doc claim narrowed to name the exception.) |
| F-004 | fixed | Lua gives only `min_operands`; `getReferenced()` at `slang-ir-insts.h` 2651. | Six rows now `(N unnamed)`; `referenced` marked with a dagger and legend. |
| F-005 | fixed | Visitors exist at the cited `slang-lower-to-ir.cpp` lines. | Added `visit*` names to AST-origin cells of pack, IsType, size/align/count, LateRequireCapability rows. |
| F-006 | fixed | Manifest lists the six watched passes (lines 1013-1018). | Source paragraph now says a subset is watched. |
| F-007 | fixed | `_common.md` contract. | Merged intended-reader sentence into first paragraph; removed emphasis on weakly. |
