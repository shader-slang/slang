---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:59:15Z
target_doc: ast-reference/values.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: cd5fe829a30362fa6bacf71395cf957c34b3269ebbdb651fde1109f4bea47fac
source_commit: 56b1b0b83fb96f88f36da7e37cd9c2c7f27af70c
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
  major: 0
  minor: 4
  nit: 0
---

# Review report for ast-reference/values.md

## Summary

The document covers all 62 concrete non-Type classes, and all explicit line-number citations match their source locations. It needs localized corrections and contract cleanup. Most notably, the worked example compares identically spelled array bounds while claiming to demonstrate equality between differently spelled bounds.

## Items checked

- Verified against the **current working tree**, which the operator confirms equals the document’s source commit throughout `source/`.
- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show ast-reference/values.md`.
- Compared all 62 concrete `FIDDLE()` classes against the node rows: no missing or extra classes.
- Checked more than 10 substantive claims, including decl-ref operands and folding, integer type slots, polynomial representation, witness operands, modifier inheritance, differentiation operands, capability bitmasks, link-time constants, equality, substitution caching, and emitter behavior.
- Verified all 21 explicit source line-number citations, including all seven inheritance call sites. No citation drift found.
- Resolved all 46 Markdown link occurrences, covering 24 unique destinations; checked linked files at the recorded commit, peer manifest membership, and fragment targets. Read the dependency page only for cross-reference checks.
- Checked required headings, table columns, front-matter syntax and keys, Mermaid conventions, Liquid safety, and size: 36,089 bytes against a 49,152-byte cap.
- No files were written; executable examples were not run.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Notable nodes` → `### TypeEqualityWitness`, lines 458–472 | `Every construction site funnels through SemanticsVisitor::createTypeEqualityWitness` overstates the otherwise accurate inventory of semantic-checking callers. Substitution constructs a witness directly through the builder cache. | `source/slang/slang-ast-val.cpp:312–319`: `TypeEqualityWitness::_substituteImplOverride` calls `astBuilder->getOrCreate<TypeEqualityWitness>(type, type)`. | Restrict the statement to the listed semantic-checking creation sites and mention that substitution rebuilds the witness directly through `getOrCreate`. |
| F-002 | minor | `### Hash-consing and the ASTBuilder`, lines 537–553 | The explanation says `c = a` demonstrates canonicalization of two different spellings. However, both `c` and `a` have the bound `2*N+3`; only `b` uses `3+2*N`. That assignment does not exercise the distinction the paragraph claims it proves. | Target lines 540–541 contain the identical bounds. `source/slang/slang-ast-base.h:434–437` compares resolved identities; polynomial factor and term sorting occur in `source/slang/slang-ast-val.cpp:1840` and `:1873`. | Use an assignment between `b` and `c`, or give `c` the alternate bound. Remove the historical execution claim for any revised example unless that exact example is verified. |
| F-003 | minor | `## Nodes`, lines 175–376, especially line 257 | The catalog is complete but does not follow the required single-table structure. The `DiffTypeInfoWitness` row also omits the types of `thisTypeDiffWitness`, `returnTypeDiffWitness`, and the parameter-witness operands. | `docs/generated/design/_meta/prompts/_common.md`, AST reference contract item 4, requires one table and `name: Type` cells. `source/slang/slang-ast-val.h:1018–1024` declares all three witness accessor categories as `SubtypeWitness*`. | Consolidate the node rows into one table, moving family explanations outside it. Add `SubtypeWitness` types to the omitted witness operands, following the table’s existing pointer-omission convention. |
| F-004 | minor | `### Modifier values`, lines 294–338 | The detailed walkthrough of attributed-type emission and SPIR-V image-format selection exceeds the AST reference contract’s prohibition on IR-level information. These details are supported by source, but belong outside this page’s node reference. | `docs/generated/design/_meta/prompts/_common.md`, AST reference forbidden-content paragraph; `source/slang/slang-emit-c-like.cpp:484–493` handles `IRAttributedType`, and `source/slang/slang-emit-spirv.cpp:3347–3380` selects image formats from IR attributes. | Retain the AST modifier-value semantics and a short statement that target handling varies. Replace the backend walkthrough with the existing link to `../pipeline/06-emit.md`. |