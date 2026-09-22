# Slice 201: Typed wave prefix bit count

**Checkpoint status:** The implementation and focused checks pass, but final regression acceptance
is blocked by another GPU bus-loss fault. Slice 201 is not complete.

## Motivation

The existing frozen shader calls `WavePrefixCountBits(bool(idx & 5))` from an eight-thread group.
NVRTC executes it correctly, but direct NVVM rejects the compound CUDA GenericAsm expression in
`WaveMaskPrefixCountBits`. This is an extension-tier shader already in the corpus, not a request
to expand the four-gap historical MVP denominator.

## Proposed solution

Express the CUDA implementation as a synchronized typed ballot, a mask of lower lane indices,
and a population count. All operations already have canonical Slang IR and provider support.
Leave active-mask synthesis, other targets, and provider ABI revision 35 unchanged.

## Change summary

- `source/slang/hlsl.meta.slang`: compose the CUDA prefix-count producer from existing operations.
- `tests/hlsl-intrinsic/wave-prefix-count-bits.slang`: add permanent SM80 direct O0/O3 lanes.
- `tests/hlsl-intrinsic/wave-prefix-count-bits-cuda.slang`: add per-lane GPU correctness checks
  for full-warp boundaries, all-false/all-true/mixed predicates, explicit sparse masks, and divergence.
- The checkpoint plan/report and blocked native-Linux manifest preserve measured evidence.
  Historical design/capability counts remain unchanged until the corpus gate completes.

## Concepts and vocabulary

An **exclusive prefix** counts only participating lanes with lower indices. The **membership mask**
identifies the threads participating in the synchronized ballot. The **lower-lane mask** selects
bit positions below the current lane; it does not replace the synchronization mask. A **typed
producer** emits Slang operations whose semantics survive without interpreting CUDA source text.

## Process report

Consider the existing shader:

```slang
[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);
    outputBuffer[idx] = int(WavePrefixCountBits(bool(idx & 5)));
}
```

`WavePrefixCountBits` delegates to `WaveMaskPrefixCountBits(WaveGetActiveMask(), value)` on CUDA.
The original implementation produced one opaque GenericAsm for ballot, lane mask, and popcount.
Preflight correctly rejected that unsupported source expression. The input shader and its mask
are valid; the standard-library producer unnecessarily hid established semantics in text.

The replacement calls `WaveMaskBallot(mask, value)`, whose existing `nvvmWaveMaskBallot` annotation
becomes a typed NVVM operation. `WaveGetLaneIndex` and `countbits` similarly use the established
`nvvmWaveLaneIndex` and `nvvmCountBits` annotations. Ordinary unsigned shift/subtract/and operations
produce the lower-lane filter. NVVM legalization and preflight therefore see already supported
semantic operations, and provider emission remains unchanged.

CUDA lane indices are 0..31. `(uint(1) << lane) - uint(1)` produces zero for lane zero and
0x7fffffff for lane 31, without shifting by 32. The caller's original mask remains the ballot's
synchronization argument. Public calls inside divergent control flow retain existing synthesized
active masks; explicit masked calls retain their supplied membership mask.

Self-review inventory: no new helper, fallback, type classification, source parser, or provider
special case. The sole branch change replaces the existing CUDA producer body. The canonical
input representation is preserved rather than repaired downstream. Without that change both new
direct GPU lanes fail E52017 on the original GenericAsm while the NVRTC lane passes. Adding an
opaque-expression resolver or a new provider operation was rejected because every primitive
already exists at the correct semantic boundary.

The focused fixture initializes each output to 99 and writes one failure bitmask per lane, so a
missing execution cannot look like success. It checks all 32 outputs, including lane 31. Explicit
odd-lane membership and divergent public calls verify that inactive lanes do not contribute. The
original eight-thread shader supplies partial-warp coverage for the pending corpus replay.

The [HLSL contract](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/waveprefixcountbytes)
defines the exclusive active-lane count. The synchronized ballot preserves the CUDA membership
contract documented in the [CUDA programming guide](https://docs.nvidia.com/cuda/archive/12.5.0/pdf/CUDA_C_Programming_Guide.pdf).

## Validation

Native Linux Debug uses CUDA 13.4, target SM80, RTX A6000 SM86, and the isolated LLVM14 provider.
The new fixture passed only its NVRTC lane before the fix (1/3); after the fix it passed all three
lanes with no skips. Both original and focused shaders compiled at direct O0/O3 and assembled
through CUDA13 ptxas: 8/8 compiler/assembler commands passed.

Validation evidence is under `build/nvvm-slice201`. The affected regression domain is the fixed
61-identity frozen wave/quad subset, intended to execute freshly in all three modes. The run was
interrupted during NVRTC, so all prior corpus acceptance remains at slice 200. No inherited or
partial results are claimed as a fresh full run. The focused backend unit gate checks shared
compiler/provider paths separately. Measured checks and the blocked attempt are recorded in the manifest.

The focused unit run passed 474/474 with the existing Windows-only integer-bit fixture skipped.
The wave corpus run was interrupted by GPU loss before either NVVM mode started: 14 NVRTC rows
passed, 46 reported infrastructure failures, and the final row was interrupted. These partial
results do not establish a regression or an accepted feature gain. No final merged corpus result
was written, and slice-200 evidence remains unchanged.

Kernel diagnostics record Xid 79 at 14:43:07 UTC, followed by Xid 154 requesting OS Reboot. The
stalled wave-broadcast subprocess had a different PID than the one named in Xid 79; no individual
shader is identified as the cause. All owned GPU test processes were stopped. The earlier reboot
approval covered the prior recovery, and no second reboot was attempted. The feature loop is
paused for maintainer direction on diagnosing host stability versus using another GPU.

Self-review and an independent read-only review found no new helper, fallback, or misplaced
representation fix. Shared shuffle transport was audited as the next prerequisite for rotation,
but no implementation of that next slice has started. The checkpoint is ready to resume after
stable hardware is available; raw diagnostics remain under ignored build directories.
