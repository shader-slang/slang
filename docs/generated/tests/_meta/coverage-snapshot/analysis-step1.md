# Step 1 analysis — coverage ratchet (the guardrail)

**Goal of this step:** add the missing _enforcement_ so a coverage regression
like the one that started this whole effort can never land silently again.
This step deliberately adds **no tests** — it is the safety net for steps 2–3.

## What shipped

- `tools/coverage/coverage-floor.py` — `record` / `check` a **suite-level**
  floor on slangc compiler coverage.
- `docs/generated/tests/_meta/coverage-snapshot/floors.json` — the recorded
  floor (committed; small, config-like — unlike the bulky `*-coverage.json`
  data which stays local).
- Commit: `tools/coverage: add coverage ratchet (suite-level floor)`.

## Floor recorded (post-fan-out state, from the `step1` run)

| metric           | value                |
| ---------------- | -------------------- |
| covered lines    | **131,242** (52.00%) |
| covered regions  | 60,346 (53.94%)      |
| covered branches | 45,524 (49.94%)      |

`step1` coverage equals `after2` to the line (252,405 total; 131,242 covered) —
confirming Step 1 changed no tests. The only delta is 2 branches of run-to-run
nondeterminism, well within noise.

## Does it work?

Yes — verified both directions:

- `check` against `step1` (= floor) → **exit 0**, "coverage at or above floor."
- `check` against the pre-fan-out low `after` report (130,665 lines) →
  **exit 1**, reporting `lines_covered −577, regions −227, branches −249`.

So had this gate existed during the original regen, the −3,351-line drop would
have failed CI instead of merging unnoticed.

## Design choices & caveats (for review)

- **Suite-level, not per-bundle.** The runner already produces a merged suite
  total; per-bundle floors would need each bundle measured in its own profile
  (68 separate runs). Suite-level is enough to catch a net regression and is
  cheap. Per-bundle is a possible future refinement.
- **The floor is intentionally re-recordable** (`record --reason ...`). A
  legitimate compiler refactor can move coverage; the ratchet must not become
  a straitjacket. The discipline is "lower the floor only with a written
  reason," not "never lower it."
- **The floor is currently BELOW baseline** (131,242 vs the pre-regen 134,016
  = still −2,774 / −1.10pp). Step 1 locks in _today's_ state so it can't slip
  further. **Steps 2 and 3 are expected to RAISE this floor** as doc-driven
  expansion recovers coverage; I will re-record the floor upward after each.

## Step 1 outcome

Guardrail in place; floor = 131,242 lines. No coverage change (by design).
Proceeding to Step 2 (variant-matrix expansion), which is where coverage should
start climbing back toward the 134,016 baseline.
