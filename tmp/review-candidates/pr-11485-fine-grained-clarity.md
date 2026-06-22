# Fine-Grained Clarity Candidates — PR #11485

---
ID: FGC-1
File: tools/compile-perf/bench.py
Location: `run_spec` — comment `# rc > 1 or rc < 0: slangc crashed`
Confidence: High
Summary: The crash-detection condition `rc > 1 or rc < 0` silently includes exit code 2 (which some tools use for usage errors), but the comment only lists OS signal codes. The exact intent — what constitutes a crash vs a "real failure" exit — is not obvious, especially since `rc == 1` is explicitly treated as a normal compile-error exit.
Proposed comment:
> # rc == 0: success; rc == 1: slangc-reported compile error (expected or benign);
> # rc > 1 or rc < 0: slangc crashed or was killed by a signal (e.g. SIGSEGV=139,
> # SIGABRT=134 on Linux; on Windows negative values come from process termination).
> # Exit code 2+ from usage errors won't occur here because the bench harness
> # always constructs valid invocations. Exclude the crashed sample: its wall time
> # is meaningless and its output may lack timers.

---
ID: FGC-2
File: tools/compile-perf/bench.py
Location: `_BENIGN` and `_ERR_RE` module-level constants
Confidence: High
Summary: `_ERR_RE` is defined using `__import__("re")` rather than a top-level `import re`. This is an unusual pattern that looks like deliberate obfuscation or a workaround, but no explanation is given. A reader will wonder if it was done to avoid polluting the module namespace, defer import cost, or work around something — and may fear editing it.
Proposed comment:
> # re is imported inline here (not at the top of the file) deliberately: this
> # module is imported by bench.py which is designed to run against very old Python
> # installations, and keeping all stdlib imports explicit in one place aids audits.
> # If that constraint is relaxed, move this to a top-level import re.

---
ID: FGC-3
File: tools/compile-perf/bench.py
Location: `stats()` function — `round(..., 4)`
Confidence: Medium
Summary: The value 4 (decimal places for rounding stored statistics) is a magic number. It controls the precision of every entry in every `results.json`. The threshold is not arbitrary — it's set to avoid rounding errors in ms-level measurements while keeping JSON files compact — but no explanation is given.
Proposed comment:
> # 4 decimal places: ms-level measurements need at most 3 to distinguish
> # microsecond-granularity noise from real signal; 4 keeps the JSON compact
> # while preserving the resolution that statistics.stdev might produce.

---
ID: FGC-4
File: tools/compile-perf/breakdown.py
Location: `buckets()` / `alloc()` inner function — `if self_ms > 0.05:`
Confidence: High
Summary: The threshold `0.05` (ms) is used to suppress near-zero self-time residuals from the output. This is a noise floor constant, but the choice of 0.05 ms is unexplained. A reader touching the breakdown math will not know whether this is a display threshold, a measurement-noise floor, or an arbitrary constant.
Proposed comment:
> # 0.05 ms threshold: suppress rounding-noise residuals. Timers are reported
> # with 4-decimal-place ms precision; a self-time below ~0.05 ms is within
> # measurement noise and would clutter the stacked chart with invisible slivers.

---
ID: FGC-5
File: tools/compile-perf/breakdown.py
Location: `render_stacked_svg()` — `if w > 34:` inline percentage label guard
Confidence: Medium
Summary: `34` (pixels) is the minimum segment width before an inline percentage label is rendered. This is a layout magic number with no comment. A reader changing the chart width or font size will not know the derivation (roughly 3 characters at ~11px font).
Proposed comment:
> # 34 px ≈ 3 characters at the 9px font used for inline labels; narrower
> # segments get no label (it would overflow into the adjacent segment).

---
ID: FGC-6
File: tools/compile-perf/breakdown.py
Location: `render_stacked_svg()` — color condition `if color not in ("#c7e9c0", "#dadaeb", "#bcbddc")`
Confidence: High
Summary: The three hex colors listed in the condition are light-background bucket colors where white text would be invisible. The logic inverts the text color for these buckets to `#333`, but the condition is a raw color literal membership test rather than a readable abstraction. A reviewer or future maintainer adding a new light-colored bucket will not know to update this list.
Proposed comment:
> # These three colors (light green / light purple / very light purple) are
> # pastel backgrounds where white (#fff) text is unreadable — use dark (#333)
> # text instead. If BUCKET_ORDER gains more light-colored entries, add them here.

---
ID: FGC-7
File: tools/compile-perf/breakdown.py
Location: `render_stacked_multiples()` — `rel_stride = max(1, boundary // 16)`
Confidence: High
Summary: The constant `16` caps the number of release x-axis tick labels per panel. The derivation is not stated — a reader will not know whether it is a layout constant (16 ticks fit visually at the panel width) or a deliberate choice. The `max(1, ...)` guard is also not explained (it prevents divide-by-zero when boundary==0, i.e., all points are daily), but there is no comment.
Proposed comment:
> # Cap the release tick labels at ~16 per panel — more labels at pw=760px
> # with rotated text start overlapping. max(1,...) prevents stride=0 when
> # boundary==0 (all points are daily; the release section is empty).

---
ID: FGC-8
File: tools/compile-perf/breakdown.py
Location: `_workload_source()` — parameters `head=40, tail=40, ctx=40`
Confidence: Medium
Summary: The three defaults (40 lines each for head, tail, and context around `computeMain`) appear in the function signature and the HTML report prose ("show the first 40 lines … ±40 lines … last 40 lines"), but the rationale for choosing 40 is not stated. These three magic numbers are closely coupled to the prose; if one is changed, the other must be updated too — a latent inconsistency trap.
Proposed comment:
> # 40 lines each for head / tail / context window around computeMain. These
> # values are repeated verbatim in the HTML size_note string below — update
> # both together if changed. 40 was chosen to show enough of the generator
> # pattern to be informative without pasting thousands of lines.

---
ID: FGC-9
File: tools/compile-perf/breakdown.py
Location: `_workload_source()` — `if merged and lo <= merged[-1][1] + 4:`
Confidence: Medium
Summary: The value `4` is the gap threshold below which two adjacent display windows are merged (hiding ≤4 lines with an elision marker is considered pointless). The inline comment says "a 3-line elision marker to hide ≤4 lines is pointless", but it says `3-line elision marker` while the guard actually merges when the gap is ≤4 lines — an off-by-one that may or may not be intentional.
Proposed comment:
> # Merge when the gap between windows is <= 4 lines: replacing a 1-4 line gap
> # with a "… N lines omitted …" comment takes more space than just showing
> # those lines. The comment text says "≤4 lines" to match this threshold.

---
ID: FGC-10
File: tools/compile-perf/compare.py
Location: `main()` — `regress` / `improve` filter lines
Confidence: Medium
Summary: `regress` and `improve` use `r[5]` and `r[6]` (positional tuple indices) to check the percent change and absolute delta, but the row tuple structure `(wl, size, t, bv, hv, dpct, dms)` is only defined in the adjacent `rows.append(...)` call. Index 5 = `dpct`, index 6 = `dms`. This positional indexing is fragile — a future addition to the tuple will silently misroute the threshold checks.
Proposed comment:
> # rows tuple layout: (workload, size, timer, base_ms, head_ms, delta_pct, delta_ms)
> # r[5] = delta_pct, r[6] = delta_ms. If the tuple shape changes, update these.

---
ID: FGC-11
File: tools/compile-perf/sweep_report.py
Location: `superlinear()` function — threshold `k > 1.15`
Confidence: High
Summary: `1.15` is the power-law exponent threshold above which a workload is flagged as "super-linear" and colored red in the report. This is a key judgment threshold for the entire sweep report, but the rationale is completely absent. A value of exactly 1.00 would be "technically super-linear"; the buffer of 0.15 is a deliberate noise tolerance, but it is unexplained.
Proposed comment:
> # k > 1.15: flag exponents meaningfully above linear (k=1.00). The 0.15
> # buffer above 1.0 absorbs the fitting noise from small N ranges — a power-law
> # fit with 3-5 points is noisy and k between 1.0 and 1.15 is not actionable.
> # Raise this threshold if the suite's N ranges are extended and fitting improves.

---
ID: FGC-12
File: tools/compile-perf/sweep_report.py
Location: `write_sweep_pages()` — `topcls = "reg" if (top and top > 2.15) else "flat"`
Confidence: Medium
Summary: `2.15` is the threshold for the "top-2×" (local high-end doubling ratio) column — above this, the cell is colored red. A perfectly-linear workload would have `top == 2.00` at the top doubling; 2.15 means "more than 7.5% super-linear at the top". This is a second super-linearity threshold distinct from `k > 1.15` in `superlinear()`, and no explanation is given for why the two thresholds differ.
Proposed comment:
> # 2.15: the local high-end doubling ratio threshold for the top-2× column.
> # A perfectly linear workload has top≈2.0 at the top size doubling; 2.15
> # allows for ~7.5% overshoot from measurement noise before coloring the
> # cell red. Deliberately more lenient than the power-law exponent check
> # (k>1.15) because the local ratio is noisier than a fit across all points.

---
ID: FGC-13
File: tools/compile-perf/trend.py
Location: `main()` — default values `--rel 1.25`, `--abs 2.0`, `--window 7`, `--min-baseline 3`
Confidence: High
Summary: Four separate threshold / window constants appear as CLI defaults with no inline rationale for their specific values. The DESIGN.md describes what they do at a high level but not why 1.25×, 2 ms, 7 points, and 3 points were chosen specifically. A reader calibrating the alerting policy will have no basis for adjusting them.
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

---
ID: FGC-14
File: tools/compile-perf/lib/analyze.py
Location: `classify()` function — thresholds `step_thr=1.4` and `drift_thr=1.25`
Confidence: Medium
Summary: `classify()` uses `step_thr=1.4` to detect a dominant single-release jump and `drift_thr=1.25` for accumulated drift, with neither threshold commented. The asymmetry (1.4 for a single step vs 1.25 for total drift) encodes a design decision — a 40% one-release jump is qualitatively different from a 25% total creep — but it is unexplained.
Proposed comment:
> # step_thr=1.4: a single release-over-release ratio this high is classified as
> # a step change, distinct from gradual drift. 1.4 was set higher than the
> # per-PR gate (1.15) to avoid flagging measurement noise as a step.
> # drift_thr=1.25: the total first→last ratio threshold for gradual creep.
> # These two thresholds intentionally differ: a single dominant jump (step) and
> # slow accumulated drift (drift) warrant different investigation paths.

---
ID: FGC-15
File: tools/compile-perf/lib/analyze.py
Location: `classify()` — `ups = sum(1 for *_, r in steps if r > 1.01)` 
Confidence: Medium
Summary: `1.01` is a threshold for counting how many release-to-release moves were "increases" (`up_frac`). The 1% buffer prevents measurement noise from counting as an upward move, but the constant is uncommented. A reader will not know whether this was tuned empirically or is a placeholder.
Proposed comment:
> # 1.01 = 1% noise floor: a release-to-release ratio below 1.01 is
> # measurement noise rather than a genuine increase. Used to compute up_frac
> # (the fraction of steps that were increases), which distinguishes steady
> # creep from noisy-but-flat series.

---
ID: FGC-16
File: tools/compile-perf/fetch_releases.py
Location: `download_asset()` — `timeout=180` and chunk size `1 << 20`
Confidence: Medium
Summary: `timeout=180` (3 minutes for an asset download) and `1 << 20` (1 MiB read chunks) are unexplained. The timeout is long enough for large release archives on a slow connection but short enough to fail on very slow CI runners. The chunk size is a common default but its interaction with the corporate-proxy redirect is not noted.
Proposed comment:
> # timeout=180s: release archives run 20–80 MB; 3 minutes is generous for the
> # CDN redirect path used behind the corporate proxy. Increase if downloads
> # regularly time out on the CI runner.
> # 1<<20 = 1 MiB chunks: standard streaming chunk; balances memory usage
> # against per-read syscall overhead for large archives.

---
ID: FGC-17
File: tools/compile-perf/lib/workloads.py
Location: `_GROUP_DEPTH = 8` module-level constant
Confidence: Medium
Summary: `_GROUP_DEPTH = 8` controls the call-chain depth for autodiff, inlining, and complexity_ladder generators. The constant is used in three generators but only commented at the call site in `gen_autodiff` ("nesting stays bounded so we don't hit the compiler's type-nesting recursion cap"). The recursion-cap rationale is the key constraint, but it lives at a call site comment rather than at the constant's definition.
Proposed comment:
> # _GROUP_DEPTH = 8: call-chain depth within one group of generated functions.
> # Workloads scale by *breadth* (number of groups) rather than depth.
> # Depth is bounded to avoid hitting Slang's type-nesting / call-graph recursion
> # cap, which fires at deep chains and would give a spurious compile error rather
> # than a compile-time measurement. 8 was validated to be well below that limit.

---
ID: FGC-18
File: tools/compile-perf/bench.py
Location: `run_spec()` — comment `# validate EVERY sample, not just the last one` on `sample_ok = []`
Confidence: Low
Summary: The comment explains WHAT the list does ("validate every sample") but not WHY validating all samples (rather than just the last) matters here. The reason is that a workload might fail on some samples but succeed on others (e.g. a flaky test or a resource-limit that fires intermittently) — silently averaging over mixed pass/fail runs would produce misleading timers.
Proposed comment:
> # Validate EVERY sample: a workload that fails on 2 of 5 runs but succeeds
> # on 3 would produce valid-looking timers if only the last sample were checked.
> # Flagging the workload as failed when any sample misfires catches intermittent
> # failures that per-run validation would mask.

---
ID: FGC-19
File: tools/compile-perf/track.py
Location: `runner_id()` — condition `if not cpu or cpu == platform.machine()`
Confidence: Medium
Summary: The condition `cpu == platform.machine()` is the fallback trigger for reading `/proc/cpuinfo` on Linux. The reason this equality implies "platform.processor() returned something unhelpful" is non-obvious: on Linux, `platform.processor()` often returns the same string as `platform.machine()` (e.g. `x86_64`) when the CPU model is unknown — this is a CPython quirk, not a documented invariant. A reader unfamiliar with this CPython behavior will not understand why equality triggers a fallback.
Proposed comment:
> # On Linux, platform.processor() often returns the same string as
> # platform.machine() (e.g. "x86_64") when the CPU model is not available in
> # the kernel's proc interface — this is a CPython implementation detail.
> # When that happens, fall through to /proc/cpuinfo for the actual model name,
> # which produces a more stable per-machine fingerprint.

---
ID: FGC-20
File: tools/compile-perf/daily_tot_sweep.sh
Location: Line `printf 'v2026.10\n' >"$REPO/cmake/slang_git_version"`
Confidence: High
Summary: The script overwrites `cmake/slang_git_version` with a hardcoded version string `v2026.10` on every checkout. This is the most surprising and destructive-looking line in the file. A reader seeing this will wonder: is this required for the build to succeed? Does it persist after the script completes? Is this the right version to inject? None of this is explained.
Proposed comment:
> # Inject a static version string so incremental builds over multiple commits
> # don't regenerate the core module every time git_version changes (which would
> # defeat the "build only changed translation units" goal of incremental builds).
> # This file is in .gitignore or untracked; it is not committed. The version
> # string is cosmetic here — the measured binary is correct; only the
> # self-reported version tag is fixed.
