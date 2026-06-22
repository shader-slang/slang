# High-Level Clarity Candidates — PR #11485

---
ID: HLC-1
File: tools/compile-perf/lib/analyze.py
Location: `results_dir_for` function (line ~2987)
Confidence: High
Summary: The fallback comment says "canonical location for release tags" but the real purpose is "default path for not-yet-created results", which is the confusing part a reader would question.
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
---

---
ID: HLC-2
File: tools/compile-perf/lib/analyze.py
Location: `_linfit` / `_powfit` — both functions are private (underscore-prefixed) but `ladder_scaling.py` calls `analyze._linfit` directly at line ~2910
Confidence: High
Summary: `_linfit` is called via a private-name access from outside its module (`analyze._linfit(xs, ys)` in `ladder_scaling.py`), suggesting these helpers should be public API. The leading underscore says "internal implementation detail" but the usage says "shared utility". The mismatch will confuse a reader who tries to follow the API surface.
Proposed comment:
> Consider making `_linfit` and `_powfit` public (`linfit` / `powfit`) since
> `ladder_scaling.py` already imports and calls `analyze._linfit` directly.
> The underscore convention signals "do not call from outside this module",
> but that contract is already broken. Alternatively, expose a single
> `slope_report`-style public entry point so callers never touch the raw fit
> helpers.
---

---
ID: HLC-3
File: tools/compile-perf/bench.py
Location: `run_spec` function body — the `sample_ok` list and the `ok` flag derivation
Confidence: High
Summary: The variable `sample_ok` is described only by its inline comment `# validate EVERY sample, not just the last one`, but the condition `all(sample_ok)` silently returns `True` when `sample_ok` is empty (all samples crashed). The empty-list edge case is load-bearing for correctness but not called out anywhere.
Proposed comment:
> `sample_ok` records pass/fail for each non-crashed sample. Note: if ALL
> samples crashed, `sample_ok` is empty and `all([])` is `True`, so `ok` is
> decided solely by `crash_codes` being non-empty. This is intentional —
> a run with zero clean samples is caught by `crash_codes` — but worth
> stating explicitly to avoid a "why isn't `all([])` a bug?" question during
> review.
---

---
ID: HLC-4
File: tools/compile-perf/breakdown.py
Location: `buckets` function — the "children overshoot parent" scaling comment
Confidence: High
Summary: The doc comment says the scaling "previously made generateOutput (self) vanish at v2026.7" but gives no explanation of *why* proportional scaling to the parent budget is the right fix rather than, e.g., clamping or emitting a warning. The WHY is missing; only the WHAT is described.
Proposed comment:
> Allocate compile time into mutually-exclusive buckets summing to
> `compileInner`, top-down from the budget at each level. At each node, if
> the named children sum to more than their parent's measured time (which
> happens when sub-timers were added mid-window and overlap in ways older
> releases did not account for), scale all children proportionally to fit
> within the parent's budget. This keeps the overshoot local: without
> proportional scaling, a child-sum exceeding its parent would produce a
> negative self-residual, propagate up, and zero out an ancestor's self-time
> (as happened with `generateOutput (self)` at v2026.7). Proportional
> scaling is preferred over clamping because it preserves the relative
> child proportions and keeps the visual stacked area meaningful.
---

---
ID: HLC-5
File: tools/compile-perf/breakdown.py
Location: `_workload_source` function signature and doc comment
Confidence: High
Summary: The function returns `(n, [(filename, display_code)])` but the doc comment describes `n` only as "the EXACT compiled Slang at the workload's real size". `n` is the default size integer, not the source code — the return type annotation in the docstring is the only signal, and the caller has to destructure it to learn what `n` means.
Proposed comment:
> Return `(default_size, [(filename, source_snippet)])` — the workload's
> default size `n` and a list of `(filename, code)` pairs suitable for
> display. Long source files are trimmed to three windows: the first `head`
> lines, the `±ctx` lines around `computeMain`, and the last `tail` lines,
> with elided regions marked by a comment. Overlapping windows are merged.
> The `default_size` is returned alongside the source so callers can display
> it in the page without re-reading the spec.
---

---
ID: HLC-6
File: tools/compile-perf/compare_repro.py
Location: Module-level docstring and `ci` function name
Confidence: High
Summary: The function `ci` (line ~2422) uses `ci` as a local abbreviation for `compileInner`, but `ci` is also the common abbreviation for Continuous Integration. In a file about reproducing benchmark runs, a reader seeing `ci(rec, stat)` will read it as a CI-system helper before understanding it extracts the `compileInner` timer value. The name conflicts with the dominant meaning of the abbreviation in the surrounding CI-oriented codebase.
Proposed comment:
> Rename `ci` to `compile_inner` (or at minimum `get_compile_inner`) so it
> is unambiguous. In a CI-oriented tool, `ci(rec, stat)` reads as a
> CI-system helper; only the function body reveals it extracts the
> `compileInner` timer value from a result record.
---

---
ID: HLC-7
File: tools/compile-perf/track.py
Location: `runner_id` function — the `cpu` fallback chain
Confidence: High
Summary: The function has a subtle guard `if not cpu or cpu == platform.machine()` to detect the case where `platform.processor()` returned the machine arch (e.g. `x86_64`) instead of the CPU model name. This guard is non-obvious: `platform.processor()` returning the same string as `platform.machine()` is an undocumented CPython implementation detail. A reader will wonder why the comparison is against `machine()` and whether the condition is correct.
Proposed comment:
> Build a stable per-machine fingerprint for comparing absolute timings.
> Two machines with different fingerprints must never be compared directly.
>
> `platform.processor()` is unreliable: on Linux it is often empty; on some
> systems it returns the same string as `platform.machine()` (e.g. `x86_64`)
> rather than a human-readable CPU model. When that happens, fall back to
> `/proc/cpuinfo`'s `model name` line. The condition `cpu == platform.machine()`
> catches the "processor() returned the arch string" case, which is a common
> CPython behaviour on Linux hosts that have not been configured with a CPU
> model string.
---

---
ID: HLC-8
File: tools/compile-perf/sweep.py
Location: The `complete` closure inside `main` (line ~4369)
Confidence: High
Summary: The `complete(wl)` closure is defined inside a `for` loop over `ready` releases, but it closes over `args`, `manifest`, and `sizes` — all of which are defined outside the loop. Defining a named closure inside a loop to avoid repeating the two-branch condition is unusual and will confuse a reader scanning for where `complete` is defined. It reads as if it might accidentally capture the loop variable.
Proposed comment:
> `complete` is a small helper that checks whether a specific workload is
> already fully covered in the previously-saved results. It is defined here
> (inside the loop body) because it closes over `sizes`, `args.sweep`, and
> `manifest`, all of which are stable across iterations — it does NOT
> capture the loop variable `rec` or `tag`. Consider hoisting it above the
> loop and passing `sizes` explicitly; the current placement makes it look
> like it might capture the per-iteration state.
---

---
ID: HLC-9
File: tools/compile-perf/DESIGN.md
Location: "Data model — the tracking series" section
Confidence: Medium
Summary: The DESIGN.md data model uses `++` as a concatenation operator in pseudocode (`tracking = [...] ++ [...]`). This notation is not standard in Python or shell contexts and is borrowed from Haskell/functional languages. Combined with the ASCII-art layout table that uses a different visual style, a reader unfamiliar with ML-family syntax may not recognize `++` as list concatenation.
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
---

---
ID: HLC-10
File: tools/compile-perf/lib/manifest.py
Location: Module docstring — "Phase 2" and "Phase 3" references
Confidence: High
Summary: The module docstring says "A WorkloadSpec drives both generation (Phase 2) and the release sweep (Phase 3)" but there is no other mention of Phase 1, Phase 2, or Phase 3 anywhere in the PR. These phase numbers appear to be artefacts of an internal design/planning document that never made it into the public codebase. A reader of this code has no way to know what the phases are or where to find their definitions.
Proposed comment:
> Remove the "(Phase 2)" and "(Phase 3)" references from the module docstring,
> or replace them with a brief inline explanation of what they mean. As-is, a
> reader has no context: "Phase 2" and "Phase 3" appear to reference an
> internal project plan that is not captured anywhere in this repository.
> Consider: "A WorkloadSpec drives both workload generation (`gen` callable)
> and the release sweep (`bench.py` / `sweep.py`)."
---

---
ID: HLC-11
File: tools/compile-perf/bench.py
Location: `build_commands` function — "target mode" comment at the end
Confidence: Medium
Summary: The function has three distinct code paths (module, link, and a fallback that the trailing comment labels "target mode"), but the fallback path is also used for multi-file corpus workloads like `mdl_dxr` via `spec.main_file`. The comment `# target mode` understates the function's responsibility and will mislead a reader trying to understand why `spec.main_file` and `-I gen_dir` appear in a path supposedly only for the simple target case.
Proposed comment:
> # "target" mode: single or multi-file compile to a GPU target.
> # For single-file workloads the first .slang file (or spec.main_file) is
> # the entry. For corpus workloads (e.g. mdl_dxr), spec.main_file names the
> # root file; -I gen_dir lets sibling imports resolve without explicit paths.
> # reflection_json attaches a per-run output path so the layout/reflection
> # serializer is exercised without polluting the results directory.
---

---
ID: HLC-12
File: tools/compile-perf/trend.py
Location: `gh` and `summary` function names (lines ~4983-4993)
Confidence: Medium
Summary: `gh` is used as the name for "emit a GitHub Actions workflow command", but `gh` is also the name of the GitHub CLI tool that is used throughout the rest of the Slang repository. A reader scanning `trend.py` and seeing `gh(...)` calls will initially assume the function launches the `gh` CLI. `summary` is similarly ambiguous — it is the GitHub step-summary writer, not a general summarization helper.
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
---

---
ID: HLC-13
File: tools/compile-perf/lib/workloads.py
Location: `_buf` helper function (line ~3558)
Confidence: Medium
Summary: `_buf()` returns the string `"RWStructuredBuffer<float> outBuf;\n\n"` with no doc comment. The name `_buf` is too terse to be self-documenting — it could mean "output buffer declaration", "buffer helper", or a one-off formatting helper. Every workload calls it, so a reader will encounter it repeatedly before finding a definition.
Proposed comment:
> Return the standard output-buffer declaration used by every compilable
> workload: `RWStructuredBuffer<float> outBuf;`. All workload generators
> write results into `outBuf` so that nothing is dead-code-eliminated before
> the targeted compiler stage runs.
---
