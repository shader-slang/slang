---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:40:00+00:00
target_doc: name-resolution/scopes.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 26030eb29f1e4941372a37ba0ce2800a984b637cb1b9877ad3dbf32d012bd625
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 0
  major: 4
  minor: 1
  nit: 0
---

# Review report for name-resolution/scopes.md

## Summary
The scopes page has valid front matter, resolving peer links, and the required top-level sections, but several rule-level claims do not match the source. The most important issue is that the scope-bearing node table treats inherited fields and parser-populated scopes as the same thing, which both omits real `ContainerDecl` scope owners and claims fresh scopes for statements whose parser paths do not populate `scopeDecl`.

## Items checked
- Ran `regenerate.py show name-resolution/scopes.md` and checked the per-doc prompt, size cap, dependencies, watched paths, and resolved files.
- Read the target document, `_review.md`, `_common.md`, `name-resolution-scopes.md`, and the three depends-on AST reference docs.
- Verified the target front matter and section order against the name-resolution family contract.
- Resolved peer links to `lookup.md`, `visibility.md`, `overload-resolution.md`, AST reference pages, the parsing pipeline page, and the glossary.
- Checked source claims across all nine watched files, including `Scope`, `ContainerDecl::ownedScope`, `ScopeStmt::scopeDecl`, parser push/pop helpers, block/for/if-let/catch/lambda parsing, sibling-scope wiring, lookup walk order, and edge-case diagnostics.
- Spot-checked more than 10 factual/source-alignment claims and verified the document's line-number citations; several cited lines are stale.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Scope-bearing AST nodes`, lines 80-99 | The scope-bearing table does not name every concrete watched-header class that carries `ContainerDecl::ownedScope`; it uses broad parent rows and ellipses instead. Required concrete scope owners such as `SemanticDecl` and `AttributeDecl` are absent, and several `AggTypeDecl`/`CallableDecl` subclasses are only implied. | `source/slang/slang-ast-decl.h:732` declares `SemanticDecl : public ContainerDecl`, `source/slang/slang-ast-decl.h:1102` declares `AttributeDecl : public ContainerDecl`, and `source/slang/slang-ast-decl.h:420`, `source/slang/slang-ast-decl.h:434`, `source/slang/slang-ast-decl.h:567`, `source/slang/slang-ast-decl.h:575`, and `source/slang/slang-ast-decl.h:653` declare additional concrete subclasses inheriting an owned scope through `AggTypeDecl` or `CallableDecl`. | Expand the table or add grouped rows that explicitly name every concrete scope-owning class required by the prompt, including `SemanticDecl`, `AttributeDecl`, `SynthesizedStructDecl`, `GLSLInterfaceBlockDecl`, `AssocTypeDecl`, `GlobalGenericParamDecl`, and `FuncAliasDecl`. |
| F-002 | major | `### Scope-bearing AST nodes`, lines 77-98 | The text says every listed node introduces a fresh `Scope`, but some listed `ScopeStmt` subclasses do not have parser-populated statement scopes. `WhileStmt` and `DoWhileStmt` parse without creating or assigning a `ScopeDecl`; `SwitchStmt` gets a scoped block body rather than its own `scopeDecl`; `TargetSwitchStmt` and `StageSwitchStmt` create per-case `ScopeDecl`s without assigning `stmt->scopeDecl`; and `UnscopedForStmt` explicitly skips the scope push for HLSL. | `source/slang/slang-parser.cpp:7070` through `source/slang/slang-parser.cpp:7079` parse `WhileStmt` without scope creation, `source/slang/slang-parser.cpp:6186` through `source/slang/slang-parser.cpp:6195` parse `SwitchStmt` by calling `parseBlockStatement()`, `source/slang/slang-parser.cpp:6228` through `source/slang/slang-parser.cpp:6230` create case-local target-switch scopes, and `source/slang/slang-parser.cpp:7014` through `source/slang/slang-parser.cpp:7033` skip `pushScopeAndSetParent(scopeDecl)` for HLSL `UnscopedForStmt`. | Split the table into "declares/inherits a scope field" versus "parser creates a fresh scope", or revise the affected rows so they describe the actual parser behavior for while/do, switch, target/stage switch, and HLSL `UnscopedForStmt`. |
| F-003 | major | `### Scope-bearing AST nodes`, line 99 | The `LambdaDecl` row says the parser pushes `lambdaExpr->paramScopeDecl` as a dedicated `LambdaDecl` parameter scope, but that parameter-list scope belongs to the lambda expression path, not to `LambdaDecl` in `slang-ast-decl.h`. `LambdaDecl` itself is a `StructDecl` with only `funcDecl` declared in this header, so the row conflates two different scope owners. | `source/slang/slang-ast-decl.h:682` declares `LambdaDecl : public StructDecl` and `source/slang/slang-ast-decl.h:686` declares only `funcDecl`; the parser creates and pushes `lambdaExpr->paramScopeDecl` at `source/slang/slang-parser.cpp:8159` through `source/slang/slang-parser.cpp:8163`. | Change the row to describe `LambdaDecl` only as an aggregate/container scope, or expand watched paths and add a separate `LambdaExpr::paramScopeDecl` discussion if expression-level lambda parameter scopes should be covered. |
| F-004 | major | `### Sibling scopes` and `## Edge cases and failure modes`, lines 175-199 and 305-313 | The sibling-scope section says only three concrete uses are visible, and the `UsingDecl` edge case does not describe how `using` appears in the scope chain. In source, `SemanticsDeclScopeWiringVisitor::visitUsingDecl` imports namespace/module scopes by adding sibling scopes, and it diagnoses invalid non-namespace arguments with `Diagnostics::ExpectedANamespace`. | `source/slang/slang-check-decl.cpp:16442` through `source/slang/slang-check-decl.cpp:16449` add sibling scopes for `UsingDecl`, and `source/slang/slang-check-decl.cpp:16475` through `source/slang/slang-check-decl.cpp:16479` emit `Diagnostics::ExpectedANamespace` when no valid namespace is found. | Add `UsingDecl` as a sibling-scope use, describe that it injects namespace/module scopes by calling `addSiblingScopeForContainerDecl`, and include the invalid-argument diagnostic in the edge-case bullet. |
| F-005 | minor | line-number citations throughout | Several line-number citations are stale by far more than a few lines, so following them lands on unrelated code. Examples include `parseIfLetStatement` cited at line 6820, `ParseForStatement` cited at line 6928, `parseUsingDecl` cited at line 4198, and import/namespace sibling wiring cited around lines 16061/16408. | `source/slang/slang-parser.cpp:6820` is inside `parseBlockStatement`, while `parseIfLetStatement` starts at `source/slang/slang-parser.cpp:6897`; `ParseForStatement` starts at `source/slang/slang-parser.cpp:7005`; `parseUsingDecl` starts at `source/slang/slang-parser.cpp:4269`; module-import sibling wiring is at `source/slang/slang-check-decl.cpp:16163`, and namespace sibling wiring is at `source/slang/slang-check-decl.cpp:16510`. | Refresh all line-number citations against `eb9403ef595a99c2ff6def1d538dbd7a792d9371`, or cite function names without exact line numbers where the generated docs cannot keep them stable. |
