# Campaign mode — autonomous lang-ref test generation

## What this is

A protocol document for an operator (or autonomous agent) running a
batch test-generation campaign against `docs/language-reference/`.
The campaign walks the language-reference tree, creating
`docs/generated/tests/language-reference/<doc>/` bundles for any
lang-ref document that hasn't been covered yet, and writes
boundary/corner-case tests against each documented claim.

This is the **B-tree** side of the
[`_common.md § Where the test lives`](prompts/_common.md) policy.

## Pre-flight

Before starting a campaign session:

1. Build slang locally (`cmake --build --preset release --target slang-test slangi slangc`).
2. Confirm `regenerate.py verify --help` is callable (the verify
   subcommand must be on the branch).
3. Confirm `regenerate.py lang-ref-coverage` returns the current
   baseline. This is the campaign's odometer.

## Per-document workflow

For each lang-ref doc in priority order:

1. **Triage the doc.**
   - Read it. Is the content normative (grammar productions,
     behavioural claims, table rows, "shall" / "must" sentences)?
     If it's pure prose intro, glossary, or marked `> TODO`, skip
     and record the skip rationale in
     `_meta/CAMPAIGN.md § Skipped docs`.
   - How many testable claims? If fewer than ~3, skip; the bundle
     overhead is not worth it.
2. **Decompose into sub-areas and pick claims.** A lang-ref doc
   usually clusters into 4–8 sub-areas (e.g. a "types" doc into
   declaration / element access / operators / element-wise vs
   matrix-multiply / standard aliases / memory layout). The bundle
   should produce ≥1 test per sub-area, several per the central
   sub-areas. Within each sub-area, prioritise:
   - Explicit numeric edges (MIN/MAX/zero) — boundary tests.
   - Carve-outs the doc explicitly names (e.g. "Slang follows HLSL
     in that …" — those are unusual claims unlikely to be tested
     elsewhere).
   - Yield contracts (prefix vs postfix; "yields old vs new value").
   - Suffix/grammar tables — one row per row, value-correctness
     check, plus a couple of corner cases.
   - Negative tests for claims that say "is rejected" / "diagnoses".
3. **Target test density.** Each bundle's count should be
   commensurate with the doc's surface area, not the bundle's
   author's appetite. Targets:

   | Doc size (anchors) | Minimum tests per bundle |
   | --- | --- |
   | Small (1–5 anchors) | 15–30 |
   | Medium (5–15 anchors) | 30–60 |
   | Large (15+ anchors) | 60–100+ |

   Counts come from **multiplying along axes** the doc already
   names: every documented numeric type × every documented edge
   value; every documented operator × every documented operand
   shape (scalar / vector / matrix); every system-value × every
   documented direction; every documented backend where emit
   behaviour is observable. A claim that "covers itself" with one
   test usually means a missed multiplication.

4. **Functional + emission pairs are the default.** For any claim
   that the doc says is observable in emitted text on a particular
   target, the bundle gets **two** test files:
   - a **functional** test — `//TEST:INTERPRET(filecheck=CHECK):`
     or `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu` —
     that observes the runtime value or behaviour via `printf` /
     output buffer.
   - an **emission** test —
     `//TEST:SIMPLE(filecheck=CHECK):-target hlsl` (and / or
     `-target spirv-asm` / `-target glsl` / `-target cuda`) —
     that pins the user-visible emitted text for the same claim.

   Naming convention: `<topic>-<sub-area>-functional.slang` and
   `<topic>-<sub-area>-emission.slang`. Cross-target emission can
   either live in one file with multiple `//TEST` directives or be
   split file-per-target when the emit pattern differs enough
   that CHECK lines would collide.

5. **Bundle README structure carries the detail.** The
   `## Functional coverage` table groups tests by sub-area; the
   Claim cell names the **specific** sub-area surface, not the
   doc's top-level concept. Reviewers should be able to read the
   table and know what every test is for without opening the
   `.slang` file. Example tone (excerpt from one bundle's table):

   > | `WaveActiveSum` after a divergent `if/else` reconverges and sums across the originally-divergent threads. | functional | [#wave-divergence](...) | wave-divergent-if-functional.slang |
   > | Same claim, observed via emitted SPIR-V `OpGroupNonUniformIAdd` / `OpControlBarrier`. | functional | [#wave-divergence](...) | wave-divergent-if-emission.slang |

   Lead with the **observation** (what the test asserts) and pin
   the diagnostic code (E####) or specific intrinsic / SPIR-V op
   when the claim hangs on one. Vague rows like "vector operators
   work" are a sign the bundle is under-decomposed.

6. **Create the bundle.**
   - Manifest entry under
     `docs/generated/tests/_meta/manifest.yaml` (alphabetical
     within the `language-reference/` group).
   - `source_doc` = the lang-ref path. `watched_paths` includes the
     lang-ref doc plus 1-2 most-relevant compiler source files.
   - Per-section prompt at
     `_meta/prompts/language-reference-<doc>.md`. Document the
     sub-areas, the claims under each, and the expected test
     density per sub-area. Reviewers and the next generation pass
     read the per-section prompt to understand the bundle's
     coverage strategy.
   - Bundle directory:
     `docs/generated/tests/language-reference/<doc>/`. Compute
     digests with `regenerate.py digest <bundle>`.
   - Bundle README with front-matter, Intent, Functional coverage,
     Untested claims, Doc gaps observed. The Functional coverage
     table follows the detailed format described in step 5.
7. **Write tests.** One `.slang` file per claim, with a `//META`
   block citing the most specific lang-ref anchor that covers the
   claim. Use the established patterns:
   - **Function-param to defeat the constant folder** for tests
     that observe optimization-pass behaviour or runtime arithmetic.
   - **Overload-probe** for tests that observe the *type* of a
     literal or expression (`int probe(int) { return 1; }
     int probe(uint) { return 2; }` etc.).
   - **`//TEST:INTERPRET(filecheck=CHECK):`** for value-correctness
     observations through `slangi` + `printf`.
   - **`//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu`** for
     dispatch-shape behavior (thread IDs, group IDs, atomics,
     groupshared, wave ops) where INTERPRET cannot model the
     execution model.
   - **`//TEST:SIMPLE(filecheck=CHECK):-target <backend>`** for
     emission-pinning tests. Pair with the corresponding functional
     test (step 4).
   - **`//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`** for "is rejected"
     claims, with caret-anchored `^^^^` annotations and an `E####`
     code.
8. **Verify each test as you write it.** `slang-test <path>` should
   print `100% of tests passed (1/1)` before you move to the next.
9. **When a test reveals a compiler bug:**
   - **Do NOT** run `regenerate.py findings file <id>` in campaign mode.
   - Write the structured finding YAML under
     `docs/generated/tests/_meta/findings/<id>.yaml` (not `/filed/`).
   - Add the affected test to
     `docs/generated/tests/_meta/expected-failures.txt` with a
     comment naming the pending-finding YAML path. The
     expected-failures lint accepts `_meta/findings/<id>.yaml` as a
     valid "tracking link" during campaign mode.
   - Continue.
10. **When a test reveals a doc-vs-compiler disagreement that isn't a
    compiler bug** (e.g. the doc fabricated a claim, the doc is
    ambiguous, the doc names a feature that doesn't exist): record a
    `drift-from-source` / `ambiguous-claim` row in the bundle README's
    `## Doc gaps observed` section. The doc-regen workflow consumes
    these.
11. **Lint and verify the bundle.**
    ```bash
    python3 docs/generated/tests/_meta/regenerate.py lint <bundle>
    python3 docs/generated/tests/_meta/regenerate.py verify <bundle>
    ```
    Lint must be clean. Verify must show `FAILED: 0` (any failures
    must be in `expected-failures.txt`).
12. **Commit per bundle.** One commit per bundle keeps the history
    reviewable. Suggested message:
    `language-reference/<doc>: <N> tests + <K> findings`.

## Stopping criteria

End the session and commit-+-push the current state if any of these
hold:

- Per-bundle verify shows `FAILED > 0` and the failure isn't a
  filed compiler bug.
- A doc's content is too sparse / non-normative to yield the
  density target in step 3 — record the skip and move on, don't
  end the session.
- You've added ≥150 tests in one session (diminishing returns;
  freshness review benefits from human eyes). At the per-bundle
  density targets in step 3 this is typically 2–4 bundles of
  medium / large size, or 5–6 small ones.
- Anchor coverage of the current bundle's `source_doc` reaches
  100% of normative anchors (every `##` / `###` heading that
  carries a behavioural claim has ≥1 test). Move to the next
  doc rather than padding the current bundle.

Do **not** end the session merely because the bundle has hit some
fixed test-count ceiling. The per-bundle target is a floor, not
a cap. If a doc warrants 80 tests, write 80.

## After the session

A human review pass:

1. Walk `_meta/findings/` (the un-filed YAMLs) and decide which to
   file as tracking issues via `regenerate.py findings file <id>`.
2. For findings that share a root cause, file one and de-dupe the
   others via `regenerate.py findings dup <id> --of <issue-number>`.
3. Update `expected-failures.txt` to replace the
   `_meta/findings/<id>.yaml` reference comments with the
   tracking-issue URLs the previous step produced.

## Coverage odometer

Run `regenerate.py lang-ref-coverage` at the start and end of every
session. The campaign's goal is monotonic-increasing per-file
coverage% across `docs/language-reference/`.

The full-coverage target:

- **Anchor coverage**: 100% of *normative* anchors (every `##` /
  `###` heading that carries a behavioural claim). Intro,
  glossary, navigation, and explicitly-`TODO`-marked sections are
  excluded by being recorded in `## Skipped docs` below — they
  stop counting against the goal.
- **Test density**: 800–1000 lang-ref-anchored tests total when
  the campaign is complete (≈ 200 normative anchors × ≈ 4 tests
  per anchor, derived from the per-bundle targets in step 3).
- **Distributed across**: ~20–25 substantive bundles, one per
  non-skipped lang-ref doc.

A bundle is **not** considered done at 1 test per anchor. The
density target in step 3 is the minimum before the bundle is
considered complete; coverage% (the anchor measurement) tells you
*which* anchors still have zero tests, density tells you whether
the anchors you have hit are tested in depth.

## Skipped docs (running list)

Update this list when a doc is triaged and deemed
unsuitable-for-now. Include the reason so future sessions don't
re-litigate the decision.

| Doc | Reason for skip | When |
| --- | --- | --- |
| `docs/language-reference/expressions-conversions.md` | Marked `> TODO`; section body is empty. | 2026-06-01 |
| `docs/language-reference/expressions-evaluation-classes.md` | Content is taxonomy of compile-time-vs-runtime classification with few user-observable claims; "static const forces compile-time" is the main testable claim but its observability is brittle. Revisit when reflection/sizeof surface is richer. | 2026-06-01 |
