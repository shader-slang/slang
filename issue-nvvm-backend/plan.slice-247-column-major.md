# Establish the CUDA column-major mismatch contract

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow explicitly requires this completed plan
and its report to be committed; raw evidence stays under ignored `build/`.

## Purpose and Observable Result

Resolve the ownership of `compute/non-square-column-major.slang#discovery-1`'s three existing
runtime mismatches without changing its identity, input bytes, or oracle. Independently derive the
CUDA ABI result, reproduce it, and identify the producer/consumer boundary. If supporting graphics
column-major packing requires a broad ABI change, finish a bounded research slice with an explicit
implementation gate instead of changing the old oracle to claim a fix.

## Progress

- [x] 2026-09-25: Read repository, plan, workflow, status, and local slang-build instructions.
- [x] 2026-09-25: Confirm clean checkout at `739ead0d725f1075b4d382ec531ebe9070e60ea1`.
- [x] 2026-09-25: Reproduce original three cells and small runtime gate with accepted binaries.
- [x] 2026-09-25: Establish independent layout/output contracts and inspect generated CUDA/NVVM.
- [x] 2026-09-25: Record bounded ownership decision, final identities, preservation, and handoff.
- [x] 2026-09-25: Parent independently accepts the research and exact preservation.

## Surprises and Discoveries

The original source explicitly disables CUDA because it ignores matrix layout and uses different
alignment. The discovery adapter selects an active graphics contract and changes its target to CUDA.
Fresh controls prove an incompatible host-data contract. All modes honor column-major stride12;
the original oracle expects stride16. The fixture comment and general user-guide claim are stale
for this measured shape. CUDA reflection size24/alignment4, HLSL extent28/alignment16 (not32).
Three pre-dispatch driver errors were retained: missing standalone render-test, indentation, and
a replacement hitting the wrong block; the corrected controls use the existing embedded tool.

## Decision Log

2026-09-25, worker: prioritize the existing all-three-mode mismatch over texture dimensions, narrow
FP8 admission and arbitrary RequirePrelude, as requested by the parent. Research first; no production
change is authorized by evidence until the intended target ABI and actual consumer agree or differ.

2026-09-25, worker: retain research-only closure because both backends, canonical storage IR,
reflection, PTX and independent controls agree. Changing compiler stride would break the established
CUDA compact layout. Do not correct old fixture comments in this slice: preserve full input identity.

## Outcomes and Retrospective

Research independently accepted by the parent. Original three cells remain mismatches;
18 independent controls and 6 assemblies pass. No compiler/runner/fixture change, build, support
unlock or cadence increment. Full246 remains current. Next implementation gate is a separate
explicit CUDA contract and full checkpoint if corpus/runner selection changes.

## Context and Current Pipeline

The fixture supplies eight floats `[1,0,10,0,0,1,20,0]` to `ConstantBuffer<float3x2>` and computes
`mul(float3(1,2,1), M)`. Discovery `_adapt_arguments_to_cuda` preserves the column-major option and
expected output while replacing the graphics target. Trace layout through `_createTypeLayout`,
CUDA source matrix representation, and direct NVVM matrix/buffer lowering.

## Scope and Non-Goals

One existing mismatch; preserve frozen452/1356, discovery113/339 and all failure history. No driver,
system, GPU, FP8, texture, or unrelated compiler work. No commit/push by this worker. Do not change
runner selection/oracles or promote additional corpus entries during research. Shared layout,
lowering, runner or ABI changes would require revising this plan and a full checkpoint.

## Architecture and Invariants

Raw host bytes and target-specific matrix packing are separate from logical matrix arithmetic.
Derive expected output from explicit offsets and independently verify with distinguishable values.
The original graphics oracle remains an unresolved original cell even if CUDA ABI controls pass.

## Interfaces and Dependencies

Native Linux RelWithDebInfo compiler/provider ABI40; CUDA12.9; targetSM80 on L4. Inspect/source
`build/nvvm-loop/slice-203-env.sh`. Use matching tools. At most4CPU total; sequential GPU suites;
30-minute command bounds. Starting full246 is inherited only after matching identities are checked.

## Milestones

1. Small runtime gate; focused unchanged discovery replay with `--match non-square-column-major`.
2. Derive offset tables and outputs from repository contract, generated CUDA/PTX and direct NVVM IR.
   Build disposable controls only under `build/nvvm-loop/slice-247-research`; compare fixed ABI
   expectations across NVRTC O3/NVVM O0/O3 without modifying the original source or expected file.
3. Document responsible layer, explicit next gate, helper/input-shape audit, and compact evidence.

## Validation and Acceptance

Run `timeout --kill-after=30s 30m python3 extras/validate-nvvm-runtime.py --config RelWithDebInfo
--cuda-path /usr/local/cuda-12.9 --architecture 80 --output build/nvvm-loop/slice-247-before/runtime`.
Run matching discovery with accepted manifest, `--match non-square-column-major --keep-mirrors`,
explicit bin/provider/architecture and output `build/nvvm-loop/slice-247-before/discovery`.
Require exactly three requested cells and retain exact classifications, counts, outputs and commands.
Research-only changes inherit all untouched246 runtime/support cells, explicitly marked historical;
no compiler suite/checkpoint is required absent implementation. Verify final source/binary/input
hashes, all retained raw evidence, no source/oracle drift, and fresh controls' exact inventory.

## Failure and Recovery

Keep failed attempts and diagnostics. Stop GPU work on device loss. Do not overwrite accepted raw
evidence. Throwaway controls are additive and do not become compiler support claims. If contract is
broad or uncertain, hand off a bounded implementation gate; no arbitrary layout fallback.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-247-before` and `slice-247-research`; compact
`semantic-evidence.slice-247.json`, completed plan/report and accepted STATUS. Parent-audit.py/json verifies all144 output values,18 executions, three exact
old outcomes,158 indexed artifacts,12 primary-source snapshots and166 compact references before
its own two audit references; source/binary/input identities and41/16 histories are exact.
