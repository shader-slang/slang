# Preserve live CUDA clock reads through NVVM

This ExecPlan follows `.agent/PLANS.md` and WORKFLOW.md. The maintainer requires completed NVVM
plans/reports committed with each slice; the parent owns acceptance and commits. Base is
`cc0982c68365933cbd6b7885e94cc033d3eea566` on native Linux branch `nvvm-backend`.

## Purpose and Observable Result

Support exact CUDA GenericAsm `clock` uint() and `clock64` int64_t() with live observations at
NVVM O0/O3. Two original frozen clock cells become correct; an independently expected registered
fixture and unchanged research236 buffers prove observations are neither commoned nor hoisted.

## Progress

- [x] 2026-09-25 Read workflow, full235, accepted research236 and native slang-build skill.
- [x] 2026-09-25 Write plan before production changes and finalize readable runtime fixture.
- [x] 2026-09-25 Final readable fixture passed NVRTC and rejected direct O0/O3 on accepted compiler.
- [x] 2026-09-25 Added typed operations, ABI37, provider sideeffect PTX and strict negative tests.
- [x] 2026-09-25 Formatted explicit C++, restored unrelated hunks, built matching optimized tools; smoke4/4 precedes all GPU suites.
- [x] 2026-09-25 Final fixture3/3 and unchanged research replay60/60 pass; independent raw-buffer checker agrees.
- [x] 2026-09-25 Full checkpoint 1,683 fresh cells / 1,640 correct; only two clock fixes and three additions. Exact preservation and separate audit pass.
- [x] 2026-09-25 Completed helper/input-shape audit, report, STATUS and compact evidence; handoff ready without worker commit.

## Surprises and Discoveries

Research236 proves LLVM clock intrinsics common at O0/O3 and hoist at O3 on libNVVM12.9. Existing
canonical spelling resolver and semantic catalog already enforce whole-body, arity and exact types.
No new compiler helper is needed. Clock reads require sideeffect, not a convergence or fence claim.
Initial formatting lacked environment tool paths and changed nothing; sourcing the inspected env
made explicit-path formatting succeed. Unrelated historical C++ and design hunks were exactly
reversed. Final build and all gates run afterward; no production/test input changed during gates.
O3 public PTX has three loop reads; O0 uses two helper bodies invoked three times per iteration,
so static move counts alone are not an observation oracle.

## Decision Log

- 2026-09-25: Use append-only semantic operations and ABI37, exact single-source catalog overloads,
  and provider inline PTX matching research. Do not map LLVM clock intrinsics or alter producers.
- 2026-09-25: Provider contract forces full checkpoint even though implementation cadence is zero.
  Retain all full235 failures/histories; only two clock cells may change among old cells.

## Outcomes and Retrospective

Implementation and full validation are accepted after independent parent review. Exact typed ABI37
operations retain live per-SM observations through side-effecting PTX. All gates pass: smoke4,
fixture3, research60, units481 plus one existing skip, toolkit18, contracts6, frozen1356,
discovery327, material6 compile/assembly. Full comparison retains all1,635 old correct cells,
fixes only two frozen clock cells and adds three fixture cells. Total1,683 fresh /1,640 correct;
43 unresolved and14 resolved histories remain explicit. No inherited final-source cells or reset.
All35 source /12 artifact /557 registered input identities match each gate; all556 old inputs and
241 indexed research artifacts remain unchanged. Parent acceptance verified 917 unique evidence
references, exact corpus/history preservation and all raw clock buffers; local commit completes
this bounded slice. Latest full checkpoint is 237 and implementation cadence is zero.

## Context and Current Pipeline

Consider `uint2 a = getRealtimeClock(); uint b = getRealtimeClockLow(); uint2 c = getRealtimeClock();`.
CUDA target switches in hlsl.meta.slang produce `clock64` signed64() and `clock` unsigned32()
GenericAsm helpers with NonUniformReturn. `_resolveNVVMGenericAsmValueOperation` resolves their
whole-body canonical form through a spelling table and `_resolveNVVMSemanticValueOperation`;
NVVMSemantics catalog owns typed validity and provider capability checks. Provider
`_emitIntrinsic` owns live backend observations. Existing unsigned casts/word splitting are proven.

## Scope and Non-Goals

Only emitter spelling entries, semantic catalog, provider/API contract, negative/unit tests,
runtime fixture/discovery registration and evidence/docs. No frontend/library/runner changes,
immutable frozen edits, timestamp equality, globaltimer, fences or material runtime/performance claim.
Material application bindings/textures/LUT/input/oracle remain absent.

## Architecture and Invariants

Zero parameters do not imply purity. `mov.u32 %clock` / `mov.u64 %clock64` inline assembly carries
LLVM sideeffect. Exact scalar result types and zero arity are validated by existing semantic lookup
before provider loading; provider validates descriptors again. No syntax reconstruction, fallback,
new equivalence, operand search or special-case word conversion.

## Interfaces and Dependencies

Append CLOCK/CLOCK64 operations and bump ABI36 to37; existing exact-version negotiation rejects
old providers. CUDA12.9.2/NVRTC12.9.86, LLVM14, L4 SM89 target80 driver580.126.09, optimized host.
Use inspected `build/nvvm-loop/slice-203-env.sh`; build with CMAKE_BUILD_PARALLEL_LEVEL=1 and
`cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.

## Milestones

1. Final fixture: dynamic 0/2/5/16 rounds, independent affine-work expectations and modulo clock
   order/bracket/progress invariants; before compiler must pass NVRTC and reject both direct modes.
2. Add exact operations and test invalid widths/signedness/vector/arity/whole-body shapes plus
   provider serialized sideeffect calls in both LLVM dialects and descriptor rejection.
3. Build/format; four smoke cases first. Fresh fixture3, research public3/control2 modes using all
   twelve unchanged buffers, relevant unit2servers, toolkit18 and runner contracts6.
4. Full frozen explicit `--workload-ids-from issue-nvvm-backend/census.slice-195.tsv`452/1356,
   discovery108 old/324 plus fixture3, material6 compile/assembly; max four CPUs, sequential GPU.
5. Compare exact classification, return_code, complete execution_counts, diagnostic, canonical_shape
   against full235; preserve 31 old source/12 artifact provenance and all556 input hashes.

## Validation and Acceptance

All suites bounded by `timeout --kill-after=30s 30m`. Existing scripts/CLI commands follow WORKFLOW.
Raw roots `build/nvvm-loop/slice-237-before` and `slice-237-after`. Capture per-gate final source/
artifact/input identities. Expected cumulative1683 cells/1640 correct/43 unresolved/14 resolved,
with exactly two fixed old clock direct cells and three fixture additions. Original histories remain.
Research replay compares relational predicates and deterministic work/sentinels, never exact ticks.
Naive intrinsic failures remain immutable research counterexamples. Full235 other evidence is an
obligation, not inherited final-source passing evidence. Smoke gate must pass before GPU suites.

## Failure and Recovery

Stop on actual GPU loss or unresolved regression/consequential semantic choice; preserve incomplete
logs separately. Investigate ordinary failures. Never edit running scripts or reset acceptance to
hide losses. No driver changes, reboot, push or worker commit.

## Artifacts and Hand-Off

Completed plan/five-part report/design/status, compact runtime-validation237 and census TSVs.
Raw logs, sources, buffers, exact implementation patch, provenance and independent checker remain
under ignored build/. Parent reviews final diff/evidence and owns acceptance/local commit.

Parent acceptance completed on 2026-09-25 using `slice-237-after/parent-audit.py`; all checks pass.
