# Admit narrow-integer masked min/max

## Motivation

The original frozen prefix minimum/maximum workloads reach valid signed 8-bit helpers that the NVVM
identity recipe rejects. Consider this dynamic example:

```slang
[CUDAKernel]
void computeMain(uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> data)
{
    uint lane = cudaThreadIdx().x;
    uint mask = uint(data[0]);
    if ((mask & (1u << lane)) != 0)
    {
        int8_t value = int8_t(data[32 + lane]);
        data[64 + lane] = int(WaveMultiPrefixExclusiveMin(value, uint4(mask, 0, 0, 0)));
        data[96 + lane] = int(WaveMultiMin(value, uint4(mask, 0, 0, 0)));
    }
}
```

With mask 3 and inputs -128 and 127, the exclusive prefix returns 127 and -128; the reduction
returns -128 at both callers. Unsigned 8-bit values instead use 255 as their minimum identity and
compare 128 as positive. Research 228 establishes independent prefix expectations. This slice also
establishes the reduction contract before admitting the shared bounded integer MIN/MAX family.

## Proposed solution

Extend `_getNVVMMaskedWaveScalarIdentity` to accept signed/unsigned 8/16-bit MIN/MAX and derive
identities from the already-validated width. Keep aggregate classification structural and put
unchanged shuffle width admission in its own resolver. Materialize identity bits as signed values at
their destination width, as required by the provider constant contract. Keep existing typed lane
transport, signed/unsigned min/max, select, scalar scan and aggregate leaf traversal. No provider or
ABI change is needed. Admission belongs to the shared integer algebra: both reductions and prefixes
select original representable integers. A separate prefix-only mode gate would split an identical
proven algebra. Arithmetic/bitwise, 64-bit integers and FP16 keep their previous admission
boundaries.

## Change summary

- `source/slang/slang-emit-nvvm.cpp` restricts additional widths to integer MIN/MAX and computes
  exact signed/unsigned extrema instead of hardcoded 32-bit masks. It separates aggregate structure
  from consumer width policies and replaces hardcoded 32-bit constant interpretation.
- `tests/cuda/nvvm-narrow-masked-minmax.slang` uses runtime inputs and independent wide-integer
  mask-set expectations for scalar/vector2/vector4 prefixes/reductions and matrix2x2 reductions.
- The discovery manifest adds that one source. Existing identities, inputs and output oracles remain.
- The plan, report, design, STATUS, result manifest and cumulative censuses retain scope, validation and
  first-known failure history. Raw executable evidence remains in ignored `build/nvvm-loop`.

## Concepts and vocabulary

An _identity_ is the initial accumulator that leaves every legal operand unchanged under the
operation; an empty exclusive prefix returns it. A _homogeneous aggregate leaf_ is a scalar reached
through the existing vector/matrix/array representation. _Typed lane transport_ moves original low
bits through a 32-bit shuffle, reconstructing the original width before comparison.

## Process report

The example specializes through `hlsl.meta.slang` to canonical `_wavePrefixExclusiveMin(($1).x, $0)`
or `_waveMin($1.x, $0)` GenericAsm. Vector/matrix reductions produce the corresponding `Multiple`
spelling. `_resolveNVVMMaskedWaveScalarOperation`, or `_resolveNVVMAggregateWaveOperation` and its
homogeneous leaf classifier, calls `_initializeNVVMMaskedWaveScalarOperation`. Its identity helper
previously rejected widths 8/16. This is valid canonical checked IR, not an accidental
representation or a producer bug. Existing semantic type descriptors remain the source of truth; no
syntax or structural match is reconstructed.

For integer MIN, the identity is `(1 << (width - 1)) - 1` when signed and `(1 << width) - 1` when
unsigned. For MAX, signed identity bits are `1 << (width - 1)` and unsigned is zero. Shifts use
uint64_t operands, and admitted integer widths are 8, 16 and 32, so no shift reaches the host width
or invokes signed-overflow behavior. Signed minimum is supplied as its original-width
two's-complement bits. Float32/64 identities keep their exact existing constants. No arithmetic or
bitwise narrow identity is admitted by the new predicate.

The existing semantic MIN/MAX descriptor selects signed/unsigned integer comparison at the original
width. Provider `_emitWaveReadLaneAt` zero-extends low bits to i32 for shuffle and truncates back;
that transport does not decide comparison signedness. `_emitNVVMMaskedWaveScalarValue` scans the
selected mask and uses existing inclusive/exclusive membership predicates. Reduction callers admit
all mask members. Scalar values and aggregate leaves use the same recipe and identity.

For reductions, CUDA `_waveReduceScalar`/`_waveReduceMultiple` uses descending XOR butterfly offsets
for low contiguous power-of-two masks, ascending original-input scans for other nonsingletons, and
returns the caller for singleton masks. `WaveOpMin/Max` selects one operand after the usual integer
promotion. For signed/unsigned 8/16 values both promoted operands are representable in int; the
selected operand remains representable at its original width. Integer min/max is associative and
idempotent, so butterfly versus scan ordering and duplicate singleton initialization cannot change
the mathematical extrema. This differs from the source-order FP64 case in slice 227.

Before production edits, scalar/vector reductions passed 1,344 source launches and 32 direct
compilations rejected E52017. The aggregate audit then identified already-valid matrix reduction
leaves: matrix `WaveMultiMin/Max` already permits CUDA, while matrix prefix capability remains
unavailable. The proof therefore includes matrix2x2 reductions and explicit wide-input patterns.
Reversing the exact production patch and rebuilding the original emitter proves that the unchanged
final fixture passes source mode and rejects both direct modes. The final reduction prototype passes
1,568 source launches against independent mask-set extrema, while 32 minimal direct probes reject.
The implementation patch is restored and matching optimized tools are rebuilt afterward. The first
fixture's generic `int(T)` constraint error is retained separately; final expectations compare
results to representable `T(expected)` values, while the oracle itself decodes low bits using wide
integer arithmetic. Research output checks also verify exact sign/zero extension into 32-bit words.

The identity-only candidate exposed a second gate in `_getNVVMHomogeneousWaveAggregateLeafType`,
which rejected canonical narrow vectors before scalar recipe admission. That helper serves both
masked operations and shuffles. Its numeric leaf classification now describes structure; masked
width policy stays in the scalar identity recipe, and the unchanged 32-bit/FP64 shuffle policy moves
into the shuffle resolver. This avoids an operation-dependent structural classifier or a second
MIN/MAX width list. Eight narrow vector shuffle probes reject before and after; arithmetic/bitwise,
64-bit integer and FP16 exclusion probes remain negative. The shared classifier change triggers a
full checkpoint rather than the initially planned selected frozen run.

After aggregate admission, the next focused run reaches `_emitNVVMMaskedWaveScalarValue` and fails
E52018 while constructing an integer identity. It cast identity bits through int32_t, so UInt8
identity 255 remained positive 255 and signed Int8 minimum bits 128 remained positive 128. Provider
`_getIntegerConstant` intentionally requires a signed value fitting its LLVM destination width
(`llvm::isIntN`) before `ConstantInt::getSigned`. Its rejection is correct. The existing 32-bit cast
is replaced with destination-width signed interpretation: subtract `2^width` when the sign bit is
set. The admitted integer widths are at most 32, so every conversion/subtraction fits int64_t and
every shift is defined. This follows the existing selected-integer-literal convention without
changing the provider or introducing a second representation. Both intermediate failure logs are
retained. The complete final emitter patch is reversed, and the original emitter is rebuilt to
repeat the unchanged final fixture proof. All changes are restored and rebuilt before final
validation.

The helper/fallback inventory contains no new helper or fallback. The new narrow admission predicate
survives because canonical 8/16-bit MIN/MAX is now proven by source execution and independent
mathematical expectations. Replacing it with an assertion would reject valid advertised overloads.
The original-emitter revert drill proves this consumer boundary owns the rejection. Width-derived
extrema replace existing constants in the same identity authority; no competing mapping is added.
Existing matrix reduction aggregation is tested without changing any matrix capability rule. The
modified structural classifier and moved shuffle guard both survive: structure belongs to the
classifier; operation support belongs to its consumer. The width-aware provider argument replaces an
invalid hardcoded conversion at its producer, without masking malformed input.

The final full checkpoint records 1,671 fresh cells: 1,620 correct and 51 known failures. All 1,617
previous correct cells survive, and the new fixture adds three correct cells. Frozen remains 452
identities with 1,335 correct, 16 preflight and five infrastructure cells. Discovery has 105
identities with 285 correct, 22 infrastructure, four mismatch and four preflight cells. Exactly four
old diagnostics change; classification, return code, execution counts and canonical shape remain
unchanged everywhere. All 552 previous input hashes and 104 previous discovery rows remain intact.
There are no missing or duplicate cells. All 51 first-known failure records and six resolved
histories are preserved, with the prior prefix diagnostics retained in their full history.

The original minimum/maximum prefix cells now stop at E52017 for
`_wavePrefixExclusiveMin(($1).x, $0)` and `_wavePrefixExclusiveMax(($1).x, $0)`, both with canonical
`int64_t(int64_t, vector<uint,4>)` signatures at O0/O3. These remain unsupported cells, not fixes.
The slice stops at that independent boundary. The reduced fixture and independent runtime replay
establish narrow support without claiming the original mixed-width workloads are fully supported.

| Validation                                      | Result                                                             |
| ----------------------------------------------- | ------------------------------------------------------------------ |
| Final registered fixture                        | 3/3; original compiler passes source and rejects both direct modes |
| GPU smoke                                       | 4/4                                                                |
| Relevant units                                  | 479/479 plus one existing Windows-only skip                        |
| Toolkit/assembly                                | 18/18                                                              |
| Discovery runner contracts                      | 6/6                                                                |
| Excluded operation/width and shuffle signatures | 88 unchanged before/after rejections                               |
| Full frozen/discovery                           | 1,671 fresh cells, 1,620 correct, 51 retained failures             |
| Material compile/assembly                       | 6/6; no runtime claim                                              |

Research replay is separate from registered cells: 47,712 primary prefix launches, 672 wide-input
prefix launches and 4,704 reduction launches all match independent expectations. Together these
53,088 launches check 46,663,680 output words, including inactive sentinels, and verify unchanged
input buffers. Every research 228 source/input/expectation is preserved, including all accepted
32-bit control executions. The final reduction replay uses unchanged before sources, inputs and
expectations. All 18 prefix and 12 reduction PTX artifacts assemble. All 64 minimal prefix and 32
minimal reduction direct compilations now succeed. The final fixture's original-emitter drill uses
the exact accepted 227 compiler-library hash, and its source bytes remain unchanged afterward.

The shared classifier change triggered the full checkpoint; no runtime cells are inherited in
this acceptance. Full 225 plus accepted 227 remains the preservation baseline. Successful full 229
resets implementation cadence to zero following independent parent acceptance. Frozen and discovery each return 2
for retained known failures; exact structured results determine acceptance. The full result manifest
and census files retain every per-cell outcome and evidence reference.

The final compiler-library SHA-256 is
`577c8eea1039a9b6090db4bfd993bc143f53bb78d5c75f9d4d26b4753e8a11af`; provider remains
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI 36. Source base is
`16207a0c3781d8e9158f205aee2e48e68cc11c3a` plus the recorded emitter, fixture and discovery changes.
All 28 source, 12 artifact and 553 current runtime-input hashes match after the gates. Raw evidence
is under `build/nvvm-loop/slice-229-before` and `slice-229-after`; no commit or push was made by the
worker. No material runtime or performance claim is made without its application contract.

Parent acceptance independently verifies 192 unique evidence references, all 28 source and 12
artifact hashes, all 553 current inputs, the 1,671-cell comparison and every semantic replay launch.
All old failure histories are preserved. The inherited provenance revision label was corrected to
the actual tested base recorded above; source and binary identities were unchanged.
