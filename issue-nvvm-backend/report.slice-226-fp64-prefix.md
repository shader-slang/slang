# Establish FP64 masked prefix min/max semantics

## Motivation

The original frozen prefix-min/max tests now pass context admission but reject a canonical double
exclusive-prefix operation. Consider this dynamic scalar reproduction:

```slang
[CUDAKernel]
void computeMain(uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> data)
{
    uint lane = cudaThreadIdx().x;
    uint mask = uint(data[0]);
    if ((mask & (1u << lane)) != 0)
    {
        uint64_t bits = uint64_t(uint(data[32 + 2 * lane])) | (uint64_t(uint(data[33 + 2 * lane])) << 32);
        double result = WaveMultiPrefixExclusiveMin(bit_cast<double>(bits), uint4(mask, 0, 0, 0));
        uint64_t resultBits = bit_cast<uint64_t>(result);
        data[96 + 2 * lane] = int(uint(resultBits));
        data[97 + 2 * lane] = int(uint(resultBits >> 32));
    }
}
```

The source is valid and the helper spelling is intentional. Before extending direct support, we
need to preserve which raw floating-point operand the CUDA source algorithm selects, including
NaN payloads and signed zeros. An identity change alone cannot establish that behavior.

## Proposed solution

Keep this slice research-only. Exercise CUDA source prefixes against an independent integer
binary64 oracle, preserving exact input and output words. Establish both source algorithm branches
and retain direct O0/O3 rejection evidence for inclusive/exclusive min/max. Use the finding as the
contract for a later bounded compiler change.

## Change summary

- Raw research under `build/nvvm-loop/slice-226-prefix` adds dynamic scalar/double2/double4 probes,
  a CUDA driver launcher and an integer-only expected-value model.
- `semantic-evidence.slice-226.json` records executions, exact diagnostics, artifact references and
  unchanged full 225 source/input/binary identities.
- This completed plan/report and STATUS capture the implementation handoff. No compiler, prelude,
  provider, test harness or registered workload changes are made.

## Concepts and vocabulary

The _transmitted value_ is the inclusive partial result sent to another lane at the next shuffle-up
offset. The _returned accumulator_ is seeded separately for exclusive prefixes. A _low contiguous
power-of-two mask_ contains lanes 0 through N−1, where N is 1, 2, 4, 8, 16 or 32. Other nonempty masks use
the source's ascending original-input scan. An _ordered comparison_ is false for NaN operands;
`a < b ? a : b` also selects b on equal values, including signed-zero ties.

## Process report

`hlsl.meta.slang` maps scalar/vector `WaveMultiPrefixInclusive/ExclusiveMin/Max` directly to canonical
CUDA GenericAsm helpers. `WaveOpMin/Max` supplies ordered comparison/selection. Inclusive initial
state is the caller operand; exclusive double minimum uses positive infinity and maximum uses
negative infinity. `_wavePrefixScalar` and `_wavePrefixMultiple` then select one of two algorithms.
Low contiguous power-of-two masks use shuffle-up offsets 1, 2, 4,... and track transmitted inclusive
state separately from the returned accumulator. Each participating lane combines only when its
lane index reaches the offset. Other masks scan original lane inputs in ascending order and combine
only earlier participating lanes. Singleton inclusive prefixes preserve their original payload;
singleton exclusive prefixes return the appropriate infinity.

This order is observable. With low-4 mask and distinct quiet NaNs in lanes 0 and 1, exclusive minimum
at lane 2 first reads lane 1 at offset 1, then lane 0 at offset 2. Its result is lane 0's payload
`0x7ff8000112340001`. A plain ascending scan returns lane 1's `0xfff8000212340002` instead. The recorded
countermodel differs from observed source results in 8,658 binary64 words across 32 cases. It is an
explicitly rejected implementation model, not an adjusted correctness oracle.

The Python oracle classifies binary64 words and compares their sign/magnitude ordering using integer
operations. It does not perform host floating-point arithmetic or use a second shader operation as
its oracle. Hand-derived assertions cover signed-zero ties, NaNs, singleton seeds and small tree/
scan cases. Runtime inputs cover 14 masks and 14 families: finite/infinite values, signed zeros,
quiet/signaling and mixed NaNs, NaNs at first/middle/last participating lanes, adjacent finite values
and subnormals. Payloads vary in both 32-bit halves. Four operations each return scalar, double2 and
double4 values; inactive lanes retain sentinels and input words are checked unchanged after launch.

All 196 executions match exactly: 57,624 active binary64 results and 117,992 inactive sentinels,
175,616 total binary64 comparisons (351,232 u32 words). One source PTX artifact assembles successfully;
the 196 launches reuse it with dynamic inputs. All eight independent direct scalar probes reject
before PTX with the canonical double GenericAsm signature. No direct FP64 prefix execution or
performance claim follows from this source-only experiment.

The initial matrix probe exposed a separate capability declaration: matrix prefix overloads require
`glsl_spirv` at `hlsl.meta.slang:19082`, rejecting CUDA with E36100/E36107 before emission. The initial
source/log and a return-code recheck are retained. The executable experiment was narrowed to scalar
and vectors; matrix capability repair remains separate. No registered source or oracle was changed.

The producer shape is canonical. `_getNVVMMaskedWaveScalarIdentity` deliberately rejects double
min/max prefixes, and `_initializeNVVMMaskedWaveScalarOperation` currently restricts source-order
min/max recipes to reductions. The next slice should implement the established prefix state/order
at that recipe/emission boundary, reusing typed compare/select and mask classification. It must
preserve existing reduction/arithmetic recipes and validate before/after output. Simply adding
FP64 identities or using numeric min/max would not establish this contract. Stop at the next
independent unsupported operation in the original workloads rather than folding it into the fix.

GPU smoke passes 4/4. All 26 recorded source hashes, 12 artifact hashes and 551 runtime input hashes
match accepted full 225. Its 1,665 cells/1,614 correct/51 known failures and six resolved histories
are inherited; units/toolkit/material/full corpora are not claimed freshly executed. Latest full 225
and cadence 0 remain unchanged. Compiler hash is
`14e80c03ff1571a2248f935cc795a9465ec963f9d3ac5cf4a7f80ba076a48ae1`, provider ABI 36 unchanged.
Fresh-context delegation again hits the agent-thread limit; local parent review uses WORKFLOW's
fallback and does not imply independent worker review. No GPU loss, system change or push occurred.
