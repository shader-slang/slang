# Qualify BF16 vector values and storage boundaries

This ExecPlan follows `.agent/PLANS.md`. The maintainer requires completed NVVM plans/reports
committed per slice. The research worker does not commit; root owns independent acceptance.

## Purpose and Observable Result

Qualify canonical BF16 vector construction, splat, extraction, bit transport, helper parameters and
results, Float32 component casts and the distinction between register values and local/storage ABI
for widths 2/3/4. Deliver an exact bounded implementation handoff, with no production changes.

## Progress

- [x] 2026-09-25: Read instructions, workflow, accepted238/239 evidence and local build skill/environment.
- [x] 2026-09-25: Create plan before experiments.
- [x] 2026-09-25: Capture37 source/12 artifact/558 input identities; run smoke4 and frozen BF16 three-cell probe.
- [x] 2026-09-25: Trace canonical Slang IR, CUDA representation and LLVM value/storage alternatives.
- [x] 2026-09-25: Run source NVRTC and raw LLVM O0/O3 controls with independent expected outputs.
- [x] 2026-09-25: Audit complete outputs/identities; finish design, report, compact evidence and STATUS.

## Surprises and Discoveries

LLVM14 vector store/allocation/alignment: BF2 4/4/4, BF3 6/8/8, BF4 8/8/8. Actual CUDA layouts
are4/4,6/2,8/2. Raw BF3 helper ABI uses8/8, while component-array local storage uses6/2.
BF2 array type ABI alignment2 differs from its explicit alloca alignment4. BF4 AST/IR CUDA layout
producers use alignment8 despite the prelude's2; future external storage requires a producer fix.
Final bitcast lowering scalarizes all source transport probes; raw i48 is feasibility only.
Layout setup attempts preserve an incorrect BF macro and an existing NVCC BF-only prelude macro
dependency. Final actual-prelude assertions enable Half and BF16 and pass; no production patch.

## Decision Log

2026-09-25: Select vector transport/casts because frozen scalar-bf16 now rejects helper parameter
vector<BFloat16,4>. Exact integer construction and source-ordered dot remain separate. Material6
already compile/assemble at239; absent runtime bindings/inputs/oracle justify this cadence override.
Do not add speculative material edits. Research does not advance implementation cadence zero.

2026-09-25: Qualify physical i16 vectors for value/by-value internal helper roles only. Internal
helper ABI need not equal CUDA C++ ABI. Array-based locals are measured candidates, not general
storage admission. Defer BF4 layout producer correction and new storage roles; preserve all existing
classifiers outside explicit BF16 value operations. Revisit only with separate layout/pointer evidence.

## Outcomes and Retrospective

Research complete. Smoke4, exact frozen3, sourceNVRTC3 and rawLLVM6 controls pass their stated
expectations; six generated direct source controls remain rejected. Nine SM80 assemblies pass.
Every run covers73190 records in each lane. All31,618,089 words pass (13,613,340 active,
18,004,749 preserved), independently checked by worker and parent. All37 source/12 artifact/558
input identities and117 accepted238 indexed artifacts remain unchanged. No production support,
build, corpus addition, ABI change or cadence reset. Independent parent acceptance passes; the authorized local commit remains root-owned.

The next implementation is value/by-value helpers, construction/splat/extraction and helper branch/phi selection and
matching-lane-count BF16/Float32 casts, preserving existing bitcast lowering. New local/storage
admission is separate despite successful isolated local candidates. Full/implementation239,
targeted233 and cadence0 remain. No further research needed to select this bounded implementation.

## Context and Current Pipeline

The frozen source bitcasts uint64 to BF4, converts to float4 and computes BF16 dot. Valid core.meta
producers retain makeVector, makeVectorFromScalar, element extraction, BitCast and FloatCast.
Audit producer IR before choosing role-specific NVVM admission. Existing scalar semantic BF16,
role caches, helper transport and byte/storage traversal are the reuse boundary.

## Scope and Non-Goals

Research only. No production/provider/ABI/build/frontend/library/runner/corpus/input modifications,
no commit/push, no system/driver changes. Exact integer construction and dot are inherited238 facts,
not new investigation. Raw artifacts stay under build/nvvm-loop/slice-240-bf16-vectors.

## Architecture and Invariants

Semantic BF16 remains distinct from Half and integers. Register lane values are not a byte-addressed
storage ABI. Every BF16 payload must transport exactly; Float32 narrowing NaNs are classification-only;
SM80 expansion preserves exact high-word bits. Audit BF3 padding and BF2/4 alignment explicitly.

## Interfaces and Dependencies

Base946f3f3b1dfb6ce2e468afdd707aced032843e2e, native Ubuntu24.04, L4SM89 driver580.126.09,
targetSM80 CUDA12.9.2/NVRTC12.9.86 LLVM14 ABI38, matching RelWithDebInfo.
Compiler a6cd5bd057defd8fc896ebd75813d8b7e5737fe33a095e20840fc73b72233412;
provider cefb3cd3cb44fb0d2c6a201f210ea3c98e1913c2fcac554ea5ad912d6afbcfd7.
Use inspected slice-203-env.sh (overrides stale Debug paths). Four CPUs total, sequential GPU suites.

## Milestones

1. Preserve identities and run `timeout --kill-after=30s 30m python3 extras/validate-nvvm-runtime.py
--config RelWithDebInfo --cuda-path /usr/local/cuda-12.9 --architecture 80 --output build/nvvm-loop/slice-240-bf16-vectors/runtime`.
2. Trace isolated generated vector controls and exact type/signature/layout roles.
3. Derive projections from immutable238 controls/convert.input.bin and convert.expected.bin;
   exercise every BF payload and all7654 additional Float32 records in each lane, dynamic helper and
   local roundtrip. Compile/assemble/run source NVRTC and matched raw i16-vector/array controls O0/O3.
4. Stop at qualified bounded implementation handoff; preserve failed attempts and limitations.

## Validation and Acceptance

Compare all37 sources/12 artifacts/558 inputs before/after with239. Fresh smoke4 and exact frozen3
must preserve239 outcomes; all other1683 corpus cells and material6 are inherited explicitly.
No full rerun: sources/binaries/inputs/toolchain unchanged. Verify inventories, every active output,
input and inactive sentinel. Independent integer oracle rechecks derived projections; retain hashes
and artifact index. Parent independently audits before acceptance/local commit.

## Failure and Recovery

Stop GPU work on device loss and notify root. Keep failures under unique attempt paths; never edit
executing scripts or accepted artifacts. Native commands bounded30m. bwrap failsRTM_NEWADDR;
routine execution uses require_escalated. No routine permission questions are needed.

## Artifacts and Hand-Off

Completed plan, five-part report, semantic-evidence.slice-240.json, durable design note and STATUS.
All scripts, IR/PTX, logs and complete buffers remain under ignored rawroot with hash index.

## Executed Commands and Exact Results

After sourcing inspected `build/nvvm-loop/slice-203-env.sh`, run `capture.py`, the smoke command
above and `run-compute-census.py --config RelWithDebInfo --bin-dir build/RelWithDebInfo/bin
--provider build/RelWithDebInfo/bin --architecture 80 --workload-ids-from RAW/frozen-selection.tsv
--jobs 2 --output RAW/frozen` (also recorded in commands.json).
RAW is `build/nvvm-loop/slice-240-bf16-vectors`. The exact scripts are `make-controls.py`,
`compile-source.py`, `make-llvm.py`, `run-controls.py`, `audit.py` and `package.py`. Commands/results
are retained in controls/compile-results.json, llvm/results.json, commands.json and runtime-controls.json.
The standalone layout.cpp links existing LLVM14 SDK libraries; cuda-layout.cu includes the real
prelude and asserts7 layout conditions. No compiler/provider rebuild occurred.

Each of three source runs and six raw runs returns3,513,121 words. Width2 has1,024,660 active words,
width3 1,536,990, width4 1,976,130 per mode. The remaining words are preserved. Worker audit.json
checks all outputs and separately recomputes RNE expectation by carry/tie bits. Parent independently
agrees. All mode inventories are exact with no missing/duplicate/extra cells. Finish packaging then
release checkout; root independently accepts and owns the local commit.

2026-09-25 parent acceptance:252 evidence references,132 indexed artifacts,12 primary source hashes,
all unchanged baseline identities and every complete output buffer verified independently.
Research240 is accepted; full239/targeted233/cadence0 remain unchanged.
