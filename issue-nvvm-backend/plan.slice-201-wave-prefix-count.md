# Slice 201: Express wave prefix bit count through typed operations

This ExecPlan follows `.agent/PLANS.md`. The maintainer requires completed NVVM slice plans and
reports committed with implementation; raw execution evidence remains under ignored `build/`.

## Purpose and Observable Result

Make the existing frozen `hlsl-intrinsic/wave-prefix-count-bits.slang#cuda-1` execute correctly
through NVVM at O0/O3, and validate masked/divergent and lane-boundary cases. An exclusive prefix
counts true predicates only in participating lanes with lower lane indices.

## Current Status

Accepted on 2026-09-24 after the maintainer authorized resuming on the replacement L4 host.
The fixed 61-identity wave/quad gate completed all 183 cells with no old-correct regression.
NVRTC passed 61/61; NVVM O0/O3 each passed 53 with eight existing preflight gaps. Prefix count
is the only changed outcome, now correct in both direct modes. This is a bounded replay, not a
fresh full frozen/discovery run. The original A6000 fault remains historical and unexplained.

## Progress

- [x] 2026-09-22: Inspected the slice-200 corpus failure and existing standard-library producers.
- [x] 2026-09-22: Identified existing ballot, lane-index, integer algebra, and population-count operations.
- [x] 2026-09-22: New fixture passed NVRTC but failed both NVVM lanes before the change;
      replaced the CUDA producer body and completed the native Debug build.
- [x] 2026-09-22: Focused GPU fixture passed 3/3 and both shaders passed 8/8 O0/O3
      compile/assembly commands.
- [x] 2026-09-22: Focused unit suite passed 474/474 with one existing Windows-only skip.
- [x] 2026-09-24: Complete all 183 wave/quad cells on L4/CUDA 12.9.2; preserve all 165
      previously correct cells and gain the two direct prefix-count cells.
- [x] 2026-09-24: Rerun focused CUDA tests (6/6), runtime gate (4/4), selected units
      (473/473, one Windows-only skip), and prefix compilation/assembly (8/8 commands).
- [x] 2026-09-24: Complete self-review and durable acceptance records for the reconciliation commit.

## Surprises and Discoveries

The original wave selection was reconstructed byte-for-byte from the durable slice-195 rows
using the original id/source CSV writer convention (CRLF). Its SHA-256 matches the interrupted
attempt: `409eb106a0cc7a7e4b1616388f9ee1d2aa334cc2f146caf65979908663354360`.

`WaveMaskPrefixCountBits` is an opaque CUDA GenericAsm expression even though all its components
already have typed IR semantics. Adding a provider operation or parsing that text would duplicate
existing operations. Lane indices are 0..31 on CUDA, so `(1u << lane) - 1u` handles both endpoints
without ever shifting by 32.

## Decision Log

- 2026-09-24: Maintainer authorized resuming acceptance on L4/CUDA 12.9.2. Compare exact
  identities against the durable slice-195 rows; the slice-200 manifest records preservation of
  direct results but its raw per-row logs are unavailable on this host. Do not fabricate a
  reconstructed slice-200 result set or claim unrelated cases were rerun.

- 2026-09-22: Compose the CUDA standard-library implementation from WaveMaskBallot,
  WaveGetLaneIndex, unsigned arithmetic, and countbits. Keep the public active-mask producer intact.
- 2026-09-22: Refresh all 61 frozen wave/quad identities because this producer is wave-specific.
  Preserve provenance for the other 391 frozen and all 82 discovery identities; no full-rerun claim.
- 2026-09-22: Preserve historical corpus identities and scope; this unlocks an extension case,
  not one of the four historical MVP gaps. Retain provider ABI revision 35.

## Outcomes and Retrospective

Slice 201 is accepted with the eight existing wave/quad capability gaps retained. The fresh
[census](census.slice-201-wave.tsv) preserves per-identity outcomes; the
[manifest](runtime-validation.slice-201.json) records the exact comparison, toolchain hashes,
commands, known gaps, focused checks, and complete historical interrupted attempt. No compiler
change was needed during reconciliation. The other 391 frozen and all 82 discovery identities
retain earlier evidence and are not claimed as rerun. Historical denominators are unchanged.

The old slice-200 raw results are absent on this host. Exact comparisons use checked-in slice-195
rows, with slice-200's durable preservation summary linked separately. A new session should establish
a full current-host baseline before the next feature, rather than infer it from this bounded replay.

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

For the completed reconciliation use native Linux Debug, CUDA 12.9.2 SM80, NVIDIA L4, and the
existing isolated LLVM14 provider. The former CUDA 13.4/A6000 run remains historical. Build through
`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md` with the debug preset;
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

Use CUDA_PATH/CUDA_HOME/LIBNVVM_HOME=/usr/local/cuda-12.9 and selected toolkit LD_LIBRARY_PATH.
Use SLANG_NVVM_TEST_ARCH=80 for unit tests. Require actual expected GPU output and no skipped
focused cases. The original corpus shader must pass both NVVM modes; no old-correct identity may
regress. Run the named NVVM/routing/reporter prefixes and runtime gate; record actual counts and explicit
skips. The reconciliation selection ran 473 unit tests, rather than relabeling it as the old 474.

## Failure and Recovery

Retain reconciliation outputs under `build/nvvm-slice201-reconcile`; old evidence paths remain
recorded in the historical interrupted attempt. A new diagnostic is a blocker to trace, not a success.
Do not count unavailable hardware as passing; stop for user direction if semantics require a scope
change. Recovery from another device fault is separate from feature correctness.

## Artifacts and Hand-Off

Commit focused tests, the producer change, completed plan/report, and native-Linux status with
explicit provenance. Keep prior slice-200 manifests unchanged. The lead owns integration/commits.

## Historical Regression Interruption (superseded by acceptance above)

At 14:43:07 UTC the kernel reported Xid 79 for PID 29855 (`slang-test`) and then Xid 154 with
recovery action OS Reboot. The remaining wave-broadcast subprocess had a different PID (29907);
its being stalled does not establish the fault's cause. The corpus runner was stopped before the
NVVM modes started. Its partial logs contain 14 passes, 46 infrastructure failures after device
loss, and one interrupted reference; they cannot satisfy the 183-row regression gate. Kernel logs
are preserved under build/nvvm-slice201/driver-diagnostics and build/nvvm-feature-loop/slice-201.
Do not carry those device-loss outcomes into historical feature status. The maintainer authorized
the replacement-host replay on 2026-09-24; it completed successfully as recorded above. Shared
shuffle transport and rotation have not been implemented.
