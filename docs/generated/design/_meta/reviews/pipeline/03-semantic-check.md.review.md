---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:35:00+00:00
target_doc: pipeline/03-semantic-check.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 028fc0023a149337cabae05a0de4a7ebf1eec342be28f4f423b6a59e178c6578
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for pipeline/03-semantic-check.md

## Summary

The page covers the required semantic-checking structure and mostly matches the watched checker sources. The main issue is a source-ownership mismatch in the implicit-synthesis section: `slang-ast-synthesis.cpp` provides `ASTSynthesizer` helpers for emitting AST fragments, while the checker routines that synthesize constructors and requirement witnesses live in `slang-check-decl.cpp`. All links resolve, and the detailed generic-inference, body-parsing, diagnostic, and shader-entry claims spot-checked against source were otherwise supported.

## Items checked

- Verified the target front matter, including `source_commit` and `watched_paths_digest`, against the document's own recorded values.
- Ran a targeted relative-link resolution check for all 47 Markdown links in `pipeline/03-semantic-check.md`; all resolved in the workspace.
- Checked that all required sections from `pipeline-03-semantic-check.md` are present, including `## SemanticsVisitor`, `## Two-pass interaction with the parser`, `## Name lookup and DeclRef`, `## Generic specialization and constraints`, `## Synthesizing implicit code`, and `## Failure modes`.
- Verified the watched-file coverage requirement: every `slang-check-*.cpp` file in the manifest is mentioned at least once.
- Spot-checked 20 concrete claims against source: `checkTranslationUnit`, `SharedSemanticsContext`, `SemanticsDeclVisitorBase::checkModule`, `SemanticsVisitor`, `maybeParseStmt`, `parseUnparsedStmt`, `TryCheckOverloadCandidateConstraints`, `trySolveGenericArguments`, `OverloadCandidate::explicitGenericArgCount`, `GenericArgumentInferenceFailure`, `CompleteOverloadCandidate`, `findWitnessForInterfaceRequirement`, `visitGenericTypeConstraintDecl`, `getInheritanceInfo`, `_calcInheritanceInfo`, `_isInheritanceInfoBeingComputed`, `collectGenericStructTypeUses`, `createSpecializedGlobalAndEntryPointsComponentType`, `ExpectATypeRepr`, and `maybeDiagnoseDiscardedNoDiscardResult`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Synthesizing implicit code`, lines 190-198 | The section says default conformance witnesses, generated comparison / construction methods, and built-in conformances are "synthesized in `slang-ast-synthesis.cpp`." That overstates the role of this file: it only defines `ASTSynthesizer` helpers for creating AST expressions/statements, while semantic synthesis routines such as constructor and requirement-witness synthesis are implemented in `slang-check-decl.cpp`. | `source/slang/slang-ast-synthesis.cpp:5-220` defines helpers like `ASTSynthesizer::emitBinaryExpr`, `emitVarExpr`, `emitInvokeExpr`, and `emitVarDeclStmt`. `source/slang/slang-check-decl.cpp:639-645` declares constructor synthesis, and `source/slang/slang-check-decl.cpp:7753` / `source/slang/slang-check-decl.cpp:8238` define requirement-witness synthesis entry points. | Reword the section to say `slang-ast-synthesis.cpp` supplies AST-building helpers used by synthesis, while the semantic checker routines that decide what to synthesize live primarily in `slang-check-decl.cpp`. |

## No-issues notes

- The two-pass parser interaction matches `SemanticsVisitor::maybeParseStmt` moving `UnparsedStmt` tokens into `parseUnparsedStmt`.
- The generic-inference description matches `explicitGenericArgCount`, the fixpoint solver path, and the selected-candidate diagnostic switch in `CompleteOverloadCandidate`.
- The shader-specific callouts match the comments and gates around generic-struct capability collection and unspecialized generic entry-point rejection.
