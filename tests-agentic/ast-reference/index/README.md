---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:58:41Z
source_commit: 30ae111120515b7406aa6f427a4eaaa28a0903d8
watched_paths_digest: fcff49f791602541714393b9894002eb276fdcaee6ae6a3a261e8e4c97b0e9f1
source_doc: docs/llm-generated/ast-reference/index.md
source_doc_digest: 74458dc9e2353d3c6bf43e66bf8d2b66ca18568f491325d3d0f32c657a8cf089
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ast-reference/index

## Intent

Tests verify the **cross-cutting orientation claims** made by
[`docs/llm-generated/ast-reference/index.md`](../../../docs/llm-generated/ast-reference/index.md):
the family taxonomy is real (the abstract roots partition the
concrete AST surface), every well-formed Slang program composes
concrete leaves from multiple family roots, the AST flows
through parse + check + AST-to-IR lowering into emitted target
text, the syntax-as-declaration model maps keywords and
attributes to AST node classes (so unknown attributes are
rejected), and IR lowering retires most AST nodes (so emitted
text carries the target's idiom rather than the Slang-AST shape).

The bundle is intentionally small (5 tests). The index doc is
mostly a navigation page that points to per-family pages; the
per-node and per-family details belong to the seven peer
bundles under `tests-agentic/ast-reference/`, and we route them
there via `## Untested claims` rather than duplicating.

Strategy: one observation per cross-cutting claim that the index
doc itself asserts, using the lightest runner that makes the
observation visible.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| The index doc names the syntax-as-declaration model (via SyntaxDecl and AttributeDecl) as the mapping from keywords and attributes to AST node classes; an attribute that has no declared mapping is diagnosed. | negative | [#cross-cutting-topics](../../../docs/llm-generated/ast-reference/index.md#cross-cutting-topics) | [`syntax-as-declaration-unknown-attribute-rejected.slang`](syntax-as-declaration-unknown-attribute-rejected.slang) |
| The index doc states that AST nodes lower to Slang IR which retires most of them; a Slang-level matrix-vector multiply emits as the target's mul-idiom rather than as a Slang-AST-shaped artifact. | functional | [#cross-cutting-topics](../../../docs/llm-generated/ast-reference/index.md#cross-cutting-topics) | [`ast-lowers-and-retires-into-target-idiom.slang`](ast-lowers-and-retires-into-target-idiom.slang) |
| The index doc's cross-cutting topics name parse, semantic-check, and AST-to-IR lowering as the AST's life cycle; a struct declaration survives all three phases and appears as a struct in emitted HLSL target text. | functional | [#cross-cutting-topics](../../../docs/llm-generated/ast-reference/index.md#cross-cutting-topics) | [`ast-survives-parse-check-lower-to-hlsl.slang`](ast-survives-parse-check-lower-to-hlsl.slang) |
| A working program composes concrete leaves from every AST root family the index taxonomy names (Decl, Expr, Stmt, Type, Modifier, Val), demonstrating that the family partition spans the surface language. | functional | [#family-taxonomy](../../../docs/llm-generated/ast-reference/index.md#family-taxonomy) | [`multi-family-composition-program.slang`](multi-family-composition-program.slang) |
| The AST family taxonomy is real: a Stmt root and an Expr root are distinct, so a bare break statement outside a loop or switch is diagnosed by the checker (a Stmt-shape constraint, not an Expr-shape one). | negative | [#family-taxonomy](../../../docs/llm-generated/ast-reference/index.md#family-taxonomy) | [`family-taxonomy-stmt-vs-expr-rejected.slang`](family-taxonomy-stmt-vs-expr-rejected.slang) |

## Untested claims

| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| Concrete `Decl` leaves (`StructDecl`, `FuncDecl`, `InterfaceDecl`, `GenericDecl`, ...) -- see `tests-agentic/ast-reference/declarations/`. | out-of-bundle | [#decl](../../../docs/llm-generated/ast-reference/index.md#decl) | Covered by a sibling bundle; see the appropriate `tests-agentic/<sibling>/` directory. |
| Concrete `Expr` leaves (binary, unary, `InvokeExpr`, `MemberExpr`, conversion exprs, ...) -- see `tests-agentic/ast-reference/expressions/`. | out-of-bundle | [#expr](../../../docs/llm-generated/ast-reference/index.md#expr) | Covered by a sibling bundle; see the appropriate `tests-agentic/<sibling>/` directory. |
| Concrete `Modifier` / attribute leaves (per-attribute semantics) -- see `tests-agentic/ast-reference/modifiers/`. | out-of-bundle | [#modifier](../../../docs/llm-generated/ast-reference/index.md#modifier) | Covered by a sibling bundle; see the appropriate `tests-agentic/<sibling>/` directory. |
| Abstract roots (`NodeBase`, `SyntaxNode`, `Val`, `Type`, `Decl`, `Expr`, `Stmt`, `Modifier`) and their per-root observable consequences -- see `tests-agentic/ast-reference/base/`. | out-of-bundle | [#nodebase](../../../docs/llm-generated/ast-reference/index.md#nodebase) | Covered by a sibling bundle; see the appropriate `tests-agentic/<sibling>/` directory. |
| `SourceLoc`-bearing diagnostic placement at a specific node level -- see `tests-agentic/ast-reference/base/` (claim `syntaxnodebase-source-loc-in-diagnostic.slang`); the index bundle does not re-test this because every family page inherits the same `SourceLoc` shape, and the cross-cutting observation reduces to the base claim. | out-of-bundle | [#sourceloc](../../../docs/llm-generated/ast-reference/index.md#sourceloc) | Covered by a sibling bundle; see the appropriate `tests-agentic/<sibling>/` directory. |
| Concrete `Stmt` leaves (`BlockStmt`, `IfStmt`, loops, `ReturnStmt`, ...) -- see `tests-agentic/ast-reference/statements/`. | out-of-bundle | [#stmt](../../../docs/llm-generated/ast-reference/index.md#stmt) | Covered by a sibling bundle; see the appropriate `tests-agentic/<sibling>/` directory. |
| Concrete `Type` leaves (`VectorType`, `MatrixType`, buffer types, `OptionalType`, ...) -- see `tests-agentic/ast-reference/types/`. | out-of-bundle | [#type](../../../docs/llm-generated/ast-reference/index.md#type) | Covered by a sibling bundle; see the appropriate `tests-agentic/<sibling>/` directory. |
| `Val` non-Type leaves (`DeclRefBase`, `IntVal`, `Witness`, ...) -- see `tests-agentic/ast-reference/values/`. | out-of-bundle | [#val](../../../docs/llm-generated/ast-reference/index.md#val) | Covered by a sibling bundle; see the appropriate `tests-agentic/<sibling>/` directory. |

## Sibling-bundle overlap

The following peer-bundle behaviors are intentionally not
re-tested here to avoid duplication:

- `expr-carries-type-overload.slang` and
  `expr-type-mismatch-diagnoses.slang` (base bundle) cover the
  per-root claim that `Expr` carries a `QualType`. The index
  bundle's `family-taxonomy-stmt-vs-expr-rejected.slang` pins
  the cross-cutting claim that the family partition itself is
  enforced, not the per-root type-attachment claim.
- `structdecl-emit-multitarget.slang` (declarations bundle)
  covers the per-leaf claim that a `StructDecl` emits per
  target. The index bundle's
  `ast-survives-parse-check-lower-to-hlsl.slang` cites the
  index doc's three-step parse + check + lower pipeline and
  uses HLSL as a single witness; it does not duplicate the
  multi-target sweep.
- The modifiers bundle owns per-attribute semantics; the index
  bundle's `syntax-as-declaration-unknown-attribute-rejected.slang`
  cites only the index doc's mapping-claim
  (SyntaxDecl/AttributeDecl) and the consequence that an
  unmapped name has no class to bind to.

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#shape-parent-class-fields-grammar-source-rather-than-behavior](../../../docs/llm-generated/ast-reference/index.md#shape-parent-class-fields-grammar-source-rather-than-behavior) | undocumented-behavior | The index doc says the family pages describe "shape (parent class, fields, grammar source) rather than behavior", but this is a doc-organization claim and is not directly testable through any `slangc` directive without instrumenting the C++ AST. No test was authored for it; a future revision of the index could either drop the meta-claim or restate it as a user-observable consequence (e.g. "every documented field appears in a grammar production we can point at"). |  |
| [#how-to-navigate](../../../docs/llm-generated/ast-reference/index.md#how-to-navigate) | undocumented-behavior | The index doc's `#how-to-navigate` section is purely doc-navigation guidance (which page to read first, what each page's table looks like) with no slangc-observable consequence. The bundle therefore did not cite that anchor. |  |
| [#pages](../../../docs/llm-generated/ast-reference/index.md#pages) | undocumented-behavior | The `#pages` table and the `#family-taxonomy` mermaid claim approximate concrete-class counts ("~60", "~90", ...). The numeric approximations are guaranteed only "rounded to the nearest five at the source_commit"; no agent can sensibly anchor a test to a "~60" approximation. Treating the counts as testable would require the doc to commit to a stable exact number or to a programmatic invariant. |  |
