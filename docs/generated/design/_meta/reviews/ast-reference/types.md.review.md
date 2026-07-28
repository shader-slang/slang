---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:26:37+00:00
target_doc: ast-reference/types.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 69b1b4b4ebfd2e31c72694cb96999536c58a4466e09db936fc79e843d50fc2ad
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 2
  minor: 2
  nit: 0
---

# Review report for ast-reference/types.md

## Summary
The page is useful, but it does not fully satisfy the AST reference contract. The most important issue is that the `## Nodes` table often says only `(operand-encoded)` where the prompt requires concrete key-field data, so readers lose the data shape for central types such as `FuncType`, `VectorExpressionType`, and `ExtractExistentialType`.

## Items checked
- Ran `regenerate.py show ast-reference/types.md` and reviewed the target document, `_common.md`, `ast-reference-types.md`, and dependency docs `ast-reference/base.md` and `ast-reference/values.md`.
- Checked the resolved watched files `source/slang/slang-ast-base.h`, `source/slang/slang-ast-type.h`, and `source/slang/slang-ast-val.h`.
- Spot-checked more than 10 source-backed claims: `Val`, `Type`, `m_operands`, `DeclRefType`, `BasicExpressionType`, `VectorExpressionType`, `MatrixExpressionType`, `FuncType`, `SubpassInputType`, `SamplerStateType`, `BuiltinGenericType`, `ExtractExistentialType`, `ThisType`, `AndType`, and pack types.
- Verified the required AST reference sections are present and checked the relative links seen in the body against existing generated docs or workspace files.
- Checked front matter for required keys, the requested source commit, and the document's recorded watched-path digest.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Family hierarchy` | The hierarchy diagram starts with `Val --> Type` and omits the required `NodeBase --> Val` edge, even though the per-doc prompt says the section must make `NodeBase -> Val -> Type -> <concrete types>` explicit. | `docs/generated/design/_meta/prompts/ast-reference-types.md:12-16` requires that relationship; `source/slang/slang-ast-base.h:379-380` declares `Val : public NodeBase`, and `source/slang/slang-ast-base.h:568-570` declares `Type : public Val`. | Add `NodeBase --> Val` to the diagram, or otherwise show the full `NodeBase -> Val -> Type` chain in `## Family hierarchy`. |
| F-002 | minor | `## Family hierarchy` | The diagram puts `SubpassInputType`, `SamplerStateType`, and `BuiltinGenericType` under `ResourceType`, but the source declares all three directly under `BuiltinType`. The table later gives the correct parent for two of them, so the page is internally inconsistent. | `source/slang/slang-ast-type.h:279-287` declares `SubpassInputType` and `SamplerStateType` as `BuiltinType`; `source/slang/slang-ast-type.h:294-300` declares `BuiltinGenericType : public BuiltinType`. | Move those diagram edges under `BuiltinType`; keep only `TextureTypeBase` under `ResourceType` unless another source-declared resource subclass is shown. |
| F-003 | major | `## Nodes` table | The `Key fields` column frequently uses only `(operand-encoded)`, which does not meet the family contract's required `name: Type` data-shape format. This hides concrete operand semantics for central rows such as `FuncType`, `VectorExpressionType`, `MatrixExpressionType`, and `ExtractExistentialType`. | `docs/generated/design/_meta/prompts/_common.md:99-108` requires `Key fields` in `name: Type` form. `source/slang/slang-ast-type.h:958-966` stores `FuncType` parameter/result/error operands, `source/slang/slang-ast-type.h:715-723` exposes vector element/count accessors, and `source/slang/slang-ast-type.h:1229-1242` exposes `ExtractExistentialType` operands. | Replace vague cells with concrete operand semantics, for example `paramTypes: Type*, result: Type*, error: Type*`, `elementType: Type*, elementCount: IntVal*`, and `declRef: DeclRef<VarDeclBase>, originalInterfaceType: Type*, originalInterfaceDeclRef: DeclRef<InterfaceDecl>`. |
| F-004 | minor | opening paragraphs | The universal contract says the first body paragraph must state both what the document covers and who the intended reader is. The first paragraph only says it is a reference for concrete `Type` subclasses; the intended-reader statement is separated into a later `Audience:` paragraph. | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first paragraph to include both coverage and intended reader. | Fold the audience phrase into the first paragraph, or rewrite the first paragraph so it includes both the coverage and the intended reader. |
