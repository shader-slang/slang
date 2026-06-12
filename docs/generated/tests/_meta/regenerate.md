# Regeneration Workflow — docs/generated/tests

This document describes how to (re)generate, review, and remediate
test bundles under `docs/generated/tests/`. The pipeline is deliberately
**operator-driven**: the driver script (`regenerate.py`) tracks state,
but bundle generation is an out-of-band agent invocation that the
operator performs.

The shape mirrors the documentation regeneration workflow at
[`../../design/_meta/regenerate.md`](../../design/_meta/regenerate.md);
the two systems are siblings.

## Prerequisites

- Python 3.9+ (no third-party packages required; PyYAML is used if
  available but not mandatory).
- A working `git` and a clean checkout.
- An AI agent that can read files in the workspace and write text
  files. Generation, review, and remediation are model-agnostic at the
  driver level; soft refusal layers (see below) enforce which model
  family runs which stage.
- For Phase B2 and later: a `slang-test` build (Release) so bundles
  can be exercised.

## The driver

All commands run from the repository root.

```bash
python3 docs/generated/tests/_meta/regenerate.py <subcommand> [args...]
```

| Subcommand                                          | Purpose                                                                                             |
| --------------------------------------------------- | --------------------------------------------------------------------------------------------------- |
| `list`                                              | Print every bundle in the manifest                                                                  |
| `list-stale`                                        | Classify each bundle as `missing`, `stale`, or `fresh`                                              |
| `digest <bundle>`                                   | Compute current watched-paths and source-doc digests                                                |
| `show <bundle>`                                     | Manifest entry + resolved source files + source doc                                                 |
| `mark-fresh <bundle> [--commit SHA] [--model NAME]` | Record a fresh entry                                                                                |
| `lint [<bundle>...]`                                | Structural linter (README.md front-matter, every `.slang` has a `//META` block, `doc_ref` resolves) |
| `index [--write]`                                   | (Re)generate the suite `INDEX.md` navigation table                                                  |
| `doc-gaps [--source-doc <path>] [--format md\|json]` | Aggregate doc-gap rows across bundles, grouped by source doc                                       |
| `coverage-gaps <bundle> [--from <report.txt>]`      | (Phase E support) per-bundle uncovered-target rollup; bundle-level only                             |
| `expansion-candidates [--from <report.json>]`       | (Phase E) rank bundles by under-coverage; outputs bundle keys + scores only                         |
| `review-status / mark-reviewed / mark-remediated`   | (Phase D) two-stage review/remediation. Stubs currently.                                            |
| `findings list [--include-filed]`                   | (Phase F) list pending compiler-bug findings                                                        |
| `findings show <id>`                                | (Phase F) render the issue body markdown for a finding                                              |
| `findings file <id> [--dry-run]`                    | (Phase F) `gh issue create` + set project fields; moves YAML to `findings/filed/`                   |
| `findings dup <id> --of <issue-number>`             | (Phase F) record a finding as duplicate of an existing issue; no filing                             |

`<bundle>` is the manifest key (e.g. `pipeline/03-semantic-check`),
which equals the bundle directory under `docs/generated/tests/`.

## Phase status

| Phase | Description | Status |
| --- | --- | --- |
| A | Framework scaffold (driver, schemas, base prompts, manifest) | implemented |
| B1 | Bootstrap generation across 44 behaviorally-normative bundles | implemented |
| B1.5 / B1.6 / B1.7 / B1.8 | Boundary expansion, coverage-driven metadata refinement, catalog sweep, GPU-target expansion | implemented |
| B2 | `slang-test` nightly job runs the suite via `-test-dir docs/generated/tests/` | implemented |
| C | Cross-link pass — bundles consume each other's READMEs | planned |
| D | Review + remediation against a non-Claude model | scaffolded (schemas + prompts + state file in place) |
| E | Coverage-driven expansion loop | scaffolded (`coverage-gaps`, `expansion-candidates` stubs the data flow) |
| F | Structured compiler-bug findings + operator-driven filing | implemented |

To check the suite's current state:

```bash
python3 docs/generated/tests/_meta/regenerate.py list           # 44 bundle keys
python3 docs/generated/tests/_meta/regenerate.py list-stale     # should report 0 stale on a clean checkout
python3 docs/generated/tests/_meta/regenerate.py lint           # 0 errors, modest pre-existing warnings
python3 docs/generated/tests/_meta/regenerate.py show pipeline/01-lex-preprocess
```

## Phase B1 — Bootstrap generation

For each bundle, in dependency order (consult `depends_on` in the
manifest; bundles that list dependencies should be generated _after_
their dependencies so the agent can read those bundles' README.md
files as additional context):

1. Open the per-section prompt at
   `docs/generated/tests/_meta/prompts/<key>.md`. If it does not yet exist,
   author one using
   [`prompts/pipeline-03-semantic-check.md`](prompts/pipeline-03-semantic-check.md)
   as a template (target / required structure / doc sources /
   quality checklist).
2. Show the agent:
   - `_meta/prompts/_common.md`,
   - the per-section prompt,
   - the bundle's `source_doc` (the docs/generated/design/ file),
   - any allowed secondary docs the per-section prompt names,
   - any already-generated `depends_on` bundles' README.md.
3. Ask the agent to emit `README.md` plus N `.slang` files at
   `docs/generated/tests/<key>/`.
4. Run `regenerate.py lint <key>`. Fix structural issues by
   re-prompting — **never** by hand-editing.
5. Build `slang-test` (Release). Spot-check that each new `.slang`
   compiles under its declared target. The lint pass does not invoke
   `slangc`; this is a human sanity check.
6. `regenerate.py mark-fresh <key> --model <model-id>`.

## Phase B2 — Slang-test wiring

Landed via
`.github/workflows/ci-agentic-tests-nightly.yml`. The nightly job
invokes `regenerate.py verify`, which wraps `slang-test` with
`-test-dir docs/generated/tests` and applies the suite-level
expected-failures list + `requires-tool` filtering. The nightly is
advisory: it does not gate PR merges.

## Phase C — Cross-link pass

After every bundle has been generated at least once, re-run each
bundle with peer bundles' README.md as additional context. This pass
typically aligns terminology and removes redundancy between bundles
whose claims overlap (e.g. parser-level claims appearing in both
`pipeline/02-parse-ast` and `ast-reference/declarations`).

## Phase D — Review / remediation

Two-stage review identical in shape to the documentation flow:

1. **Review** — by a non-Claude model. Produces a structured report
   under `_meta/reviews/<key>.review.md`. Refusal banner enforced by
   the prompt and by `mark-reviewed` (when the wiring lands).
2. **Remediation** — by a Claude model. Edits the bundle to fix
   findings; records actions in `_meta/remediations/<key>.remediation.md`.

The schemas at `_meta/schema/{review-report,remediation-report}.schema.json`
define the report contracts.

The `mark-reviewed` / `mark-remediated` driver commands and the
`review-status` classifier are stubbed in Phase A; they print a
not-yet-implemented notice. The wiring lands when the first review
cycle is run.

## Phase E — Expansion loop

Wired into the nightly job after Phase B2 stabilizes.

Inputs to the loop: per-night coverage report (JSON) listing per-file
line-coverage percentages.

The driver computes per-bundle scores by averaging the coverage of
the bundle's `coverage_targets`. Bundles with the lowest scores are
re-prompted with `prompts/_expand.md`. The expansion prompt
deliberately does not pass source-line information to the agent — it
asks the agent to re-read the source doc, find under-tested claims,
and add tests for them.

If documented behavior cannot reach uncovered code, the agent records
that as a doc-gap finding in the bundle's `README.md` under
`## Doc gaps observed`. The loop does not paper over gaps by writing
source-targeted tests.

To trigger an expansion locally:

```bash
python3 docs/generated/tests/_meta/regenerate.py expansion-candidates \
    --from path/to/coverage-report.json
```

Output is bundle-key + score; no source-line detail leaks.

## Hand-edit policy

- `.slang` files in `docs/generated/tests/<key>/`: **no hand-edits**.
- `README.md`: **no hand-edits**.
- `_meta/manifest.yaml`, `_meta/schema/*`, `_meta/prompts/*`: hand-edited
  (these are the source of truth for regeneration).
- `_meta/freshness.json`, `_meta/review-state.json`: driver-edited.

If you find a test that's wrong, the right move is one of:

1. Improve the bundle's per-section prompt and re-run generation.
2. Improve the source documentation (a docs/generated/design change is
   a separate PR against the docs PR / docs branch).
3. Improve the manifest (add a watched path, raise the size cap).

Then `regenerate.py mark-fresh <bundle>`.

## CI integration (Phase B2 and later)

The intended attachment points:

- **Nightly run.** `ci-agentic-tests-nightly.yml`, scheduled
  `0 4 * * *` (after `coverage-nightly`'s `02:00` slot), runs
  `regenerate.py verify` which wraps
  `slang-test -test-dir docs/generated/tests`. Advisory only; never
  blocks PRs.
- **Lint on PR.** A check workflow that runs
  `regenerate.py lint` and `regenerate.py list-stale` on any PR
  touching `docs/generated/tests/` or `docs/generated/design/`. Soft warning;
  not a gate.

The driver has no third-party Python dependencies, so wiring this into
CI is just a `python3` invocation.
