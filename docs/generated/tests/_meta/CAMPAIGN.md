# Campaign mode — autonomous test generation

## What this is

A protocol document for an operator (or autonomous agent) running a
batch test-generation campaign against a **source-doc tree**. The
campaign walks the tree, creating a test bundle for any doc that hasn't
been covered yet, and drives each bundle to 100% claim coverage.

The campaign is parameterized by which tree it runs against:

| Campaign        | Source-doc tree            | Bundle root                         | Source-of-truth weight                                                                  |
| --------------- | -------------------------- | ----------------------------------- | --------------------------------------------------------------------------------------- |
| **conformance** | `docs/language-reference/` | `docs/generated/tests/conformance/` | Authoritative human-written spec.                                                       |
| **design**      | `docs/generated/design/`   | `docs/generated/tests/design/`      | Reverse-engineered from (possibly buggy) source; defers to the spec when they disagree. |

See [`prompts/_common.md` § Source-of-truth hierarchy](prompts/_common.md)
for why `conformance/` outranks `design/` when both cover the same claim,
and [`prompts/_common.md` § Where the test lives](prompts/_common.md)
for the B-tree/A-tree placement policy.

The claim-enumeration and claim-to-test methodology is **tree-agnostic**
and lives in [`prompts/_claims.md`](prompts/_claims.md). This document is
the orchestration loop around it: triage, pacing, stopping criteria, and
findings handling.

## Pre-flight

Before starting a campaign session:

1. Build slang locally (`cmake --build --preset release --target slang-test slangi slangc`).
2. Confirm `regenerate.py verify --help` is callable (the verify
   subcommand must be on the branch).
3. For a **conformance** campaign, confirm `regenerate.py lang-ref-coverage`
   returns the current baseline. This is the campaign's odometer.

## Per-document workflow

For each doc in the source-doc tree, in priority order:

1. **Triage the doc.**
   - Read it. Is the content normative (grammar productions,
     behavioural claims, table rows, "shall" / "must" sentences)? If
     it's pure prose intro, glossary, or marked `> TODO`, skip and
     record the skip rationale in `§ Skipped docs`.
   - How many testable claims? If fewer than ~3, skip; the bundle
     overhead is not worth it.
2. **Enumerate claims and map them to tests** following
   [`prompts/_claims.md`](prompts/_claims.md) §§ 1–3. The enumerated
   claim list is the bundle's target; test count falls out of it.
3. **Create the bundle.**
   - Manifest entry under `_meta/manifest.yaml`, keyed
     `conformance/<doc>` or `design/<area>/<doc>` (alphabetical within
     its group). `source_doc` = the doc path; `watched_paths` includes the
     doc plus 1–2 most-relevant compiler source files.
   - Bundle directory: `docs/generated/tests/<key>/`.
   - Co-located `_prompt.md` in the bundle directory. Document the
     sub-areas and the claim-extraction strategy (which sentences in the
     doc count as claims, which are non-normative). The prompt does not
     duplicate the bundle README's claim list — it explains how that
     list was derived so a future regeneration produces a consistent
     enumeration.
   - Bundle README with front-matter, Intent, **Claims**, Functional
     coverage, Untested claims, Doc gaps observed — per
     [`prompts/_claims.md`](prompts/_claims.md) § 4. Compute digests
     with `regenerate.py digest <bundle>`.
4. **Write tests** following [`prompts/_claims.md`](prompts/_claims.md)
   §§ 3 & 5 — one `.slang` per (claim × dimension), functional +
   emission pairs by default, each with a `//META` block citing the most
   specific source-doc anchor.
5. **Verify each test as you write it.** `slang-test <path>` should
   print `100% of tests passed (1/1)` before you move to the next.
6. **When a test reveals a compiler bug:**
   - **Do NOT** run `regenerate.py findings file <id>` in campaign mode.
   - Write the structured finding YAML under
     `_meta/findings/<id>.yaml` (not `/filed/`). Its `bundle:` field is
     the bundle key (`conformance/<doc>` or `design/<area>/<doc>`).
   - Add the affected test to `_meta/expected-failures.txt` with a
     comment naming the pending-finding YAML path. The
     expected-failures lint accepts `_meta/findings/<id>.yaml` as a
     valid "tracking link" during campaign mode.
   - Continue.
7. **When a test reveals a doc-vs-compiler disagreement that isn't a
   compiler bug** (the doc fabricated a claim, is ambiguous, or names a
   feature that doesn't exist): record a `drift-from-source` /
   `ambiguous-claim` row in the bundle README's `## Doc gaps observed`
   section. The doc-regen workflow consumes these.
8. **Lint and verify the bundle.**
   ```bash
   python3 docs/generated/tests/_meta/regenerate.py lint <bundle>
   python3 docs/generated/tests/_meta/regenerate.py verify <bundle>
   ```
   Lint must be clean. Verify must show `FAILED: 0` (any failures must
   be in `expected-failures.txt`).
9. **Commit per bundle.** One commit per bundle keeps the history
   reviewable. Suggested message: `<key>: <N> tests + <K> findings`
   (e.g. `conformance/types-array: 6 tests`).

## Stopping criteria

A bundle is complete per
[`prompts/_claims.md`](prompts/_claims.md) § 6: every claim enumerated
in `## Claims` is either covered in `## Functional coverage` (along the
dimensions the doc commits to) or listed in `## Untested claims` with a
classified reason. Move to the next bundle once that holds.

End the session and commit-+-push the current state if any of these
hold:

- Per-bundle verify shows `FAILED > 0` and the failure isn't a filed
  compiler bug.
- A doc is non-normative (intro / glossary / navigation only) — record
  the skip in `§ Skipped docs` and move on, don't end the session.
- The next bundle's source_doc has unresolved structural questions about
  what counts as a claim (e.g. doc is mid-rewrite). Record the question
  in the bundle's `_prompt.md` and end the session for human review.
- Several bundles in sequence are surfacing compiler bugs that block
  forward progress; the findings backlog needs human triage before more
  tests are written against the same broken surface.

## After the session

A human review pass:

1. Walk `_meta/findings/` (the un-filed YAMLs) and decide which to file
   as tracking issues via `regenerate.py findings file <id>`.
2. For findings that share a root cause, file one and de-dupe the others
   via `regenerate.py findings dup <id> --of <issue-number>`.
3. Update `expected-failures.txt` to replace the
   `_meta/findings/<id>.yaml` reference comments with the tracking-issue
   URLs the previous step produced.

## Coverage odometer

For a **conformance** campaign, run `regenerate.py lang-ref-coverage` at
the start and end of every session. The campaign's goal is
monotonic-increasing per-file coverage% across `docs/language-reference/`.
(A **design** campaign uses the `coverage-targets` /
`expansion-candidates` reports against `docs/generated/design/` instead.)

Two coverage measures:

- **Anchor coverage** (what the tool currently reports): % of normative
  anchors in the doc that have at least one test referencing them. This
  is a proxy — easy to compute, but a 100% anchor-covered bundle can
  still have un-tested claims _within_ an anchor's section.
- **Claim coverage** (the real target): per the bundle README, the
  fraction of `## Claims` entries that appear in `## Functional
coverage`. 100% claim coverage is the bundle's done-state. The
  remaining `## Untested claims` must have a classified reason
  (`out-of-bundle` / `compiler-bug-pending` / `non-normative` /
  `unclassified`); `unclassified` rows are a reviewer-flag, not a closed
  bundle.

The campaign is complete when every non-skipped doc in the tree has a
corresponding bundle with 100% claim coverage. Test count falls out of
that — it is an output, not a target.

Anchor coverage is a useful proxy on the way there: a 0% anchor on a
normative section always signals that claim extraction for that section
hasn't happened yet. But 100% anchor coverage does not imply 100% claim
coverage — use the bundle README's `## Claims` vs `## Functional
coverage` diff as the authoritative measure.

## Skipped docs (running list)

Update this list when a doc is triaged and deemed unsuitable-for-now.
Include the reason so future sessions don't re-litigate the decision.

| Doc                                                         | Reason for skip                                                                                                                                                                                                                                  | When       |
| ----------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------- |
| `docs/language-reference/expressions-conversions.md`        | Marked `> TODO`; section body is empty.                                                                                                                                                                                                          | 2026-06-01 |
| `docs/language-reference/expressions-evaluation-classes.md` | Content is taxonomy of compile-time-vs-runtime classification with few user-observable claims; "static const forces compile-time" is the main testable claim but its observability is brittle. Revisit when reflection/sizeof surface is richer. | 2026-06-01 |
