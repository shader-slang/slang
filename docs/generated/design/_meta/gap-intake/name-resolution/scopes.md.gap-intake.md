---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:40:39Z
target_doc: name-resolution/scopes.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 6
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for name-resolution/scopes.md

## Summary

No gap was escalated: every behaviour the queue reported is what the
watched parser and checker sources say, so nothing here is a compiler
defect. Five gaps are `fixed` by two consolidated edits — three
paragraphs plus a three-line lambda example appended after the
scope-bearing-nodes table (`767e963b3518`, `f48cf5bdd4a9`,
`2807210bc5bf`), and two paragraphs appended to `### Sibling scopes`
(`467577e95bbb`, `cb2a054d7df2`). One gap (`035f0e259087`, the surface
that selects HLSL parsing) is `deferred`: the mechanism is confirmed,
but it lives entirely in `source/slang/slang-options.cpp`, which is not
in this page's `watched_paths`, and the manifest may not be edited from
this stage. There are no rejections.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 035f0e259087 | deferred | Within the watched paths the choice is only ever *read*: `Parser::ParseForStatement` tests `getSourceLanguage() == SourceLanguage::HLSL` (`source/slang/slang-parser.cpp:7419`) and `parseSourceFile` takes `sourceLanguage` as a parameter (`:9966`); `Linkage::loadSourceModuleImpl` hard-sets `SourceLanguage::Slang` for every `import`ed/`__include`d module and only ever overrides it to GLSL by path (`source/slang/slang-session.cpp:1364-1371`), so HLSL is unreachable on that surface. The observed rule is real but is decided in `source/slang/slang-options.cpp`: `OptionsParser::addInputPath` returns `addInputSlangPath` for any `.slang` path *before* consulting the `langOverride` (`:1841-1849`), which is why `-lang hlsl file.slang` has no effect, while a `.hlsl` path falls through to `findSourceLanguageFromPath` (`:1855-1858`); `-lang` reaches `addInputPath` from `OptionKind::Language` (`:3416-3441`). Blocked on a `watched_paths` expansion to add `source/slang/slang-options.cpp`; the text is one sentence once that lands | — |
| 767e963b3518 | fixed | `Parser::ParseWhileStatement` (`source/slang/slang-parser.cpp:7475`) and `ParseDoWhileStatement` (`:7487`) call `ParseStatement()` with no scope pushed; a declaration in statement position reaches `Parser::parseVarDeclrStatement` (`:7255`), which passes `currentScope->containerDecl` as the parent container (`:7260`) and diagnoses nothing for a `VarDeclBase` (`:7263-7284`). `SemanticsStmtVisitor::visitWhileStmt` (`source/slang/slang-check-stmt.cpp:620`) and `visitDoWhileStmt` (`:270`) check the body as an ordinary statement. The name is also never marked `hiddenFromLookup`, since `visitBlockStmt` sets that flag only on `DeclStmt`s that are direct children of the block body's `SeqStmt` (`:82-116`). Block-bodied contrast pinned by `while-body-local-not-visible-outside.slang` and `do-while-body-local-not-visible-outside.slang` | Added a paragraph after the scope-bearing-nodes table stating that a declaration used as the body of a scope-less statement (`while`, `do`, `if`) is accepted and leaks into the enclosing container, and that only a `{ ... }` body isolates it |
| 2807210bc5bf | fixed | `parseTargetSwitchStmtImpl` (`source/slang/slang-parser.cpp:6611`) creates a `ScopeDecl` and calls `pushScopeAndSetParent` at the top of each case group (`:6622-6623`) and `PopScope` at the bottom of the same loop iteration (`:6702`), so case groups are siblings, not nested. `ParseSwitchStmt` (`:6580`) instead gives the whole body one `parseBlockStatement(AllowCaseDefaultStatements::Allow)` (`:6588`), so a plain `switch` has a single body scope; the after-the-statement half is pinned by `switch-scope-local-not-visible-outside.slang` | Added a paragraph contrasting the two switch forms: one body scope for plain `switch` (visible in textually later cases, gone after the statement) versus a per-case-group `ScopeDecl` for `__target_switch` / `__stage_switch` (not visible from any other case); consolidated with 767e963b3518 and f48cf5bdd4a9 in one edit region |
| f48cf5bdd4a9 | fixed | `parseLambdaExpr` (`source/slang/slang-parser.cpp:8441`) creates `paramScopeDecl` and pushes it at `:8446`, adds each `ParseParameter` to it at `:8449`, parses the body — `parseBlockStatement` or a synthesized `ReturnStmt` — at `:8456-8465`, and pops at `:8467`, so the parameter scope spans exactly the parameter list and body. Both halves pinned by `lambda-param-visible-in-lambda-body.slang` (functional) and `lambda-param-not-visible-outside-lambda.slang` (`E30015` on the bare `p`) | Added a sentence naming the boundary (the parameter scope ends with the lambda expression, and covers both body forms) plus a three-line example mirroring the bundle's verified `applyF((int p) => p * c, 5)` shape; consolidated with 767e963b3518 and 2807210bc5bf |
| 467577e95bbb | fixed | `SemanticsDeclHeaderVisitor::visitImportDecl` splices into `getModuleDecl(decl)->ownedScope` (`source/slang/slang-check-decl.cpp:17112`), so an inner `import` and a file-scope `using namespace` both land on the importing module's own sibling chain. `isOwnModuleOrIncludedFileScope` (`source/slang/slang-check-expr.cpp:333`) admits only `containerDecl == moduleDecl` or a `FileDecl` with `parentDecl == moduleDecl` (`:346-348`), and `importModuleIntoScope` skips everything else (`source/slang/slang-check-decl.cpp:17066-17081`); the `__exported` escape hatch is the recursion at `:17085-17092` | Added a three-module example (C declares `namespace Foo`; A does `import C;` plus `using namespace Foo;`; B does `import A;`) with a note on exactly what B sees, why the namespace and C's files are dropped, and that `__exported import` is the supported way to re-export |
| cb2a054d7df2 | fixed | `_lookUpInScopes` (`source/slang/slang-lookup.cpp:786`) iterates `for (auto link = scope; link; link = link->nextSibling)` (`:806`) with no early exit, accumulating every sibling's hits into one `LookupResult`; the stop test runs only after the sibling loop and keeps walking while the result is overloaded or overloadable (`:1052-1064`). An unresolved overloaded result is reported by `SemanticsVisitor::diagnoseAmbiguousReference` with `Diagnostics::AmbiguousReference` (`source/slang/slang-check-expr.cpp:1497-1511`), reached from `_resolveOverloadedExprImpl` after `refineLookup` / `resolveOverloadedLookup` fail to narrow it (`:1526-1576`) | Added a paragraph to `### Sibling scopes` stating that same-name siblings both land in one `LookupResult` — an overload set for overloadable decls, `Diagnostics::AmbiguousReference` otherwise — with a cross-link to `lookup.md` for the refinement steps |
