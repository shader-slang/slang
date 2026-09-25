# Admit FP16 masked MIN/MAX with finite source seeds

## Motivation

Four original frozen direct prefix cells reject valid half MIN/MAX helpers after the previously
supported floating and integer types. Research 232 establishes their raw binary16 source contract.
Consider this dynamic source:

```slang
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> data)
{
    uint lane = cudaThreadIdx().x;
    uint mask = data[0];
    if ((mask & (1u << lane)) != 0)
    {
        half value = bit_cast<half>(uint16_t(data[64 + lane]));
        half result = WaveMultiPrefixExclusiveMin(value, uint4(mask, 0, 0, 0));
        data[96 + lane] = uint(bit_cast<uint16_t>(result));
    }
}
```

With mask 3 and positive infinity in both inputs, the source returns raw `0x7bff` (+65504) to both
callers. Negative infinities with exclusive MAX similarly return `0xfbff` (-65504). These finite
source seeds differ from FP64 infinity seeds and are not neutral extrema over all floating values.

## Proposed solution

Admit half MIN/MAX through the existing typed source comparison/select recipe and its finite
exclusive seeds. Reuse the source-order reduction/prefix algorithms and separate transmitted state
introduced in earlier floating slices. Existing scalar recipes also serve canonical homogeneous
aggregate leaves. No provider operation, ABI change or new representation is needed.

## Change summary

- `source/slang/slang-emit-nvvm.cpp`: extend only `_getNVVMMaskedWaveScalarIdentity` and
  `_initializeNVVMMaskedWaveScalarOperation`; preserve one seed authority and existing recipe flags.
- `tests/cuda/nvvm-fp16-masked-minmax.slang`: dynamic raw16 fixture with closed-form expected operands,
  scalar/vector2/vector4 reductions and four prefixes, plus matrix2x2 reductions. Three native
  modes and one discovery registration preserve its inputs and output oracle.
- Completed plan, this report, design, STATUS and slice233 manifests retain exact fresh/inherited
  acceptance, source/binary identities, unresolved and resolved histories. Ignored raw artifacts
  stay under `build/nvvm-loop/slice-233-before` and `slice-233-after`.

## Concepts and vocabulary

A _source seed_ initializes the returned exclusive accumulator. It need not be an algebraic
identity. _Transmitted state_ is the caller-seeded inclusive partial value read by subsequent
prefix shuffles. _Ordered compare/select_ returns the second operand on ties or unordered NaN
comparisons, retaining its raw payload. A _low contiguous power-of-two mask_ names lanes 0 through
N-1 for N=1,2,4,8,16,32; it selects the source tree recipe rather than an original-input scan.

## Process report

`hlsl.meta.slang` specializes the six masked MIN/MAX operations into canonical scalar or Multiple
GenericAsm helpers. `_resolveNVVMMaskedWaveScalarOperation` and aggregate leaf resolution validate
the checked signatures. Matrix reduction leaves arrive as an admitted `Array<vector<half,2>,2>`
with an out parameter. This is intentional semantic data; there is no alternative AST/IR spelling
or malformed upstream representation to repair. The missing policy is at the emitter recipe
admission boundary.

The complete production helper/guard/special-case inventory has two retained entries and no new
helper or fallback. First, `_getNVVMMaskedWaveScalarIdentity` admits half only for MIN/MAX and
stores raw 7bff/fbff in the existing seed mapping. A shared local `isMinMax` predicate preserves the
existing narrow/64-bit integer classification. The function comment now describes an initial
accumulator and explains the infinity example. `_emitNVVMMaskedWaveScalarValue` already materializes
this semantic-width floating constant; no host conversion or secondary seed mapping is added.
The fixture's infinity and singleton checks distinguish the finite contract from infinity seeds.

Second, `_initializeNVVMMaskedWaveScalarOperation` includes half in `usesSourceMinMax` for reductions
and prefixes. The existing derived prefix flag selects strict earlier-lane combination, ascending
shuffle-up offsets and separate caller-seeded transmitted state. Low contiguous power-of-two
reductions retain descending XOR stages; other masks scan original inputs in ascending order.
No tree/scan code, phi construction, aggregate classifier or operation requirement closure changes.
Numeric half MIN/MAX provider descriptors stay excluded: the recipe requests ordered comparisons
and SELECT instead. Removing admission restores the exact unchanged fixture's E52017 failures;
changing the source algorithm would contradict accepted research counterexamples.

The registered fixture does not copy the production tree/scan algorithm. Its 25 constant families
include both infinity signs, quiet/signaling NaNs, signed zeros, extrema and normal/subnormal limits.
Two distinct signed-NaN families and alternating zeros have closed-form selected lane indices:
XOR reductions select the opposite caller, prefix trees select the first member, and scans select
the last eligible original member. Positive/negative monotonic magnitude families straddle the
normal/subnormal boundary and select first/last extrema. Full, low16/high16, low15/high17, parity
and singleton partitions exercise these identities. All raw values originate in dynamic buffers;
expected bits use integer arithmetic and no wave intrinsic or floating conversion. The final readable
fixture passed NVRTC and rejected both direct modes on the accepted library before any production
edit. Its TEST_INPUT lines, complete source and hash remain unchanged afterward; no rebuild drill
or discarded fixture attempt was needed.

The broad replay reads accepted 232 source, input and expectation binaries directly. All three
modes now execute 1,478 family launches and 1,478 control launches each: 8,868 launches and 10,783,488
output words. It preserves each 192-word input region and inactive sentinels, including signaling
NaN payloads. The 512 full-mask batches cover every raw binary16 encoding among lane/component
slots; 69 structured patterns also cover 14 masks. This is not exhaustive tuples, masks or callers.
The source/expectation contract was independently derived and audited in 232, rather than obtained
from NVRTC outputs. All 40 minimal direct family probes compile; six runtime PTX artifacts assemble.
Twelve matrix prefix capability rejects remain exact, and catalog checks retain four supported
half operations and two excluded numeric MIN/MAX descriptors.

Acceptance results and exact registered transitions are recorded in `runtime-validation.slice-233.json`.
The affected domain is 107 selected frozen wave/quad/double/helper/vector/matrix identities and all 107
discovery identities, 642 fresh cells. The remaining 1,035 frozen cells explicitly inherit full 229.
The bounded emitter policy has no shared lowering/provider/ABI/runner impact; no full-checkpoint
trigger occurred. Latest full checkpoint remains 229; targeted 233 moves implementation cadence from
one to two. Material reassessment covers six compile/assembly cells only. Application bindings,
textures/LUT/input and output oracle remain absent, so no material runtime or performance claim follows.

The four original prefix cells now execute successfully at direct O0/O3. Their complete prior
failure records, first-known observations, reproductions and diagnostic histories move into resolved
history with each exact transition and fresh runtime proof. The selected frozen suite has 319
correct cells and two unchanged quad-control preflight stops. The remaining independent boundary is
`tests/hlsl-intrinsic/quad-control/quad-control-comp-functionality.slang`, with the exact diagnostic
`direct NVVM lowering does not support Slang IR instruction or shape 'RequireMaximallyReconverges'`
at O0/O3. Its existing minimal source and reproduction remain in the ledger; no further investigation
or quad change is included.

All final gates pass: smoke 4/4 before expensive GPU suites, focused 3/3, units 479/479 plus one
existing Windows-only skip, toolkit 18/18, discovery contracts 6/6 and material compile/assembly 6/6.
The 92 excluded arithmetic signatures and 14 ordinary aggregate shuffle signatures retain exact
E52017 before/after rejections. Discovery has 291 correct cells and its unchanged 30 known gaps;
all 318 old cells preserve every comparison field. The three fixture cells are separate additions.
The cumulative ledger is 1,677 cells, 1,630 correct, 47 unresolved failures and ten resolved histories.
Of the 1,623 old correct cells, 603 are freshly preserved and 1,020 explicitly inherit full229.
All 554 old runtime input hashes are unchanged; the fixture adds input 555. Historical healthy
denominators 427/72 remain fixed. No missing/duplicate cells or unexpected transitions were found.

Both source revision fields name the actual tested base `54ec1bfe5d92f5f7361ae85a7815ae1767e7a46a`.
Every final gate captures 29 old source paths, the new fixture and 12 artifacts; final identities
match after the gates. Compiler library SHA-256 is
`92ae81d069aeda9a6ff2a61edec43f572b2af02bb7ec677fc444490ea9a966f1`; provider remains
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI 36. The native Ubuntu/L4
SM89 host, SM80 target, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14 and driver 580.126.09 are unchanged.
No GPU loss, driver/system change, reboot or push occurred. Independent parent acceptance verified
669 unique compact evidence references, all source/artifact/input hashes, complete cumulative
censuses and failure histories, and every raw replay output buffer against accepted expectations.
Slice 233 is accepted; latest full remains 229 and implementation cadence is two.
