# Slice 201: Express wave prefix bit count through typed operations

This ExecPlan follows `.agent/PLANS.md`. The maintainer requires completed NVVM slice plans and
reports committed with implementation; raw execution evidence remains under ignored `build/`.

## Purpose and Observable Result

Make the existing frozen `hlsl-intrinsic/wave-prefix-count-bits.slang#cuda-1` execute correctly
through NVVM at O0/O3, and validate masked/divergent and lane-boundary cases. An exclusive prefix
counts true predicates only in participating lanes with lower lane indices.

## Current Status

Implementation and focused checks pass, but the slice is incomplete. The GPU suffered a second
Xid 79 during regression validation at 14:43:07 UTC; the driver requests OS Reboot. All GPU work
is stopped pending user direction about host stability or an alternative GPU. No reboot is authorized
by the earlier one-time reboot approval.

## Progress

- [x] 2026-09-22: Inspected the slice-200 corpus failure and existing standard-library producers.
- [x] 2026-09-22: Identified existing ballot, lane-index, integer algebra, and population-count operations.
- [x] 2026-09-22: New fixture passed NVRTC but failed both NVVM lanes before the change;
      replaced the CUDA producer body and completed the native Debug build.
- [x] 2026-09-22: Focused GPU fixture passed 3/3 and both shaders passed 8/8 O0/O3
      compile/assembly commands.
- [x] 2026-09-22: Focused unit suite passed 474/474 with one existing Windows-only skip.
- [ ] Blocked: complete the 61-case wave/quad corpus comparison after hardware recovery.
- [ ] Self-review, update durable results and documentation, format, and commit.

## Surprises and Discoveries

`WaveMaskPrefixCountBits` is an opaque CUDA GenericAsm expression even though all its components
already have typed IR semantics. Adding a provider operation or parsing that text would duplicate
existing operations. Lane indices are 0..31 on CUDA, so `(1u << lane) - 1u` handles both endpoints
without ever shifting by 32.

## Decision Log

- 2026-09-22: Compose the CUDA standard-library implementation from WaveMaskBallot,
  WaveGetLaneIndex, unsigned arithmetic, and countbits. Keep the public active-mask producer intact.
- 2026-09-22: Refresh all 61 frozen wave/quad identities because this producer is wave-specific.
  Preserve provenance for the other 391 frozen and all 82 discovery identities; no full-rerun claim.
- 2026-09-22: Preserve historical corpus identities and scope; this unlocks an extension case,
  not one of the four historical MVP gaps. Retain provider ABI revision 35.

## Outcomes and Retrospective

The producer change and new GPU regression are implemented and locally validated: 3/3 focused
GPU lanes, 8/8 compile/assembly commands, and 474/474 focused units passed. The broader wave
run was interrupted by a repeated GPU bus-loss fault; no completed comparison or corpus gain
is claimed. This checkpoint preserves the code, plan, report, and explicitly blocked manifest.

## Context and Current Pipeline

`WavePrefixCountBits(value)` calls `WaveMaskPrefixCountBits(WaveGetActiveMask(), value)` on CUDA.
The latter emits `__popc(__ballot_sync($0, $1) & _getLaneLtMask())` as untyped GenericAsm, rejected
by NVVM preflight. The source input and active-mask representation are valid. The standard-library
producer should express the operation using typed primitives already understood by legalization,
preflight, and the isolated LLVM provider. This avoids teaching emission to interpret CUDA source.

## Scope and Non-Goals

Own hlsl.meta.slang's CUDA prefix-count body and focused prefix-count tests. No new shader API,
provider ABI, mask synthesis algorithm, FP8 support, or arbitrary GenericAsm interpretation.

## Architecture and Invariants

The supplied mask remains the synchronization/membership mask for the ballot. Intersect the ballot
with lower lane bits, then count the bits. Preserve existing convergence/active-mask synthesis and
all other target implementations. Named lanes must obey the existing wave-mask call contract.

## Interfaces and Dependencies

Use the native Linux Debug build, CUDA 13.4 SM80, RTX A6000, and the existing LLVM14 provider.
Build through `/tmp/slang-skills-feature-loop/skills/slang-build/SKILL.md` with the debug preset;
existing CUDA-enabled nonembedded core-module configuration remains in use.
External semantics: https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/waveprefixcountbytes
and https://docs.nvidia.com/cuda/archive/12.5.0/pdf/CUDA_C_Programming_Guide.pdf (warp vote functions).

## Milestones

1. Add GPU regressions for full/partial warps, all-false/all-true/mixed predicates, sparse explicit
   masks, divergent public calls, and lanes zero/31; prove old direct compilation fails.
2. Replace only the standard-library producer's CUDA expression with existing typed operations.
3. Build slangc/slang-test/render-test; execute focused NVRTC/NVVM O0/O3 cases and assemble PTX.
4. Refresh the fixed 61-identity frozen wave/quad subset in all modes, compare against slice 200,
   and explicitly inherit unaffected frozen/discovery evidence; run focused unit gates.

## Validation and Acceptance

Use CUDA_PATH/CUDA_HOME/LIBNVVM_HOME=/usr/local/cuda-13.4 and selected toolkit LD_LIBRARY_PATH.
Use SLANG_NVVM_TEST_ARCH=80 for unit tests. Require actual expected GPU output and no skipped
focused cases. The original corpus shader must pass both NVVM modes; no old-correct identity may
regress. Run the 474-test focused prefix set and runtime gate as appropriate to the changed layer.

## Failure and Recovery

Retain outputs under build/nvvm-slice201. A new diagnostic is a blocker to trace, not a success.
Do not count unavailable hardware as passing; stop for user direction if semantics require a scope
change. Recovery from another device fault is separate from feature correctness.

## Artifacts and Hand-Off

Commit focused tests, the producer change, completed plan/report, and native-Linux status with
explicit provenance. Keep prior slice-200 manifests unchanged. The lead owns integration/commits.

## Regression Interruption

At 14:43:07 UTC the kernel reported Xid 79 for PID 29855 (`slang-test`) and then Xid 154 with
recovery action OS Reboot. The remaining wave-broadcast subprocess had a different PID (29907);
its being stalled does not establish the fault's cause. The corpus runner was stopped before the
NVVM modes started. Its partial logs contain 14 passes, 46 infrastructure failures after device
loss, and one interrupted reference; they cannot satisfy the 183-row regression gate. Kernel logs
are preserved under build/nvvm-slice201/driver-diagnostics and build/nvvm-feature-loop/slice-201.
Do not carry those device-loss outcomes into historical feature status. Resume only after user
direction; shared shuffle transport and rotation have not been implemented.
