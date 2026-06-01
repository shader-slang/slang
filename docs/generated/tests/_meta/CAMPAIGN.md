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
2. **Pick claims.** Prioritize:
   - Explicit numeric edges (MIN/MAX/zero) — boundary tests.
   - Carve-outs the doc explicitly names (e.g. "Slang follows HLSL
     in that …" — those are unusual claims unlikely to be tested
     elsewhere).
   - Yield contracts (prefix vs postfix; "yields old vs new value").
   - Suffix/grammar tables — one row per row, value-correctness
     check, plus a couple of corner cases.
   - Negative tests for claims that say "is rejected" / "diagnoses".
3. **Create the bundle.**
   - Manifest entry under
     `docs/generated/tests/_meta/manifest.yaml` (alphabetical
     within the `language-reference/` group).
   - `source_doc` = the lang-ref path. `watched_paths` includes the
     lang-ref doc plus 1-2 most-relevant compiler source files.
   - Per-section prompt at
     `_meta/prompts/language-reference-<doc>.md`. Keep it concise:
     reference `_common.md` for universal rules; document the
     specific claims this bundle should cover and the observation
     tooling.
   - Bundle directory:
     `docs/generated/tests/language-reference/<doc>/`. Compute
     digests with `regenerate.py digest <bundle>`.
   - Bundle README with front-matter, Intent, Functional coverage,
     Untested claims, Doc gaps observed.
4. **Write tests.** One `.slang` file per claim, with a `//META`
   block citing the most specific lang-ref anchor that covers the
   claim. Use the established patterns:
   - **Function-param to defeat the constant folder** for tests
     that observe optimization-pass behaviour or runtime arithmetic.
   - **Overload-probe** for tests that observe the *type* of a
     literal or expression (`int probe(int) { return 1; }
     int probe(uint) { return 2; }` etc.).
   - **`//TEST:INTERPRET(filecheck=CHECK):`** for value-correctness
     observations through `slangi` + `printf`.
   - **`//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`** for "is rejected"
     claims, with caret-anchored `^^^^` annotations or an `E####`
     code (preferred).
5. **Verify each test as you write it.** `slang-test <path>` should
   print `100% of tests passed (1/1)` before you move to the next.
6. **When a test reveals a compiler bug:**
   - **Do NOT** run `regenerate.py findings file <id>` in campaign mode.
   - Write the structured finding YAML under
     `docs/generated/tests/_meta/findings/<id>.yaml` (not `/filed/`).
   - Add the affected test to
     `docs/generated/tests/_meta/expected-failures.txt` with a
     comment naming the pending-finding YAML path. The
     expected-failures lint accepts `_meta/findings/<id>.yaml` as a
     valid "tracking link" during campaign mode.
   - Continue.
7. **When a test reveals a doc-vs-compiler disagreement that isn't a
   compiler bug** (e.g. the doc fabricated a claim, the doc is
   ambiguous, the doc names a feature that doesn't exist): record a
   `drift-from-source` / `ambiguous-claim` row in the bundle README's
   `## Doc gaps observed` section. The doc-regen workflow consumes
   these.
8. **Lint and verify the bundle.**
   ```bash
   python3 docs/generated/tests/_meta/regenerate.py lint <bundle>
   python3 docs/generated/tests/_meta/regenerate.py verify <bundle>
   ```
   Lint must be clean. Verify must show `FAILED: 0` (any failures
   must be in `expected-failures.txt`).
9. **Commit per bundle.** One commit per bundle keeps the history
   reviewable. Suggested message:
   `language-reference/<doc>: <N> tests + <K> findings`.

## Stopping criteria

End the session and commit-+-push the current state if any of these
hold:

- Per-bundle verify shows `FAILED > 0` and the failure isn't a
  filed compiler bug.
- A single bundle takes more than ~30 minutes to write.
- A doc's content is too sparse / non-normative to yield ≥3 tests
  (record the skip and move on, don't end the session).
- You've covered ≥6 new bundles in one session (diminishing
  returns; freshness review benefits from human eyes).

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

## Skipped docs (running list)

Update this list when a doc is triaged and deemed
unsuitable-for-now. Include the reason so future sessions don't
re-litigate the decision.

| Doc | Reason for skip | When |
| --- | --- | --- |
| (none yet) | | |
