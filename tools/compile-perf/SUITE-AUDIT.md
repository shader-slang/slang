# Compile-perf suite audit: are the tests measuring what they claim, and are they stable?

Prompted by a false alarm on a diagnostics-related test. Three questions:
does each workload measure its stated target, are the numbers stable enough
for the trend alert, and what should be added, changed or removed.

Two data sources, and it matters which is which:

- **Production**: the public results repo `shader-slang/slang-compile-perf`,
  73 nightly points (2026-06-25 .. 2026-09-12) on the Windows perf pool
  (`Windows-AMD64-AMD64 Family 25 Model 97`, i.e. Zen 4, 14-16 CPU). Every
  claim about stability and false alarms below comes from here.
- **Local**: macOS arm64, v2026.17.1 release binary, 5 repetitions x 5 samples
  of the whole suite plus ladder sweeps. Used for target-share analysis and
  for the size experiments, which are ratios and travel fine.

The local machine is **not** representative for noise, and §6 explains why
that is worth knowing anyway.

---

## 1. The false alarm, identified

`diagnostics_clean`, nightly of **2026-09-10**: 21.66 ms -> 25.71 ms
(**+18.7%**), then straight back to 21.79 ms (**-15.2%**) the next night.

It was the **only** workload that moved. Every other one of the 33 moved by
under 4%, and 28 of them by under 1.5%:

| workload            | 09-09 | 09-10 |    in |   out |
| ------------------- | ----: | ----: | ----: | ----: |
| `diagnostics_clean` |  21.7 |  25.7 | +18.7 | -15.2 |
| `sema_generics`     | 853.2 | 886.1 |  +3.9 |  -0.0 |
| `minimal`           |   9.4 |   9.0 |  -3.5 |  +2.7 |
| `parse`             | 303.3 | 300.9 |  -0.8 |  +1.5 |

The per-compile floor (`minimal`) did not move, so this was not a stdlib
change leaking through. The five samples that night tell the story:

```
diagnostics_clean  [21.55, 30.11, 27.33, 25.02, 25.71]   spread 40%
minimal            [ 9.12,  8.96,  9.04,  8.76,  9.10]   spread  4%
parse              [303.85, 300.93, 301.01, 298.99, 300.34]  spread 1.6%
```

One workload's samples dispersed while its neighbours in the same run stayed
tight. That is not host contention -- contention hits everything. It is that
**`diagnostics_clean` is too small to absorb a perturbation**. It measures
23 ms, of which ~9 ms is the per-compile floor, leaving ~13 ms of signal. An
absolute excursion of ~5 ms is 1.6% of `parse` and invisible; on this
workload it is 23%, and three of five samples were enough to carry the
median past the 10% gate.

**The trend check is not the problem.** Replaying `trend.py`'s rule
(ratio >= 1.10 AND delta >= 2 ms vs a 7-point trailing median) over all 73
nights gives **5 firings**, and reading each one's context:

| firing                          | verdict                                                                                                    |
| ------------------------------- | ---------------------------------------------------------------------------------------------------------- |
| `sema_generics` 07-16 and 07-17 | **real** -- 848/865 -> 966/970 -> 872/868, a two-day step that was reverted. The alert did its job, twice. |
| `emit_metal` 07-08              | **real** -- 1027 -> 1152, and later 1103 -> 333 when it was properly fixed.                                |
| `interface_depth` 07-19         | **baseline lag** -- the value was already ~48 for three nights; the trailing median caught up late.        |
| `diagnostics_clean` 09-10       | **FALSE** -- the spike above.                                                                              |

One false alarm in 73 nights, on the smallest non-trivial workload in the
suite. The alerting rule is sound; the workload is not.

---

## 2. `diagnostics_clean` does not measure diagnostics

Beyond being too small, it does not test what its name says. The generator:

```python
for i in range(n):
    s.append(f"static const float k_{i} = {i % 7}.0;\n")
    s.append(f"float ok_{i}(float x) {{ return x + k_{i} * {i % 5 + 1}.0; }}\n")
```

No errors. No warnings. **The diagnostic sink is never exercised**, so
nothing about diagnostic formatting, source-location resolution or error
recovery is measured. The README is candid that this replaced an
error-emitting workload "so `SemanticChecking` reflects pure checking cost
without diagnostic emission" -- but the bucket is still called
`diagnostics`, and the effect is that the suite silently lost **all**
coverage of diagnostic emission and kept a misleading name.

What it actually measures: `frontEndExecute` 81%, `SemanticChecking` 37% --
N trivial declarations through the front end. `parse` (307 ms) and
`conformance` (50 ms) already cover that axis at 13x and 2x the size. Note
also that only `ok_0` is ever called, so N-1 functions are dead code after
the front end; the back half of the pipeline never sees them.

---

## 3. Two workloads can never raise an alert

`trend.py` needs BOTH a 10% ratio and a 2 ms absolute delta. For a workload
whose median is a few milliseconds, the absolute floor is the binding
constraint:

| workload               | median (prod) | delta needed to fire | within-run spread (p90) | firings in 73 nights |
| ---------------------- | ------------: | -------------------: | ----------------------: | -------------------: |
| `generic_nesting`      |        2.9 ms |    2.0 ms = **+69%** |                   28.5% |                    0 |
| `generic_nesting_eval` |        4.2 ms |    2.0 ms = **+48%** |                   38.9% |                    0 |

They would need a 50-70% regression before saying anything, while their own
run-to-run spread is 28-39%. They are incapable of detecting any realistic
regression, and they have never fired.

**Their documented purpose is also gone.** The README describes
`generic_nesting` as "substitution / inheritance-witness cost vs generic
nesting DEPTH (known exponential, ~3-4x per level; sweep it)". Measured on
v2026.17.1, taking `min` over 6 runs to strip scheduling noise:

| N (nesting depth)       |   16 |   24 |   32 |   40 |   48 |   64 |
| ----------------------- | ---: | ---: | ---: | ---: | ---: | ---: |
| `compileInner` (ms)     | 3.00 | 3.23 | 3.49 | 3.86 | 3.82 | 4.40 |
| `SemanticChecking` (ms) | 0.45 | 0.52 | 0.62 | 0.65 | 0.73 | 0.90 |

Quadrupling the depth doubles `SemanticChecking`, from 0.45 ms to 0.90 ms.
That is sub-linear, not exponential. Whatever the workload was built to
catch was fixed at some point and the documentation was never updated.
Extending the ladder does not help -- the cost simply is not there.

---

## 4. Declared targets that are not what the workload measures

Each workload declares `primary_timers`; those names are what `trend.py`
alerts on and what the README presents as the workload's purpose. Measured
share of `compileInner` at the tracked default size:

| workload                | declared target                 |  actual share | what actually dominates     |
| ----------------------- | ------------------------------- | ------------: | --------------------------- |
| `serialize`             | `writeSerializedModuleIR`       |    **absent** | `frontEndExecute` 87%       |
| `serialize`             | `writeSerializedModuleAST`      |      **7.5%** | (sema 45% + generateIR 38%) |
| `existential_aggregate` | `legalizeExistentialTypeLayout` |        **1%** | `generateOutput` 72%        |
| `resource_aggregate`    | `legalizeResourceTypes`         |        **2%** | `linkAndOptimizeIR` 29%     |
| `generic_nesting`       | `SemanticChecking`              |        **5%** | the per-compile floor (82%) |
| `generic_nesting_eval`  | `SemanticChecking`              |        **9%** | the per-compile floor (73%) |
| `module_link`           | `linkIR`                        |       **13%** | `frontEndExecute` 62%       |
| `ir_builder`            | `generateIR` / `simplifyIR`     | **13% / 14%** | `generateOutput` 64%        |
| `conformance`           | `SemanticChecking`              |       **19%** | `frontEndExecute` 65%       |
| `control_flow_ssa`      | `simplifyIR`                    |       **12%** | `frontEndExecute` 25%       |

`writeSerializedModuleIR` deserves its own line: **the compiler never emits
that timer**. `slang-serialize-ir.cpp` has `SLANG_PROFILE` in
`readSerializedModuleIR` but not in `writeSerializedModuleIR`, so the
manifest declares a counter that cannot exist. `serialize` -- the suite's
only serialization test -- therefore has no way to observe a serialization
regression: a 30% slowdown in IR writing would move its headline by 2%.

`legalizeResourceTypes` at 2% is size-dependent, not wrong in principle: it
reaches 7% at N=640 on SPIR-V, and much more on other targets. The tracked
default is simply below where the pass matters.

---

## 5. Size is the lever, and it is cheap

Swept locally, 5 repetitions x 5 samples per point. `CV%` is between-run;
`share%` is the declared primary timer's share of `compileInner`:

| workload              |             N |        ms |     CV% | share% |
| --------------------- | ------------: | --------: | ------: | -----: |
| `interface_depth`     |  64 (default) |      26.6 |    13.7 |     58 |
| `interface_depth`     |       **128** | **174.6** | **2.1** | **92** |
| `conformance`         | 600 (default) |      39.5 |     8.0 |     19 |
| `conformance`         |      **2400** | **157.6** | **2.3** |     19 |
| `overload_resolution` | 600 (default) |      30.1 |    11.5 |     42 |
| `overload_resolution` |      **2400** |  **71.0** | **3.3** |     45 |
| `resource_aggregate`  |  80 (default) |      37.9 |     7.9 |      2 |
| `resource_aggregate`  |       **320** | **104.3** | **2.0** |      5 |

`interface_depth` is the clearest case: one step up the existing ladder takes
it from 58% on-target and 13.7% noise to **92% on-target and 2.1% noise**.
The defaults were chosen for runtime budget, and the budget is not tight --
the four increases above add ~370 ms per sample, about **2 s to a nightly**
that already takes minutes.

Note `conformance` stays at 19% on-target at every size: that one is a
mislabelling (it is a front-end test), not a sizing problem.

---

## 6. Local runs on a heterogeneous CPU are not comparable

This does not affect CI, but it will mislead anyone doing a local A/B, and
it cost time during this audit.

On Apple Silicon the suite's short workloads are **bimodal**, because a
process that only runs for a few milliseconds never lives long enough to be
migrated off an efficiency core. Same binary, same input, 10 runs each:

```
default QoS (performance cores):  6.35 3.15 3.12 3.90 3.25 3.37 3.08 3.07 3.05 2.98 ms
taskpolicy -b  (efficiency cores): 11.21 19.65 11.64 11.91 10.98 17.18 14.06 12.44 10.58 29.30 ms
```

Those are exactly the two clusters seen in the suite data locally, and they
inflate between-run CV to 25-37% for `generic_nesting` and
`generic_nesting_eval`. The production runner is homogeneous Zen 4 and shows
**<= 2.0% day-to-day CV for every workload** -- verified on four consecutive
nightlies that ran the identical commit `961e4e59e` (09-05 .. 09-08).

Consequences worth writing down:

- Do not tune the suite against local numbers. Everything in §1-§4 above is
  from production data for exactly this reason.
- For local A/B work, compare `min` rather than `median`. Across the local
  5x5 run, `min` had mean CV 3.0% and worst 12.9%, against `median`'s 5.6%
  and 37.2%. `results.json` already stores `min`, `mean`, `stdev` and the raw
  samples, so this needs no re-measurement.
- **Do not** change `track.py` to record `min`. Its comment explains the
  median was chosen because it "reflects the typical run"; that reasoning is
  correct for the homogeneous runner CI actually uses, where median already
  gives <= 2% CV. Switching would break series continuity for no gain.

---

## 7. Suggested changes

Ordered by value. Nothing here is implemented on this branch -- it is a
proposal.

### Remove

1. **`generic_nesting`** and **`generic_nesting_eval`** -- cannot fire
   (§3), and the exponential they were built for no longer exists. Removing
   them also removes the two worst-behaved series in the local view.

### Replace

2. **`diagnostics_clean`** -- rebuild it as a workload that actually emits
   diagnostics: N functions each producing one warning (or one error, with
   the run expecting a non-zero exit), sized so `compileInner` lands at
   150 ms or more. That covers diagnostic formatting, source-location
   resolution and sink throughput, which nothing covers today, and it fixes
   the name. If that is not wanted, delete it outright -- as it stands it is
   a small, noisy duplicate of `parse`.

### Resize (one step up the existing ladder in each case)

3. `interface_depth` 64 -> **128**, `conformance` 600 -> **2400**,
   `overload_resolution` 600 -> **2400**, `resource_aggregate` 80 -> **320**.
   Cost: ~2 s per nightly. Every one of these is currently under the
   ~50 ms line where a single perturbed sample can carry the median past the
   gate.

### Fix declarations

4. Add `SLANG_PROFILE` to `writeSerializedModuleIR` (`slang-serialize-ir.cpp`)
   -- a one-line compiler change that makes `serialize` measurable at all.
   Then re-point `serialize`'s `primary_timers` at whatever it actually
   shows, and consider `mode="module"` with the front end minimized so the
   serialization share is not swamped by 87% front end.
5. Correct `primary_timers` for `existential_aggregate`, `ir_builder`,
   `module_link`, `conformance` and `control_flow_ssa` to name the timers
   that actually carry their cost, and update the README rows that describe
   them. A declared target at 1-19% is a claim the data does not support.

### Guard against recurrence

6. Add import-time self-checks to `lib/manifest.py`, in the style already
   there:
   - every tracked workload's declared `primary_timers` must be timers the
     compiler can emit (checkable against a recorded list);
   - a workload whose default-size `compileInner` sits below a floor
     multiple is flagged, so the next `diagnostics_clean` is caught when it
     is added rather than after it cries wolf.
     The second needs a measured number in the repo; the cheapest form is a
     committed table of default-size medians from the perf runner, refreshed
     by the nightly.
7. Consider making the alert gate per-workload -- derived from each series'
   own observed spread rather than a flat 10%/2 ms. The flat rule is what
   let a 23 ms workload with 22% within-run spread fire on noise while an
   870 ms workload needs 87 ms to say anything.

---

## Appendix: how to reproduce

```bash
# production data (no build needed)
git clone --depth 50 https://github.com/shader-slang/slang-compile-perf.git
# four nightlies of the identical commit -> pure measurement noise
ls slang-compile-perf/daily | grep 961e4e59e

# local noise: 5 repetitions of the whole suite
for r in 1 2 3 4 5; do
  python3 bench.py --slangc <slangc> --label rep$r --samples 5 --out /tmp/noise
done

# size vs stability for one workload
python3 bench.py --slangc <slangc> --label sweep --samples 5 --sweep \
    --only interface_depth,conformance,overload_resolution,resource_aggregate

# the heterogeneous-core effect, macOS
taskpolicy -b <slangc> in.slang -o out.slang-module -report-perf-benchmark
```
