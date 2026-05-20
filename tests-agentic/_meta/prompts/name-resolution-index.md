# Prompt: tests-agentic/name-resolution/index/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/name-resolution/index/`,
anchored to
[`docs/llm-generated/name-resolution/index.md`](../../../docs/llm-generated/name-resolution/index.md).

Audience: nightly CI. The bundle exercises the **subtree-level
orientation** claims made by `name-resolution/index.md`. The index
doc is intentionally brief: most of its content is _pointers_ to
peer bundles. The per-page details belong to peer bundles
(`scopes`, `lookup`, `visibility`, `overload-resolution`). This
bundle therefore should be **small** and focus on the
**cross-cutting claims** the index doc makes that no single peer
page is responsible for.

The bundle target is **5–10 tests**. If you cannot anchor a claim
to text in the index doc itself, route it to a peer bundle via
`## Out of scope` (e.g. `see name-resolution/scopes`) and **do not**
duplicate a peer claim here.

## What counts as a cross-cutting index claim

A claim is in scope for this bundle iff the index doc itself
asserts it. The index doc's load-bearing claims are:

1. **End-to-end resolution**: an identifier in source text becomes
   a resolved `DeclRef` (the subtree's product).
2. **Four-phase flow**: scope walk → raw `LookupResult` →
   visibility filter → overload resolution → `DeclRef +
   breadcrumbs`. Observable by composing two or more phases in one
   test (e.g. lookup-then-visibility, lookup-then-overload).
3. **Phase ordering — visibility before overload**: the visibility
   filter narrows candidates before overload ranking sees them.
   Observable by setting up a scenario where the inaccessible
   candidate, if visible, would change the overload outcome.
4. **Phases are interleaved, not strictly sequential**: the index
   doc states "in the actual compiler some of these phases are
   interleaved". Two observable corollaries are stated:
   - **Shadowing is enforced _during_ the scope walk** (see
     `lookup.md` — this bundle defers the observation there).
   - **Visibility is re-checked _inside_ overload resolution**
     (the doc names `TryCheckOverloadCandidateVisibility`). The
     observable cross-cutting consequence is that an _accessible
     base candidate_ can lose to a _less specialized but visible_
     candidate without producing a visibility diagnostic, because
     the visible candidate is still chosen.
5. **Downstream coupling**: the resolved `DeclRef` flows into
   AST-to-IR lowering, where breadcrumb chains become concrete IR
   access patterns. Observable by checking that a
   transparent-member or `this.f` reference, resolved through a
   breadcrumb chain, **actually appears in emitted target code**
   as the correct field access — not as an undefined symbol.

Pick **5–10** claims from this list. Prefer claims that exercise
the **composition** of two or more phases, since the per-phase
claims belong to peer bundles.

## What is NOT in scope for this bundle

The index doc explicitly delegates these to peer pages. Route them
to peer bundles in `## Out of scope`; do not write tests here:

- The shape of `Scope`, sibling-chain construction, file-scope
  boundaries, namespace re-opening boundaries, generic-parameter
  visibility — see `name-resolution/scopes`.
- The `LookupMask` filter, transparent-member injection,
  breadcrumb construction, member-lookup inheritance walk,
  ambiguity and member-not-found diagnostics, container-level
  overload accumulation, override-wins-over-default — see
  `name-resolution/lookup`.
- `public` / `internal` / `private` modifier semantics, accessor
  visibility, default-visibility rules — see
  `name-resolution/visibility`.
- Candidate filtering by arity / convertibility, conversion-cost
  ranking, partial generic application, ambiguous-call diagnostics
  — see `name-resolution/overload-resolution`.

If you find yourself writing a test whose sole purpose is a
single-phase behavior, stop and route it to the peer bundle.

## Required structure

1. `BUNDLE.md` with the structure named in `_common.md`, including
   a `## Out of scope` section listing the peer bundles for the
   topics not covered here.
2. **5–10 `.slang` tests**, each anchored to an anchor in the
   index doc (`#name-resolution`, `#flow-diagram`,
   `#where-this-fits-in-the-pipeline`). The `#pages` and
   `#related-glossary-terms` anchors are pure pointer sections and
   should not be cited.
3. Coverage rules:
   - At least one **end-to-end positive** test: a name is looked
     up, passes visibility, wins overload, resolves to the
     expected decl, and the resolved value is observable via
     `INTERPRET` or text-emit FileCheck.
   - At least one **phase-ordering** test: visibility filter
     precedes overload ranking — an inaccessible decl that would
     otherwise win an overload does not.
   - At least one **breadcrumb-flows-to-IR** test: a resolved
     reference whose breadcrumb chain is non-trivial (transparent
     member, inherited member, or `this.f`) emits a working field
     access in target code. Use `-target hlsl` or `-target spirv-asm`
     text emit and FileCheck the access. Do **not** use
     `-dump-ir`; the claim is about end-to-end flow, not pass
     internals.
   - **No more than 2 tests** for any single index anchor; the
     bundle is breadth-over-depth.

4. Naming: `<composition>-<axis>.slang`, e.g.
   `end-to-end-resolution-positive.slang`,
   `visibility-filters-before-overload.slang`,
   `breadcrumb-flows-into-emitted-access.slang`,
   `phases-interleave-shadowing-during-walk.slang`. Avoid name
   collisions with sibling bundles.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/name-resolution/index.md`

Secondary (allowed citations; use sparingly and only when the
index doc explicitly references them):

- `docs/llm-generated/name-resolution/scopes.md`
- `docs/llm-generated/name-resolution/lookup.md`
- `docs/llm-generated/name-resolution/visibility.md`
- `docs/llm-generated/name-resolution/overload-resolution.md`
- `docs/llm-generated/pipeline/02-parse-ast.md`
- `docs/llm-generated/pipeline/03-semantic-check.md`
- `docs/llm-generated/pipeline/04-ast-to-ir.md`

If you would cite anything else, stop and record a doc-gap finding
in `BUNDLE.md`.

## Test directives

Cross-cutting flow is mostly target-independent at the resolution
level. Pick the lightest runner per claim:

- Positive end-to-end resolution → `//TEST:INTERPRET(filecheck=CHECK):`.
- Phase-ordering (visibility-before-overload) — typically observed
  as **a positive resolution to the visible candidate** rather than
  a diagnostic. Use `//TEST:INTERPRET(filecheck=CHECK):`. If your
  setup naturally produces a diagnostic, use
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):` and pin
  the visibility diagnostic.
- Breadcrumb-to-IR coupling → `//TEST:SIMPLE(filecheck=CHECK):-target hlsl`
  (or another text-emit target) and FileCheck the field-access
  pattern.

Do **not** use any GPU-only directive. Do **not** use `-dump-ir`
in this bundle — observing IR pass internals belongs to a
different doc subtree.

## Sibling-bundle anti-duplication

Before writing each test, confirm the observation is not already
made by the sibling bundles under
`tests-agentic/name-resolution/`. If a sibling already exercises
the same exact phase behavior, do **not** duplicate. Either skip
the test or change the angle to be specifically about
**composition across phases**.

Record the duplications you intentionally avoided in
`BUNDLE.md` under `## Sibling-bundle overlap`.

## Drop policy

If you cannot anchor a candidate test to text in the index doc
after **3 attempts** at re-reading the doc, drop the test and
record the would-be claim in `BUNDLE.md` under `## Doc gaps
observed`. The bundle's purpose is orientation; failing to find a
testable index-level claim is itself a useful signal about the
doc.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Bundle has 5–10 tests; not fewer, not more.
- [ ] Every test's `doc_ref` anchor exists in `index.md` (not
      `#pages`, not `#related-glossary-terms`).
- [ ] At least one end-to-end positive resolution test exists.
- [ ] At least one phase-ordering test exists (visibility before
      overload).
- [ ] At least one breadcrumb-to-IR coupling test exists.
- [ ] No test duplicates a sibling-bundle test verbatim.
- [ ] `BUNDLE.md` has `## Out of scope` listing the peer bundles
      that own each delegated topic.
- [ ] `BUNDLE.md` has `## Sibling-bundle overlap` listing
      intentionally-avoided peer claims.
- [ ] No `-dump-ir` directive is used. No GPU-only directive.
