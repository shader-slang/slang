# Prompt: docs/generated/tests/design/name-resolution/lookup/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/design/name-resolution/lookup/`,
anchored to
[`docs/generated/design/name-resolution/lookup.md`](../../../design/name-resolution/lookup.md).

Audience: nightly CI. The bundle exercises the **lookup algorithm**
described in the source doc — how a name resolves once a lookup has
started: the per-step walk of `_lookUpInScopes`, the
`LookupMask`-driven category filter, the inheritance walk for member
lookup, transparent-member injection, ambiguity detection, and
shadowing rules. It is the companion of
[`name-resolution-scopes.md`](name-resolution-scopes.md): scopes
answers "where is name X visible from?", lookup answers "once we
start looking up name X here, what does it resolve to?". Do not
duplicate scopes claims.

The test surface is exactly the same shape as scopes: positive tests
observe a runtime value via `//TEST:INTERPRET(filecheck=CHECK):` and
negative tests observe a diagnostic via
`//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`. Lookup is fixed at semantic
check, so a single directive per claim is sufficient (the
`_common.md` "Multi-backend rule" calls this case out as primary
`INTERPRET`).

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. A test file per **verifiable lookup claim** in
   `name-resolution/lookup.md`. A claim is "verifiable" when its
   user-visible consequence is one of:
   - **a name resolves to the documented declaration** —
     positive `//TEST:INTERPRET` whose `printf` echoes the resolved
     value;
   - **a same-name overload set accumulates and a specific overload
     fires** — positive `//TEST:INTERPRET`;
   - **two reachable decls cause an ambiguity** —
     `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):` matching
     `ambiguous reference to '~name'` (code 39999);
   - **a name is filtered out by `LookupMask`** — observable through
     either a positive resolution that finds the _other_ category
     (e.g. a value-name'd type does not shadow a function call) or a
     negative diagnostic when no decl of the right kind survives
     the filter;
   - **a member lookup walks inheritance / transparent / pointer
     dereference** — positive `//TEST:INTERPRET` that calls the
     inherited / transparent / deref'd member through its breadcrumb
     chain;
   - **a member lookup miss** —
     `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` matching
     `'~name' is not a member of '~type'` (code 30027) at the
     `expr.name` site.

3. Coverage of the doc's structural sections. The doc's load-bearing
   sections are these and each should be represented by at least
   one test (positive or negative, whichever is observable):
   - `#unqualified-lookup` — scope walk, sibling chain, short-circuit
     on non-overloadable hit.
   - `#member-lookup` — `obj.name` dispatch on the type shape;
     `DeclRefType` recurses through the inheritance walk.
   - `#transparent-members` — `cbuffer C { float4 f; }` lowering;
     `f` resolves through `anon1.f` via a `Deref` + `Member`
     breadcrumb chain.
   - `#breadcrumbs` — the breadcrumb-walk produces canonical
     navigation, observable through a `this.g` resolution inside a
     method.
   - `#block-local-shadowing` — `hiddenFromLookup`; a forward
     reference is rejected.
   - `#container-level-overload-accumulation` — same-name decls
     accumulate; overload set fires.
   - `#deduplication-there-isnt-any-at-the-lookupresult-level` — the
     doc explicitly states no dedup; the observable consequence is
     that an ambiguity diagnostic fires when both paths reach the
     same type (covered by an ambiguity claim).
   - `#module-and-namespace` — re-opened namespace lookups merge;
     `using namespace` makes unqualified names reachable. (Scopes
     covers the _boundary_; this bundle covers the _merge_.)
   - `#interface-requirements-vs-default-implementations` — a
     conforming override wins over an interface default.
   - `#keyword-vs-identifier` — the `LookupMask` separates keywords
     and identifiers, so a user-declared local with the same spelling
     as a keyword does not silently take its meaning where the
     keyword is required.
   - `#generic-parameters` — a generic parameter is reachable from
     the inner decl but not from sibling decls. (The negative
     boundary belongs to scopes; this bundle covers the positive
     "resolves through the generic scope" claim.)
   - `#edge-cases-and-failure-modes` — ambiguous reference,
     forward-reference inside block, member-lookup miss.

4. Coverage rules:
   - At least one **functional** test per algorithm-shape claim
     (unqualified, member-lookup-inheritance, transparent-member,
     breadcrumb, container-overload, namespace-merge, override-wins,
     generic-resolves).
   - At least one **negative** test per failure-mode the doc
     documents under `#edge-cases-and-failure-modes` (ambiguous,
     member-not-found, forward-ref-rejected). Use the
     `ambiguous-reference` diagnostic for the ambiguity case and the
     `member not found` diagnostic for the member-miss case.
   - At least one **mask-filtering** test (positive or negative)
     that observes the `LookupMask` filter — e.g. a same-name local
     value does not block a function call of that name in the same
     scope chain, or a same-spelling keyword resolves to the keyword
     rather than a user value at a use site that asks for a type.
   - At least one **`IgnoreInheritance`** observation through the
     surface language: lookup on a struct via member access still
     hits same-type extension methods (the doc names this special
     case explicitly).
   - At least one **pointer-deref breadcrumb** observation: a
     `ConstantBuffer<T>` member is reachable through unqualified
     name (transparent + deref breadcrumb pair) or through `cb.x`
     (deref breadcrumb).

5. Naming: `<algorithm-aspect>-<axis>.slang`, e.g.
   `unqualified-walks-to-parent.slang`,
   `member-lookup-walks-inheritance.slang`,
   `transparent-cbuffer-resolves-field.slang`,
   `ambiguous-reference-diagnostic.slang`,
   `member-not-found-diagnostic.slang`,
   `overload-accumulates-in-container.slang`,
   `shadowing-forward-reference-rejected.slang`.

   **Avoid name collisions with the sibling
   `name-resolution/scopes/` bundle.** If a test name there exists
   for the same observable behavior, the scopes bundle already owns
   it; pick a different angle in this bundle.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/name-resolution/lookup.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly references them):

- `docs/generated/design/name-resolution/scopes.md`
- `docs/generated/design/ast-reference/base.md`
- `docs/generated/design/ast-reference/values.md`
- `docs/generated/design/glossary.md`

If you would cite anything else, stop and instead record a doc-gap
finding in `README.md`.

## Source files you may consult for _verification only_

You may look at these files to verify a claim in the doc is
realizable through the surface language, or to confirm the exact
text/code of a diagnostic. You may **not** mine them for behaviors
the doc does not claim.

- `source/slang/slang-lookup.h`
- `source/slang/slang-lookup.cpp`
- `source/slang/slang-check-decl.cpp`
- `source/slang/slang-check-inheritance.cpp`
- `source/slang/slang-check-stmt.cpp`
- `source/slang/slang-diagnostics.lua` (verification-only; pick the
  real diagnostic code for the ambiguity / member-not-found tests)

## Test directives

Lookup is target-independent: the same lookup result is computed
before any backend lowering, and both the resolved decl and the
`ambiguous-reference` / `member not found` diagnostics surface from
every target. Pick the lightest-weight runner per claim:

- Positive resolution of a name → `//TEST:INTERPRET(filecheck=CHECK):`.
- "Name X is ambiguous" →
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):` (the
  ambiguity diagnostic fires alongside cascading "see candidates"
  notes; using `non-exhaustive` keeps the test pinned to the
  ambiguity error).
- "Name X is not a member of T" →
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` (the diagnostic fires
  twice — once short, once with the name interpolated; both must be
  annotated in exhaustive mode).
- "Forward reference inside a block is rejected" →
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` (the
  `undefined identifier` diagnostic also fires twice). This
  duplicates scopes' `block-scope-forward-ref-rejected.slang` if
  written naively; **angle differently here**, e.g. by showing a
  same-name decl in the outer scope and pinning that the inner
  forward use is undefined rather than resolved to the outer.

Do **not** add gratuitous multi-backend directives. Lookup is fixed
at semantic check, so one directive per claim is enough.

Do not use any GPU-only directive.

## DIAGNOSTIC_TEST gotchas (from earlier bundles)

These apply here:

- Caret columns are matched **exactly**. If a token is at columns
  1–9, use the block-comment form `/*CHECK: ... */`.
- The runner's "Suggested annotations you can copy" output is the
  source of truth for exact column positions.
- `non-exhaustive` is an argument inside the parens:
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):`.
- The `undefined identifier` and `member not found` diagnostics fire
  twice (once short, once with the name interpolated). Both must be
  annotated in exhaustive mode.

## INTERPRET gotchas (from earlier bundles)

- `slangi` printf does **not** support `%s`. For string-typed
  observations, use `-target hlsl` text-emit instead.
- Slang prefers constructor-style casts (`int('A')`) over C-style
  casts. C-style cast returns 0 under the interpreter.
- `slangi` cannot instantiate `class` and cannot direct-invoke
  lambdas; structs and free functions are fine.

## Sibling bundle to avoid duplicating

`docs/generated/tests/design/name-resolution/scopes/` covers the following lookup-
adjacent behaviors. Do **not** re-claim them here; either skip the
behavior or pick a different angle that is specifically about how
the lookup algorithm resolves once it starts:

- block-scope name not visible outside (boundary) →
  scopes owns this.
- function/namespace/struct scope boundaries → scopes owns these.
- generic-parameter not visible in sibling decl → scopes owns this.
- namespace re-opening merges members → scopes owns the positive
  "qualified Foo::a and Foo::b both resolve" case; **this bundle**
  can still cover the _unqualified_ `using namespace` case because
  that exercises the sibling-scope lookup path explicitly.
- builtin-from-implicit-core-module → scopes owns this.

When in doubt, write a one-line note in `README.md` under
`## Sibling-bundle overlap` recording the choice.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an existing anchor in
      `name-resolution/lookup.md` (or one of the allowed secondary
      docs listed above).
- [ ] Each of the algorithm-shape sections (unqualified, member,
      transparent, breadcrumb, container-overload, namespace-merge,
      override-wins, generic-resolves) is represented by at least
      one test.
- [ ] Each of the failure modes (ambiguous, member-not-found,
      forward-ref-in-block) has a `DIAGNOSTIC_TEST`.
- [ ] At least one test exercises a transparent-member resolution
      (the `cbuffer`/`ConstantBuffer` lowering or an explicit
      transparent member of a struct).
- [ ] At least one test exercises a pointer-like
      `ConstantBuffer`/`ParameterBlock` auto-dereference.
- [ ] No test depends on a GPU. Only `//TEST:INTERPRET` and
      `//DIAGNOSTIC_TEST` directives are used.
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-lookup.cpp:NNNN`", stop. Re-read the doc.
- [ ] `README.md` `## Doc gaps observed` is honest. If you wanted a
      test you could not anchor, write down which claim the doc
      would need to add.
- [ ] `README.md` `## Sibling-bundle overlap` is honest. List the
      scopes claims you intentionally avoided.
