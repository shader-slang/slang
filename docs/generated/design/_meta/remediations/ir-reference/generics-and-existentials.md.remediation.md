---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:09:22Z
target_doc: ir-reference/generics-and-existentials.md
review_report: ../../reviews/ir-reference/generics-and-existentials.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/generics-and-existentials.md

## Summary

All six findings were verified against the current source and fixed in the document with minimal edits. The document stays at about 53 KB, under the 65536-byte cap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | source/slang/slang-lower-to-ir.cpp:12346 uses setOperand; slang-ir.cpp:5325-5336 adds entries as globals | interface requirement entries described as operands referencing global entries, not children |
| F-002 | fixed | source/slang/slang-ir.cpp:5023-5055 requires slotArgCount >= 2 and returns value for DynamicType | wrapExistential paragraph states slot-count condition and DynamicType identity case |
| F-003 | fixed | FuncTypeOf is produced in lowering; the table also has unproduced dispatchers and SpecializeExistentialsInType from slang-ir-specialize.cpp:3214 | exclusivity claim replaced with "produced during AST lowering" |
| F-004 | fixed | slang-ir-insts.h:3009-3011 shows IRSetBase is abstract | "Twenty concrete wrappers ... together with the abstract IRSetBase" |
| F-005 | fixed | slang-ir-lower-dynamic-dispatch-insts.cpp:1114-1120 only registers processing; construction is at 1245 and 1290 | dropped lines 1115 and 1119 from the producer cells |
| F-006 | fixed | _common.md IR-reference contract requires the slang-ir.h link | added slang-ir.h link in the Source section |
