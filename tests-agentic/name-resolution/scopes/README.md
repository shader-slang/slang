---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T18:00:00+00:00
source_commit: 3250005059a2746ebc504a9d3f71ed112f1f2b94
watched_paths_digest: 0e24c839aae758e71e82ec905ea23d5fa0a613909e9d55418f42457a1e2bd89a
source_doc: docs/llm-generated/name-resolution/scopes.md
source_doc_digest: b163eb95da44a7485f2a712a76e0f23eac29f3fa131adb146f13413b1dc7f67d
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for name-resolution/scopes

## Intent

Tests verify the lexical-scoping behaviors described in
[`docs/llm-generated/name-resolution/scopes.md`](../../../docs/llm-generated/name-resolution/scopes.md):
which AST nodes introduce a fresh `Scope`, what is reachable from
which point in the source, how lookup walks the parent chain plus the
sibling chain, and the documented edge cases (block-local
`hiddenFromLookup`, extension-member exclusion, generic-parameter
isolation, namespace reopening).

The bundle pairs each scope-kind claim with a **positive** test that
resolves a name inside the scope and a **negative** test that proves
the same name is not visible just outside it. Diagnostic claims use
`//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` to pin the
`undefined identifier` error at the use site (with caret columns
matching the runner's "Suggested annotations" output). Positive
resolution claims use `//TEST:INTERPRET(filecheck=CHECK):` because
scoping is target-independent (the scope chain is fixed at semantic
check, before any backend lowering).

Multi-backend rule: every claim in this doc is target-independent, so
each test uses a single directive.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                       | Claim (one line)                                                                                          | Tests                                                                                                       |
| -------- | -------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------- |
| C-01     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | `BlockStmt` introduces a fresh scope; locals declared inside are not visible outside.                     | [`block-scope-name-not-visible-outside.slang`](block-scope-name-not-visible-outside.slang)                                                                |
| C-02     | [#scope-walking-order-during-lookup](../../../docs/llm-generated/name-resolution/scopes.md#scope-walking-order-during-lookup)                | Lookup walks to parent scopes; an inner block sees an enclosing block's local.                            | [`block-scope-resolves-outer-from-inner.slang`](block-scope-resolves-outer-from-inner.slang)                                                               |
| C-03     | [#scope-walking-order-during-lookup](../../../docs/llm-generated/name-resolution/scopes.md#scope-walking-order-during-lookup)                | Step 1 visits the current scope first; an inner same-name decl shadows the outer one inside the block.    | [`block-scope-shadowing.slang`](block-scope-shadowing.slang)                                                                               |
| C-04     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/scopes.md#edge-cases-and-failure-modes)                          | Block-local `hiddenFromLookup` hides a local until its DeclStmt; forward reference is rejected.           | [`block-scope-forward-ref-rejected.slang`](block-scope-forward-ref-rejected.slang)                                                                    |
| C-05     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | `FuncDecl` owns a scope; locals are not visible from another function.                                    | [`function-scope-local-not-visible-from-caller.slang`](function-scope-local-not-visible-from-caller.slang)                                                        |
| C-06     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | A CallableDecl's scope contains its parameter decls; the body sees the parameter.                         | [`function-scope-parameter-visible-in-body.slang`](function-scope-parameter-visible-in-body.slang)                                                            |
| C-07     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | A function parameter is not visible from a sibling function at file scope.                                | [`function-parameter-not-visible-from-sibling-function.slang`](function-parameter-not-visible-from-sibling-function.slang)                                                |
| C-08     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/scopes.md#edge-cases-and-failure-modes)                          | The `hiddenFromLookup` rule is scoped to block-locals; file-scope forward references between funcs work.  | [`file-scope-forward-reference-allowed.slang`](file-scope-forward-reference-allowed.slang)                                                                |
| C-09     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | `ModuleDecl`/`FileDecl` members are reachable from any function via the parent chain.                     | [`file-scope-const-visible-in-function.slang`](file-scope-const-visible-in-function.slang)                                                                |
| C-10     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | `NamespaceDecl` owns a scope; members are not visible from the enclosing module without qualification.    | [`namespace-member-needs-qualification.slang`](namespace-member-needs-qualification.slang)                                                                |
| C-11     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | A name inside a NamespaceDecl scope is reachable via `Namespace::name` qualification.                     | [`namespace-qualified-name-resolves.slang`](namespace-qualified-name-resolves.slang)                                                                   |
| C-12     | [#sibling-scopes](../../../docs/llm-generated/name-resolution/scopes.md#sibling-scopes)                                                      | Re-opening a namespace reuses the existing NamespaceDecl; both bodies' members merge under the same name. | [`namespace-reopened-merges-members.slang`](namespace-reopened-merges-members.slang)                                                                   |
| C-13     | [#implicit-scopes](../../../docs/llm-generated/name-resolution/scopes.md#implicit-scopes)                                                    | A generic type parameter lives in the GenericDecl scope and is reachable from the inner decl's body.      | [`generic-param-visible-in-body.slang`](generic-param-visible-in-body.slang)                                                                       |
| C-14     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/scopes.md#edge-cases-and-failure-modes)                          | A sibling decl cannot reach a generic parameter; its scope chain does not pass through the GenericDecl.   | [`generic-param-not-visible-in-sibling-decl.slang`](generic-param-not-visible-in-sibling-decl.slang)                                                           |
| C-15     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | `AggTypeDecl`/`StructDecl` owns a scope; a method body sees the struct's fields by unqualified name.      | [`member-scope-fields-visible-from-method.slang`](member-scope-fields-visible-from-method.slang)                                                             |
| C-16     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | Struct fields are not in the enclosing file scope; unqualified `field` is undefined in a free function.   | [`member-field-not-visible-outside-struct.slang`](member-field-not-visible-outside-struct.slang)                                                             |
| C-17     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | `ExtensionDecl` adds members to the extended type; they are callable through the receiver.                | [`extension-method-callable-via-receiver.slang`](extension-method-callable-via-receiver.slang)                                                              |
| C-18     | [#edge-cases-and-failure-modes](../../../docs/llm-generated/name-resolution/scopes.md#edge-cases-and-failure-modes)                          | ExtensionDecl members are reached only via member lookup; not by unqualified file-scope reference.        | [`extension-method-not-in-enclosing-scope.slang`](extension-method-not-in-enclosing-scope.slang)                                                             |
| C-19     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | `ForStmt` owns a fresh scope; the init variable is not visible after the loop.                            | [`for-loop-init-not-visible-outside.slang`](for-loop-init-not-visible-outside.slang)                                                                   |
| C-20     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | The ForStmt's scope holds the init variable; the body sees it for the duration of the loop.              | [`for-loop-var-visible-in-body.slang`](for-loop-var-visible-in-body.slang)                                                                        |
| C-21     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | `SwitchStmt` is a BreakableStmt with a fresh scope; a local declared in one case does not leak past it.   | [`switch-scope-local-not-visible-outside.slang`](switch-scope-local-not-visible-outside.slang)                                                              |
| C-22     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | A typedef declared inside a block is scoped to that BlockStmt; the alias is undefined outside.            | [`typedef-in-block-not-visible-outside.slang`](typedef-in-block-not-visible-outside.slang)                                                                |
| C-23     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | A `let`-bound local lives in its enclosing BlockStmt's scope; it is undefined outside the block.          | [`let-in-block-not-visible-outside.slang`](let-in-block-not-visible-outside.slang)                                                                    |
| C-24     | [#scope-bearing-ast-nodes](../../../docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes)                                    | IfStmt does not own a scope, but its body's BlockStmt does; the then-branch local is not visible after.   | [`if-block-local-not-visible-outside.slang`](if-block-local-not-visible-outside.slang)                                                                  |
| C-25     | [#sibling-scopes](../../../docs/llm-generated/name-resolution/scopes.md#sibling-scopes)                                                      | The core module's symbols are reachable without qualification via the sibling-scope chain.                | [`builtin-from-implicit-core-module.slang`](builtin-from-implicit-core-module.slang)                                                                   |

## Tests in this bundle

| File                                                          | Intent     | Doc anchor                          |
| ------------------------------------------------------------- | ---------- | ----------------------------------- |
| [`block-scope-forward-ref-rejected.slang`](block-scope-forward-ref-rejected.slang)                      | negative   | `#edge-cases-and-failure-modes`     |
| [`block-scope-name-not-visible-outside.slang`](block-scope-name-not-visible-outside.slang)                  | negative   | `#scope-bearing-ast-nodes`          |
| [`block-scope-resolves-outer-from-inner.slang`](block-scope-resolves-outer-from-inner.slang)                 | functional | `#scope-walking-order-during-lookup`|
| [`block-scope-shadowing.slang`](block-scope-shadowing.slang)                                 | functional | `#scope-walking-order-during-lookup`|
| [`builtin-from-implicit-core-module.slang`](builtin-from-implicit-core-module.slang)                     | functional | `#sibling-scopes`                   |
| [`extension-method-callable-via-receiver.slang`](extension-method-callable-via-receiver.slang)                | functional | `#scope-bearing-ast-nodes`          |
| [`extension-method-not-in-enclosing-scope.slang`](extension-method-not-in-enclosing-scope.slang)               | negative   | `#edge-cases-and-failure-modes`     |
| [`file-scope-const-visible-in-function.slang`](file-scope-const-visible-in-function.slang)                  | functional | `#scope-bearing-ast-nodes`          |
| [`file-scope-forward-reference-allowed.slang`](file-scope-forward-reference-allowed.slang)                  | functional | `#edge-cases-and-failure-modes`     |
| [`for-loop-init-not-visible-outside.slang`](for-loop-init-not-visible-outside.slang)                     | negative   | `#scope-bearing-ast-nodes`          |
| [`for-loop-var-visible-in-body.slang`](for-loop-var-visible-in-body.slang)                          | functional | `#scope-bearing-ast-nodes`          |
| [`function-parameter-not-visible-from-sibling-function.slang`](function-parameter-not-visible-from-sibling-function.slang)  | negative   | `#scope-bearing-ast-nodes`          |
| [`function-scope-local-not-visible-from-caller.slang`](function-scope-local-not-visible-from-caller.slang)          | negative   | `#scope-bearing-ast-nodes`          |
| [`function-scope-parameter-visible-in-body.slang`](function-scope-parameter-visible-in-body.slang)              | functional | `#scope-bearing-ast-nodes`          |
| [`generic-param-not-visible-in-sibling-decl.slang`](generic-param-not-visible-in-sibling-decl.slang)             | negative   | `#edge-cases-and-failure-modes`     |
| [`generic-param-visible-in-body.slang`](generic-param-visible-in-body.slang)                         | functional | `#implicit-scopes`                  |
| [`if-block-local-not-visible-outside.slang`](if-block-local-not-visible-outside.slang)                    | negative   | `#scope-bearing-ast-nodes`          |
| [`let-in-block-not-visible-outside.slang`](let-in-block-not-visible-outside.slang)                      | negative   | `#scope-bearing-ast-nodes`          |
| [`member-field-not-visible-outside-struct.slang`](member-field-not-visible-outside-struct.slang)               | negative   | `#scope-bearing-ast-nodes`          |
| [`member-scope-fields-visible-from-method.slang`](member-scope-fields-visible-from-method.slang)               | functional | `#scope-bearing-ast-nodes`          |
| [`namespace-member-needs-qualification.slang`](namespace-member-needs-qualification.slang)                  | negative   | `#scope-bearing-ast-nodes`          |
| [`namespace-qualified-name-resolves.slang`](namespace-qualified-name-resolves.slang)                     | functional | `#scope-bearing-ast-nodes`          |
| [`namespace-reopened-merges-members.slang`](namespace-reopened-merges-members.slang)                     | functional | `#sibling-scopes`                   |
| [`switch-scope-local-not-visible-outside.slang`](switch-scope-local-not-visible-outside.slang)                | negative   | `#scope-bearing-ast-nodes`          |
| [`typedef-in-block-not-visible-outside.slang`](typedef-in-block-not-visible-outside.slang)                  | negative   | `#scope-bearing-ast-nodes`          |

## Doc gaps observed

- The `## Sibling scopes` section enumerates three concrete uses of
  the sibling chain (multi-file modules, imports, re-opened
  namespaces) but does not promise a user-visible diagnostic for the
  "two siblings define the same name" ambiguity case. The closely
  related ambiguous-reference diagnostic is anchored from
  `name-resolution/lookup.md` in the pipeline/03-semantic-check
  bundle; if scopes.md were extended with a one-line note about the
  ambiguity surface, the test could be re-anchored here.
- The `## Implicit scopes` section names `parseIfLetStatement`'s
  twin-`ScopeDecl` desugaring but does not state a user-visible
  consequence (e.g. "the unwrapped variable is visible only in the
  positive branch"). A test for `if (let x = opt)` would have to
  anchor to an unstated claim, so it is deferred. Adding a sentence
  along the lines of "the unwrapped variable is visible in the
  positive branch but not in the else branch or after the if" would
  unblock that test.
- The `## Edge cases and failure modes` section mentions
  `UnscopedForStmt` but only describes its existence
  ("`for` loop's initialization variable leaks into the surrounding
  scope as HLSL semantics demand"). There is no documented surface
  for invoking HLSL mode from a `.slang` file at `slang-test` time,
  so the test cannot anchor to a single-source-file run that flips
  the language between Slang and HLSL. The doc could note which
  invocation switch enables UnscopedForStmt behavior to unblock that
  test.
- The `## Edge cases and failure modes` section mentions
  `UnparsedStmt` capturing `currentScope` and `outerScope` at parse
  time, but its observable consequence (which decls a deferred-body
  function sees once it is finally parsed) overlaps with
  pipeline/03-semantic-check's
  `function-body-checked-after-parser-handoff` test. No additional
  scopes-specific surface is documented; if the doc enumerated a
  scope-visibility consequence distinct from the parse-handoff one,
  a separate test could anchor here.
- The `## Concepts` section gives a precise description of `Scope`'s
  three fields and the `containerDecl` ownership convention. None of
  those properties are observable through `slang-test`; they show up
  only in IR/AST dumps or in internal asserts. Tests for those would
  have to fish through `-dump-ir` (which lowers past scopes already)
  and are not anchorable here.
- The `## Concepts` section lists `LambdaDecl (parameter scope)` and
  `parseIfLetStatement (synthetic)` in the scope-bearing table.
  Lambdas have a complete surface in `tests/language-feature/lambda/`
  but no scope-specific claim is restated in scopes.md beyond "a
  dedicated ScopeDecl for the parameter list". A test for
  "lambda parameter is not visible outside the lambda body" would
  anchor to an implicit consequence rather than an explicit one;
  deferred until the doc enumerates that boundary.

## Out of scope (no-GPU runner)

None for this bundle. All documented scoping behaviors are observable
through `slangi` (positive resolutions) or via the
`undefined identifier` diagnostic (negative boundaries); neither
requires a GPU.
