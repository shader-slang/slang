---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:33:05+00:00
target_doc: name-resolution/scopes.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: eae539a7f092c52656f90a6cc15a5dd8219d0bf3e4eb91d3a6e30b34edcab687
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for name-resolution/scopes.md

## Summary
The scopes page is factually aligned with the watched parser, AST, lookup, and semantic-checking sources in the checked sample. The one finding is a formatting artifact: two wrapper-style tags leaked into the end of the generated Markdown body.

## Items checked
- Ran `regenerate.py show name-resolution/scopes.md` and reviewed the target document, the common contract, its per-document prompt, and the resolved watched-file set (`source/slang/slang-ast-base.h`, `source/slang/slang-ast-decl.h`, `source/slang/slang-ast-stmt.h`, `source/slang/slang-check-decl.cpp`, `source/slang/slang-check-expr.cpp`, plus 4 more).
- Checked front matter for all required keys, the recorded source commit, the warning string, and a 64-character hex watched-path digest.
- Resolved the relative links used by the page to peer name-resolution docs, AST-reference dependencies, pipeline docs, glossary, and watched source files.
- Verified required name-resolution sections: Source, Concepts, Rules, Edge cases and failure modes, and See also.
- Spot-checked source-backed claims against the watched files, including `Scope::containerDecl`, `Scope::parent`, `Scope::nextSibling`, `ContainerDecl::ownedScope`, `ScopeStmt::scopeDecl`, `Parser::parseBlockStatement`, `Parser::ParseForStatement`, `Parser::parseIfLetStatement`, `parseLambdaExpr`, `addSiblingScopeForContainerDecl`, `lookUp`, `hiddenFromLookup`, and `findClosestInScopeName`.
- Read the dependency docs listed by the manifest: `ast-reference/base.md`, `ast-reference/declarations.md`, and `ast-reference/statements.md`.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | trailing lines 372-373 | The document body ends with literal wrapper tags, `</content>` and `</invoke>`, after the intended See also section. These are not Markdown content and violate the generated-doc style contract. | `docs/generated/design/name-resolution/scopes.md` lines 372-373 contain the leaked tags; `_common.md` requires Markdown headings/body content and forbids generated artifacts that are not source-backed documentation. | Delete the two trailing wrapper-tag lines so the document ends after the glossary See also bullet. |

## No-issues notes
- The document keeps generated-doc links workspace-relative and avoids absolute source paths.
- The parser scope-construction claims matched the cited implementations for block, for-loop, if-let, catch, target-switch case, generic, namespace, and lambda scopes.
- The sibling-scope discussion matched `addSiblingScopeForContainerDecl` and its semantic-checker call sites for file, module import, namespace, and using-declaration wiring.
