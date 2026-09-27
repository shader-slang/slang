# Measure failed material generic-inference cost

## Motivation

Frequency observation 286 found 1,314 failed bitwise-OR candidates among 7,995 inferences. For example,
this existing material helper contains one of the 18 observed OR sites:

```slang
public void flip_bit<T : __EnumType>(inout T value, T flag)
{
    value = is_set(value, flag) ? (value & ~flag) : (value | flag);
}
```

Counts alone cannot establish CPU cost or justify removing candidates. Measure the unchanged
`eval_buffer` NVVM O3 compile before prioritizing a compiler optimization.

## Proposed solution

Use temporary owning-thread CPU observation with untouched, disabled, count-only and timed controls.
Eight balanced measured rounds find a stable instrumented OR-failure median of 14.368 ms, or 3.593% of
semantic-root CPU. The paired timed-minus-count-only root difference is 18.594 ms across all 7,995
observations. That global overhead is not OR-specific and does not prove OR work negligible; it
prevents treating this experiment as a precise uninstrumented or avoidable-cost estimate. Deprioritize
OR pruning and return to language-surface qualification. No optimization or observer remains installed.

## Change summary

[Evidence294](research-evidence.slice-294.json) retains 51 compile obligations, native/guard comparisons,
measurement summaries, failure history and restoration. The completed plan/report and navigation are
the only changes retained. A fresh author prepared the observer and scripts; a separate reused reviewer
checked source, method, execution, raw timing and restoration. Raw sources, every sample, binaries and
logs remain under `build/nvvm-inference-cost294`; the main corpus and baseline 293 stay unchanged.

## Concepts and vocabulary

An exclusive inference span subtracts only its direct children's inclusive spans. The root residual
is root CPU minus all exclusive inference spans; it also includes observer bookkeeping and excluded
local cleanup. It is not pure non-inference work. Untouched root CPU is unavailable. Disabled mode
retains the experimental binary and root clock, so its difference from untouched includes build and
layout effects. Inference success still precedes candidate applicability and ranking.

## Process report

`ResolveInvoke` and coercion lookup reach `addOverloadCandidatesForCallToGeneric`, then
`inferGenericArguments`. The temporary call guard starts before existing `ensureDecl` work, copies the
already-produced generic declaration, and records each original return outcome without repeating type,
resolution, substitution or solver operations. These are valid semantic inputs, not malformed shapes
requiring a producer repair. The existing public `getShared()` simply returns the private context
pointer. Its use corrects the first build's access error without adding semantic work.

The helper inventory is temporary lifetime IDs, owner/scope checks, bounded primitive rows, return
cleanup, thread clocks, and post-root serialization. All are removed. Count-only and timed share
capture; timed adds clocks and direct-child accounting. Capacity is 16,384 rows; overflow, foreign
threads, missing exits or invalid nesting reject an observation. Root allocation precedes timing;
dump and 256 paired clock calibrations follow it. Existing `SLANG_PROFILE` wall time still includes
those operations. Inference finish precedes local-object destruction, so cleanup remains in an active
parent or root residual. No calibration or global-overhead subtraction is applied.

Loader pilots verify different accepted/experimental compiler hashes with the same ABI 42 provider.
Three mode pilots and all six registered material configurations preserve 293 PTX, cubin and parsed
resources exactly. Native suites preserve all 1,110 unit and 1,248 semantic identities/statuses. Six
unchanged 267 GPU guards pass 48 complete output words at NVRTC O3 and NVVM O0/O3. They preserve the
already-fixed aggregate-memory behavior; they do not establish material runtime correctness.

The guard driver's first absolute selector ran zero tests and was rejected, leaving five unrun.
`options.cpp` normalizes positional prefixes with `NoRoot` but leaves `-test-dir` unchanged. A reviewed
relative-path amendment fixes the invocation; it preserves the original failed record and reuses the
five completed qualification phases. No shader, oracle or compiler change was required. Two read-only
root audit mistakes are retained: comparing patch index metadata as source content, and expecting
numeric zero instead of null for a missing native summary. Neither repeated a workload.

The frozen experiment runs two warmups per treatment and eight measured rounds, rotating order and
its reverse so every treatment occupies every position twice. All 40 compiles preserve output; no
sample is removed. Count/timed runs reproduce 7,995 calls: 810 arity failures,1,965 successes and5,220
solver-null results, including 60 nested calls. OR retains 75 declarations, 18 sites and1,314 null results;
mapped source inventories match 286. Independent review reconstructs the raw disjoint partitions and
all signed within-round differences. The measured summaries are:

| Observation                             |      Median |        Observed range |
| --------------------------------------- | ----------: | --------------------: |
| Failed OR exclusive CPU                 |    14.368ms |       14.093–14.632ms |
| Failed OR share of root CPU             |      3.593% |          3.520–3.658% |
| Timed semantic-root CPU                 |   400.409ms |     395.491–402.338ms |
| Timed minus count-only root CPU, paired |   +18.594ms |      +16.062–21.570ms |
| Timed minus disabled root CPU, paired   |   +19.277ms |       +9.956–26.989ms |
| Untouched fresh-process wall time       | 1,357.890ms | 1,347.408–1,378.962ms |

Shares are computed per sample; marginal medians are not added. The clock-pair median is 700 ns,
reported only as an overhead diagnostic. OR spans include first-use/`ensureDecl` work, not merely
solver arithmetic. All actual calls are ordinary candidates in one root; higher-order callers,
nested/sequential roots, overflow and foreign-thread handling remain source-reviewed boundaries.
No safe pruning/cache key, compiler speedup or material/kernel runtime claim follows.

Restoration verifies 100 layout entries including links/modes, 37 runtime identities, 11 qualified source
files, 2 configurations, 576 main inputs and 22 pins. Three observer sources equal HEAD and have mtimes
newer than experimental objects; the next build must refresh version metadata and rebuild them.
Full 293 / targeted 233 / cadence 0 and 1,703 correct / 37 unresolved / 20 resolved histories remain inherited.
The loop continues with bounded language-surface work; Slack remains skipped.
