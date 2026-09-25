# Establish the FP16 masked MIN/MAX contract

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow explicitly requires this completed
research plan to be committed with its report. The parent owns acceptance and the local commit.

## Purpose and Observable Result

Determine exact binary16 semantics of CUDA-source masked MIN/MAX reductions and inclusive/exclusive
prefixes, with an independent raw-bit oracle, before proposing a bounded direct NVVM extension.
The observable result is reproducible evidence and a responsible-layer proposal, not new support.

## Progress

- [x] 2026-09-25: Read repository workflow, status, planning and native build skill.
- [x] 2026-09-25: Traced actual CUDA half compare, raw shuffle and finite exclusive seeds.
- [x] 2026-09-25: Independent raw16 oracle matched all 5,912 source/control launches.
- [x] 2026-09-25: Retained 40 direct and 12 capability rejections, four assemblies and smoke4.
- [x] 2026-09-25: Completed report, compact semantic evidence, STATUS and durable design facts.

- [x] 2026-09-25: Parent independently verified 130 unique compact references, 17,736 binaries,
      all source/artifact/input hashes, and every reconstructed source-order result.

## Surprises and Discoveries

Half exclusive seeds are finite 7bff/fbff, unlike FP64 infinities. Actual half shuffle and comparison
paths preserve raw NaN payloads with no floating conversion. Numeric half MIN/MAX descriptors are
excluded while half ordered comparisons, SELECT and lane reads are admitted. All 5,912 launches
pass; a universal ascending scan differs in 197 cases/102,116 words, and infinity seeds differ in
all 1,478 family cases/22,792 words. See `results.json` for first counterexamples.

One auxiliary Python extraction attempt failed with SyntaxError before any compiler/GPU invocation;
its script/log are retained. The corrected run passes all 12 matrix-prefix capability probes.
Smoke passed immediately after the already-started sequential research suite, rather than before
it as the workflow requests. This ordering deviation is explicit; there was no competing GPU work.

## Decision Log

- 2026-09-25: Research only on base `94209dfd6a3dc1a7ac729c0c0a928a598d570d58`.
  Production, provider, corpora and existing helpers remain unchanged. Fresh GPU suites are
  sequential; unchanged registered evidence inherits slice 231 rather than rerunning the corpus.

- 2026-09-25: Keep finite half seeds and ordered source recipes together; changing only the width
  guard or using numeric MIN/MAX is invalid. Revisit only if actual source contract changes.
- 2026-09-25: Batch all 65,536 encodings only under the full mask, while 69 structured patterns
  cover all 14 masks. Do not claim exhaustive tuples or launch a million constant-mask cases.

## Outcomes and Retrospective

Research is complete. Source scalar/vector2/vector4 prefixes and reductions plus matrix2x2
reductions match the exact integer-derived binary16 algorithm model. The unchanged provider's
half transport, ordered comparison, SELECT and six floating constants pass source and direct
O0/O3 controls. All 40 direct family probes retain E52017 without PTX. No new family support is
claimed. The follow-up is bounded emitter MIN/MAX admission and finite seeds, reusing the existing
source-order recipes. No implementation or next independent blocker work started.

The 5,912 unique launches compare 6,053,888 output words, preserve every 192-word header/input
region and retain all inactive sentinels. There are 69 structured patterns under 14 masks plus
512 full-mask batches per mode. Every raw16 encoding appears once in those batches' lane/component
inputs; tuples, mask combinations and scalar caller placements are not exhaustive. All 29 source,
12 artifact and 554 registered input hashes remain unchanged; registered acceptance inherits 231.
Parent acceptance passed after independent binary16 reconstruction and complete raw-binary audit;
this worker did not commit or push.

## Context and Current Pipeline

Four frozen direct cells reject E52017 for `_wavePrefixExclusiveMin/Max(($1).x, $0)` with
`half(half, vector<uint,4>)`. HLSL standard-module overloads select CUDA prelude WaveOp templates;
the direct emitter independently admits typed recipes. Source compare/select order, lane transport,
identity constants and FP16 conversion boundaries each require separate analysis.

## Scope and Non-Goals

Scalar/vector2/vector4 prefixes and reductions, plus existing matrix2x2 reductions. Keep matrix
prefix capability, other arithmetic families, ordinary shuffle policy and material runtime separate.
No production edits, rebuild, commit, push, driver changes or independent blocker implementation.

## Architecture and Invariants

Use raw uint16 payloads, dynamic buffers, untouched inputs and inactive sentinels. Derive ordered
binary16 comparisons from sign/exponent/fraction fields. Model source tree/scan and prefix state
explicitly; compare plausible alternatives and retain counterexamples. NaN representation changes
must be attributed to actual source conversion/transport rather than presumed from FP32/FP64.

## Interfaces and Dependencies

Native optimized matching bins/libs, CUDA 12.9.2/NVRTC 12.9.86, L4 SM89 targeting SM80, LLVM14,
ABI36. Source `build/nvvm-loop/slice-203-env.sh`. Keep compiler/provider identities from slice 231.
Generated scripts, inputs, CUDA, PTX, cubins and logs live under ignored
`build/nvvm-loop/slice-232-fp16-minmax`.

## Milestones

1. Read earlier floating research and local source/provider contracts; capture identities.
2. Compile small source and existing-operation controls; establish raw representation boundaries.
3. Run exact inventory over 14 masks and broad boundaries/payloads; retain all failed experiments.
4. Capture minimal O0/O3 family rejections with separate IR logs, smoke4, and final identity audit.
5. Complete `semantic-evidence.slice-232.json`, five-part report, STATUS and durable design facts.

## Validation and Acceptance

Fourteen masks: full, low2/4/8/16, low15, high16/high17, even/odd, sparse extremes, singleton0/7/31.
Include signed zeros, finite/subnormal limits, infinities, both signs of quiet/signaling NaNs and
payloads, repeated/mixed/order-sensitive values. Batched complete payload coverage is permitted,
without claiming exhaustive tuples. Record exact unique launch inventory and input/expected/output/
source/PTX hashes. Existing supported controls run source and direct O0/O3 where possible.
Inherit 1,674 registered cells (1,623 correct, 51 known failures), six resolved histories,
unit/toolkit/material gates from 231. Frozen452/discovery106, latest full229, cadence one remain.
Preserve all 29 tested sources, 12 artifacts and 554 registered inputs before/after.

## Failure and Recovery

Stop GPU dispatch on device loss. Retain fixture/source failures separately. If controls expose an
existing correctness problem, establish baseline and analyze only the dependency of this gate.
No silent oracle adjustments or production patches. Reruns use new paths or uniquely named runs.

## Artifacts and Hand-Off

Commit-ready documentation/evidence only. Hash-address raw references. Return checkout ownership
to the parent with results, exact limitations, changed paths and proposed next bounded action.
