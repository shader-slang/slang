---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:28:00+00:00
target_doc: ast-reference/values.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 59d9c296dab00215747c1612d4e02a2ca0687dc420a756bf65f6af88fc474010
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 1
  minor: 2
  nit: 0
---

# Review report for ast-reference/values.md

## Summary
The page covers the non-Type `Val` class list well, but several operand summaries are not aligned with `slang-ast-val.h`. The most important finding is that some rows describe different operand types or shapes than the constructors and accessors actually expose.

## Items checked
- Ran `regenerate.py show ast-reference/values.md` and reviewed the target document, `_common.md`, `ast-reference-values.md`, and dependency doc `ast-reference/base.md`.
- Checked the resolved watched files `source/slang/slang-ast-base.h` and `source/slang/slang-ast-val.h`.
- Spot-checked more than 10 source-backed claims: `Val`, `DeclRefBase`, `DirectDeclRef`, `LookupDeclRef`, `GenericAppDeclRef`, `ConstantIntVal`, `BuiltinOperationIntVal`, `PolynomialIntValFactor`, `PolynomialIntValTerm`, `PolynomialIntVal`, `SubtypeWitness`, `HigherOrderDiffTypeTranslationWitness`, `HasDiffTypeInfoWitness`, `DifferentiateVal`, and `UIntSetVal`.
- Verified the concrete class list in `slang-ast-val.h` is represented in the `## Nodes` section and that no concrete `Type` subclass is listed in the tables.
- Checked front matter for required keys, the requested source commit, and the document's recorded watched-path digest.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Nodes` tables | Several operand summaries contradict the watched header. For example, `LookupDeclRef` is described as `base: DeclRefBase`, `requirementKey`, and witness operands, but the source stores `declToLookup`, `lookupSource: Type`, and `witness: SubtypeWitness`; `PolynomialIntValFactor` says `param: DeclRefBase`, but the source stores an `IntVal*`; `HigherOrderDiffTypeTranslationWitness` says function-type operands, but the source exposes `baseWitness`; and `HasDiffTypeInfoWitness` says type operand, but the source stores a `DeclRef<HasDiffTypeInfoConstraintDecl>`. | `source/slang/slang-ast-val.h:68-76`, `source/slang/slang-ast-val.h:530-535`, `source/slang/slang-ast-val.h:1034-1040`, and `source/slang/slang-ast-val.h:1112-1122`. | Correct the affected operand cells to match the constructor/accessor names and types in the header. |
| F-002 | minor | `## Nodes` table headers | The AST reference family contract requires the standard table columns `Class`, `Parent`, `Key fields`, `Grammar`, and `Summary`. This page uses `Operand semantics` instead of `Key fields` throughout, even though its own introductory text still calls the column `"Key fields"`. | `docs/generated/design/_meta/prompts/_common.md:99-108` defines the required columns; `docs/generated/design/ast-reference/values.md:31-37` says the `"Key fields"` column lists operand slot semantics, but the tables use `Operand semantics`. | Rename the column to `Key fields` and keep operand-slot descriptions in the cells, or update the prompt if this page is intentionally allowed to deviate. |
| F-003 | minor | opening paragraphs | The universal contract says the first body paragraph must state both what the document covers and who the intended reader is. The first paragraph describes the non-Type `Val` families but leaves the reader information to a separate `Audience:` paragraph. | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first paragraph to include both coverage and intended reader. | Fold the audience phrase into the first paragraph so the first paragraph satisfies the universal content rule. |
