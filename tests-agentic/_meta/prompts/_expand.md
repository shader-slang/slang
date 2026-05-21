# Expansion Prompt — tests-agentic bundles

You are being asked to **expand** an existing test bundle —
`tests-agentic/<bundle-key>/` — because a nightly coverage measurement
ranked it as under-covering its `coverage_targets`. The expansion loop
exists so that doc-anchored testing deepens in the areas where it is
visibly thin.

## The hard rule

You receive **no source-line information**. You do not see uncovered
lines, file diffs, percentages, or any data about which branches of
`slangc` are unexecuted. The operator gives you only:

- the bundle name,
- the bundle's `README.md` and existing `.slang` files,
- the bundle's `source_doc`, and
- the per-section prompt + `_common.md`.

The reason for this restriction: if a test were written against a
source line we observed to be uncovered, that test would codify
implementation rather than specification, and the suite's value (every
test maps to a documented claim) would erode. See
`tests-agentic/_meta/prompts/_common.md` for the full statement of this
contract.

## What to do

1. **Re-read the source doc carefully.** Do not skim. Look for
   behavioral claims you can find evidence for in the doc but that the
   existing bundle does not test (or tests only superficially).
   Examples of claims worth catching on a re-read:
   - implicit corollaries of a documented rule (e.g. "X is permitted
     only when Y" implies a negative test for "Y absent");
   - explicit examples in the doc that the bundle did not turn into a
     test;
   - error / diagnostic claims (e.g. "this produces error E1234")
     that are not covered;
   - target-specific behavior the doc enumerates (HLSL vs SPIR-V vs
     CUDA emit) where the bundle only tests one target;
   - parameter-shape variations the doc mentions (scalar / vector /
     matrix / array; precision modes; capability gates).

2. **Cross-check the README.md `## Claims enumerated` table.** If a
   claim is listed but its `Tests` cell is thinner than peer claims,
   that is a strong signal of where to expand. If a claim is _not_
   listed and the doc supports it, add it to the table.

3. **Add new `.slang` files** to the bundle, following the
   `//META` block contract in `_common.md`. Mark every new test with:

   ```
   //META: intent=expansion
   ```

4. **Do not modify existing tests.** Bootstrap tests are stable. If
   you find a flaw in an existing test, write the finding into
   `README.md` under `## Bootstrap test issues observed` and let the
   review/remediation loop handle it.

5. **If the doc legitimately does not describe the under-tested
   behavior**, the right output is **not** a synthesized test. The
   right output is a bullet under `## Doc gaps observed` in
   `README.md`, naming the area and suggesting which prompt or doc
   section needs to grow. Doc gaps are first-class output of the
   expansion loop.

## Update README.md

After adding new tests:

- Bump `generated_at` and `model` in the front-matter.
- Recompute `watched_paths_digest` and `source_doc_digest` only if the
  source side has actually changed since the bundle was last produced.
- Append new rows to the **Tests in this bundle** table.
- If you found new claims worth testing, append them to **Claims
  enumerated** with new claim IDs.
- Update **Doc gaps observed** with anything you turned away from.

## Caps

The manifest's `size_cap_files` is a soft cap. If an expansion would
push the bundle past the cap, prioritize:

1. negative / diagnostic tests for claims that have only positive
   tests;
2. additional targets for claims that are currently tested on only one
   target;
3. additional parameter shapes (vector/matrix/array) for claims that
   are currently tested only on scalars.

If you must cut, cut depth before breadth: it is more valuable to have
one test per claim than three tests for one claim and none for the
next.

## Verify

Run, in order:

```bash
python3 tests-agentic/_meta/regenerate.py lint <bundle>
python3 tests-agentic/_meta/regenerate.py mark-fresh <bundle> --model <your-id>
```

Fix any lint errors by re-reading the source doc, not by adjusting the
tests to silence the linter.
