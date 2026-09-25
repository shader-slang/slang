# Establish 64-bit integer masked min/max semantics

## Motivation

Slice 229's four frozen prefix failures now reach signed 64-bit exclusive MIN/MAX. Consider this
complete reduced kernel, with each dynamic input encoded as low/high words:

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

With mask 3, lane 0 holding raw `8000000000000000` and lane 1 holding
`7fffffffffffffff`, signed exclusive MIN returns `7fffffffffffffff` and `8000000000000000`.
For unsigned 64-bit values, the first identity is `ffffffffffffffff`; the second result retains the predecessor's
high-bit value. Inclusive MIN at lane 1 instead returns signed minimum or unsigned `7fffffffffffffff`.
This distinction needs exact 64-bit comparison, transport and empty-prefix identities.

## Proposed solution

This research slice establishes the entire coherent integer MIN/MAX contract before changing code.
The source/oracle gate passes for signed/unsigned 64-bit scalar/vector2/vector4 reductions and all four
prefixes, plus existing matrix2x2 reduction leaves. Independent typed controls also establish that
64-bit lane transport, MIN/MAX, SELECT and constants already work through the current provider.

The next bounded implementation should extend only 64-bit integer MIN/MAX identity admission in
`_getNVVMMaskedWaveScalarIdentity` and make identity construction safe at width 64 in
`_emitNVVMMaskedWaveScalarValue`. Reuse the existing scalar scan and aggregate leaf recipes.
There is no evidence requiring a provider, ABI, frontend or aggregate classifier change. Keep
arithmetic, bitwise operations, matrix prefixes and aggregate shuffle policy separate.

## Change summary

- The completed plan, this report, STATUS and `semantic-evidence.slice-230.json` record the source
  contract, exact inventory, responsible boundaries and inherited evidence.
- The design document records the durable distinction between 64-bit integer recipe admission and
  already-supported scalar lane transport, including the width 64 identity hazards.
- Generated source, inputs, independent Python oracle, per-launch hashes, PTX/cubins, separate
  direct diagnostic/IR logs and descriptor probe remain under
  `build/nvvm-loop/slice-230-i64-minmax`.
- Compiler, provider, fixtures, corpus selection, registered outcomes and failure history remain
  unchanged. No rebuild, commit or push was performed by the worker.

## Concepts and vocabulary

A _member set_ contains the selected lanes for one reduction or prefix. An _identity_ is the
extremum used for an empty exclusive prefix. A _raw 64-bit payload_ is the exact unsigned 64-bit pattern;
its signed interpretation subtracts `2^64` when bit 63 is set. _Typed lane transport_ moves that
payload without deciding comparison signedness. A _recipe_ admits a canonical helper signature
and lists existing typed provider operations to implement it.

## Process report

`hlsl.meta.slang` generates the scalar and vector prefix overloads from
`kWaveMultiPrefixMinMaxNames`, preserving `T` and the uint4 mask. The CUDA branches produce exact
GenericAsm `_wavePrefixExclusiveMin/Max(($1).x, $0)` or the corresponding inclusive/`Multiple`
spelling. `WaveMultiMin/Max` also has an existing CUDA matrix overload. Its checked matrix shape
becomes `Array<vector<T,2>,2>` with an out parameter. This is canonical aggregate lowering, not
malformed producer data. Matrix prefixes instead lack CUDA in their declared capability set;
24 fresh source/direct cells retain E36100/E36107 before recipe admission.

The CUDA prelude defines NVRTC `int64_t`/`longlong` as `long long` and unsigned counterparts as
`unsigned long long`. `WaveOpMin<T>::doOp` and `WaveOpMax<T>::doOp` compare two values of the same
64-bit type and select an original operand. There is no narrowing promotion, floating conversion
or arithmetic on the data payload. Five host static assertions check width and preservation of
64-bit types under promotion and conditional selection; emitted CUDA confirms the NVRTC aliases.
The explicit `getInitial` specializations supply signed extrema, unsigned all-ones for MIN, and
unsigned zero for MAX. Inclusive prefixes seed with the caller; reductions use the caller or a
butterfly. Repeated selection of an integer operand cannot change the mathematical extremum.
`ElementTypeTrait` applies this exact scalar operation to vector and matrix leaves.

The Python oracle does not simulate either shuffle algorithm. It constructs mathematical integer
values from raw words, forms the appropriate member set, then calls Python `min`/`max`. Reductions
use all selected lanes, inclusive prefixes include the caller, and exclusive prefixes use only
selected predecessors. Empty exclusive sets use the exact type identity. Results are packed back
into low/high words only after computing the mathematical answer. The kernels accept dynamic raw
words and write results. The host compares every active result word and inactive `deadbeef` sentinel
against its independent oracle, and verifies that all 320 input and header words are unchanged.
Each launch records source, PTX, input, expectation and output hashes.

The 14 masks are full, low2/4/8/16, low15, high16/high17, even/odd, sparse extremes, and singletons
0/7/31. Each type/mask uses 76 patterns: 20 constant boundaries, 20 permutations, 20 repeated-value
patterns and 16 independently varying half-word patterns. Boundaries include both signed extrema,
unsigned maximum/high bit, zero/one/minus-one, both sides of 32-bit boundaries and mixed half words.
This is deliberately broad finite coverage, not exhaustive 64-bit values or tuples.

| Fresh check                                                         |      Requested | Result                |
| ------------------------------------------------------------------- | -------------: | --------------------- |
| MIN/MAX family, NVRTC O3                                            | 2,128 launches | All exact             |
| Typed 64-bit read/MIN/MAX/SELECT/constants, NVRTC O3 and NVVM O0/O3 | 6,384 launches | All exact             |
| Minimal family direct NVVM O0/O3                                    |    80 compiles | E52017, no PTX        |
| Matrix prefix capability, all three modes                           |    24 compiles | E36100/E36107, no PTX |
| Typed catalog descriptors                                           |              8 | All admitted          |
| Research PTX assembly                                               |    8 artifacts | All pass              |
| GPU smoke                                                           |              4 | All pass              |

The runtime inventory has exactly 8,512 unique launches with no missing or extra cells. They compare
14,981,120 output words: 4,915,680 active words and 10,065,440 inactive sentinels. The family kernel
contains 50 result components per caller: four prefix operations over 1+2+4 components, plus two
reductions over 1+2+4+4 matrix components. Each signedness runs 1,064 source launches. The separate
control kernel uses four scalar lane reads, four MIN/MAX/SELECT comparisons each, and four constants;
each type/mode runs the same 1,064 patterns. Lane selectors always refer to a member and vary by
caller, exercising both halves together. No experiment failed; there are no discarded initial GPU
attempts hidden by these results. Existing narrow/32-bit controls inherit unchanged slice 229 evidence.

The 80 direct cells cover both signednesses, four prefixes over scalar/vector2/vector4 and both
reductions over scalar/vector2/vector4/matrix2x2 at O0/O3. Every diagnostic retains the exact
GenericAsm shape and no PTX. Separate signed/unsigned exclusive-MIN IR dumps preserve canonical
checked input without replacing diagnostic-only logs. `_resolveNVVMMaskedWaveScalarOperation` and
`_resolveNVVMAggregateWaveOperation` reach `_initializeNVVMMaskedWaveScalarOperation`; its
`_getNVVMMaskedWaveScalarIdentity` call rejects 64-bit integer before typed operation admission. The
rejection therefore belongs to the emitter's bounded recipe policy, not the frontend producer.

The semantic catalog explicitly admits signed/unsigned I64 `WAVE_READ_LANE_AT`; its integer
MIN/MAX family uses selected scalar integers, and SELECT preserves the same type. The provider's
`_emitWaveReadLaneAt` extracts low/high 32-bit words, executes two indexed shuffles with the same
mask, source lane and clamp 31, then recombines the words into i64. Integer MIN/MAX uses signed or
unsigned LLVM comparison predicates and selects an unchanged operand. Direct PTX contains the
corresponding 64-bit integer min/max instructions. The catalog's eight descriptor checks and all
three-mode typed GPU controls agree. Ordinary aggregate shuffle admission remains separate.

Constant materialization has a distinct contract. Provider `_getIntegerConstant` takes `int64_t`,
validates that signed value at the destination width, and calls LLVM `ConstantInt::getSigned`.
Canonical UInt64 literals already retain high-bit payloads in signed `IRIntegerValue`, as documented
by `_asExecutableSelectedIntegerConstant`. Control kernels verify zero, signed maximum, sign-bit
minimum and all-ones constants at full raw width in all three modes. The provider needs no new API.
However, merely widening the recipe guard would be wrong: unsigned `(1ULL << width)-1` shifts by 64,
and slice 229's signed-identity subtraction uses `1LL << width`, also invalid at width 64. Construct
extrema without shifting by the type width and preserve signed argument bits using the existing
`Slang::bitCast` in `core/slang-common.h` where appropriate. Do not duplicate canonical type/value
representations or widen unrelated operations.

The self-review inventory contains no new production helper, fallback or special case. Research
helpers generate explicit fixtures, drive CUDA, and compute independent expectations. They are not
proposed production architecture. The only proposed special handling is the legitimate width 64
constant boundary: the source type is valid, raw identity bits are the semantic source of truth,
and singleton exclusive probes force the identity to be observable. The next implementation must
replay these exact inputs/expectations through direct O0/O3 and retain the before rejection proof.

This candidate ranks first because four current frozen cells reach its exact shape and the same
bounded algebra serves scalar and aggregate reductions/prefixes. Arithmetic/bitwise/FP16 families
and matrix-prefix capability require separate contracts. FP64 vector-by-value shuffles, quad
reconvergence and resource contexts remain independent; none was investigated here. Material
runtime remains blocked by its missing binding/texture/LUT/input/output contract, so the six existing
material compile/assembly cells are inherited without runtime or performance claims.

Tested source is accepted commit `ef0cd6bf92bced6c52245337f3abc500644ae6a9`, rather than the historical
pre-229 source revision inside the inherited result file. Before/after hashes match all 28 accepted
sources, 12 artifacts and 553 runtime inputs; submodule pins also match. Compiler library remains
`577c8eea1039a9b6090db4bfd993bc143f53bb78d5c75f9d4d26b4753e8a11af`; provider remains
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI 36.
All 1,671 registered cells, 1,620 correct outcomes, 51 unresolved failures, six resolved histories,
units/toolkit/contracts and material evidence inherit slice 229 unchanged. Frozen 452/discovery 105,
latest full 229 and implementation cadence zero remain unchanged. Research does not refresh those
registered cells. Parent acceptance verified 225 unique evidence references and independently
reconstructed all 8,512 saved inputs and mathematical expectations, matching every recorded output.
