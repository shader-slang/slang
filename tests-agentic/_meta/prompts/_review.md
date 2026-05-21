# Review Prompt — tests-agentic bundles

> **Refusal banner.** If you self-identify as a Claude / Anthropic
> model, output the line
> `REFUSED: Claude model detected; the review step requires a different model family`
> and stop. The review step is intentionally performed by a different
> model family than the one that generated the bundle, so that
> hallucinations and contract violations are caught by a model with
> different blind spots.

## Your role

You are reviewing one **test bundle** at
`tests-agentic/<bundle-key>/`. The bundle was produced by an agent
from another model family using the prompt at
`tests-agentic/_meta/prompts/<per-section>.md` and
`tests-agentic/_meta/prompts/_common.md`, against the source documentation
at the bundle's `source_doc`.

You produce a single **review report** at
`tests-agentic/_meta/reviews/<bundle-key>.review.md`.

## What you have access to

The operator gives you:

- the bundle's `README.md` and every `.slang` file in the bundle;
- the bundle's `source_doc` (the docs/llm-generated/ file the tests are
  anchored to);
- the per-section prompt;
- `_common.md`;
- the resolved watched source files at the bundle's recorded
  `source_commit` (provided for verification, not for finding new test
  ideas).

## What you are checking — the contract

For each test in the bundle:

1. **Doc-anchored.** Does `//META: doc_ref` resolve to a real anchor in
   the `source_doc`? Does the cited section's text actually contain
   the claim the test is verifying?
2. **Claim verified.** Does the test's `purpose` paraphrase the cited
   section? Does the test body actually verify what the purpose claims?
3. **Not source-targeted.** Was this test written to exercise a
   documented claim, or was it written to hit a specific line/branch
   of `slangc`? If the latter, flag it. The bundle must contain no
   source-targeted tests.
4. **Compiles.** Does the test directive line look syntactically valid?
   Does the shader body look like it should compile under `slangc` with
   the declared target? (You do not actually run `slangc`; this is a
   reading review.)
5. **Intent classified correctly.** `functional` for a primary doc
   claim; `expansion` for a corner-case derived from re-reading the
   doc; `negative` for a diagnostic / failure test; `regression` only
   if it cites a fixed issue.
6. **No duplicates.** Two tests in the same bundle should not exercise
   the same anchor with the same shape.

For the bundle as a whole:

7. **Front-matter valid.** Every required key in README.md and in every
   `//META` block.
8. **Coverage table consistent.** Every `.slang` file in the bundle
   appears in the `## Coverage` table's Tests column exactly once. The
   Claim cell of each row matches the corresponding test's
   `//META: purpose=...` line verbatim. The Anchor column matches
   `//META: doc_ref=...`. The Intent column matches
   `//META: intent=...` (comma-separated when a claim row groups
   tests of mixed intent).
9. **Doc gaps recorded.** If the doc has visible claims that the
   bundle does not cover, the agent should have recorded them as rows
   in the `## Doc gaps observed` table. If you can see uncovered
   claims that are not listed there, flag them. Also check that each
   existing row's `Kind` matches the prose: e.g., a row whose Gap
   says "doc lists X but does not name a Slang surface" should be
   `missing-surface`, not `undocumented-behavior`.

## Report format

Save your output to
`tests-agentic/_meta/reviews/<bundle-key>.review.md`. Use exactly this
shape:

```markdown
---
review_report: true
reviewer_model: <your model identifier, must NOT contain "claude" or "anthropic">
reviewed_at: <ISO 8601 timestamp, UTC>
target_bundle: <bundle-key>
target_bundle_source_commit: <source_commit from README.md>
target_bundle_watched_paths_digest: <digest from README.md>
target_bundle_source_doc_digest: <digest from README.md>
source_commit: <git HEAD when you reviewed>
checklist:
  doc_anchored: pass | partial | fail
  claims_verified: pass | partial | fail
  test_compiles: pass | partial | fail
  no_source_targeting: pass | partial | fail
  intent_classification: pass | partial | fail
  front_matter_validity: pass | partial | fail
finding_count: <int>
severity_breakdown:
  critical: <int>
  major: <int>
  minor: <int>
  nit: <int>
---

# Review: <bundle-key>

## Summary

(One paragraph: overall verdict and the headline issue, if any.)

## Findings

| ID   | Severity | File                              | Description                                                                                                                                           | Evidence                                               | Recommendation                                                      |
| ---- | -------- | --------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------ | ------------------------------------------------------------------- |
| F-01 | major    | overload-prefer-specialized.slang | doc_ref points at #overload-resolution but the cited section says nothing about specialization preference; closest claim is in #generic-substitution. | source_doc lines 412–438 do not mention "specialized". | Either change doc_ref to #generic-substitution, or remove the test. |
| ...  | ...      | ...                               | ...                                                                                                                                                   | ...                                                    | ...                                                                 |

(If no findings, write `(no findings)` under `## Findings` and set
finding_count to 0 and every severity to 0.)
```

## Severity scale

- `critical` — bundle is unsafe to use (hallucinated claim, mismatched
  digests, test would never pass).
- `major` — wrong intent, broken doc_ref, source-targeted test.
- `minor` — duplicate coverage, misleading purpose text.
- `nit` — naming, prose, ordering.

## What you must NOT do

- Edit the bundle. Only the remediation step (next stage) may modify
  test files.
- Add suggestions that would require reading uncovered source-line
  numbers. The review is doc-anchored, not source-anchored.
- Invent claims. If the doc doesn't say it, flag the test for removal
  rather than inventing a citation for it.
