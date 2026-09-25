# Establish the CUDA quad reconvergence contract

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer requires completed slice plans and
reports to be committed; the parent owns acceptance and commit. This worker makes no commit.

## Purpose and Observable Result

Determine precisely why the remaining frozen quad-control source rejects direct NVVM O0/O3,
what QuadAny/QuadAll promise on CUDA, and the smallest principled follow-up. Research does not
admit an operation or turn incidental inactive-lane output into an oracle.

## Progress

- [x] 2026-09-25: Read workflow, status, accepted 233 report and prior quad references. Confirm clean
      branch `nvvm-backend`, HEAD `f77142a2e8d54b66482dc85b4e99e2a6e28de777`.
- [x] 2026-09-25: Trace marker producers, target consumers, quad helpers and provider primitives.
- [x] 2026-09-25: Run fresh smoke 4, then bounded dynamic CUDA probes and unchanged direct rejections.
- [x] 2026-09-25: Record external contracts, hash-addressed evidence, five-part report and durable handoff.

## Surprises and Discoveries

Initial alias probes failed E45001 and remain preserved. Standalone markers reach direct E52017
but CUDA source E99999; real GenericAsm intrinsic bodies are replaced wholesale by source emission.
No global marker no-op is justified. Complete source quads can rendezvous across divergent SM80
shuffle instructions; absent source lanes are undefined, unlike active-only SPIR-V votes. The
runtime probe uses only justified domains. A missing optional BeautifulSoup package and an LLVM
site fetch reset were handled with standard-library extraction and the official LLVM repository;
no package/system change occurred.

The standard library unconditionally emits both RequireMaximallyReconverges and
RequireQuadDerivatives before its target switch. Its explanatory comment scopes their execution
modes to SPIR-V/GLSL. CUDA helpers use four full-mask indexed shuffles. Official participation rules and the probes establish the contract separately; the marker alone
is not proof of a CUDA reconvergence promise.

## Decision Log

- 2026-09-25, worker: Select two remaining quad direct preflight cells for reusable control-flow
  research. Preserve accepted 233/229 evidence. Defer unrelated matrix layout mismatch (all three
  modes; source documents ignored CUDA layout), texture dimensions and arithmetic families.
- 2026-09-25, worker: Material reconsidered: six inherited compile/assembly cells pass, but missing
  application bindings, textures/LUT/input and oracle prohibit runtime claims. No new question.

- 2026-09-25, worker: Retain the canonical helper representation and stop at typed helper
  admission. Both markers belong to target requirements within that helper; source replacement,
  official synchronization contracts and independent runtime controls support a later helper
  recipe, not global marker suppression. Partial source quads are excluded from the oracle domain.

## Outcomes and Retrospective

Research and independent parent acceptance are complete. Smoke 4 and all 2,048 dynamic launches pass, with
131,072 independent output words and unchanged inputs/sentinels. Fourteen PTX artifacts assemble.
The three selected frozen cells preserve all five outcome fields. The next independent blocker is
typed GenericAsm `_slang_quadAny/All, signature=bool(bool)`; no production patch is justified by
ignoring markers alone. Implementation cadence remains two since full 229; research does not advance
it. All 555 inputs, 30 source paths and 12 artifacts preserve accepted 233.

## Context and Current Pipeline

Frozen `tests/hlsl-intrinsic/quad-control/quad-control-comp-functionality.slang` partitions 16
threads into four quad-aligned branches and invokes QuadAny/QuadAll. `hlsl.meta.slang` emits the
requirements and CUDA intrinsic names; `core.meta.slang` declares the marker IR operations.
`prelude/slang-cuda-prelude.h` implements the four full-mask shuffles. Direct NVVM rejects the
first marker in preflight. Trace the second marker and helper independently without production edits.

## Scope and Non-Goals

Research only. No compiler, provider, ABI, registered source, runner, corpus or oracle changes.
No rebuild. No commit/push/system changes. Stop at a precise next independent blocker. Preserve
555 registered inputs, 30 tested source paths and 12 artifacts. Generated probes and logs live
under ignored `build/nvvm-loop/slice-234-quad`.

## Architecture and Invariants

Target-specific execution-mode requirements are distinct from lane communication. Source CUDA
participation and NVVM/PTX convergence contracts must be explicit. Independently compute expected
boolean results only for defined participating lanes. Retain source, inputs, expected and actual
buffers unchanged; characterize undefined cases without promoting them to tests.

## Interfaces and Dependencies

Native Ubuntu; L4 SM89, driver 580.126.09; CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, target SM80,
provider ABI 36; matching RelWithDebInfo bin/lib through inspected slice-203-env.sh. Compiler hash
`92ae81d069aeda9a6ff2a61edec43f572b2af02bb7ec677fc444490ea9a966f1`; provider hash
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Use official CUDA/PTX/NVVM and Khronos specifications for uncertain external semantics.

## Milestones

1. Capture baseline hashes and exact canonical trace; inspect reusable 232/233 driver scaffolding.
2. Run `extras/validate-nvvm-runtime.py --config RelWithDebInfo --cuda-path /usr/local/cuda-12.9
--architecture 80 --output build/nvvm-loop/slice-234-quad/runtime` under bounded supervision.
3. Compile minimal marker/helper controls at direct O0/O3; generate CUDA and NVRTC PTX. Execute
   small dynamic quad truth-pattern, reconverged branch and helper cases with independent oracles.
   Prove or reject divergent participation eligibility from official specifications before launch.
4. Record exact limitations and minimal next action in report, semantic evidence and STATUS.

## Validation and Acceptance

Fresh smoke 4/4 is required before GPU probes. Sequential GPU suites, maximum four CPU workers,
bounded commands and no retries after device loss. Unchanged accepted 233 cumulative ledger is
1,677 cells / 1,630 correct / 47 unresolved / ten resolved histories, 642 fresh then and 1,035
inherited full 229. Frozen 452 identities/1,356 cells, discovery 107/321. No full rerun is required
for unchanged research. Preserve 30 source/12 artifact hashes and all 555 inputs before/after.
Evidence includes exact commands, return codes, source/PTX/cubin/input/expectation/actual hashes,
raw logs and source revision. A reproducible unsupported contract is a valid research result.

## Failure and Recovery

Retain failed probe attempts under distinct names. A timeout is incomplete; inspect it before
rerunning. Stop GPU dispatch on device loss. Do not change driver or reboot. If source behavior
is undefined under partial participation, retain diagnostic evidence and limit the oracle domain.

## Artifacts and Hand-Off

`report.slice-234-quad-reconvergence.md`, `semantic-evidence.slice-234.json`, this plan and concise
STATUS/design facts. Raw evidence under `build/nvvm-loop/slice-234-quad`. Return write ownership
to parent with exact counts and limitations; parent independently accepts and commits.
