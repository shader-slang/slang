# Slice 207: Execute wave rotation through typed indexed shuffles

Status: accepted full checkpoint after parent implementation, preservation and provenance review.

## Motivation

The two unchanged frozen rotation sources stop at untagged CUDA source intrinsics in direct NVVM.
They instantiate scalar and vector float, half, signed/unsigned 8/16/32/64-bit integers and Boolean
values. Consider a lane-dependent payload whose high and low words both matter:

```slang
uint lane = WaveGetLaneIndex();
uint64_t value = (uint64_t(0xF0000000u + lane) << 32) | uint64_t(0xABC00000u + lane * 7);
uint64_t rotated = WaveRotate(value, 33);
uint64_t clustered = WaveClusteredRotate(value, 11, 8);
```

The first result comes from the next lane with wraparound. The second comes from three lanes ahead
within an eight-lane cluster. The old CUDA prelude implements this with indexed shuffles; direct
NVVM has no semantic contract for the rotation GenericAsm spelling. Its existing tagged indexed
shuffle also only admits 32-bit scalar values, so rewriting the call alone would expose another
unsupported width.

Both new fixture sources passed their independent NVRTC O3 oracles and failed NVVM O0/O3 before
production changes. The final fixture bytes are identical to those baseline sources. Each lane
writes its own output; the original frozen sources, which write a common output element, remain
unchanged. `nvvm-wave-rotation.slang` checks all vector2/3/4 components with distinct lane values,
signed/narrow/high-bit payloads, Boolean bit patterns, whole-wave wraparound, deltas 0/1/31/32/33/
UINT_MAX and cluster sizes 1/2/4/8/16/32. Independent input-table columns specify the expected source
lanes for the main rotations; successful outputs are 1000 through 1031.

`nvvm-wave-shuffle-widths.slang` independently checks the shared transport boundary: distinct high
and low 64-bit words, signed and unsigned narrow payloads, Boolean values, bit-exact negative zero
and quiet-NaN payloads in half/float/double, and a valid low 16-lane participation mask. Its outputs
are 2000 through 2031. These tests prove scalar width transport and rotation, not reconvergence or
partitioned reductions.

## Proposed solution

For CUDA SM7 and newer, compose scalar rotation from ordinary lane arithmetic and the established
`WaveMaskReadLaneAt` semantic. Keep the original full participation mask. Apply the scalar
rotation componentwise for vectors. Extend the existing scalar indexed-shuffle catalog with the
nine missing scalar types, and legalize their payload bits to the provider's 32-bit indexed
shuffle. Provider ABI 35, operation IDs and all public source contracts stay unchanged.

Rotation already advertises SM5, while `subgroup_basic` and `subgroup_shuffle` advertise SM7 in
`slang-capabilities.capdef`. The older target branch therefore retains its established CUDA
source/prelude implementation. The `_cuda_sm_7_0` capability atom selects composition where those
public APIs are available. This SM7 threshold is existing Slang capability policy, not a new claim about the hardware
minimum for a lane-register read or shuffle. No broad capability-policy change is needed. The first attempted
all-CUDA composition failed core-module capability validation; the retained branch corrects that
contract mismatch instead of weakening validation or narrowing the existing public requirement.

## Change summary

- `hlsl.meta.slang` adds SM7-specific scalar/vector composition to the four rotation overloads.
- `slang-nvvm-semantic-catalog.h` adds exact scalar indexed-shuffle signatures for bool, signed and
  unsigned 8/16/64-bit integers, half and double, with three reusable scalar descriptors.
- `slang-llvm-nvvm.cpp` adds scalar bit transport and removes the now-unreachable old catalog
  dispatch case. The existing 32-bit intrinsic emitter remains unchanged and is reused.
- Two runnable CUDA fixtures and disjoint discovery entries add independent output contracts.
  A compile-only legacy fixture preserves source branch selection at CUDA SM5/6.
  Frozen sources and existing discovery selection remain unchanged.
- A real-provider unit compares serialized clean/rejection modules. Three cases in the existing
  fake-provider unit preserve malformed semantic-signature rejection before provider discovery.
- The completed plan, manifest, portable outcomes, design facts and STATUS retain exact evidence
  and unresolved failure history. Raw logs and IR remain under ignored build directories.

## Concepts and vocabulary

A **tagged indexed shuffle** is the existing `nvvmWaveReadLaneAt` semantic emitted by the standard
library, carrying mask, scalar payload and source-lane operands. A **transport word** is one 32-bit
piece accepted by the native shuffle. A **participation mask** identifies invocations executing
the synchronized shuffle; selecting an inactive source is outside this fixture's contract.

## Process report

The input shape is canonical, intentionally supported source behavior. The unsupported form was
an opaque CUDA GenericAsm implementation, not a malformed AST value. The standard library owns
rotation arithmetic, so it now computes `(lane + delta) % 32` or
`clusterStart + ((lane - clusterStart + delta) % clusterSize)` and calls the existing scalar API.
`WaveMaskReadLaneAt` emits its producer-owned semantic tag. `_legalizeNVVMSemanticIntrinsics`
replaces tagged GenericAsm with `IRNVVMIntrinsic` and discards the CUDA spelling.
`_resolveNVVMSemanticValueOperation` checks the complete specialized helper signature against
`NVVMSemantics::find`. No rotation spelling parser, new operation ID, syntax reconstruction,
equivalence relation or alternative value representation is introduced.

The vector branches use `[ForceUnroll]` component loops. Reusing the existing opaque vector
shuffle helper would not cover the intended input: that direct consumer deliberately supports
only selected 32-bit vector types. Applying the scalar operation preserves each component's
existing type and reuses ordinary vector construction, including packed Boolean value handling.
The unchanged frozen vector tests and the new every-component fixture both exercise this path.

`_emitWaveReadLaneAt` is the sole new production helper. Its input descriptor has already matched
an exact catalog row in `_emitOperation`. The 32-bit row delegates to `_emitIntrinsic`, preserving
the existing UInt/Int/Float emission. For new widths, the helper checks insertion state, every
actual LLVM operand type and operand availability before creating any instructions or intrinsic
declarations. Wrong actual payload types for all nine new rows, plus wrong UInt64 mask/lane
operands, return invalid argument and leave the serialized module byte-identical to a clean
control module. Wrong declared mask, result/payload relation and lane types fail earlier in direct
preflight with E52017; the fake provider is not loaded or mutated.

Narrow integers and bool are zero-extended to a 32-bit transport word and truncated to their exact
original type afterward. This is bit transport, so signedness does not require sign extension.
Half and double are bitcast to equal-width integers, preserving signed zero and NaN payloads.
For 64-bit values, the low 32 bits and logical-shifted high 32 bits are shuffled with the same original
mask, source lane and clamp 31. Zero-extension, shift and bitwise OR reconstruct the exact result.
Both calls use the same existing convergent LLVM intrinsic declaration. The direct partial-mask
fixture checks this contract on the GPU. The provider owns native transport width; adding numeric
conversion or word-splitting logic to each library rotation would duplicate this responsibility.

The capability-specific branch is the only new target special case. It is justified by existing
public declarations, not by a fixture path or unsupported-input fallback. No suitable lower-level
shared tagged helper with the old SM5 contract exists: the current scalar shuffle and lane query
APIs own their SM7 requirements, while the old rotation CUDA source directly calls the prelude.
Keeping that branch preserves the lower-target behavior and avoids an unrelated capability refactor.

Self-review retains the provider helper, nine catalog rows and four capability branches. All are
covered by before-failing runtime fixtures or lower-target preservation probes. There is no
silent default, malformed-value repair, graph search, new storage ABI or discarded failure.
The full checkpoint has 1629 fresh runtime cells: 1572 correct and 57 retained known failures.
All 1562 previously correct cells remain correct. Exactly four old cells improve from preflight
failure to correct execution: the two unchanged frozen rotation sources at NVVM O0/O3. The six
new fixture/mode cells pass separately. Every other old classification, return code, full execution
count, diagnostic and canonical shape matches accepted 206. Keys are exact, with no missing,
duplicate or extra old cells. Frozen counts are 449/440/440 over 452; discovery counts are 81/81/81
over 91 for NVRTC O3/NVVM O0/NVVM O3. All unresolved failures retain prior provenance and reproduction.

Focused tests pass 8/8, the runtime gate 4/4, units 474/474 with one Windows-only skip, toolkit
18/18 and all six complex compile/assembly cells. Final source and artifact hashes match after
all gates. The base is `1214f6b4d969ee0a5395dce748a5d460899eac92`; the manifest records exact
modified-source hashes and final compiler/provider binaries. Raw evidence lives under
`build/nvvm-loop/slice-207-before` and `slice-207-after`.

The final Slang dump after `checkUnsupportedInst` shows canonical scalar UInt64/Int64/Half/Double/
Boolean helpers with `nvvmSemantic(17)` on GenericAsm. This dump precedes target semantic
legalization; it is not mislabeled as already containing IRNVVMIntrinsic. Final O0 PTX independently
shows each of the three 64-bit helper instantiations issuing exactly two indexed shuffles with
the same mask, lane and clamp before reassembly.

Four permanent legacy source tests confirm actual calls to the historical scalar/vector CUDA
intrinsics. Four representative scalar64/vector4 entry/architecture probes compile and assemble
at actual SM50 and SM60. A first broader probe revealed that the existing NVRTC 12.8+ adapter
clamps its default output to SM75 even when source capabilities request SM50/60. An explicit
`-Xnvrtc -arch=compute_50` or `compute_60` override makes the intended architecture measurable;
the resulting PTX targets and ptxas exits are recorded. No adapter code changed.

The broader frozen rotation source at explicit SM50 also encounters unavailable `hsin`/`hcos`
and related half-math declarations in the unchanged CUDA prelude. Its emitted source confirms
legacy branch selection. That failed exploratory probe remains in the manifest with source and
prelude hashes; the bounded legacy acceptance claim covers the representative integer fixture,
not half-math support on SM50. This is separate from the unchanged registered SM80 runtime ledger.
The compile-only fixture was added after the full runtime checkpoint and independently validated;
no compiler/provider source or runtime selection changed, so those completed corpora remain valid.

The wave rotations rank ahead of quad control, partitioned wave-multi operations and the remaining
FP8/BF16/prelude/harness gaps because their lane-selection contract is explicit and runnable.
Accepted 204/205/206 were complex-driven. All six registered material cells already compile and
assemble; absent application bindings and output oracles still prevent material runtime claims.
