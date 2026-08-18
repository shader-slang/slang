---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:17:03+00:00
target_doc: ast-reference/base.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: ee1fb3c40342ae89da97ad9ebe3c1853ce01e7825a3528db059959204e5a2fc6
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 0
  minor: 3
  nit: 1
---

# Review report for ast-reference/base.md

## Summary

The page is largely accurate and structurally complete, with no critical or major findings. The main factual issue is that it calls `Scope` “not parsed,” although the parser creates scopes and attaches them to parsed declarations. Three smaller findings concern explicit requirements in the per-document prompt.

## Items checked

- Verified 32 factual claims against the three watched headers at commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`, including every hierarchy edge, all ten root field summaries, and 11 support-type/API descriptions.
- Confirmed the document body contains no line-number citations; therefore there were no numbered citations requiring individual re-derivation.
- Resolved all 50 relative-link occurrences (14 unique targets) and confirmed all nine generated-doc targets are manifest entries.
- Read the `pipeline/02-parse-ast.md` dependency and checked parser-related claims against its recorded source context.
- Recomputed the watched-path digest as `ee1fb3c40342ae89da97ad9ebe3c1853ce01e7825a3528db059959204e5a2fc6`, matching the target front matter.
- Compared section order, root order, support-type coverage, and page scope against `_common.md` and `ast-reference-base.md`.

## Findings

| ID    | Severity | Location                                                             | Description                                                                                                                                                                                                                                                | Evidence                                                                                                                                                                                                                                                 | Recommendation                                                                                                                                          |
| ----- | -------- | -------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | minor    | `## Root hierarchy`, lines 72-74; `## Support types`, line 261       | The page says `Scope` “does not appear in the parsed AST” and is “not parsed.” `Scope` is not a syntax node, but the parser does create it and stores it on parsed `ContainerDecl` nodes, so the unqualified wording is inaccurate.                        | `source/slang/slang-parser.cpp:144-150` creates a `Scope` in `Parser::PushScope` and assigns it to `containerDecl->ownedScope`; `source/slang/slang-ast-decl.h:141` declares that field.                                                                 | Replace both claims with “not a syntax node; created during parsing to support lookup and attached to container declarations.”                          |
| F-002 | minor    | `### DeclBase (ModifiableSyntaxNode)`, line 133                      | The page gives `DeclBase` the family line `[declarations.md](declarations.md)`, but the per-document prompt explicitly classifies `DeclBase` as having no dedicated family page.                                                                           | `docs/generated/design/_meta/prompts/ast-reference-base.md:42-46` requires `(no dedicated family page)` for `DeclBase`.                                                                                                                                  | Change the `DeclBase` family line to `Family page: (no dedicated family page)`.                                                                         |
| F-003 | minor    | `### DeclBase`, lines 123-127; `## Support types`, lines 249 and 259 | The page names concrete leaf nodes such as `DeclGroup`, `FuncDecl`, `StructDecl`, `BuiltinOperatorExpr`, and `BuiltinOperationIntVal`, despite the prompt’s explicit rule that concrete leaves belong in family pages rather than this abstract-root page. | `docs/generated/design/_meta/prompts/ast-reference-base.md:61-62` forbids concrete leaves; `source/slang/slang-ast-decl.h:18-22` and `source/slang/slang-ast-expr.h:288-293` show `DeclGroup` and `BuiltinOperatorExpr` are concrete `FIDDLE()` classes. | Remove concrete class examples from the root prose and support table, or replace them with generic descriptions and links to the relevant family pages. |
| F-004 | nit      | `### NodeBase`, lines 78-100; `### Stmt`, lines 166-174              | Each of these root callouts has only one prose sentence describing the class, while the required structure asks for 2-4 sentences per root.                                                                                                                | `docs/generated/design/_meta/prompts/ast-reference-base.md:32-41` defines the required callout form and sentence count.                                                                                                                                  | Add one concise role sentence to each callout, moving existing non-field detail from the `NodeBase` bullets into prose where practical.                 |

## No-issues notes

- Every inheritance edge in the Mermaid diagram matches an immediate `: public` relationship in `source/slang/slang-ast-base.h`.
- The ten required root subsections are present in the required order, and their declared field names and types match the source.
- The front matter has every mandatory key, a valid full SHA and hex digest, and the exact required warning.
