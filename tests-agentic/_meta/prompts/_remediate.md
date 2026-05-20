# Remediation Prompt — tests-agentic bundles

> **Refusal banner.** If you do NOT self-identify as a Claude /
> Anthropic model, output the line
> `REFUSED: non-Claude model detected; the remediation step requires the same model family that generated the bundle`
> and stop. Remediation is intentionally performed by the same family
> that produced the tests, because the response actions (rewriting
> tests to match cited claims, adjusting metadata, escalating doc gaps)
> are tightly coupled to how the original generation interpreted the
> prompt.

## Your role

You are responding to a **review report** for one test bundle. The
report is at
`tests-agentic/_meta/reviews/<bundle-key>.review.md` and lists
findings against `tests-agentic/<bundle-key>/`.

For every finding, you take exactly one of these actions:

| Action                  | When                                                                                                                                                                      |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `fixed`                 | The finding is correct; edit the bundle to resolve it.                                                                                                                    |
| `rejected-bogus`        | The finding is factually wrong; the test is correct as written. Record why.                                                                                               |
| `rejected-out-of-scope` | The finding is correct in spirit but belongs in a different artefact (the doc, the prompt, the manifest), not the bundle. Record where the issue should be filed instead. |
| `deferred`              | The finding is real but resolving it requires regenerating the bundle from scratch or expanding a doc gap. Record what is blocked on.                                     |
| `escalated`             | The finding implies a human decision (e.g. a contested behavioral claim). Record what the human needs to decide.                                                          |

You produce a single **remediation report** at
`tests-agentic/_meta/remediations/<bundle-key>.remediation.md`. You
may also edit the test files in the bundle.

## What you have access to

The operator gives you:

- the review report from the previous stage;
- the bundle's `BUNDLE.md` and every `.slang` file;
- the bundle's `source_doc` and the per-section prompt + `_common.md`;
- the resolved watched source files at the **current** HEAD (not the
  bundle's recorded `source_commit` — if HEAD has moved, your fixes
  should reflect current state, and the operator will `mark-fresh` after
  you finish).

## Editing the bundle

When you take a `fixed` action that edits a `.slang` file:

- Update its `//META` block: bump `generated_at`, set `model` to your
  identifier, set `source_commit` to current HEAD, recompute
  `doc_section_digest` if `doc_ref` changed.
- Keep the rest of the `//META` contract intact (every required key,
  `generated=true`, `warning` banner).
- If you delete a test entirely, update the **Tests in this bundle**
  and **Claims enumerated** tables in `BUNDLE.md`.
- If you add a new test in response to a finding (e.g. the reviewer
  said an enumerated claim has no test), the new test's `//META` block
  uses your model identifier and **must** still derive from the source
  doc. Do not invent claims.

When you take a `fixed` action that edits `BUNDLE.md`:

- Bump `generated_at`, set `model` to your identifier, set
  `source_commit` to current HEAD.
- Recompute `watched_paths_digest` and `source_doc_digest` only if you
  also changed the underlying state.

## When to NOT fix

- **Source-line targeting.** If the reviewer flagged a test as written
  against a specific source line/branch, the correct response is
  `fixed`: rewrite or delete it. Do not preserve source-targeted tests.
- **Hallucinated claims.** If a test cites an anchor that doesn't
  contain the claim, the correct response is either `fixed` (rewrite
  the test against a real anchor) or `fixed` (delete it). Do not fix
  by inventing a new doc citation.
- **Doc gaps.** If the finding is "this claim should be tested but the
  doc never makes it", the correct response is `rejected-out-of-scope`
  with a pointer to file a doc improvement task. Do not add a test
  whose `doc_ref` you cannot defend.

## Report format

Save your output to
`tests-agentic/_meta/remediations/<bundle-key>.remediation.md`. Use
exactly this shape:

```markdown
---
remediation_report: true
remediator_model: <your model identifier, MUST contain "claude" or "anthropic">
remediated_at: <ISO 8601 timestamp, UTC>
target_bundle: <bundle-key>
review_report: tests-agentic/_meta/reviews/<bundle-key>.review.md
target_bundle_source_commit_before: <source_commit from BUNDLE.md before your edits>
target_bundle_source_commit_after: <git HEAD now>
actions:
  fixed: <int>
  rejected_bogus: <int>
  rejected_out_of_scope: <int>
  deferred: <int>
  escalated: <int>
---

# Remediation: <bundle-key>

## Summary

(One paragraph: how many findings, how many fixed vs deferred, any
escalations.)

## Actions

| Finding ID | Action                | Rationale                                                                                                            | Edits made                                                                        |
| ---------- | --------------------- | -------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------- |
| F-01       | fixed                 | doc_ref pointed at the wrong anchor; the claim about specialization preference is actually in #generic-substitution. | Updated overload-prefer-specialized.slang //META: doc_ref and doc_section_digest. |
| F-02       | rejected-out-of-scope | The reviewer wants a test for `extern` linkage corner cases, but the source_doc does not describe them.              | Filed as a doc-gap entry in BUNDLE.md "Doc gaps observed".                        |
| ...        | ...                   | ...                                                                                                                  | ...                                                                               |
```

Every finding ID from the review report must appear exactly once in
the actions table. The lint pass enforces this.

## After you finish

Run, in order:

```bash
python3 tests-agentic/_meta/regenerate.py lint <bundle>
python3 tests-agentic/_meta/regenerate.py mark-fresh <bundle> --model <your-id>
# (mark-remediated is a Phase D wiring; for now, the report is just saved.)
```

If `lint` fails, fix the structural issue (front-matter, doc_ref,
duplicate names) before declaring done.
