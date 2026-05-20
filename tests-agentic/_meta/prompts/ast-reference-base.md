# Prompt: tests-agentic/ast-reference/base/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/ast-reference/base/`,
anchored to
[`docs/llm-generated/ast-reference/base.md`](../../../docs/llm-generated/ast-reference/base.md).

Audience: nightly CI. The bundle exercises the abstract AST root
classes — `NodeBase`, `SyntaxNodeBase`, `SyntaxNode`, `Modifier`,
`ModifiableSyntaxNode`, `DeclBase`, `Decl`, `Stmt`, `Expr`, `Val`,
`Type`, `DeclRefBase` — through their **observable consequences** in
source-level behavior. The doc is fundamentally about internal compiler
structure, so most claims are not directly testable; the bundle is
deliberately small and high-signal.

## The translation rule: claims to observations

`base.md` describes shape of internal AST classes. Slang as a compiler
does **not** expose its AST to the user. So a claim such as "`Expr`
carries a `QualType` filled in by the checker" is testable only via
its observable surface: type-checked expression contexts (overload
resolution by expression type, type-mismatch diagnostics, type
inference at variable initialization, etc.). This bundle therefore
follows the rule:

- **Testable** ⇔ "if the doc claim were false, the program-text
  behavior we wrote would change in a way `slangc` reports."
- **Not testable through slangc** ⇔ "the claim is about which C++
  class the parser allocated, the value of a private field, or the
  in-memory layout of a list."

Concretely, here's the split for this doc:

### Observable claims (write tests for these)

- **`Expr` carries a type** (`#expr-syntaxnode`). An expression
  context is type-checked: an overload set keyed on the expression's
  type resolves to the right overload; a type-mismatched assignment
  diagnoses. Use overload-by-arg-type and the resulting overload that
  fires as the observable.
- **`SyntaxNodeBase` carries a `SourceLoc`**
  (`#syntaxnodebase-nodebase`). A user-facing diagnostic includes the
  source location of the offending node. A `DIAGNOSTIC_TEST` with a
  caret at a precise column verifies that the parser preserved the
  location of the syntax node that became the error.
- **`Modifier` linked-list on a `ModifiableSyntaxNode`**
  (`#modifier-syntaxnode`, `#modifiablesyntaxnode-syntaxnode`).
  Multiple modifiers stack onto a single declaration; they all
  attach. Verify by attaching two modifiers (e.g. `[mutating]` plus a
  semantic) to a method declaration and confirming the resulting
  decl checks.
- **`Stmt` is `ModifiableSyntaxNode`** (`#stmt-modifiablesyntaxnode`):
  a statement can carry attributes. Verify by attaching `[loop]` /
  `[unroll]` to a `for` statement and observing the statement still
  type-checks.
- **`Decl` carries a name and a parent**
  (`#decl-declbase`). Verify by referencing a member from outside its
  parent scope (qualified lookup): the name resolves through the
  parent-decl chain only when the qualifier matches.
- **`Decl` capability requirements are inferred during checking**
  (`#decl-declbase`). Verify by writing a function that uses a
  capability-gated feature and observing the diagnostic when called
  from a context that lacks the capability. (Only the diagnostic is
  observable; the internal `inferredCapabilityRequirements` field is
  not.)
- **`Val` deduplication** (`#val-nodebase`). Types are hash-consed:
  two textually identical type expressions denote the same type, so
  an overload set keyed on the type resolves identically whether you
  use the type once or twice. Verify by writing two parameters of
  the same type that resolve to the same overload.
- **`Type` is the type-as-Val** (`#type-val`). Every concrete type
  carries through to overload resolution and to a diagnostic when
  ill-formed. Negative: a malformed type at a declaration site
  diagnoses.
- **`DeclRefBase` references a declaration**
  (`#declrefbase-val`). Use-site reference to a declared name
  resolves; an undeclared name produces a diagnostic.

### Not testable through slangc (do NOT write tests for these)

- That `NodeBase` carries an `ASTNodeType` discriminator.
- That `as<T>()` / `dynamicCast<T>()` use the FIDDLE-generated tag.
- That `ASTBuilder` allocates the node and stores a back-pointer.
- That `Val::m_operands` carries operands generically.
- That `SyntaxClass<T>` reflects a concrete class.
- That `Scope` is a `NodeBase`.
- The contents of `slang-ast-support-types.h` as types (`DeclRef<T>`,
  `Modifiers`, `QualType`, `SubstitutionSet`, `LookupResult`,
  `TypeExp`, `WitnessTable`).

If you find yourself thinking "this would verify that the AST node
allocated is class X", stop — that is a source-targeting probe in
disguise.

## Required structure

1. `BUNDLE.md` with the structure named in `_common.md`. The
   `## Out of scope (no-GPU runner)` section here doubles as the
   place to list claims that aren't observable via slangc at all (not
   GPU-related, but identical in spirit: out of reach of the runner).
2. 6 to 15 `.slang` test files. Quality matters more than quantity;
   the doc has only a handful of slangc-observable claims.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/ast-reference/base.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly hands off):

- `docs/llm-generated/pipeline/02-parse-ast.md`
- `docs/llm-generated/syntax-reference/grammar.md`

If you would cite anything else, stop and record a doc-gap finding in
`BUNDLE.md`.

## Source files you may consult for _verification only_

You may look at these files to confirm that the doc's claim about an
abstract base class corresponds to the actual header structure (e.g.
that `Modifier` really derives from `SyntaxNode`). You may **not**
mine them for behavioral claims that the doc does not make, and you
may **not** write tests that probe internal class identity.

- `source/slang/slang-ast-base.h`
- `source/slang/slang-ast-support-types.h`

## Test directives

Most claims here are target-independent (they're about the
parser/checker that runs before any backend), so:

- `//TEST:INTERPRET(filecheck=CHECK):` — primary directive. Use
  `printf` and FileCheck the output to assert that a typed expression
  picked the expected overload or that a name resolved correctly.
- `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` — for negative claims:
  expression type-mismatch, undeclared decl-ref, capability-required
  feature called from a non-capable context, malformed type at a
  declaration site.
- Multi-backend is **not** appropriate here. AST-level behavior is
  pre-backend; running the same claim against HLSL and GLSL only
  re-tests the backend infrastructure, not the claim.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ast-reference/base.md` (or one of the listed secondary docs).
- [ ] No test asserts the C++ identity of an AST node.
- [ ] At least one negative / diagnostic test per claim family that
      has a natural negative form (type mismatch on `Expr`,
      undeclared decl-ref for `DeclRefBase`).
- [ ] No test depends on a GPU.
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-check-*.cpp:NNNN`", stop and re-read the doc.
- [ ] Claims about internal AST shape (NodeBase discriminator,
      ASTBuilder pointer, `Val::m_operands` layout, support-type
      structure) are recorded under
      `## Out of scope (no-GPU runner)` in `BUNDLE.md`, not as
      tests.
