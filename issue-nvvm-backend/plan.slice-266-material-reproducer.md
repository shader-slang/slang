# Isolate the material constant-loss asymmetry

This bounded ExecPlan follows `.agent/PLANS.md` and the NVVM exception requiring completed
plans/reports in local commits. On 2026-09-26 the user authorized experiment 1: reduce the material,
preserve the NVRTC/NVVM constant-loss difference, explain its boundary, then stop. This does not
authorize a production optimization or resumption of the general development loop.

## Purpose and Observable Result

Produce a small independently runnable shader for which NVRTC O3 eliminates a provably constant
exponential path and NVVM O3 retains it. Preserve a runtime-dependent positive control. Explain the
source-to-Slang-IR-to-CUDA/LLVM-to-PTX path and which experiment should follow. If reduction cannot
retain the difference, report the smallest established boundary and failed hypotheses honestly.

## Progress

- [x] 2026-09-26: Read PLANS, WORKFLOW, STATUS, HANDOFF, RESULTS and research264/265.
- [x] 2026-09-26: Confirm clean starting checkout at 95da2a79b on nvvm-backend.
- [x] 2026-09-26: Accepted262 binary/toolkit identities match; fresh original material O3
      compiles/assemblies confirm zero NVRTC versus six NVVM exponentials for both entries.
- [x] 2026-09-26: Reduce with controlled variants; retain attempts under ignored build/.
- [x] 2026-09-26: Final six wrappers pass 18/18 GPU cells and 12/12 O3 assemblies; final source
      and included header hashes match exact snapshots. Both original material O3 pairs pass assembly.
- [x] 2026-09-26: Independent source/trace and final evidence review approved; no blocking findings.
- [x] 2026-09-26: Complete compact report/evidence and handoff for the closing local commit.
      Completion notification and commit identity are recorded in the task closeout; stop after closure.

## Surprises and Discoveries

Research264's grouped probe retains exponentials in both backends; separated storage eliminates
them in both. It establishes an aggregate optimization opportunity but not the material asymmetry.
Fresh reductions show that replacing its runtime index seed with the material's constant counter
sequence removes exponentials in both backends, even with graph-return and inout-populate helpers.
Helper return alone is insufficient. The accepted262 provenance also includes the results reporter;
its expected change in265 is recorded separately from matching compiler/toolkit identities.
Top-down reduction preserves zero versus three exponentials with a simple absorption consumer and
a simplified graph producer. Removing normal preparation or the later layer write loses the
asymmetry. A single dynamic layer write suffices; a literal layer index or resetting the next-layer
counter immediately before the write removes the difference. This narrows the investigation to
counter preservation before sibling-array stores, rather than helper return alone.
A standalone 55-line graph fixture reproduces zero versus three exponentials without textures or
material code. Its final NVVM PTX has no calls, branches or initialization loop: those are not
necessary surviving blockers. Further reductions preserve the difference when returning payload
only or removing weights/retroreflection; in that earlier shape, removing counter masks, the helper branch, or one
prepare call removes it. The mask mirrors the material's high-bit closure-index decoding; it is
not a bounds check. Material-only layer controls must not be generalized to the final reduced core.

## Decision Log

- 2026-09-26, lead: Limit this slice to research fixtures and explanations. Keep compiler/provider
  and original material unchanged. A discovered optimization is a proposal for a subsequent slice.
- 2026-09-26, lead: Use one fresh-context experiment writer per WORKFLOW and separate read-only
  review. Lead owns plan/status/report integration. Serialize all compiles/GPU tests.

## Outcomes and Retrospective

The standalone masked fixture reproduces constant NVRTC/NVVM exponential counts 0/3 and passes an
independent GPU oracle in all three modes. Branchless gives0/0; unmasked still gives0/3 in the final
observable-output shape, overturning a universal mask hypothesis. Nonconstant controls produce 12/6
from input absorption 1/2; normal/layer outputs also pass. Eighteen final GPU cells and twelve O3
assemblies pass with no ignored tests. Full original material is freshly 0/6 in both entries.

Valid receiver snapshots and typed aggregate stores survive to input LLVM; optimized NVVM code
retains counter/payload loads after its calls and helper branch disappear. No particular libNVVM
pass has been identified and no production change is justified yet. Accepted262 source/build,
all 22 pins and 32 recorded artifact identities are preserved; the expected prior reporter265 hash
change is separate. The next discussion is the existing forwarding/optimization boundary, not
another automatic compiler slice. No support, material runtime or performance claim is added.

The initial 18 runtime failures were missing-entry errors before GPU execution: render-test ignores
the named entry in its compute compile path. Six wrappers exposing `computeMain` fix fixture
packaging without changing the harness. Logs and exact failed-wrapper reconstructions are retained;
the first failed shared-header whitespace snapshot is unavailable, with semantic reconstruction and
raw dumps retained instead. Final accepted snapshots are exact. C++ formatting distorted Slang
attributes; manual Allman restoration was followed by final3's complete fixture requalification.

## Context and Current Pipeline

`tests/cuda/complex/tiled_brass_material.slang` builds a graph in `make_material_instance`, returns
it through a helper, and extracts payloads. Research264 found zero absorption/false retroreflection
reloaded after dynamically indexed writes, with six exponentials retained by NVVM and none by NVRTC.
Shared Slang optimization feeds CUDA source/NVRTC or `slang-emit-nvvm.cpp` plus the NVVM provider
and libNVVM. Local float3 padding explains a separate stack difference, not proven arithmetic cause.
Existing probes are in `issue-nvvm-backend/experiments/material-attribution/`.

## Scope and Non-Goals

Source reduction, controlled compile/assembly comparisons, runnable fixtures and a precise code
trace. No compiler edits, math-option changes, blanket alias/inbounds annotations, general benchmark,
material runtime claim, ABI redesign, correctness-gap repair, dependency update or push.

## Architecture and Invariants

Treat checked Slang IR and ordinary typed field addresses as semantic truth. Establish whether the
shape is canonical before recommending its responsible optimization layer. The fixture must preserve
known constants, dynamic writes and observable outputs; a nonconstant control prevents vacuous dead
code elimination. NVRTC output comparison supplements an independent arithmetic oracle.

## Interfaces and Dependencies

Accepted262 compiler source 49593da72, provider ABI42, LLVM14, CUDA12.9.2, SM80 on L4; verify bytes
against runtime-validation.slice-262.json and restoration264 before use. Read the native build skill
only if a build becomes necessary; this experiment should use the unchanged accepted tools.
Raw root: `build/nvvm-material-reproducer266/`. At most four CPU workers, sequential GPU work.

## Milestones

1. Verify provenance and compile the unchanged material entries with O3 in each backend.
2. Reduce the material; vary graph-return vs payload-return, helper boundaries, aggregate shape and
   indexed writes independently. Preserve failing/non-differential attempts and commands.
3. Retain the smallest explanatory differential fixture and informative controls. Dump Slang IR,
   emitted CUDA/LLVM and PTX in untimed runs; assemble SM80 outputs.
4. Validate final fixture bytes, review, write five-part report and structured outcomes, close locally.

## Validation and Acceptance

Use RESULTS environment and `build/RelWithDebInfo/bin/slangc <fixture> -target ptx -stage compute
-entry <entry> -O3 -capability cuda_sm_8_0 -o <unique.ptx>`, adding `-emit-cuda-via-nvvm` for NVVM.
Use `-dump-intermediates -dump-intermediate-prefix <unique-prefix>` for downstream inspection;
inspect compiler help for the Slang IR dump option. Assemble via CUDA12.9 `ptxas -arch=sm_80 -v`.
Run final fixtures using `slang-test <paths> -use-test-server -server-count 1 -disable-retries`,
with NVRTC O3/NVVM O0/O3 directives and explicit expected outputs. Record executed/passed/ignored
counts, diagnostics, hashes and instructions by entry, not just process exit. No production changes
means no new full compiler checkpoint. Prove accepted compiler identities unchanged at closeout.

## Failure and Recovery

Keep every attempt in unique build paths with timeouts; retain compile/test failures. Do not edit
the original shader to manufacture a baseline. If runtime or reduction hits an independent blocker,
record its exact scope and close research with qualified evidence rather than claiming success.

## Artifacts and Hand-Off

Durable fixtures/README under `experiments/material-reproducer/`, report266, this completed plan,
one structured research record, and concise STATUS/HANDOFF/HISTORY/design updates. Raw snapshots,
commands, logs and full dumps stay ignored. Lead reviews, formats and makes the authorized local
research commit. Check notification capability at closeout and follow WORKFLOW's standing once-per-
slice policy; report unavailable delivery. Stop after this experiment.
