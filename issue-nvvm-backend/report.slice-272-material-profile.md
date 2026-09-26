# Refresh material compilation attribution

## Motivation

Helper decomposition267 removed retained material work, and subsequent correctness changes altered
the accepted compiler. Earlier phase ratios are insufficient to choose the next optimization. Measure
current eval_buffer/sample_buffer compilation costs at NVRTC O3 and NVVM O0/O3 while preserving the
fixed material and representative shader outputs.

## Proposed solution

Temporarily apply the existing host-timer patch, independently review its current scope boundaries,
qualify identical outputs, and run the fixed material protocol. Restore accepted270 exactly afterward.
The measured next target is shared semantic checking, before choosing any production optimization.

## Change summary

[Evidence272](timing-evidence.slice-272.json) records six timing cells,42 output/resource controls,
139 unit identities, source/binary lineage, protocol, limitations and exact restoration. This report,
completed plan and navigation are retained. No compiler change survives. Samples, timer logs, plots,
full snapshots and exhaustive indices remain under `build/nvvm-material-profile272`; the existing
[attribution experiment](experiments/material-attribution/README.md) defines reproduction.

## Concepts and vocabulary

Inclusive scopes contain child work. The reporter partitions durations and percentages per sample
before computing medians/IQRs; stage medians need not add to wall median. Vendor compilation is an
opaque API interval, not pure optimization. Fresh processes still share warmed filesystem/toolkit
caches. Static registers, stack and instruction counts do not measure GPU execution speed.

## Process report

Independent review traced all six patched functions and profiler RAII. Host timers preserve calls,
arguments, errors, destruction order and ABI42. NVVM emission includes capability checks, construction,
serialization and teardown; query/write each materialize and verify through the provider. Downstream-compiler library
loading belongs to remaining wall time; NVVM provider loading is included in target preparation. The fixed CLI path has two builtin loads and one invocation
of each other required scope; every observed sample satisfies those counts and interval containment.
No semantic helper, fallback, AST/IR representation or provider contract is changed.

Fresh quality36 and exactly verified accepted270 material6 establish the control outputs. All42
instrumented PTX/cubin pairs and403 named resource records match; all139 selected downstream,
serialization and NVRTC-PCH unit IDs match the accepted map. Two opposite-order rounds run2warmups
and9measured compiles per cell/round (132total), followed by2warmups and9measured assemblies per cell
(66total). Every artifact agrees within its cell and with270. No sample is retried or removed.
Independent review recomputes all18 group summaries and confirms zero rounded residuals.

| Entry      | NVRTC O3 wall median | NVVM O0   | NVVM O3   | NVRTC O3 / NVVM O3 |
| ---------- | -------------------- | --------- | --------- | ------------------ |
| Evaluation | 1395.34ms            | 1284.33ms | 1362.80ms | 1.0239             |
| Sampling   | 1424.52ms            | 1326.09ms | 1454.46ms | 0.9794             |

These are observations within one instrumented session, not causal improvement over older sessions.
Per-round medians/IQRs are retained in the record. Current NVVM O3 costs are:

| Disjoint stage               | Evaluation median | Sampling median  |
| ---------------------------- | ----------------- | ---------------- |
| Builtin loading              | 214.73ms (15.8%)  | 214.98ms (14.8%) |
| Front end                    | 544.04ms (40.0%)  | 541.98ms (37.4%) |
| IR linking/optimization      | 299.03ms (22.0%)  | 314.42ms (21.7%) |
| Target preparation/emission  | 36.38ms (2.7%)    | 42.62ms (2.9%)   |
| Explicit vendor verification | 10.42ms (0.8%)    | 11.66ms (0.8%)   |
| Vendor compile API           | 161.41ms (11.8%)  | 230.58ms (15.8%) |

Percentages are per-sample statistics, not ratios of marginal medians. Remaining wall time is retained
separately. Nested semantic checking costs385–397ms across the measured cells; IR generation142–146ms,
simplification122–129ms and specialization117–124ms are further candidates. These nested values must
not be added to their parents. A bounded semantic-check call-path/profile and reproducer investigation
is the next experiment. Serializer write is only7.86/8.85ms at NVVM O3; a provider API change is not
selected by this evidence, and query/write differences are not measured savings.

O3 material results preserve267: NVVM52/62 registers and zero stack/spills; NVRTC48/63 registers and
592/624-byte stacks. Both O3 backends retain zero exponential instructions. No material runtime claim
is made because bindings, textures/LUTs and input/output contracts remain unavailable. All44 NVRTC
logs lack directly observed PCH markers; this is unavailable cache observation, not disabled caching.
Default math-option behavior is source-inferred rather than captured through an API trace.

The experimental compiler is HEAD31c51bdbb plus the recorded six-file patch; cached version metadata
is intentionally held at2026.18.3-286-g06a26a3f7 for this discarded measurement build. The accepted
compiler retains its original06a26a3f7+patcha35e26dc identity. Reversal restores all six source hashes;
full layout replacement restores100 files/symlinks with no extras,37runtime hashes,576inputs and22pins.
Restored source mtimes trigger the next real rebuild, which must refresh version metadata. Two
review-script schema/roundoff assumptions were corrected without workload reruns; those incidents
and the initial sandbox-launch failure are retained. Baseline270, failure histories and full-checkpoint
cadence are inherited unchanged. The authorized development loop continues; Slack remains skipped.
