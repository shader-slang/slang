# Qualify FP8 scalar values on SM80

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer exception requires the completed plan and report to be committed; raw artifacts remain ignored.

## Purpose and Observable Result

Qualify, without production admission, distinct E4M3/E5M2 scalar i8 values and internal helper transport, exact Float32 widening, and CUDA12.9 Float32 narrowing. Preserve an independently checked SM80 NVRTC/raw LLVM O0/O3 experiment. If casts cannot be qualified within this slice, retain transport proof and a concrete smaller next boundary.

## Progress

- [x] 2026-09-25: Read repository workflow, accepted242, local build skill and environment.
- [x] 2026-09-25: Freeze baseline identity and pass runtime smoke4 before research GPU work.
- [x] 2026-09-25: Trace source producers and installed authoritative conversion contracts; freeze oracle inputs.
- [x] 2026-09-25: Execute NVRTC and raw libNVVM O0/O3 controls with complete buffers and ptxas.
- [x] 2026-09-25: Complete independent-ready evidence, design contract, report, preservation audit and handoff.

## Surprises and Discoveries

CUDA Float32 constructors use RNE/SATFINITE on SM80. Independent parent oracle agrees on 24,290 frozen words. Twelve isolated controls pass145,740 words; two actual Slang NVRTC controls pass another24,290. Shared literal conversion is not equivalent: literal E4M3(256) becomes448 and minimum subnormals misfold. Overflow differs by existing policy, explicitly tested in unit-test-math.cpp. The first raw LLVM attempt omitted target data layout and was retained before correction.

## Decision Log

- 2026-09-25 worker: Choose FP8 over exact BF16 integer casts (research238 double-rounding counterexample) and texture GetDimensions (existing reference mismatch). Material runtime reconsidered but unavailable bindings/textures/LUT/input/oracle still prevent a runtime claim; support/correctness cadence override applies.
- Research only: no compiler rebuild or full replay when all accepted242 identities remain exact. Latest full242, targeted233, cadence0 remain authoritative.

## Outcomes and Retrospective

Runtime transport and Float32 casts are qualified as research only:14 launches/170,030 full words,zero mismatches; separate five-case literal probe retains41 observational words. All40sources/12artifacts/560inputs and117/132/88/129 old indexed artifacts are exact. No GPU loss or production edits. No production support changed. The package and independent parent acceptance are complete; the parent owns the authorized local commit. Finite/subnormal constant producer repair ranks before backend promotion; existing overflow-policy differences need separate treatment.

## Context and Current Pipeline

Frozen substandard-fp-folding stops at FloatE4M3 helper parameter in both direct modes. Dynamic-dispatch-substandard-float stops at result A containing FP8 fields; aggregate/dynamic-object requirements are separate. Trace canonical core FP8 type, literal/bitcast and FloatCast producers into CUDA constructors and NVVM type planning before describing a future admission.

## Scope and Non-Goals

Only new raw research and documentation. Exclude production/frontend/prelude/ABI/test-runner changes, registered tests or corpus changes, external CUDA ABI, storage/resources/aggregates, vectors, dynamic dispatch, arithmetic, and integer/Half/double constructors. Do not investigate independent next blockers or start another slice.

## Architecture and Invariants

Use physical i8 with format-distinct semantic descriptors; never assume native LLVM float8. Use only SM80-compatible operations. Dynamic inputs prevent constant folding from substituting for runtime conversion proof. Independent rational/integer oracles define expected bits before recipes run; NVRTC is supplemental. Preserve source input and sentinel words.

## Interfaces and Dependencies

Native Ubuntu; accepted RelWithDebInfo/provider ABI40, CUDA12.9.2/NVRTC12.9.86, LLVM14, targetSM80 on L4SM89. Inspect installed cuda_fp8 headers and repository source. No network/setup changes. Four CPU workers total; GPU sequential; commands bounded30min.

## Milestones

1. Snapshot all40 source hashes,12 artifacts,560 runtime inputs and old indexed research; smoke4.
2. Inspect contracts; freeze all256 encodings per format and Float32 exact midpoint/adjacent/sign/zero/subnormal/overflow/inf/NaN grid with independent expected buffers.
3. Public CUDA NVRTC and raw LLVM/libNVVM O0/O3 transport and conversion controls; assemble every PTX and run on GPU. Retain failures distinctly.
4. Write durable contract and five-part report; index artifacts and audit unchanged baseline; release checkout to parent.

## Validation and Acceptance

Run `source build/nvvm-loop/slice-203-env.sh` then bounded `python3 extras/validate-nvvm-runtime.py --config RelWithDebInfo --cuda-path /usr/local/cuda-12.9 --architecture 80 --output build/nvvm-loop/slice-243-fp8/runtime`. Prototype scripts under that raw root record exact commands, compiler/header/tool identity, generated LLVM/PTX/cubins, dynamic inputs, expected and actual complete outputs. Promotion requires parent oracle and exact recipe review; qualification is not production support. No new corpus cells; inherited1692/1651correct/41unresolved/16resolved histories remain unchanged.

## Failure and Recovery

Stop GPU work on device loss. Preserve failed attempts and do not overwrite accepted artifacts. Discard unqualified cast recipes without weakening expected output; transport remains separately reportable. No commit, push, driver change or reboot by worker.

## Artifacts and Hand-Off

`build/nvvm-loop/slice-243-fp8/` holds raw scripts, data and index. Deliver this plan, `report.slice-243-fp8.md`, `semantic-evidence.slice-243.json`, `docs/design/nvvm-fp8-scalar-contract.md` and STATUS update. Parent owns independent acceptance and local commit.

2026-09-25 parent acceptance: independently verified180 indexed artifacts,19 primary sources, all
baseline identities and prior indices, exact frozen boundary grids and170,030 qualified runtime words.
The separate41-word producer probe confirms three finite literal errors and two policy differences.
Nine parent evidence artifacts are retained; no production or corpus state changed.
