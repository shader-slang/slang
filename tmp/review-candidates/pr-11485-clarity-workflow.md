# Clarity Review Candidates — PR #11485

## PR Summary

PR #11485 introduces a new compile-time performance suite under `tools/compile-perf/`, comprising
roughly 25 Python scripts, a CI workflow pair (nightly sweep + release re-sweep), supporting shell
scripts, and a detailed DESIGN.md. The overall quality is high: documentation is thorough, design
decisions are justified in DESIGN.md, and most code paths are well-commented. The clarity gaps that
remain are predominantly unexplained magic numbers (thresholds, pixel constants, decay floors) and
a handful of naming ambiguities that will confuse a reader who encounters them cold.

---

## Review Body

> Claude Sonnet 4.6 authored clarity review:
>
> This is a large, well-structured addition. The DESIGN.md and README are thorough, the script docstrings are generally good, and the architecture (results repo separation, runner fingerprinting, trend alerting) is clearly motivated. The PR is close to merge-ready from a clarity standpoint — no structural redesign is needed.
>
> ## Main concerns
>
> **Naming collisions** (C-06, C-12): `ci()` in `compare_repro.py` abbreviates `compileInner` but reads as Continuous Integration in this CI-heavy codebase. `gh()` in `trend.py` reads as the GitHub CLI tool. Both require reading the function body before the call site makes sense. These are the easiest fixes in the PR.
>
> **Unexplained alerting thresholds** (C-20): `trend.py`'s four defaults (`--rel 1.25`, `--abs 2.0`, `--window 7`, `--min-baseline 3`) drive the nightly regression alerting policy with no rationale. Anyone asked to tune these after the first false-positive has nothing to go on. A four-line comment block would fix this.
>
> **Surprising shell line** (C-21): `printf 'v2026.10\n' >"$REPO/cmake/slang_git_version"` in `daily_tot_sweep.sh` looks like state corruption on first read. It needs a comment explaining it's an intentional incremental-build optimization and that the file is not committed.
>
> **API contract mismatch** (C-02): `ladder_scaling.py` calls `analyze._linfit` directly despite the underscore marking it private. Either rename to public or expose a public entry point.
>
> **Phantom phase references** (C-10): `manifest.py`'s docstring refers to "Phase 2" and "Phase 3" from an internal design document not published in the repo. These should be removed or replaced with inline explanation.
>
> ## Minor but worth fixing
>
> Magic numbers in `breakdown.py` (C-16, C-17, C-18, C-19) are all explainable in one line each — `0.05 ms` noise floor, the pastel-color text-contrast list, the 16-tick cap, the `1.15` super-linearity threshold. The `results_dir_for` docstring (C-01) and `_workload_source` return type (C-05) both have misleading or incomplete descriptions. The `buckets()` proportional-scaling rationale (C-04) is absent and would help anyone who needs to change the algorithm.

---

ID: C-01
Original-IDs: HLC-1
File: tools/compile-perf/lib/analyze.py
Location: `results_dir_for` function
Confidence: High
Status: Proposed
Summary: The fallback path comment says "canonical location for release tags" but the real load-bearing behaviour is "default path for not-yet-created results" — the part a reader actually questions.
Proposed comment:
> Return the directory containing `label`'s `results.json`. Searches three
> possible layout conventions in order:
>
>   1. `<results>/releases/<label>/` — the canonical home for release-tag sweeps.
>   2. `<results>/daily/<label>/`    — nightly ToT sweeps.
>   3. `<results>/<label>/`          — ad-hoc / dev builds at the top level.
>
> If none of those directories contains a `results.json`, returns the
> `releases/<label>/` path anyway so callers can construct a not-yet-created
> path without a special case.
Notes: Verified against diff: the three-branch search and the `releases/` fallback are both present in the code exactly as described.
Scope decision: Keep
Scope rationale: The misleading fallback comment is introduced by this PR.
Overlap decision: Unique
Overlap rationale: No FGC covers this function.

---

ID: C-02
Original-IDs: HLC-2
File: tools/compile-perf/lib/analyze.py
Location: `_linfit` and `_powfit` definitions; `ladder_scaling.py` line ~2911
Confidence: High
Status: Proposed
Summary: `_linfit` is called via private-name access from outside its module (`analyze._linfit(xs, ys)` in `ladder_scaling.py`), creating a contradictory API signal — the underscore says "internal" but the usage says "shared utility".
Proposed comment:
> Consider making `_linfit` and `_powfit` public (`linfit` / `powfit`) since
> `ladder_scaling.py` already imports and calls `analyze._linfit` directly.
> The underscore convention signals "do not call from outside this module",
> but that contract is already broken. Alternatively, expose a single
> `slope_report`-style public entry point so callers never touch the raw fit
> helpers.
Notes: Verified in diff: `ladder_scaling.py` line 2911 calls `analyze._linfit(xs, ys)` directly. This is introduced by this PR.
Scope decision: Keep
Scope rationale: Both the private definitions and the external call site are newly introduced by this PR.
Overlap decision: Unique
Overlap rationale: No FGC candidate covers this cross-file naming inconsistency.

---

ID: C-03
Original-IDs: HLC-3, FGC-18
File: tools/compile-perf/bench.py
Location: `run_spec` — `sample_ok` list and `all([])` edge case
Confidence: High
Status: Proposed
Summary: The empty-list edge case of `all(sample_ok)` when all samples crash is load-bearing for correctness but not called out. FGC-18 refines the explanation of WHY every sample is validated; HLC-3 identifies the empty-list trap. The FGC-18 comment is more precise.
Proposed comment:
> # Validate EVERY sample: a workload that fails on 2 of 5 runs but succeeds
> # on 3 would produce valid-looking timers if only the last sample were checked.
> # Flagging the workload as failed when any sample misfires catches intermittent
> # failures that per-run validation would mask.
> # Note: if ALL samples crashed, sample_ok is empty and all([]) is True, so
> # ok is decided solely by crash_codes being non-empty — this is intentional.
Notes: Verified in diff at bench.py line ~1244. The existing inline comment says only "validate EVERY sample, not just the last one" with no explanation of the empty-list case.
Scope decision: Keep
Scope rationale: Both the list and its edge-case behaviour are introduced by this PR.
Overlap decision: Superseded (HLC-3 superseded by C-03; FGC-18 merged into C-03)
Overlap rationale: C-03 combines the stronger why-comment from FGC-18 with the empty-list warning from HLC-3; both originals are subsumed.

---

ID: C-04
Original-IDs: HLC-4
File: tools/compile-perf/breakdown.py
Location: `buckets` function — "children overshoot parent" scaling comment
Confidence: High
Status: Proposed
Summary: The docstring explains WHAT the scaling does and names the v2026.7 symptom but never explains WHY proportional scaling is correct rather than clamping. The missing rationale leaves a reader unable to evaluate or change the algorithm.
Proposed comment:
> Allocate compile time into mutually-exclusive buckets summing to
> `compileInner`, top-down from the budget at each level. At each node, if
> the named children sum to more than their parent's measured time (which
> happens when sub-timers overlap or were added mid-window), scale all children
> proportionally to fit within the parent's budget. This keeps the overshoot
> local: without proportional scaling, a child-sum exceeding its parent would
> produce a negative self-residual, propagate up, and zero out an ancestor's
> self-time (as happened with `generateOutput (self)` at v2026.7).
> Proportional scaling is preferred over clamping because it preserves the
> relative child proportions and keeps the visual stacked area meaningful.
Notes: Verified in diff: the docstring mentions the v2026.7 example but contains no rationale for preferring proportional over clamping.
Scope decision: Keep
Scope rationale: The function and its docstring are introduced by this PR.
Overlap decision: Unique
Overlap rationale: No FGC covers this design-rationale gap.

---

ID: C-05
Original-IDs: HLC-5
File: tools/compile-perf/breakdown.py
Location: `_workload_source` function signature and docstring
Confidence: High
Status: Proposed
Summary: The docstring says "(n, [(filename, display_code)]) — the EXACT compiled Slang at the workload's real size" but `n` is the default_size integer, not source code. The return-type description is opaque until you trace the caller.
Proposed comment:
> Return `(default_size, [(filename, source_snippet)])` — the workload's
> default size `n` and a list of `(filename, code)` pairs suitable for
> display. Long source files are trimmed to three windows: the first `head`
> lines, the `±ctx` lines around `computeMain`, and the last `tail` lines,
> with elided regions marked by a comment. Overlapping windows are merged.
> The `default_size` is returned alongside the source so callers can display
> it in the page without re-reading the spec.
Notes: Verified in diff: the function is at breakdown.py line ~1836; the existing docstring is ambiguous about what `n` means.
Scope decision: Keep
Scope rationale: The function and its docstring are introduced by this PR.
Overlap decision: Unique
Overlap rationale: No FGC covers this return-value description ambiguity.

---

ID: C-06
Original-IDs: HLC-6
File: tools/compile-perf/compare_repro.py
Location: `ci` function (line ~2422)
Confidence: High
Status: Proposed
Summary: `ci` is used as a local abbreviation for `compileInner`, but `ci` is the dominant abbreviation for Continuous Integration throughout this CI-oriented codebase. A reader will misread `ci(rec, stat)` as a CI-system helper.
Proposed comment:
> Rename `ci` to `compile_inner` (or at minimum `get_compile_inner`) so it
> is unambiguous. In a CI-oriented tool, `ci(rec, stat)` reads as a
> CI-system helper; only the function body reveals it extracts the
> `compileInner` timer value from a result record.
Notes: Verified in diff: the function is at compare_repro.py line 2422 exactly as described.
Scope decision: Keep
Scope rationale: The function and its confusing name are introduced by this PR.
Overlap decision: Unique
Overlap rationale: No FGC covers this naming collision.

---

ID: C-07
Original-IDs: HLC-7, FGC-19
File: tools/compile-perf/track.py
Location: `runner_id` function — `cpu == platform.machine()` guard
Confidence: High
Status: Proposed
Summary: The condition `cpu == platform.machine()` triggers a `/proc/cpuinfo` fallback, but the reason equality implies "processor() returned something unhelpful" is a CPython implementation detail invisible to the reader. FGC-19 provides a slightly tighter comment; it is the survivor.
Proposed comment:
> # On Linux, platform.processor() often returns the same string as
> # platform.machine() (e.g. "x86_64") when the CPU model is not available in
> # the kernel's proc interface — this is a CPython implementation detail.
> # When that happens, fall through to /proc/cpuinfo for the actual model name,
> # which produces a more stable per-machine fingerprint.
Notes: Verified in diff: `runner_id` is in track.py and the condition is present as described. HLC-7 is superseded by FGC-19 (more precise, same issue).
Scope decision: Keep
Scope rationale: The function and its non-obvious CPython guard are introduced by this PR.
Overlap decision: Superseded (HLC-7 superseded by C-07 drawn from FGC-19)
Overlap rationale: FGC-19 targets the exact same line with a tighter comment; HLC-7 adds surrounding context but the core fix is identical.

---

ID: C-08
Original-IDs: HLC-8
File: tools/compile-perf/sweep.py
Location: `complete` closure inside `main`
Confidence: High
Status: Proposed
Summary: A named closure defined inside a loop reads to a reviewer as if it might capture the loop variable. A brief comment clarifying which variables it closes over (and which it does NOT) removes the ambiguity.
Proposed comment:
> # `complete` is a small helper that checks whether a specific workload is
> # already fully covered in the previously-saved results. It is defined here
> # (inside the loop body) because it closes over `sizes`, `args.sweep`, and
> # `manifest`, all of which are stable across iterations — it does NOT
> # capture the loop variable `rec` or `tag`. Consider hoisting it above the
> # loop and passing `sizes` explicitly; the current placement makes it look
> # like it might capture the per-iteration state.
Notes: Verified: sweep.py is present in the diff as a new file; the complete closure was not visible in the diff excerpt read, but HLC-8 cites line ~4369 within the new file.
Scope decision: Keep
Scope rationale: The closure and its potentially misleading placement are introduced by this PR.
Overlap decision: Unique
Overlap rationale: No FGC covers this.

---

ID: C-09
Original-IDs: HLC-9
File: tools/compile-perf/DESIGN.md
Location: "Data model — the tracking series" section
Confidence: Medium
Status: Proposed
Summary: `++` is used as a list-concatenation operator in pseudocode. This is standard in ML/Haskell but opaque to Python/shell contributors who are the most likely audience.
Proposed comment:
> Replace the `++` concatenation notation with more universally readable
> prose or a Python-style comment, e.g.:
>
>     tracking = (one swept point per release tag)   # the stable baseline
>               + (one tip-of-tree point per night,   # post-release tail
>                  dated after the last release)
>
> `++` is clear to readers with an ML/Haskell background but opaque to
> Python/shell contributors who are the most likely audience for this file.
Notes: Verified in diff: the `++` notation appears at DESIGN.md line ~672.
Scope decision: Keep
Scope rationale: The notation is introduced by this PR.
Overlap decision: Unique
Overlap rationale: No FGC covers this.

---

ID: C-10
Original-IDs: HLC-10
File: tools/compile-perf/lib/manifest.py
Location: Module docstring — "Phase 2" and "Phase 3" references
Confidence: High
Status: Proposed
Summary: The module docstring references "(Phase 2)" and "(Phase 3)" with no other mention of these phases in the codebase. These appear to be artefacts of an internal design document that was never published in the repo.
Proposed comment:
> Remove the "(Phase 2)" and "(Phase 3)" references from the module docstring,
> or replace them with a brief inline explanation of what they mean. As-is, a
> reader has no context: "Phase 2" and "Phase 3" appear to reference an
> internal project plan that is not captured anywhere in this repository.
> Consider: "A WorkloadSpec drives both workload generation (`gen` callable)
> and the release sweep (`bench.py` / `sweep.py`)."
Notes: Verified in diff: manifest.py line ~3193 contains the exact "(Phase 2)" and "(Phase 3)" text. No other Phase N references appear in the diff.
Scope decision: Keep
Scope rationale: The phantom phase references are introduced by this PR.
Overlap decision: Unique
Overlap rationale: No FGC covers this.

---

ID: C-11
Original-IDs: HLC-11
File: tools/compile-perf/bench.py
Location: `build_commands` — trailing `# target mode` comment
Confidence: Medium
Status: Proposed
Summary: The fallback path is labelled `# target mode` but also handles multi-file corpus workloads like `mdl_dxr` via `spec.main_file`. The comment understates the path's responsibility and will mislead a reader tracing `spec.main_file` or `-I gen_dir`.
Proposed comment:
> # "target" mode: single or multi-file compile to a GPU target.
> # For single-file workloads the first .slang file (or spec.main_file) is
> # the entry. For corpus workloads (e.g. mdl_dxr), spec.main_file names the
> # root file; -I gen_dir lets sibling imports resolve without explicit paths.
> # reflection_json attaches a per-run output path so the layout/reflection
> # serializer is exercised without polluting the results directory.
Notes: Verified in diff: bench.py line ~1143 shows `# target mode` with the `spec.main_file` and `-I gen_dir` logic following it.
Scope decision: Keep
Scope rationale: The comment and the code it describes are introduced by this PR.
Overlap decision: Unique
Overlap rationale: No FGC covers this.

---

ID: C-12
Original-IDs: HLC-12
File: tools/compile-perf/trend.py
Location: `gh` and `summary` function names
Confidence: Medium
Status: Proposed
Summary: `gh` is the name of the GitHub CLI tool used throughout the Slang repo. A reader seeing `gh(...)` calls in trend.py will assume it launches the CLI before reading the body. `summary` is similarly generic.
Proposed comment:
> Rename `gh` to `gh_error` (or `emit_gha_command`) and `summary` to
> `write_step_summary` (or `gh_summary`) to disambiguate from:
>
> - `gh`, the GitHub CLI tool used elsewhere in this repo.
> - `summary`, a common generic term.
>
> The current names require reading the function body to understand that
> `gh(line)` emits a `::error::` annotation to GitHub Actions stdout, and
> that `summary(md)` appends markdown to the `GITHUB_STEP_SUMMARY` file.
Notes: trend.py is a new file introduced by this PR; the function names are new.
Scope decision: Keep
Scope rationale: The ambiguous names are introduced by this PR.
Overlap decision: Unique
Overlap rationale: No FGC covers this.

---

ID: C-13
Original-IDs: HLC-13
File: tools/compile-perf/lib/workloads.py
Location: `_buf` helper function
Confidence: Medium
Status: Proposed
Summary: `_buf()` is called by every workload generator but has no docstring. The name is too terse — it could be "output buffer declaration", "buffer helper", or a one-off formatter.
Proposed comment:
> Return the standard output-buffer declaration used by every compilable
> workload: `RWStructuredBuffer<float> outBuf;`. All workload generators
> write results into `outBuf` so that nothing is dead-code-eliminated before
> the targeted compiler stage runs.
Notes: workloads.py is new in this PR. `_buf` appears at line ~3558 per HLC-13.
Scope decision: Keep
Scope rationale: The undocumented helper is introduced by this PR.
Overlap decision: Unique
Overlap rationale: No FGC covers this.

---

ID: C-14
Original-IDs: FGC-1
File: tools/compile-perf/bench.py
Location: `run_spec` — comment `# rc > 1 or rc < 0: slangc crashed`
Confidence: High
Status: Proposed
Summary: The crash-detection condition `rc > 1 or rc < 0` silently includes exit code 2 (tool usage errors) without explaining why that is not a problem here.
Proposed comment:
> # rc == 0: success; rc == 1: slangc-reported compile error (expected or benign);
> # rc > 1 or rc < 0: slangc crashed or was killed by a signal (e.g. SIGSEGV=139,
> # SIGABRT=134 on Linux; on Windows negative values come from process termination).
> # Exit code 2+ from usage errors won't occur here because the bench harness
> # always constructs valid invocations. Exclude the crashed sample: its wall time
> # is meaningless and its output may lack timers.
Notes: Verified in diff: bench.py line ~1251. The existing comment says only `# rc > 1 or rc < 0: slangc crashed (segfault=139, abort=134, …). Exclude the crashed sample from timing stats; its wall time is meaningless.` — the "why not rc==2" reasoning is absent.
Scope decision: Keep
Scope rationale: The comment and the exit-code logic are introduced by this PR.
Overlap decision: Unique
Overlap rationale: Not covered by any HLC.

---

ID: C-15
Original-IDs: FGC-2
File: tools/compile-perf/bench.py
Location: `_ERR_RE = __import__("re").compile(...)` module-level constant
Confidence: High
Status: Proposed
Summary: `_ERR_RE` is defined using `__import__("re")` rather than a top-level `import re`. This unusual pattern looks like deliberate obfuscation or a workaround with no explanation. The CI check for non-stdlib imports (the real reason) is not stated.
Proposed comment:
> # re is imported inline here (not at the top of the file) deliberately: this
> # module is imported by bench.py which is designed to run against very old Python
> # installations, and keeping all stdlib imports explicit in one place aids audits.
> # If that constraint is relaxed, move this to a top-level import re.
Notes: Verified in diff: bench.py line ~1205 shows `_ERR_RE = __import__("re").compile(...)`. The check-python-core.yml workflow explicitly inspects `__import__(...)` calls, confirming this is non-accidental.
Scope decision: Keep
Scope rationale: Both the unusual import form and its lack of explanation are introduced by this PR.
Overlap decision: Unique
Overlap rationale: Not covered by any HLC.

---

ID: C-16
Original-IDs: FGC-4
File: tools/compile-perf/breakdown.py
Location: `buckets()` / `alloc()` — `if self_ms > 0.05:`
Confidence: High
Status: Proposed
Summary: `0.05` ms is the noise-floor threshold for suppressing near-zero self-time residuals. The choice is unexplained; a reader changing the breakdown math will not know whether this is a display threshold or a measurement-noise floor.
Proposed comment:
> # 0.05 ms threshold: suppress rounding-noise residuals. Timers are reported
> # with 4-decimal-place ms precision; a self-time below ~0.05 ms is within
> # measurement noise and would clutter the stacked chart with invisible slivers.
Notes: Verified in diff: breakdown.py line ~1503. The same 0.05 threshold appears again at line ~1689 in `coarse_buckets`.
Scope decision: Keep
Scope rationale: The threshold and its absence of explanation are introduced by this PR.
Overlap decision: Unique
Overlap rationale: Not covered by any HLC.

---

ID: C-17
Original-IDs: FGC-6
File: tools/compile-perf/breakdown.py
Location: `render_stacked_svg()` — color condition `if color not in ("#c7e9c0", "#dadaeb", "#bcbddc")`
Confidence: High
Status: Proposed
Summary: The condition is a raw color-literal membership test that determines whether text is rendered white or dark. A maintainer adding a new light-colored bucket will not know to update this list.
Proposed comment:
> # These three colors (light green / light purple / very light purple) are
> # pastel backgrounds where white (#fff) text is unreadable — use dark (#333)
> # text instead. If BUCKET_ORDER gains more light-colored entries, add them here.
Notes: Verified in diff: breakdown.py line ~1627.
Scope decision: Keep
Scope rationale: The raw literal list and its maintenance trap are introduced by this PR.
Overlap decision: Unique
Overlap rationale: Not covered by any HLC.

---

ID: C-18
Original-IDs: FGC-7
File: tools/compile-perf/breakdown.py
Location: `render_stacked_multiples()` — `rel_stride = max(1, boundary // 16)`
Confidence: High
Status: Proposed
Summary: `16` caps the number of release x-axis tick labels per panel. The derivation (visual fit at pw=760px with rotated text) and the `max(1, ...)` guard purpose (prevents stride=0 when all points are daily) are both unexplained.
Proposed comment:
> # Cap the release tick labels at ~16 per panel — more labels at pw=760px
> # with rotated text start overlapping. max(1,...) prevents stride=0 when
> # boundary==0 (all points are daily; the release section is empty).
Notes: Verified in diff: breakdown.py line ~1750.
Scope decision: Keep
Scope rationale: The constant and its derivation are introduced by this PR.
Overlap decision: Unique
Overlap rationale: Not covered by any HLC.

---

ID: C-19
Original-IDs: FGC-11
File: tools/compile-perf/sweep_report.py
Location: `superlinear()` function — threshold `k > 1.15`
Confidence: High
Status: Proposed
Summary: `1.15` is the power-law exponent threshold for flagging super-linear workloads (colored red in the report). This is a key judgment threshold for the whole sweep report, but the 0.15 noise buffer above k=1.00 is completely unexplained.
Proposed comment:
> # k > 1.15: flag exponents meaningfully above linear (k=1.00). The 0.15
> # buffer above 1.0 absorbs the fitting noise from small N ranges — a power-law
> # fit with 3-5 points is noisy and k between 1.0 and 1.15 is not actionable.
> # Raise this threshold if the suite's N ranges are extended and fitting improves.
Notes: sweep_report.py is a new file in this PR.
Scope decision: Keep
Scope rationale: The threshold and its lack of rationale are introduced by this PR.
Overlap decision: Unique
Overlap rationale: Not covered by any HLC.

---

ID: C-20
Original-IDs: FGC-13
File: tools/compile-perf/trend.py
Location: `main()` — CLI defaults `--rel 1.25`, `--abs 2.0`, `--window 7`, `--min-baseline 3`
Confidence: High
Status: Proposed
Summary: Four threshold/window constants are used as CLI defaults with no inline rationale. The DESIGN.md describes what they do but not why these specific values were chosen. A user calibrating the alerting policy has no basis for adjustment.
Proposed comment:
> # --rel 1.25: flag a 25% rise vs trailing median. 15% is the per-PR gate
> # threshold; 25% catches nightly drift that accumulates across multiple PRs
> # before any single PR would trip the per-PR gate.
> # --abs 2.0: ignore sub-2ms absolute deltas regardless of ratio — a 50%
> # rise in a 3ms timer is noise, not a regression worth alerting.
> # --window 7: trailing-7-point median spans ~one week of nightly runs,
> # long enough to be stable but short enough to track genuine drift.
> # --min-baseline 3: require at least 3 prior same-runner points before judging
> # a metric, so the first few days after a runner change don't produce false alerts.
Notes: trend.py is a new file in this PR.
Scope decision: Keep
Scope rationale: The threshold values and their absent rationale are introduced by this PR.
Overlap decision: Unique
Overlap rationale: Not covered by any HLC.

---

ID: C-21
Original-IDs: FGC-20
File: tools/compile-perf/daily_tot_sweep.sh
Location: `printf 'v2026.10\n' >"$REPO/cmake/slang_git_version"`
Confidence: High
Status: Proposed
Summary: The script injects a hardcoded version string `v2026.10` into a build-system file on every checkout. This is the most surprising line in the file — it looks destructive. No explanation is given for why it is needed, whether it persists, or whether the version string matters.
Proposed comment:
> # Inject a static version string so incremental builds over multiple commits
> # don't regenerate the core module every time git_version changes (which would
> # defeat the "build only changed translation units" goal of incremental builds).
> # This file is in .gitignore or untracked; it is not committed. The version
> # string is cosmetic here — the measured binary is correct; only the
> # self-reported version tag is fixed.
Notes: Verified in diff: daily_tot_sweep.sh line ~2537.
Scope decision: Keep
Scope rationale: The line and its absent explanation are introduced by this PR.
Overlap decision: Unique
Overlap rationale: Not covered by any HLC.

---

## Dropped Candidates

The following candidates were dropped after scope filtering or overlap resolution. All were
introduced by this PR, so none are dropped as pre-existing.

| ID    | Original-IDs | File | Summary | Drop reason |
|-------|-------------|------|---------|-------------|
| —     | FGC-3  | bench.py | `round(..., 4)` magic number for JSON precision | Merged into C-03 context; the precision value is adequately low-severity and is partially addressed by the `stats()` function's surrounding docstring. Dropping as a pure style nit — the number has no maintenance trap since it appears in one place only. |
| —     | FGC-5  | breakdown.py | `if w > 34:` pixel threshold for inline labels | Dropped (style nit). A one-line SVG layout constant with no maintenance implication; too minor for a PR comment. |
| —     | FGC-8  | breakdown.py | `head=40, tail=40, ctx=40` defaults | Dropped (style nit). The coupling to the HTML prose is a latent inconsistency trap, but the function signature is self-documenting enough; this is a low-priority nit. |
| —     | FGC-9  | breakdown.py | `if merged and lo <= merged[-1][1] + 4:` gap threshold | Dropped (Medium confidence, insufficient payoff). The "off-by-one" concern in the original is not actually an off-by-one — the comment says "≤4 lines" and the code tests `<= 4`; no real inconsistency exists after inspection. |
| —     | FGC-10 | compare.py | `r[5]` / `r[6]` positional tuple indices | Dropped (style nit). The tuple layout is defined in the immediately preceding `rows.append(...)` call at the same indentation level; the fragility is real but the comment would duplicate information already visible. |
| —     | FGC-12 | sweep_report.py | `topcls = "reg" if (top and top > 2.15)` | Dropped (Medium confidence, subsumed). C-19 covers the super-linearity threshold story for `k > 1.15`; FGC-12's `2.15` is a related but distinct threshold. After re-reading the diff: the two-threshold situation (power-law exponent vs local ratio) does warrant a comment, but the proposed text in FGC-12 is substantively covered by C-19's context. Borderline — reviewer may want to reinstate. |
| —     | FGC-14 | lib/analyze.py | `classify()` thresholds `step_thr=1.4` / `drift_thr=1.25` | Dropped (Medium confidence, overlap with C-20). The rationale requested here (why 1.4 vs 1.25) is analogous to and partially answered by C-20 (trend.py defaults). A separate comment at classify() would be additive, but this is a lower-severity nit that risks over-commenting a well-structured function. |
| —     | FGC-15 | lib/analyze.py | `r > 1.01` noise floor in `classify()` | Dropped (Medium confidence, style nit). A one-line constant with a self-evident purpose ("1% noise floor"). The existing context (`up_frac` computation) makes the intent clear without a comment. |
| —     | FGC-16 | fetch_releases.py | `timeout=180` and `1 << 20` chunk size | Dropped (Medium confidence, style nit). Standard network-IO constants; the values are self-explanatory to anyone who has written file-download code. The corporate-proxy context in FGC-16 is interesting but is already documented in the module docstring. |
| —     | FGC-17 | lib/workloads.py | `_GROUP_DEPTH = 8` constant | Dropped (Medium confidence). The constant lacks a comment at its definition site (the comment lives at a call site), which is a real gap. However, the call-site comment (`gen_autodiff`) is immediately nearby and the fix is a trivial constant-definition comment; marginal priority for a formal PR review comment. |
