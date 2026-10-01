<!--
SPDX-FileCopyrightText: The Khronos Group, Inc.
SPDX-License-Identifier: CC-BY-4.0
-->

# What the container-statistics changes were actually worth

`findings-falcor.md` says what the profiles showed and what to do about it. This says what happened
when it was done: what the changes measure on Slang's own compile-perf suite, how that number was
arrived at (the first two attempts were wrong, in ways worth recording), and what it does and does
not predict about Falcor.

The short version: **a real 0.71% across the suite, and −4.8% on `reflection_layout`**. The changes
are worth keeping, but nobody should expect a user-visible speedup from them on this evidence. And
the part of the suite that moved is, unluckily, driven by the one change that matters _least_ on a
real application — so the Falcor number is very likely to be a different number, probably a larger
one, for entirely different reasons.

## What was changed

Nine commits on top of `4d7939e59`. Seven remove work; two are documentation and a test.

| Change                                                                                        | What it removes                                      |      Falcor count |
| --------------------------------------------------------------------------------------------- | ---------------------------------------------------- | ----------------: |
| `NamePool::getName` probes its map by slice                                                   | a `String` built only to be hashed and thrown away   | 21.9M allocations |
| `findMagicDecl` takes `const char*`                                                           | ditto, for every builtin-type lookup                 |             16.0M |
| Mangler's literal `emit` calls use `emitRaw`                                                  | a heap `String` per one- or two-character constant   |             20.3M |
| `getSpecializedLinkageName` reuses one buffer; `specializeLinkageDecoration` appends in place | two of three 1KB `StringBuilder`s per specialisation |             15.8M |
| `digestToString` sizes its buffer up front                                                    | two of three allocations per SHA-1 digest            |   10.9M regrowths |
| `ShortList<FlattenedTypeRangePair>` 16 → 2                                                    | 448 bytes _written_ per unification                  |     202.6M × 448B |
| `LegalCallBuilder::m_args` 16 → 32                                                            | a heap promotion in 35% of legalized calls           |                 — |

## The measurement, and two wrong answers before it

### Attempt 1: two `bench.py` runs, compared

Ten runs of the 30-workload suite per binary, interleaved, compared on run medians with a
one-sample t-test over the per-workload log ratios. It reported **0.9928x, p = 0.0008** — a clean,
significant 0.72%.

It was wrong. The negative control is to run the _same_ procedure on a binary against itself, by
splitting its ten runs in half:

| Comparison                                         |       Ratio |          p |
| -------------------------------------------------- | ----------: | ---------: |
| base odd runs vs base even runs — identical binary |     0.9915x |     0.0007 |
| head odd vs head even — identical binary           |     0.9935x |     0.0033 |
| base first half vs second half — identical binary  |     0.9921x |     0.0031 |
| **base vs head — the real comparison**             | **0.9928x** | **0.0008** |

A binary compared with itself produced the same effect with a _better_ p-value. Two things were
wrong at once. The machine drifts over a session — later runs are faster, consistent with page
cache and frequency settling — and `bench.py` measures one binary over the whole suite, so the two
binaries were separated by about ninety seconds of that drift. And the test treated 32 workloads as
32 independent observations when they share the per-run machine state, which is exactly what turns
a shared drift into significance. Reversing the order (comparing `base-i` with `head-(i-1)`) moved
the result to 0.9964x, p = 0.09, which is the signature of an ordering artefact.

### Attempt 2: interleave at the level of a single sample

The fix is to stop letting time separate the binaries. `tools/compile-perf/ab.py` materialises each
workload's corpus once and then runs both binaries against it in **ABBA** order — `A B B A`,
`B A A B`, … — so within each block of four the mean position of each binary is identical and a
drift that is linear across the block cancels exactly rather than being charged to whichever
binary ran second.

Four sessions were run: the real comparison, the same with the roles swapped, and each binary
against itself.

| Session | A    | B    | geomean B/A |     p |
| ------- | ---- | ---- | ----------: | ----: |
| real    | base | head | **0.9932x** | 0.027 |
| swapped | head | base | **1.0077x** | 0.019 |
| control | base | base |     0.9960x |  0.22 |
| control | head | head |     1.0008x |  0.81 |

This is the pattern a real effect produces and drift does not: the sign flips correctly when the
roles swap, and both identical-binary controls are null. Pooling both orderings over 512 ABBA
blocks:

```
geomean 0.9929x   (-0.71%)   95% CI [0.9889x, 0.9968x]   p = 0.00046
```

Three of 32 workloads move significantly, all faster, none slower:

| Workload              |       Ratio | 95% CI           |      p |
| --------------------- | ----------: | ---------------- | -----: |
| `reflection_layout`   | **0.9522x** | [0.934x, 0.970x] | 0.0001 |
| `parse`               |     0.9752x | [0.959x, 0.992x] | 0.0068 |
| `overload_resolution` |     0.9780x | [0.960x, 0.997x] | 0.0236 |

`reflection_layout` mirrors cleanly across the swap (0.950x real, 1.048x swapped) with both
controls null, so its −4.8% is as solid as this harness can make a number.

### What to take from the two failures

Both are the same class of error as the two the report itself had to withdraw: **a number was
believed because it had a small p-value, without asking what else could produce it.** The cheap
guard in every case is the null experiment — compare the thing with itself and check you get
nothing. It costs one extra session and it is the only thing standing between "p = 0.0008" and a
false claim.

## Is it likely to be better or worse on Falcor?

Probably better, but not for the reasons the suite measured, and this is an extrapolation rather
than a measurement.

The instrumented baseline (`build-cstats`, which is `4d7939e59` plus the statistics) was run over
the compile-perf suite, giving the same profile for that corpus as `findings-falcor.md` has for
Falcor. Normalising by total container constructions — a rough proxy for "amount of compiler work" —
makes the two corpora comparable:

| Change addresses                     | compile-perf |    Falcor | Falcor / compile-perf |
| ------------------------------------ | -----------: | --------: | --------------------: |
| `NamePool::getName`                  |    11,466 /M |  2,195 /M |             **0.19x** |
| `findMagicDecl`                      |       442 /M |  1,682 /M |                 3.81x |
| mangler literal `emit`               |       257 /M |  2,412 /M |                 9.37x |
| specialisation linkage names         |       225 /M |  3,961 /M |            **17.60x** |
| `FlattenedTypeRangePair` stores      |    10,411 /M | 21,288 /M |                 2.04x |
| **all addressed string allocations** |    12,390 /M | 10,250 /M |                 0.83x |

Read the first and fourth rows together, because they are the whole story. **The compile-perf
result is driven almost entirely by the one change that is least relevant to Falcor.** `NamePool`
is five times _less_ reachable per unit of work on Falcor, and the three workloads that moved —
`reflection_layout`, `parse`, `overload_resolution` — are precisely the name-lookup-heavy ones.
Meanwhile the changes that dominate on Falcor are between 2x and 18x more reachable there and are
close to invisible here: the entire specialisation-linkage path is 24,064 allocations in this
suite against 37.7 million on Falcor.

Three reasons to expect the Falcor figure to be the larger one:

- **The counts favour it slightly.** Total addressed allocations per unit of work are about the
  same (0.83x), but that parity hides a much better mix.
- **The allocations are bigger.** Falcor's addressed allocations are dominated by 1KB
  `StringBuilder`s on the specialisation path; the suite's are dominated by short name strings of a
  few dozen bytes. A removed allocation is worth more there.
- **The store-traffic change reaches twice as much of it.** `FlattenedTypeRangePair` is 448 bytes
  written per construction and is 2.04x more frequent per unit of work on Falcor — 84.5GB of stores
  removed across that profile against 0.46GB across this suite.

And two reasons for caution:

- **This is a count-to-time extrapolation, which is the exact mistake `findings-falcor.md` had to
  withdraw twice.** An allocation count says nothing about what an allocation costs in a
  long-running host with a warm allocator, and neither corpus was measured for time on the other's
  workload.
- **The measured effect here is 0.71%.** Even a threefold improvement in reach leaves the Falcor
  number in low single digits. Nothing in this data supports expecting more.

The only way to settle it is to re-run the Falcor suite against the new binary. That is cheap
relative to the effort already spent, and it is the measurement that would actually answer the
question.

## Cost when the instrumentation is off

Unchanged and confirmed: `CMakeLists.txt:158` declares the option `OFF` and defines
`SLANG_ENABLE_CONTAINER_STATS=1` only when it is set; the `#else` branch at
`slang-container-stats.h:521` expands `SLANG_CONTAINER_STATS_MEMBER` to nothing (no field on any
container), the site-parameter and initialiser macros to nothing (no extra constructor parameter,
no mem-initialiser), and every `NOTE_*` to `do {} while (0)`. The containers wrap their instrumented
special members in `#if ... #else ... = default; #endif`, so with the option off they get
compiler-generated ones.

## Reproducing

Two Release builds, one per revision, then:

```bash
python3 tools/compile-perf/ab.py <slangc-a> <slangc-b> /tmp/ab-real.json    --pairs 8
python3 tools/compile-perf/ab.py <slangc-b> <slangc-a> /tmp/ab-swapped.json --pairs 8
python3 tools/compile-perf/ab.py <slangc-a> <slangc-a> /tmp/ab-ctrl.json    --pairs 8
```

Run the third. A comparison without its null experiment is not evidence, and on this machine the
null experiment is what rejected the first two answers.

To reproduce the reach table, profile the suite with an instrumented build and compare against a
real-application profile:

```bash
SLANG_CONTAINER_STATS=/tmp/cstats-perf/perf python3 tools/compile-perf/bench.py \
    --slangc build-cstats/Debug/bin/slangc --label cstats-perf --samples 1 --warmup 0
python3 tools/container-stats/analyze.py '/tmp/cstats-perf/perf.*.json'
```
