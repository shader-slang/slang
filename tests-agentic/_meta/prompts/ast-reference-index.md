# Prompt: tests-agentic/ast-reference/index/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/ast-reference/index/`,
anchored to
[`docs/llm-generated/ast-reference/index.md`](../../../docs/llm-generated/ast-reference/index.md).

Audience: nightly CI. The bundle exercises the **subtree-level
orientation** claims made by `ast-reference/index.md`. The index
doc is intentionally narrow: it is a navigation page that points
to per-family peer pages (`base`, `declarations`, `expressions`,
`statements`, `types`, `modifiers`, `values`). The per-node and
per-family details belong to those peer bundles. This bundle
therefore should be **small** and focus on the **cross-cutting
claims** the index doc itself makes that no single family page is
solely responsible for.

The bundle target is **3–8 tests**. If you cannot anchor a claim
to text in the index doc itself, route it to a peer bundle via
`## Out of scope` (e.g. `see ast-reference/declarations`) and **do
not** duplicate a peer claim here.

## What counts as a cross-cutting index claim

A claim is in scope for this bundle iff the index doc itself
asserts it. The index doc's load-bearing cross-cutting claims are:

1. **Family taxonomy is real**: the abstract roots (`Decl`,
   `Expr`, `Stmt`, `Type`, `Modifier`, `Val`) partition the
   concrete AST surface. Observable corollary: a kind-mismatch
   between roots — e.g. a statement token used where a
   declaration is expected, or a type-expression used where a
   value-expression is expected — is diagnosed at parse/check.
2. **Each root has concrete leaves**: the index promises a
   per-family page lists concrete leaves under each root. The
   cross-cutting consequence is that user code can produce at
   least one concrete instance from each documented family root
   (a `StructDecl`, a binary `InvokeExpr`, a `BlockStmt`, a
   `VectorType`, a `Modifier`, a `Val` carried by a generic
   substitution).
3. **Shape vs behavior**: the index says the family pages
   describe "shape (parent class, fields, grammar source) rather
   than behavior", and routes behavior to pipeline pages. This
   is a doc-organization claim, not a slangc-observable claim;
   record it as a `Doc gap observed` if it tempts a test.
4. **AST is built by the parser, checked by the checker, lowered
   to IR**: the index doc names this three-step flow as the
   companion to the family pages (links to `pipeline/02-parse-ast`,
   `pipeline/03-semantic-check`, `pipeline/04-ast-to-ir`). The
   cross-cutting observable is that a single declaration token
   in source survives parse + check + lowering and appears as
   the expected entity in emitted target text. This is the
   "AST-flows-end-to-end" claim, the analogue of the
   name-resolution index's "DeclRef-flows-to-emitted-access".
5. **Syntax-as-declaration model**: the index doc explicitly
   names that "Slang's syntax-as-declaration model (via
   `SyntaxDecl` and `AttributeDecl`) maps keywords and attributes
   to AST node classes" and links
   `syntax-reference/keywords-and-builtins.md`. Observable
   corollary: applying an undefined keyword/attribute is
   diagnosed (the mapping is enforced).
6. **SourceLoc carries diagnostics**: the index doc explicitly
   names `SourceLoc`-bearing AST nodes as carriers of most
   diagnostics. Observable corollary: a diagnostic about a node
   points at the column of the offending token. This claim
   already has detailed coverage in `ast-reference/base`; if you
   want to exercise it here, frame the test as the **cross-cutting
   composition** ("any family's node, when ill-formed, points at
   the right column") rather than a single family's node.
7. **AST-to-IR retires most nodes**: the index doc says
   "AST nodes lower to Slang IR (which retires most of them)".
   Observable corollary: the same source declaration emits as a
   target-specific construct (HLSL `struct`, etc.) rather than
   as a Slang-AST-shaped artifact. This overlaps with claim 4 —
   pick one framing or the other, not both.

Pick **3–8** claims from this list. Prefer claims that exercise
**composition across families** (e.g. a `Decl` + an `Expr` + a
`Stmt`) over single-family observations, since the per-family
claims belong to peer bundles.

## What is NOT in scope for this bundle

The index doc explicitly delegates these to peer pages. Route
them to peer bundles in `## Out of scope`; do not write tests
here:

- Per-root abstract-class behaviors (`NodeBase`, `SyntaxNode`,
  `Val`, `Type`, `Decl`, `Expr`, `Stmt`, `Modifier`) — see
  `ast-reference/base`.
- Concrete `Decl` leaves (`StructDecl`, `FuncDecl`,
  `InterfaceDecl`, `GenericDecl`, ...) — see
  `ast-reference/declarations`.
- Concrete `Expr` leaves (binary/unary, `InvokeExpr`,
  `MemberExpr`, conversion exprs, ...) — see
  `ast-reference/expressions`.
- Concrete `Stmt` leaves (`BlockStmt`, `IfStmt`, loops,
  `ReturnStmt`, ...) — see `ast-reference/statements`.
- Concrete `Type` leaves (`VectorType`, `MatrixType`, buffer
  types, `OptionalType`, ...) — see `ast-reference/types`.
- Concrete `Modifier` / attribute leaves — see
  `ast-reference/modifiers`.
- `Val` non-Type leaves (`DeclRefBase`, `IntVal`, `Witness`,
  ...) — see `ast-reference/values`.

If you find yourself writing a test whose sole purpose is a
single-family behavior, stop and route it to the peer bundle.

## Required structure

1. `README.md` with the structure named in `_common.md`,
   including a `## Out of scope` section listing the peer bundles
   for the topics not covered here.
2. **3–8 `.slang` tests**, each anchored to an anchor in the
   index doc (`#ast-reference`, `#family-taxonomy`,
   `#cross-cutting-topics`, `#how-to-navigate`). The `#pages`
   anchor is a pure pointer table and should not be cited.
3. Coverage rules:
   - At least one **family-taxonomy-is-real** test: a kind
     mismatch between roots is diagnosed (an expression used as a
     statement, a type used as a value, etc.), citing
     `#family-taxonomy`.
   - At least one **AST-flows-end-to-end** test: a source
     declaration survives parse + check + lowering and appears as
     the expected construct in emitted target text. Use
     `-target hlsl` or another text-emit target and FileCheck the
     emitted text. Do **not** use `-dump-ir`; the claim is about
     end-to-end flow, not pass internals.
   - At least one **multi-family-composition** test: a single
     program exercises at least three of the family roots
     (`Decl` + `Expr` + `Stmt`, or `Decl` + `Type` + `Modifier`)
     and the program compiles and runs (or emits) as expected.
   - **No more than 2 tests** for any single index anchor; the
     bundle is breadth-over-depth.

4. Naming: `<composition>-<axis>.slang`, e.g.
   `family-taxonomy-kind-mismatch-rejected.slang`,
   `ast-survives-parse-check-lower-to-hlsl.slang`,
   `multi-family-composition-program.slang`,
   `syntax-as-declaration-unknown-attribute-rejected.slang`.
   Avoid name collisions with sibling bundles.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/ast-reference/index.md`

Secondary (allowed citations; use sparingly and only when the
index doc explicitly references them):

- `docs/llm-generated/ast-reference/base.md`
- `docs/llm-generated/ast-reference/declarations.md`
- `docs/llm-generated/ast-reference/expressions.md`
- `docs/llm-generated/ast-reference/statements.md`
- `docs/llm-generated/ast-reference/types.md`
- `docs/llm-generated/ast-reference/modifiers.md`
- `docs/llm-generated/ast-reference/values.md`
- `docs/llm-generated/pipeline/02-parse-ast.md`
- `docs/llm-generated/pipeline/03-semantic-check.md`
- `docs/llm-generated/pipeline/04-ast-to-ir.md`
- `docs/llm-generated/syntax-reference/keywords-and-builtins.md`

If you would cite anything else, stop and record a doc-gap
finding in `README.md`.

## Test directives

Cross-cutting AST flow is mostly target-independent at the
shape level. Pick the lightest runner per claim:

- Kind-mismatch / unknown-attribute (parse/check rejection) →
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):`. Copy
  diagnostic text **verbatim** from the runner's "Suggested
  annotations" output.
- Positive end-to-end shape (AST builds, checks, evaluates) →
  `//TEST:INTERPRET(filecheck=CHECK):`.
- AST-flows-to-target-emit → `//TEST:SIMPLE(filecheck=CHECK):-target hlsl`
  (or another text-emit target) and FileCheck the emitted
  construct.

Do **not** use any GPU-only directive. Do **not** use `-dump-ir`
in this bundle — observing IR pass internals belongs to a
different doc subtree.

## Sibling-bundle anti-duplication

Before writing each test, confirm the observation is not already
made by the sibling bundles under
`tests-agentic/ast-reference/`. If a sibling already exercises
the same family-level behavior, do **not** duplicate. Either
skip the test or change the angle to be specifically about
**composition across families**.

Record the duplications you intentionally avoided in
`README.md` under `## Sibling-bundle overlap`.

## Drop policy

If you cannot anchor a candidate test to text in the index doc
after **3 attempts** at re-reading the doc, drop the test and
record the would-be claim in `README.md` under `## Doc gaps
observed`. The bundle's purpose is orientation; failing to find
a testable index-level claim is itself a useful signal about the
doc.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Bundle has 3–8 tests; not fewer, not more.
- [ ] Every test's `doc_ref` anchor exists in `index.md` (not
      `#pages`).
- [ ] At least one family-taxonomy-is-real (kind-mismatch) test
      exists.
- [ ] At least one AST-flows-end-to-end (emit) test exists.
- [ ] At least one multi-family-composition test exists.
- [ ] No test duplicates a sibling-bundle test verbatim.
- [ ] `README.md` has `## Out of scope` listing the peer bundles
      that own each delegated topic.
- [ ] `README.md` has `## Sibling-bundle overlap` listing
      intentionally-avoided peer claims.
- [ ] No `-dump-ir` directive is used. No GPU-only directive.
