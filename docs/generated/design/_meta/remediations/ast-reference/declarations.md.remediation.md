---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:06:00Z
target_doc: ast-reference/declarations.md
review_report: ../../reviews/ast-reference/declarations.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/declarations.md

## Summary

All six findings were verified and fixed. The forbidden-content finding
was applied narrowly, to the two spans the reviewer named concretely,
because the same prompt separately requires short statements about which
phase synthesizes a node. The others added the missing `AssocTypeDecl`
callout and the `VarDeclBase` branches, corrected the `EnumDecl` and
`optional`-modifier claims, and merged the audience sentence into the
first paragraph. The front matter was not touched.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `docs/generated/design/_meta/prompts/ast-reference-declarations.md:63-64` forbids type-checking semantics, but lines 29-31, 40-41, and 51-52 of the same prompt require saying *that* a node is synthesized during checking. So the rule bars checking rules, not producer attribution: the `SynthesizedStructDecl`/`InheritanceDecl` lines (216-228) and the `ModuleDecl` visibility text (258-265), which already defers rules to `visibility.md`, stay. The two spans the reviewer cited by name were genuine rule exposition and were removed. | Dropped the `-experimental-feature` gating sentence from `### FuncExtensionDecl`; compressed `### FuncConstraintDecl` to its `sub`/`sup`/`callableRequirementDeclRef` shape, pointing at `pipeline/03-semantic-check.md` for the synthesis rules |
| F-002 | fixed | Confirmed: prompt lines 53-54 require the callout; `source/slang/slang-ast-decl.h:566-570` declares `AssocTypeDecl : public AggTypeDecl`, and only the Nodes-table row and one incidental mention existed. | Added a five-sentence `### AssocTypeDecl` callout before the constraint callout, covering the `AggTypeDecl` base and `parseAssocType` (`source/slang/slang-parser.cpp:4293-4315`) |
| F-003 | fixed | Confirmed: `optional` is read at `source/slang/slang-parser.cpp:1939`, and `OptionalConstraintModifier` is attached only at `:1952`, `:1970`, `:2025`, and `:2042`. The `TypeCoercionConstraintDecl` branch at `:2046-2058` handles only `ImplicitConversionModifier`. | Claim narrowed to the subtype, equality, `nonempty`, and pack-count branches, noting the coercion branch consumes `optional` without recording it |
| F-004 | fixed | Confirmed: `source/slang/slang-ast-decl.h:444-473` gives `EnumDecl` only `tagType`, and `EnumCaseDecl` only `type`, `tagExpr`, and `tagVal` — no variant payload. | "tagged-union form" replaced with "enumeration form", describing `EnumCaseDecl` children as carrying a tag expression and value |
| F-005 | fixed | Prompt line 17 requires the `VarDeclBase` branches; `source/slang/slang-ast-decl.h:339,346,583,597,605,1081,1089` declares them. | Added `VarDeclBase` edges to `VarDecl`, `ParamDecl`, `GlobalGenericValueParamDecl`, `GenericValueParamDecl`, `GenericValuePackParamDecl`, plus `VarDecl -> LetDecl` and `ParamDecl -> ModernParamDecl` |
| F-006 | fixed | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first paragraph to state coverage and intended reader. | Audience sentence merged into the opening paragraph and the separate `Audience:` paragraph removed |
