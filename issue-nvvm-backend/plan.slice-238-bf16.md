# Establish the scalar BF16 implementation contract

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer requires completed plans and reports
committed per slice; this research worker does not commit. Root owns acceptance and commit.

## Purpose and Observable Result

Determine the canonical CUDA BFloat16 representation, conversion/arithmetic semantics and the
responsible NVVM boundary for frozen `hlsl-intrinsic/scalar-bf16.slang#cuda-1`. Preserve an
independent bit-pattern runtime oracle and an implementation handoff; add no compiler support.

## Progress

- [x] 2026-09-25: Read repository instructions, workflow, full237 STATUS/report and local environment.
- [x] 2026-09-25: Write this bounded plan before experiments.
- [x] 2026-09-25: Confirm 35 source, 12 artifact and 557 input hashes against full237; run fresh smoke4.
- [x] 2026-09-25: Replay only scalar-bf16 frozen identity at NVRTC O3/direct O0/O3; capture source/IR trace.
- [x] 2026-09-25: Establish primary documentation/source contracts and bounded independent GPU controls.
- [x] 2026-09-25: Finish exact compact evidence/index, five-part report, design note and STATUS handoff.

## Surprises and Discoveries

The frozen IR intentionally retains BF16 and canonical `_slang_vector_dot`; this is a backend
format-support gap. Existing width-16 floating descriptors mean IEEE half. CUDA BF3/BF4 layout
also differs from compact Half policy. Native libNVVM rejects `bfloat` at parsing; explicit i16
transport plus format-specific narrowing works at O0/O3. Integer 16842753 rounds to BF16 0x4b81,
but nearest FP32 first gives BF16 0x4b80. Both signs are measured. NaN output bits are observations,
not universal defaults. The initial rational oracle exponent conversion was off by one and failed
its hand assertion before data generation/GPU use; retained attempt1 documents its correction.
The dynamic source-only CLI probe rejects E50100 without harness conformances; early IR is useful
source evidence only. No direct dynamic-dispatch corpus replay or support claim follows.

## Decision Log

2026-09-25, worker: select BF16 research for two measured frozen direct failures and reusable
16-bit storage/conversion boundary. FP8/dynamic dispatch, arbitrary RequirePrelude, textures and
CUDA target-wide ignored column-major matrix layout remain independent. Six accepted237 material
compile/assembly cells are reconsidered, but missing binding/texture/LUT/input/output oracle prevents
material runtime or performance claims. Research does not advance implementation cadence (zero).

2026-09-25, worker: keep physical i16 with an explicit BF16 semantic format as the implementation
handoff, because native bfloat is parser-rejected and width-only half admission is semantically wrong.
Reuse storage/helper infrastructure only after role/format audit. Integer and vector/dot boundaries
remain separately gated. Revisit if supported NVVM dialect/toolchain or target changes. The bounded
grid is complete; do not expand into FP8, broad layout or material runtime without its contract.

## Outcomes and Retrospective

Research completed 2026-09-25, accepted after independent parent review. Exact source/artifact/input identity
matches accepted 237 before/after. Smoke 4/4 and three frozen cells are fresh and unchanged. Six
GPU control launches check 1,174,423 output words and preserve 3,531,423 input/sentinel words,
all correct. Six SM80 assemblies and ten layout assertions pass. Native bfloat parsing fails at
O0/O3 as retained counterevidence. Raw index, independent rational oracle and compact summary
are retained. Full 237 remains the accepted checkpoint, 233 latest targeted, implementation cadence
zero. No production support change, new corpus entries, compiler build or commit.

The next slice needs explicit semantic BF16/provider negotiation and bounded storage/conversion
admission. Integer construction, vector transport and source-ordered dot are distinct obligations;
a frozen fix requires all its operations. Broader shared type/provider changes trigger a full
checkpoint. No more experiments are needed to select that bounded implementation.

## Context and Current Pipeline

Frozen scalar-bf16 stores sizeof2, bitcasts `0x4080404040003F80` to four BF16 lanes, converts them to
1/2/3/4 floats, converts3.0 to BF16 bits16448, and computes BF16 dot8.5. Full237 direct modes reject
`helper function result type: BFloat16`. Trace core.meta declarations through final CUDA IR and
NVVM preflight/type lowering; verify whether producers intentionally retain BFloat16. Existing Half
physical-i16 helper ABI, byte/storage and semantic cast helpers are candidates to reuse, not proof
that BF16 can be classified as Half. Source files are read-only in this slice.

## Scope and Non-Goals

No production/provider/ABI/frontend/library/test/corpus/runner changes, compiler builds, commits,
pushes or system changes. Generated source, LLVM/PTX, scripts, raw data and unsuccessful attempts
stay under ignored `build/nvvm-loop/slice-238-bf16/`. Do not change executing scripts or old evidence.
Do not implement FP8 or claim workload fixes from shifted diagnostics. Bound dynamic-dispatch
linkage by source/IR only. Stop at precise next implementation boundary or independent blocker.

## Architecture and Invariants

Semantic BFloat16 must remain distinct from Half and UInt16. Physical 16-bit transport must preserve
all bit patterns; arithmetic/conversion contracts must be explicit, including ties, NaNs, infinity,
subnormals and signed zero. Independent oracle computes finite rounding with integer/rational math;
NaN bit guarantees require primary source evidence and must be separated from observations.

## Interfaces and Dependencies

Native Ubuntu24.04, L4SM89, targetSM80, driver580.126.09, CUDA12.9.2/NVRTC12.9.86, LLVM14 ABI37.
Base `2e21fa2dfa87adbacf8ead26690c26c88896ac55`, branch nvvm-backend, matching RelWithDebInfo.
Inspect `build/nvvm-loop/slice-203-env.sh` and its sourced environment before running.
Compiler hash a89e9b370b03e62a62fe5f6becaab312399d5cec60bac6d75a53ff649a75c19f;
provider dafc5a557ce6f83d358c89956910af9761e352bb2f70f5efc6d5e7bc7f8a89ea.

## Milestones

1. Capture identity, run smoke4, replay exact frozen3. Source/binary/input mismatch blocks reuse.
2. Read installed CUDA headers and primary CUDA/NVVM/PTX documentation; preserve URLs and excerpts
   or local-source references. Trace canonical Slang shape and reusable helper boundaries.
3. At most four control families: exhaustive BF16 transport/extension; bounded Float32 rounding
   around BF16 ties/extremes; bounded BF16 binary arithmetic/dot; isolated LLVM/PTX storage/cast
   feasibility at O0/O3 if needed. Use shared data and independent expected output. At most two
   corrective versions per family; retain failed attempts separately. No broad random search.
4. Document exact handoff, rejected alternatives and support limitations; package hashes/results.

## Validation and Acceptance

Before GPU research capture and compare full237 identity sets, then
`timeout --kill-after=30s 30m python3 extras/validate-nvvm-runtime.py --config RelWithDebInfo
--cuda-path /usr/local/cuda-12.9 --architecture 80 --output build/nvvm-loop/slice-238-bf16/runtime`.
Replay immutable scalar-bf16 with run-compute-census.py explicit one-row selection and all3 modes.
Run controls sequentially with at most4 CPU workers and 30-minute bounds. Independent expected
bit patterns, output lengths and retained inputs are required. All other full237 1680 runtime cells,
units481+skip, toolkit18, runner6 and material6 are explicitly inherited, never fresh passes.
Research has no full-checkpoint trigger because compiler/provider/runner/inputs remain unchanged.

## Failure and Recovery

Stop GPU launches on device loss. Retain timeout/failure logs, source and artifacts under unique
paths; never overwrite accepted evidence. Stop the grid when the next implementation boundary is
clear; gaps outside scalar BF16 remain recorded separate work. Sandbox bwrap is unavailable, so
routine commands request escalation. No missing material semantics is invented.

## Executed Commands and Exact Results

All commands ran after sourcing the inspected slice-203 environment, sequentially with 30-minute
outer bounds and no more than four CPUs. The frozen command uses `frozen-selection.tsv` (one
immutable identity), `run-compute-census.py --config RelWithDebInfo --bin-dir
build/RelWithDebInfo/bin --provider build/RelWithDebInfo/bin --architecture 80 --jobs 2`.
Raw `trace/results.json`, `controls/compile-results.json` and `llvm/results.json` retain each exact
source/backend/optimization command, diagnostic and return code. Run scripts are `trace.py`,
`make-controls.py`, `compile-controls.py`, `make-inputs.py`, `run-controls.py`, `check-controls.py`,
`llvm-probes.py` and `run-llvm.py`, all under the slice raw root. `oracle.py` computes exact rational
expectations; binary buffers retain every input, expected result and output. `package.py` builds
the final hash index and verifies exact frozen outcome comparison and identity preservation.

The conversion data has 73,190 records: all 65,536 BF16 encodings, 7,654 additional Float32
patterns and 75 cycling distinct signed integers. CUDA/NVRTC each check five outputs per record;
raw NVVM O0/O3 each check three outputs per record and preserve remaining sentinels. CUDA arithmetic
checks 676 pairs × four operations. Public Slang dot checks 679 cases. All 1,174,423 outputs and
3,531,423 untouched words pass. The 26 distinct int double-round counterexamples are intentional
countermodel observations, not failed correct-constructor checks. Public direct conversion/dot
controls remain unsupported (four return-255 attempts); raw bfloat parse probes return6 twice.

## Artifacts and Hand-Off

Completed plan and report `issue-nvvm-backend/*slice-238-bf16.md`, compact
`semantic-evidence.slice-238.json`, durable NVVM design facts and updated STATUS. Raw source,
commands, IR, binaries, inputs/outputs, oracle and SHA256 index under ignored slice-238-bf16 root.

Parent acceptance on 2026-09-25 verified all indexed artifacts and immutable identities, exact
frozen outcomes, and every raw buffer with a second oracle. Research238 is accepted; full237 and
implementation cadence zero remain unchanged.
