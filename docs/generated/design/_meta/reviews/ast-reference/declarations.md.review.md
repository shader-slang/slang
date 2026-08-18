---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:41:52+00:00
target_doc: ast-reference/declarations.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 40846d6323a4545ce1013f919b025bb0a96aea7d0df6f90a941d573b1467ac6d
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 6
severity_breakdown:
  critical: 0
  major: 2
  minor: 3
  nit: 1
---

# Review report for ast-reference/declarations.md

## Summary

The declaration catalog is structurally strong: all concrete classes are covered and all links resolve. The most important issue is that much of `## Notable nodes` documents semantic-checking behavior that the page prompt explicitly forbids. The page also omits the required `AssocTypeDecl` callout and contains two source-level inaccuracies.

## Items checked

- Compared the table with `source/slang/slang-ast-decl.h` at the recorded commit: all 62 concrete `FIDDLE()` classes are present, and all 12 `FIDDLE(abstract)` classes are excluded.
- Spot-checked more than 10 claims, including parents and fields for `DeclGroup`, `VarDecl`, `ExtensionDecl`, `StructDecl`, `SynthesizedStructDecl`, `EnumDecl`, `EnumCaseDecl`, `InheritanceDecl`, `ConstructorDecl`, `LambdaDecl`, `ModuleDecl`, `GenericDecl`, `InterfaceDefaultImplDecl`, `GenericTypeConstraintDecl`, `FuncConstraintDecl`, `SyntaxDecl`, and `AttributeDecl`.
- Checked parser behavior around `ParseDecl`, `parseEnumDecl`, `parseNamespaceDecl`, `parseGenericDecl`, `parseFuncExtensionDecl`, `parseAccessorDecl`, `parseAssocType`, `parseInterfaceConstraintDecl`, `maybeParseGenericConstraints`, `isDeclAllowed`, and `populateBaseLanguageModule`.
- Validated all 82 relative links and grammar anchors at the recorded commit; none are dangling.
- Swept 175 identifier-like backtick citations against `source/`; all cited identifiers exist.
- Verified that the body contains no line-number citations, so there were no numeric citations to re-derive.
- Recomputed the watched-path digest as `40846d6323a4545ce1013f919b025bb0a96aea7d0df6f90a941d573b1467ac6d`, matching the front matter.

## Findings

| ID    | Severity | Location                                                                    | Description                                                                                                                                                                                                                                                           | Evidence                                                                                                                                                                                                    | Recommendation                                                                                                                                                                                     |
| ----- | -------- | --------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | major    | `## Notable nodes`, especially lines 189-202, 216-228, 258-265, and 285-377 | Large portions explain semantic-checking behavior, including feature gating, desugaring, witness construction, visibility inheritance, and synthesized differentiability constraints. The per-doc contract explicitly forbids type-checking semantics on this page.   | `docs/generated/design/_meta/prompts/ast-reference-declarations.md` lines 59-64. Examples include `semantic checking desugars` at target lines 200-202 and the checker/witness discussion at lines 360-377. | Remove checker-specific behavior and keep each callout focused on the node's AST shape and parser production. Retain links to `pipeline/03-semantic-check.md` for the omitted semantic details.    |
| F-002 | major    | `## Notable nodes`, lines 285-357                                           | The required `RequirementDecl` / `AssocTypeDecl` callout is not supplied. The page explains that no `RequirementDecl` class exists, but `AssocTypeDecl` only appears incidentally in longer constraint prose rather than receiving the required 2-5 sentence callout. | `docs/generated/design/_meta/prompts/ast-reference-declarations.md` lines 33-54 requires this callout; `source/slang/slang-ast-decl.h` lines 567-571 declares `AssocTypeDecl`.                              | Add a concise `AssocTypeDecl` callout in the required order, explaining its AST role and parser production without checker semantics; keep the note that `RequirementDecl` is not a current class. |
| F-003 | minor    | `### GenericTypeConstraintDecl as an interface requirement`, lines 321-322  | `Every spelling except __hasDiffTypeInfo ... attaches an OptionalConstraintModifier` is false for `TypeCoercionConstraintDecl`: the parser consumes leading `optional` but never attaches that modifier in the coercion branch.                                       | `source/slang/slang-parser.cpp` lines 1937-1939 reads `optional`; lines 2046-2058 constructs `TypeCoercionConstraintDecl` and only handles `ImplicitConversionModifier`.                                    | Exclude coercion constraints from the modifier claim, or state precisely which constraint branches attach `OptionalConstraintModifier`.                                                            |
| F-004 | minor    | `### AggTypeDecl, StructDecl, ClassDecl, EnumDecl`, lines 206-210           | Calling `EnumDecl` the `tagged-union form` is misleading. The AST stores ordinary enum cases with optional tag expressions and values; it has no variant payload shape.                                                                                               | `source/slang/slang-ast-decl.h` lines 445-473 and `source/slang/slang-parser.cpp` lines 6467-6564.                                                                                                          | Replace `tagged-union form` with `enumeration form` and describe its `EnumCaseDecl` children without implying sum-type payloads.                                                                   |
| F-005 | minor    | `## Family hierarchy`, lines 41-79                                          | The hierarchy does not show the concrete branches under `VarDeclBase`, although the per-doc prompt explicitly requires that part of the diagram.                                                                                                                      | `docs/generated/design/_meta/prompts/ast-reference-declarations.md` lines 14-20; `source/slang/slang-ast-decl.h` lines 321-359, 583-607, and 1081-1093 declare the relevant subclasses.                     | Add grouped edges from `VarDeclBase` to its variable, parameter, global generic value, and generic value parameter subclasses.                                                                     |
| F-006 | nit      | Introductory paragraphs, lines 12-21                                        | The first paragraph explains coverage but defers the intended reader to a separate `Audience:` paragraph, contrary to the universal requirement that the first paragraph state both.                                                                                  | `docs/generated/design/_meta/prompts/_common.md` lines 60-66.                                                                                                                                               | Merge the audience sentence into the first paragraph.                                                                                                                                              |

## No-issues notes

- The table includes easy-to-miss concrete nodes such as `FileReferenceDeclBase`, `InterfaceDefaultImplDecl`, `FuncConstraintDecl`, `GenericValuePackParamDecl`, and both semantic accessor declarations.
- Every grammar anchor and peer/source link resolves at the target source commit.
- The generated-doc front matter is complete, and its recorded digest matches the resolved watched files.
