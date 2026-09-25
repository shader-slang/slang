# Establish the CUDA clock helper contract

This ExecPlan follows `.agent/PLANS.md` and the NVVM workflow. Completed research documents and
compact evidence are committed; raw artifacts remain under ignored `build/`.

## Purpose and Observable Result

Establish the source, optimizer and backend contract for CUDA `clock` / `clock64`, which currently
block two frozen direct cells. Research must distinguish live counter reads from constants or
commoned calls and specify a meaningful runtime oracle without comparing nondeterministic ticks.
No production or corpus change is part of this slice.

## Progress

- [x] Read full checkpoint 235 and remaining clock source; selected the measured two-cell gap.
- [x] Record delegation limit and write this plan before experiments.
- [x] Capture unchanged source/artifact/input identities and run fresh smoke before GPU research.
- [x] Establish exact signatures, optimizer properties and libNVVM acceptance of clock primitives.
- [x] Run bounded source/control probes with deterministic-work and relational clock checks.
- [x] Audit evidence, complete five-part report/design/status and commit accepted research.

## Surprises and Discoveries

A fresh worker launch failed with `agent thread limit reached`; no other agents are active. The
workflow explicitly allows the same bounded handoff locally when delegation is unavailable. The
parent owns this research and will use a separate acceptance checker; no independent-worker claim.
The frozen expression cancels clock values algebraically, so its pass cannot prove timer behavior.
LLVM clock intrinsics accept but common at O0/O3 and hoist at O3 through libNVVM12.9;
side-effecting inline PTX controls preserve the measured contract. Initial missing-layout IR and
static-link setup attempts remain failures in raw evidence. A frozen selector including `tests/`
selected nothing; the corrected immutable ID then ran all three modes.
CUDA public helpers select `clock` and `clock64`, not the global timer used by other targets.

## Decision Log

- Select two concrete clock preflight failures: narrow reusable support, with an optimizer contract
  worth establishing before implementation. BF16/FP8, resource and other arithmetic gaps can wait.
- Preserve all existing tests and oracles, including the original clock source's limited test.
  Generated research probes use adequately sized buffers and meaningful live outputs.
- Material reconsidered: six compile/assembly passes remain inherited; missing application bindings,
  textures/LUT/input/oracle still prevent runtime or performance claims.

## Outcomes and Retrospective

Accepted research: 36 source/control launches pass; 18 intrinsic counterexamples prove commoning
and O3 hoisting. Three deterministic word-reconstruction launches and 20 PTX assemblies pass.
A separate checker verifies all raw buffers and unchanged identities. Accepted implementation/full
checkpoint remains 235, targeted acceptance 233,
implementation cadence zero. No compiler/provider/ABI/frontend/library/runner/test/corpus edits.

## Context and Current Pipeline

`getRealtimeClockLow` selects GenericAsm `clock` with `uint()` signature.
`__cudaCppGetRealtimeClock` selects `clock64` with `int64_t()` signature; `getRealtimeClock` splits
that value into `uint2` low/high words. These helpers have `NonUniformReturn`. Trace canonical
producer and consumer properties rather than assigning meaning from the public function name.
CUDA documentation describes a per-multiprocessor cycle counter. Direct admission and provider
availability, including optimizer-visible effects, need separate proof.

## Scope and Non-Goals

Read-only production research. Generated Slang/CUDA/NVVM IR controls may test supported backend
primitives without modifying provider code. No exact-tick differential oracle, clock-frequency
assumption, performance claim, arbitrary long spin or speculative global-timer substitution.
Stop at a precise implementation handoff or a concrete missing semantic contract.

## Architecture and Invariants

Counter observations are nondeterministic. Preserve distinct observable reads and their ordering
relative to meaningful work using the actual source/backend contract; do not assert stronger
ordering than specifications and emitted code justify. Deterministic arithmetic, buffer preservation,
word reconstruction and bounded relational checks are separate evidence. Account explicitly for
32-bit wrap, 64-bit representation and per-SM scope. Never infer purity from zero parameters.

## Interfaces and Dependencies

Native Ubuntu, L4 SM89, target SM80, driver 580.126.09, CUDA12.9.2/NVRTC12.9.86, LLVM14, ABI36.
Matching optimized bin/lib via inspected `build/nvvm-loop/slice-203-env.sh`; no rebuild planned.
Actual base `13badacf96791bca856c322108d12d9acd3dc22d`. Compiler library SHA256
`ca34db1a349ae8716785032a0a3b01b3e6cf8455f3137e9358e9d1ad4eca63cf`; provider SHA256
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.

## Milestones and Validation

1. Capture 31 source paths, 12 artifacts and 556 input hashes; run the four-test smoke first.
2. Compile minimal public/alias probes in source and direct modes; preserve exact diagnostics.
   Inspect installed LLVM14 intrinsic definitions and official CUDA/PTX/NVVM specifications.
3. Establish whether documented NVVM intrinsics or constrained side-effecting inline PTX provide
   the clock primitive. Retain successful and rejected experiments without overwriting evidence.
4. Execute a small justified input grid with independent deterministic-work expectations and
   explicitly limited clock predicates. Save complete actual buffers and inputs, compile/PTX/assembly
   logs, compiler options and hashes. No nondeterministic output equality between launches.
5. Rerun only the original frozen clock's three cells; inherit all other full235 gates explicitly.
   Confirm all source/artifact/input hashes unchanged and complete durable research handoff.

Full235 has 1,680 cells / 1,635 correct / 45 unresolved / twelve resolved histories. Frozen452/1356,
discovery108/324. Research does not alter that ledger or cadence. Sequential GPU work, at most four
CPU workers, bounded commands; stop GPU dispatch on device loss. No driver/reboot/push.

## Failure and Recovery

Retain failed constructions and backend rejections separately. A timeout is incomplete. Never edit
an executing shell script; use a new script for remaining gates. If clock observations cannot satisfy
a justified oracle, retain the limitation and establish the smallest further research step.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-236-clock`; compact `semantic-evidence.slice-236.json`, completed plan,
five-part report, STATUS and durable design facts. Separate checker validates hashes, deterministic
expectations, exact selected-cell preservation and stated relational predicates before acceptance.
