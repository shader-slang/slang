# Prompt: tests-agentic/name-resolution/scopes/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/name-resolution/scopes/`,
anchored to
[`docs/llm-generated/name-resolution/scopes.md`](../../../docs/llm-generated/name-resolution/scopes.md).

Audience: nightly CI. The bundle exercises the **lexical scoping
rules** the doc describes — which AST nodes introduce a new `Scope`,
what is reachable from which point in the source, and how block-,
function-, file-, namespace-, generic-, member-, and extension-scoped
names interact. Scoping is observable indirectly through name
resolution: the test surface is "name X resolves to declaration Y"
(positive, observe a runtime value) and "name X is not visible at
point P" (negative, observe an `undefined identifier` diagnostic).

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. A test file per **verifiable scoping claim** in
   `name-resolution/scopes.md`. A claim is "verifiable" when its
   user-visible consequence is one of:

   - **a name resolves where the doc says it does** —
     positive `//TEST:INTERPRET` whose `printf` echoes the resolved
     value;
   - **a name is *not* in scope at a particular point** —
     `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` matching the
     `undefined identifier` diagnostic at the use site;
   - **a name reachable through a sibling chain is found** —
     positive `//TEST:INTERPRET` that calls into the sibling
     (e.g. into a re-opened namespace).

3. Coverage per claim:

   - At least one positive test for each scope kind enumerated in
     the doc's "Scope-bearing AST nodes" table where the kind is
     observable from a `slangi` runtime (block, function, file,
     namespace, generic-param, member-via-method, extension-via-
     method, for-loop, switch, typedef in block, let in block).
   - At least one negative test that proves the *boundary* of each
     such scope — i.e. that the name is **not** visible just outside
     it. This is the load-bearing axis: the doc is mostly about
     boundaries.
   - Where the doc highlights an edge case ("Edge cases and failure
     modes" section), include a test that fixes that case in either
     direction: a positive that demonstrates the documented behavior
     or a negative that demonstrates the rejection.

4. Naming: `<scope-kind>-<axis>.slang`, e.g.
   `block-scope-shadowing.slang`,
   `block-scope-name-not-visible-outside.slang`,
   `function-scope-locals-not-visible-from-caller.slang`,
   `generic-param-not-visible-in-sibling-decl.slang`,
   `namespace-reopened-merges-members.slang`.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/name-resolution/scopes.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly references them):

- `docs/llm-generated/ast-reference/base.md`
- `docs/llm-generated/ast-reference/declarations.md`
- `docs/llm-generated/ast-reference/statements.md`
- `docs/llm-generated/name-resolution/lookup.md`
- `docs/llm-generated/pipeline/03-semantic-check.md`

If you would cite anything else, stop and instead record a doc-gap
finding in `README.md`.

## Source files you may consult for _verification only_

You may look at these files to verify a claim in the doc is realizable
through the surface language. You may **not** mine them for behaviors
the doc does not claim.

- `source/slang/slang-check-decl.cpp` (sibling-scope linking for
  multi-file modules and namespaces)
- `source/slang/slang-check-stmt.cpp` (entry/clear logic for
  `Decl::hiddenFromLookup` inside a block)
- `source/slang/slang-check-expr.cpp` (`addSiblingScopeForContainerDecl`)
- `source/slang/slang-session.cpp` (per-source-file `FileDecl`
  sibling chain)
- `source/slang/slang-lookup.cpp` (lookup walk: parent + nextSibling)

## Test directives

Scoping is target-independent: the same scope chain is built before
any backend-specific lowering, and the `undefined identifier`
diagnostic surfaces from every target. Pick the lightest-weight
runner per claim:

- Positive observation of a resolved name → `//TEST:INTERPRET(filecheck=CHECK):`.
- "Name X is not in scope" → `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`.
- "Same input is rejected at point P even though *other* candidates
  would resolve" → `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):`
  when you only want to pin the `undefined identifier` and ignore
  any cascading diagnostics.

Do **not** add gratuitous multi-backend directives. Scoping is fixed
at semantic check, so a single directive is enough per claim. The
exception is target-conditional scoping (HLSL `UnscopedForStmt`); the
doc names that case but its diagnostic surface is "no diagnostic"
plus a runtime value — a single `//TEST:INTERPRET` directive is
sufficient.

Do not use any GPU-only directive.

## DIAGNOSTIC_TEST gotchas (from earlier bundles)

These apply here:

- Caret columns are matched **exactly**. If a token is at columns
  1–9 (e.g. a bare `T` at indent 4), use the block-comment form
  `/*CHECK: ... */` because the `//CHECK:` prefix consumes
  columns 1–9.
- The runner's "Suggested annotations you can copy" output is the
  source of truth for exact column positions.
- `non-exhaustive` is an argument inside the parens:
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):`.
- The `undefined identifier` diagnostic fires twice (once short,
  once with the name interpolated). Both must be annotated in
  exhaustive mode.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an existing anchor in
      `name-resolution/scopes.md` (or one of the allowed secondary
      docs listed above).
- [ ] Both halves of every scope kind are covered: a positive that
      resolves a name inside the scope **and** a negative that
      proves the name is not visible outside it.
- [ ] At least one test exercises a sibling-scope chain
      (re-opened namespace, multi-file module, or an explicit
      `import`).
- [ ] At least one test exercises the documented `if (let x = ...)`
      desugaring, the typedef-in-block boundary, or another
      "implicit scopes" claim.
- [ ] No test depends on a GPU. `//TEST:INTERPRET` and
      `//DIAGNOSTIC_TEST` are the only directives used.
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-lookup.cpp:NNNN`", stop. Re-read the doc.
- [ ] README.md `## Doc gaps observed` is honest. If you wanted a
      test you could not anchor, write down which claim the doc
      would need to add.
