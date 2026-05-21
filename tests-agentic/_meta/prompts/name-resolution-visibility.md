# Prompt: tests-agentic/name-resolution/visibility/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/name-resolution/visibility/`,
anchored to
[`docs/llm-generated/name-resolution/visibility.md`](../../../docs/llm-generated/name-resolution/visibility.md).

Audience: nightly CI. The bundle exercises Slang's **declaration
visibility enforcement** — what is observable when a `private`,
`internal`, or `public` decl is or is not reachable from a specific
requesting scope. The companion bundle `name-resolution/scopes`
covers where names are *visible within* a scope chain;
`ast-reference/modifiers` covers visibility-modifier AST nodes.
This bundle's angle is **enforcement at the boundary**: the same
decl is rejected from a non-matching scope and accepted from a
matching one.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. A test file per **enforcement claim** in
   `name-resolution/visibility.md`. A claim is "verifiable" when its
   user-visible consequence is one of:

   - **a `private`/`internal` decl is accessible from inside its
     container** — positive `//TEST:INTERPRET(filecheck=CHECK):`
     whose `printf` echoes the resolved value (because access was
     permitted);
   - **a `private`/`internal` decl is *not* accessible from outside
     its container** — `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` (or
     `non-exhaustive` if cascading errors fire) matching the
     `declaration not accessible` (`30600`) diagnostic at the use
     site;
   - **a visibility modifier is rejected at the declaration site
     itself** — `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` matching
     `visibility higher than parent` (`30601`),
     `invalid private visibility` (`30603`),
     `references less visible type` (`30604`), or
     `invalid visibility modifier` (`36005`) at the declaration
     line.

3. Coverage per claim:

   - At least one positive test per visibility keyword that survives
     enforcement (accessed from a scope where the keyword permits
     it).
   - At least one negative test per visibility keyword for the
     boundary the keyword draws (rejected from a scope the keyword
     forbids).
   - Cover the four declaration-site diagnostics named in
     "Edge cases and failure modes": `higher-than-parent`,
     `invalid private`, `less-visible-type`, and the `extension`-
     specific positive cases.

4. Naming: `<visibility-axis>-<positive|rejected>.slang`, e.g.
   `private-member-callable-from-same-struct.slang`,
   `private-member-rejected-from-outside-struct.slang`,
   `public-inside-internal-struct-rejected.slang`.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/name-resolution/visibility.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly references them):

- `docs/llm-generated/name-resolution/lookup.md`
- `docs/llm-generated/name-resolution/overload-resolution.md`
- `docs/llm-generated/ast-reference/modifiers.md`

If you would cite anything else, stop and instead record a doc-gap
finding in `README.md`.

## Source files you may consult for _verification only_

You may look at these files to verify a claim in the doc is
realizable through the surface language. You may **not** mine them
for behaviors the doc does not claim.

- `source/slang/slang-check-decl.cpp` (`getDeclVisibility`,
  module default propagation)
- `source/slang/slang-check-expr.cpp` (`isDeclVisibleFromScope`,
  lookup-time filter)
- `source/slang/slang-check-overload.cpp`
  (`TryCheckOverloadCandidateVisibility`)
- `source/slang/slang-check-modifier.cpp` (`checkVisibility`,
  higher-than-parent + less-visible-type)
- `source/slang/slang-ast-modifier.h`,
  `source/slang/slang-ast-support-types.h`

Existing single-file tests under `tests/diagnostics/` that name
similar diagnostics (`private-visibility.slang`, `visibility.slang`,
`extension-private-visibility.slang`) are also useful for column
positions and verbatim diagnostic text.

## Test directives

Visibility enforcement runs at semantic check, before any backend-
specific lowering. Pick the lightest-weight runner per claim:

- Positive access permitted → `//TEST:INTERPRET(filecheck=CHECK):`.
- Access rejected at the use site →
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`. The runner emits two
  diagnostics for a single `not accessible` error (a short form and
  a full form); annotate both in exhaustive mode.
- Declaration-site rejection (less-visible-type, higher-than-parent,
  invalid-private) → `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):` at the
  declaration line.

Do **not** add gratuitous multi-backend directives. Visibility is
fixed at semantic check, so a single directive is enough per claim.

Do not use any GPU-only directive.

## Multi-file modules

Cross-module `import` visibility (e.g. `internal` decls invisible
to a different module) is mentioned in the doc but is awkward to
test in a single `.slang` file. If a cross-module claim cannot be
expressed in a single-file test using a free function at module
scope versus a member function inside the struct, record it as a
doc-gap-like note in `README.md` under
`## Out of scope (single-file runner)`.

## DIAGNOSTIC_TEST gotchas (from earlier bundles)

These apply here:

- Caret columns are matched **exactly**. Use the block-comment
  form `/*CHECK: ... */` when the offending token starts in
  columns 1–9.
- The runner's "Suggested annotations you can copy" output is the
  source of truth for exact column positions.
- The `declaration not accessible` diagnostic for `private` fires
  twice (short form + full form with the decl name interpolated);
  both must be annotated in exhaustive mode.
- `non-exhaustive` is an argument inside the parens:
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):`. Use it
  only when cascading diagnostics surface that you do not want to
  pin.
- Some diagnostics (e.g. `references less visible type`) attach
  to the function/field name, not the type token. Look at the
  actual compiler output before placing carets.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an existing anchor in
      `name-resolution/visibility.md` (or one of the allowed
      secondary docs listed above).
- [ ] Both halves of each visibility keyword are covered: a
      positive that resolves an access from a permitted scope
      **and** a negative that proves the same access is rejected
      from a non-permitted scope.
- [ ] At least one test exercises the
      `extension`-on-same-type positive case (private member
      callable across sibling extensions).
- [ ] At least one declaration-site rejection per documented
      diagnostic: `30601` (higher-than-parent), `30603` (invalid
      private), `30604` (less-visible-type).
- [ ] No test depends on a GPU. `//TEST:INTERPRET` and
      `//DIAGNOSTIC_TEST` are the only directives used.
- [ ] No test was written by inspecting an uncovered source line.
- [ ] README.md `## Doc gaps observed` is honest.
