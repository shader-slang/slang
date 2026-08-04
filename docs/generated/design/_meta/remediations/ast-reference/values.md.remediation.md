---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:58:31Z
target_doc: ast-reference/values.md
review_report: ../../reviews/ast-reference/values.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 3
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/values.md

## Summary
All three findings were verified against the watched header `source/slang/slang-ast-val.h` and the target doc at HEAD and found bogus. F-001's four operand sub-claims each describe text the doc does not contain; every cited row already matches the source. F-002's `Operand semantics` header does not exist (all tables use `Key fields`). F-003's separate `Audience:` paragraph does not exist (the first paragraph already names coverage and reader). No edits were applied; `target_doc_source_commit_after` equals `_before`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | All four cited rows already match the header. `LookupDeclRef` (values.md:133) lists `declToLookup: Decl`, `lookupSource: Type`, `witness: SubtypeWitness` — matches `slang-ast-val.h:68-77`; never says base/requirementKey. `PolynomialIntValFactor` (values.md:174) lists `param: IntVal` — matches `getParam()->IntVal*` at `slang-ast-val.h:530`; never says `DeclRefBase`. `HigherOrderDiffTypeTranslationWitness` (values.md:202) lists `baseWitness: Witness` — matches `getBaseWitness()` at `slang-ast-val.h:1039`; never says function-type. `HasDiffTypeInfoWitness` (values.md:216) lists `declRef: DeclRef<HasDiffTypeInfoConstraintDecl>` — matches `slang-ast-val.h:1116-1121`; never says "type operand". | — |
| F-002 | rejected-bogus | All nine `## Nodes` header rows read the standard columns Class, Parent, Key fields, Grammar, Summary (values.md:129,143,172,186,206,213,228,244,255), matching `_common.md:104`. The string `Operand semantics` does not occur in the document. | — |
| F-003 | rejected-bogus | The first body paragraph (values.md:12-17) states both coverage ("the reference for the non-Type `Val` subclasses...") and intended reader ("It is for a contributor reading or writing checker / IR-lowering code..."), satisfying `_common.md:65-66`. No separate `Audience:` paragraph exists. | — |
