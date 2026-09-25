# Extend masked integer MIN/MAX to 64 bits

## Motivation

Four frozen prefix cells reached signed 64-bit exclusive MIN/MAX after slice 229. Research 230
proved that both signednesses have exact integer member-set semantics and that the existing
provider already transports, compares and selects 64-bit payloads correctly. Consider this kernel:

```slang
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
{
    uint lane = cudaThreadIdx().x;
    uint mask = data[0];
    if ((mask & (1u << lane)) != 0)
    {
        uint64_t raw = uint64_t(data[64 + 2 * lane]) |
                       (uint64_t(data[65 + 2 * lane]) << 32);
        int64_t value = int64_t(raw);
        int64_t result = WaveMultiPrefixExclusiveMin(value, uint4(mask, 0, 0, 0));
        data[128 + 2 * lane] = uint(uint64_t(result));
        data[129 + 2 * lane] = uint(uint64_t(result) >> 32);
    }
}
```

With mask 3 and raw values `8000000000000000` and `7fffffffffffffff` at lanes 0 and 1, signed
exclusive MIN must return `7fffffffffffffff` and `8000000000000000`. Unsigned exclusive MIN instead
starts with `ffffffffffffffff`. The direct recipe rejected the canonical 64-bit helper before
emission, even though the source backend and underlying typed provider operations already worked.

## Proposed solution

Extend the existing masked scalar identity authority to admit width 64 for integer MIN/MAX only.
Compute unsigned maximum with an all-ones right shift, and materialize 64-bit identity arguments
with Slang::bitCast. Keep the existing narrow sign extension for widths below 64. The same scalar
recipe then serves scalar/vector prefixes and scalar/vector/matrix reductions without any new
provider operation, aggregate representation or algorithm.

## Change summary

- `source/slang/slang-emit-nvvm.cpp` changes only `_getNVVMMaskedWaveScalarIdentity` and
  `_emitNVVMMaskedWaveScalarValue`: bounded admission, width-safe unsigned extrema and bit-preserving
  signed constant arguments.
- `tests/cuda/nvvm-i64-masked-minmax.slang` adds one three-mode runtime fixture with dynamic low/high
  words and an independent word-wise oracle; discovery registers that fixture once.
- The completed plan, this report, design notes, STATUS, runtime manifest and census summaries
  record exact validation and inherited evidence. Scripts, logs, PTX and binaries remain in ignored
  `build/nvvm-loop/slice-231-before` and `slice-231-after`.

## Concepts and vocabulary

A _member set_ contains the selected lanes contributing to one reduction or prefix. An _identity_
is the initial extremum, observable when an exclusive prefix has no predecessors. _Raw identity
bits_ preserve the complete payload independently of the host argument's signed type. An
_aggregate leaf_ is one scalar component of the canonical vector or array-of-vector matrix shape.
A _recipe_ validates a helper signature and records typed operations for the provider.

## Process report

`hlsl.meta.slang` produces scalar and vector MIN/MAX prefix overloads with the original value type
and uint4 mask. The example becomes GenericAsm `_wavePrefixExclusiveMin(($1).x, $0)` with signature
`int64_t(int64_t, vector<uint,4>)`. Reduction overloads also include matrices; checked matrix data
lowers canonically to an array of vectors and uses an out parameter. Existing aggregate traversal
calls `_initializeNVVMMaskedWaveScalarOperation` for the scalar leaf. That calls
`_getNVVMMaskedWaveScalarIdentity`, which owns operation/width admission and identity bits. The
input is an intentional canonical shape. There is no alternative spelling to canonicalize and no
syntax to reconstruct from a checked value.

The old admission guard excluded integer width 64. Extending only the MIN/MAX condition is
principled because integer min/max selects an existing representable operand: scan and tree order
cannot change the mathematical extremum. Arithmetic, bitwise and FP16 contracts are separate and
remain excluded. Scalar and aggregate helper recognition, loop structure, lane transport, typed
MIN/MAX/SELECT, provider ABI 36 and matrix-prefix capability are unchanged.

Unsigned MIN needs `(2^width)-1`, but shifting a 64-bit one by 64 is invalid in C++. The new expression
`~uint64_t(0) >> (64 - type.bitWidth)` computes the same 8/16/32-bit masks and full-width all-ones
with a zero shift at 64. Signed extrema still shift by `width-1`, which is at most 63. Existing
floating identities retain their exact literal bits and branches.

`_emitNVVMMaskedWaveScalarValue` passes a signed integer argument to the existing provider constant
API, even for unsigned semantic types. Its `_getIntegerConstant` validates `llvm::isIntN` and
constructs `llvm::ConstantInt::getSigned`. For example, UInt8 all-ones must be passed as -1 and the
provider retains its low eight bits. The old code converted raw bits to int64_t then subtracted
`1 << width` when the sign bit was set. At width 64, that would combine an out-of-range signed cast
and an invalid shift. Slang::bitCast preserves the complete 64-bit payload as the signed argument.
Only narrower values need the existing subtraction, now guarded by `width < 64`. No fallback or
new helper is introduced; raw recipe identity bits stay authoritative.

The helper/special-case inventory therefore has two existing functions, one extended operation
admission condition and one necessary argument-width boundary. All survive at the recipe and
constant API boundary. No substitution, lookup, lowering, equivalence relation, provider repair
or second representation is added. Removing admission restores E52017 in both direct modes on
the final fixture. Singleton masks force every signed/unsigned identity to be observed; dynamic
low/high words force transport and comparison to preserve the high bit and independently changing
halves.

The registered fixture computes its expectations without 64-bit min/max or another wave. It
compares pairs of uint words lexicographically, flipping the high-word sign bit for signed ordering.
It walks the mathematical member set for reductions, inclusive and exclusive prefixes. Results
are split back into two words for comparison. Twenty boundary payloads are rotated across every
lane/component, five mask partitions include full, split contiguous, alternating and singleton
sets, and both signednesses exercise scalar/vector2/vector4 plus matrix2x2 reduction leaves.

Three fixture-only preparation failures are retained separately: comment reflow broke line-oriented
test directives; generic arithmetic types did not declare the assumed conversion; narrowing the
generic constraint alone did not provide a scalar cast. The final fixture uses IInteger.toUInt64
and its declared int64 constructor with normal readable Slang formatting. It then passed NVRTC and
rejected direct O0/O3 on the untouched accepted compiler, whose hashes match full checkpoint 229.
The fixture has not changed since that before proof. No compiler implementation attempt failed.

Final validation passes the focused fixture in all three modes, smoke 4/4, units 479/479 with the
existing Windows skip, toolkit 18/18, discovery contracts 6/6 and material compile/assembly 6/6.
The exact unchanged research 230 source and oracle replay now executes 6,384 family and 6,384 typed
control launches, totaling 12,768 and 28,600,320 output words. Every inherited source, input,
expectation and output hash matches; independently reconstructed expectations match every new direct
output. All 12 runtime PTX artifacts assemble and all 80 minimal direct family probes compile.
The 92 excluded-operation and 14 ordinary aggregate-shuffle cells retain exact rejection diagnostics;
24 matrix-prefix cells retain E36100/E36107 without PTX.

The bounded regression domain is all 107 selected frozen wave/quad/double/helper/vector/matrix
identities plus all 106 discovery identities, in three modes. That is 639 fresh cells, with 1,035
other frozen cells explicitly inherited from full 229. The cumulative inventory has 1,674 cells,
1,623 correct and 51 known failures. All 1,620 previous correct outcomes are preserved (600 fresh,
1,020 inherited); the new fixture adds three correct cells. Every old outcome is compared across
classification, return code, complete execution counts, diagnostic and canonical_shape. There are
no missing, extra or duplicate cells and no unexpected transitions.

Exactly four original prefix diagnostics advance from int64_t to the next independent FP16 boundary:
`_wavePrefixExclusiveMin/Max(($1).x, $0)`, signature `half(half, vector<uint,4>)`, E52017 at O0 and O3.
Their other four fields remain unchanged. These are not resolved runtime cells; all 51 first-known
failure records and six resolved histories remain, with the old prefix observation appended to each
diagnostic history. One audit assertion initially expected canonical_shape to change too; correcting
that assumption did not modify or repeat the corpus evidence. No FP16 implementation begins here.

Both source_commit and source_revision record tested base
`51adf2c8c6ab9c61de4e87dfcc187ffca865d126` plus the recorded patch. Every accepted source identity,
the new fixture, all 12 artifact identities and all 554 current input hashes were checked after the
final gates. All 553 old input hashes and every old discovery row are unchanged. Only the emitter
and discovery registration change among the 28 accepted source identities; provider ABI 36 and its
binary hash remain unchanged. Fixture preparation errors are separate from the final before proof,
and the final source/fixture/library identity is recorded for each gate.

Independent parent acceptance passed. Slice 231 is the latest implementation with one
implementation slice since full checkpoint 229; historical denominators 427/72 remain fixed. Material
runtime still lacks the application's binding, texture/LUT/input and expected-output contract, so
its six successful compile/assembly cells do not establish runtime correctness or performance.

Parent review verified 525 unique evidence references, all 29 source and 12 artifact hashes, all
554 current inputs, exact fresh/inherited census outcomes and complete first-known failure histories.
All 12,768 replay launches preserve the accepted mathematical input/expectation hashes.
