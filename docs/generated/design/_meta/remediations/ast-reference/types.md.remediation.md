---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:59:36Z
target_doc: ast-reference/types.md
review_report: ../../reviews/ast-reference/types.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 1
  rejected_bogus: 3
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/types.md

## Summary
Four findings reviewed against `source/slang/slang-ast-type.h` and `source/slang/slang-ast-base.h`. F-001, F-002, F-004 rejected as bogus: each describes a document state that contradicts the current page (the diagram already has both required edges; no separate `Audience:` paragraph exists). F-003 fixed: although its four named rows were already concrete, the underlying contract requirement (`_common.md:106`, `name: Type` Key fields) still applied to several other rows that expose source accessors but used `(operand-encoded)`. Front-matter untouched; `target_doc_source_commit_after` equals `_before` per convention since only body table cells changed.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | Diagram already contains the `NodeBase --> Val` edge: `docs/generated/design/ast-reference/types.md:36` reads `NodeBase --> Val`, line 37 `Val --> Type` — the full `NodeBase -> Val -> Type` chain required by `_meta/prompts/ast-reference-types.md:12-16` is present. | — |
| F-002 | rejected-bogus | Diagram places all three under `BuiltinType`, matching source: `types.md:74-76` read `BuiltinType --> SubpassInputType/SamplerStateType/BuiltinGenericType`; confirmed at `source/slang/slang-ast-type.h:279,287,296`. No `ResourceType` edge for them exists. | — |
| F-003 | fixed | Contract `_meta/prompts/_common.md:106` requires `name: Type` Key fields. The four reviewer-named rows were already concrete; surfaced concrete operands for further accessor-exposing rows verified in `source/slang/slang-ast-type.h` (lines 553-577, 586, 796, 902, 1066, 1343). | Rows `DeclRefType`, `ArrayExpressionType`, `TupleType`, `ConditionalType`, `AtomicType`, `OptionalType`, `CoopVectorExpressionType`, `PtrTypeBase`, `PtrType`, `ModifiedType`: replaced `(operand-encoded)` with concrete `name: Type` fields. |
| F-004 | rejected-bogus | No separate `Audience:` paragraph exists. The first body paragraph (`types.md:12-14`) already states both coverage ("every concrete `Type` subclass") and reader ("a contributor reading checker or IR-lowering code"), satisfying `_meta/prompts/_common.md:65-66`. | — |
