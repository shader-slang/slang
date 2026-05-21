# Prompt: tests-agentic/name-resolution/overload-resolution/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/name-resolution/overload-resolution/`,
anchored to
[`docs/llm-generated/name-resolution/overload-resolution.md`](../../../docs/llm-generated/name-resolution/overload-resolution.md).

Audience: nightly CI. The bundle exercises the **overload-resolution
algorithm** described in the source doc — how Slang narrows a
`LookupResult` containing multiple candidates to a single best
candidate (or to a structured ambiguity error). It covers candidate
enumeration, the `TryCheckOverloadCandidate` probe-phase filter
pipeline (arity, fixity, type/coercion, directions, constraints,
visibility), the `ConversionCost` ranking, the
`CompareOverloadCandidates` tie-breaker (status, cost, specificity,
implicit-conversion preference, export-rank, scope-distance),
partial generic application, operator overloading dispatch through
witness tables, and the documented failure modes (ambiguous-overload,
no-applicable-with-args, generic-argument-inference-failed,
not-enough/too-many-arguments, decl-is-not-visible).

It is the companion of
[`name-resolution-lookup.md`](name-resolution-lookup.md) /
[`name-resolution-scopes.md`](name-resolution-scopes.md): lookup
answers "which decls does the name see?", overload resolution answers
"once we have a candidate set, which one is the call?".

The test surface is the same shape as lookup: positive tests observe
a runtime value via `//TEST:INTERPRET(filecheck=CHECK):` (the
`printf` echo reveals which overload was picked); negative tests
observe a diagnostic via `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`.
Overload resolution is fixed at semantic check, so a single
directive per claim is sufficient.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. A test file per **verifiable overload-resolution claim** in
   `name-resolution/overload-resolution.md`. A claim is "verifiable"
   when its user-visible consequence is one of:

   - **a specific overload is picked** — positive
     `//TEST:INTERPRET` whose `printf` echoes a tagged return value;
   - **the more specific candidate wins** — positive
     `//TEST:INTERPRET`;
   - **the lower-cost candidate wins** — positive
     `//TEST:INTERPRET`;
   - **the closer-scope candidate wins** — positive
     `//TEST:INTERPRET`;
   - **two equally-good candidates produce an ambiguity diagnostic**
     — `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):` matching
     `ambiguous call to '~name'` (code 39999);
   - **no candidate is applicable** —
     `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):` matching
     `no overload applicable to arguments of type ~args` (code 39999);
   - **arity / direction / visibility / generic-inference fails** —
     `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):` matching
     the specific filter-step diagnostic;
   - **witness-table dispatch** — positive `//TEST:INTERPRET` that
     calls an interface method through a generic and observes the
     conforming-type override fires (operator overloading section).

3. Coverage of the doc's structural sections. Each load-bearing
   section should be represented by at least one test:

   - `#probe-phase-trycheckoverloadcandidate` — the per-step filter:
     arity, fixity, types, directions, constraints, visibility. At
     least one positive ("survives all steps, becomes Applicable")
     and one negative test per visible failure step (arity over,
     arity under, direction lvalue-required, type-mismatch with no
     conversion).
   - `#probe-phase-where-candidates-come-from` — exercise multiple
     candidate families: free-function, constructor, member.
   - `#finalize-phase-completeoverloadcandidate` — the chosen
     candidate's diagnostic re-emission in `ForReal` mode (e.g.
     generic-argument-inference-failed when the only candidate has
     that status).
   - `#conversion-costs` — at least one test that pins a known
     low-cost conversion beating a known high-cost conversion (e.g.
     `int` -> `int` (cost 0) wins over `int` -> `float` (cost 400)
     when both overloads are callable with an `int` argument).
   - `#tie-breaking-comparator` — distinct tests for:
     (a) lower conversion cost wins,
     (b) more specific (non-generic > generic) wins,
     (c) closer scope wins (non-generic flavors),
     (d) override-of-inherited wins (lookup-result tiebreak).
   - `#partial-generic-application` — produces a callable result
     when some generic params are pinned by arguments (positive
     INTERPRET).
   - `#operator-overloading` — a user-defined operator overload
     fires on the operands' types; an interface-method dispatch
     through a generic resolves the conforming-type implementation
     (witness-table dispatch).
   - `#edge-cases-and-failure-modes` — ambiguous, no-applicable,
     generic-argument-inference-failed.

4. Coverage rules:

   - **15-25 tests total.** At least 30% must be
     `DIAGNOSTIC_TEST` (negative — ambiguous, no-applicable, arity,
     direction, visibility, inference-failed).
   - At least one **operator overload** test (positive INTERPRET).
   - At least one **witness-table dispatch** test (positive INTERPRET
     through an interface-bounded generic).
   - At least one **partial generic application** observable case
     (positive INTERPRET).
   - At least one **conversion-cost ranking** observable case
     (positive INTERPRET where the cheaper overload is picked).

5. Naming: `<aspect>-<axis>.slang`, e.g.
   `overload-picks-exact-match-over-int-to-float.slang`,
   `overload-prefers-non-generic-over-generic.slang`,
   `ambiguous-call-equal-cost-diagnostic.slang`,
   `arity-too-few-arguments-diagnostic.slang`.

## Doc sources

Primary:

- `docs/llm-generated/name-resolution/overload-resolution.md`

Secondary (allowed citations; use sparingly):

- `docs/llm-generated/name-resolution/lookup.md`
- `docs/llm-generated/name-resolution/visibility.md`

## Source files you may consult for verification only

- `source/slang/slang-check-overload.cpp`
- `source/slang/slang-check-impl.h`
- `source/slang/slang-check-conversion.cpp`
- `source/slang/slang-diagnostics.lua`

## Test directives

Overload resolution is target-independent. Pick the lightest-weight
runner per claim:

- Positive: `//TEST:INTERPRET(filecheck=CHECK):`.
- Ambiguity / no-applicable / inference-failed: use
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):` because the
  resolver emits multiple notes (candidate locations) alongside the
  primary error.

Do not use any GPU-only directive. Do not add multi-backend
directives — one directive per claim suffices.

## DIAGNOSTIC_TEST gotchas (from earlier bundles)

- Caret columns are matched **exactly**. For tokens in columns 1-9
  use the block-comment form `/*CHECK: ... */`.
- The runner's "Suggested annotations" output is the source of truth
  for column positions.
- `non-exhaustive` is an argument inside parens:
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):`.
- The "ambiguous call" diagnostic fires with cascading candidate
  notes (code 40011) — use `non-exhaustive` to pin only the primary
  error.

## INTERPRET gotchas

- `slangi` printf does not support `%s`. Use `%d` and integer tags
  to identify which overload fired.
- `slangi` cannot host `cbuffer` or non-const `static` module
  globals. Use `static const int` for module-scope constants.
- `slangi` cannot direct-invoke lambdas. Use ordinary free
  functions and struct methods.

## Sibling bundle to avoid duplicating

`tests-agentic/name-resolution/lookup/` covers how the **candidate
set** is built. Do not re-claim:

- container-level overload accumulation (lookup owns the
  accumulation; this bundle picks the winner from the set).
- inheritance-walk member lookup (lookup owns producing the
  candidate; this bundle picks among the inherited members).
- transparent-member resolution (lookup owns the breadcrumb
  construction).

## Quality checklist

- [ ] 15-25 tests; >= 30% are DIAGNOSTIC_TEST.
- [ ] Each algorithm section in the doc has at least one test.
- [ ] At least one operator-overload test.
- [ ] At least one witness-table dispatch test.
- [ ] At least one partial generic application test.
- [ ] No GPU-only directive.
- [ ] No test invented from source-line inspection. All claims
      anchor in the doc.
- [ ] `README.md` `## Doc gaps observed` is honest.
