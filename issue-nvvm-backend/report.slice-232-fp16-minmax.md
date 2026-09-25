# Establish FP16 masked MIN/MAX source semantics

## Motivation

Four original frozen direct prefix cells now reach canonical FP16 helpers and stop with E52017.
Consider this valid dynamic source:

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

For mask 3 and positive infinity in both lanes, the source returns `0x7bff` (+65504) in both
lanes. The FP16 exclusive source seed is finite. Applying the FP64 infinity seed would change
observable results; the integer member-set extremum oracle also cannot establish floating behavior.

## Proposed solution

Keep this slice research-only. A later bounded emitter extension can admit FP16 MIN/MAX through
the existing ordered comparison/select tree and scan recipes, preserving the source's finite
exclusive initial accumulators. Reuse the typed half lane-read, comparison, SELECT and floating
constant provider contracts. No provider, ABI, frontend or aggregate classifier change is indicated.
Do not admit numeric half MIN/MAX descriptors, other arithmetic families or matrix prefixes.

## Change summary

- This completed plan, report, `semantic-evidence.slice-232.json` and STATUS record the contract,
  exact inventories, responsible boundaries and inherited acceptance evidence.
- The design document records the finite FP16 seeds and existing raw half transport contract.
- Ignored `build/nvvm-loop/slice-232-fp16-minmax` retains source/driver/oracle scripts, every launch's
  input/expectation/output binary, source/PTX/cubins, diagnostic logs, separate IR dump and catalog
  checks. A hash-addressed manifest covers all 17,736 launch binaries.
- Production, provider, fixtures, corpora and old research helpers remain unchanged. No rebuild,
  local commit or push was performed by this worker.

## Concepts and vocabulary

A _raw binary16 encoding_ is a 16-bit sign/exponent/fraction pattern. An _ordered comparison_ is false
for NaNs; the source's `a < b ? a : b` and `a > b ? a : b` choose b on ties or unordered inputs.
The _exclusive seed_ initializes the returned accumulator; it need not be a neutral mathematical
identity over all floating values. The _transmitted state_ is the caller-seeded inclusive partial
value used by later prefix shuffles. _Numeric min/max_ has a different NaN/tie contract from these
WaveOp comparisons and must not stand in for them.

## Process report

`hlsl.meta.slang` produces exact scalar/vector GenericAsm helpers for the four prefixes and two
reductions. Matrix reduction overloads are also valid CUDA source: their canonical lowering is
`Array<vector<half,2>,2>` with an out parameter. The generated CUDA loads each dynamic ushort through
`slang_bit_cast<__half>`, constructs `__half2`, `__half4` or `Matrix<__half,2,2>`, and calls the scalar
or Multiple prelude helper. This is an intentional checked shape, not malformed upstream data.
Matrix prefixes still lack CUDA capability, before emitter admission.

In `slang-cuda-prelude.h`, `WaveOpMin<__half>::getInitial` uses raw `0x7bff` and
`WaveOpMax<__half>::getInitial` uses `0xfbff` only for exclusive prefixes. Inclusive prefixes and
reductions begin with the caller value. Consequently a singleton inclusive/reduction preserves
its original NaN payload, whereas a singleton exclusive result is the finite seed. With mask 3
and negative infinity in both lanes, exclusive MAX returns `0xfbff` in both lanes. Both infinity
examples have independent hand assertions and actual source runtime cases.

CUDA 12.9's `cuda_fp16.hpp` resolves `__half` operators to `__hlt`/`__hgt`, implemented for SM80 as
`setp.lt.f16`/`setp.gt.f16`. The conditional expression selects one of two existing half values.
Half shuffle overloads use `__halves2half2`, a raw `mov.b32` packing operation, shuffle that word,
then extract the low half. There is no float promotion, arithmetic rounding or NaN quieting on this
source data path. Actual source PTX contains half comparisons and bit selections and no
half-to-float or float-to-half conversions. All payload checks agree, including both signs of
signaling NaNs. This finding is scoped to these operations and this recorded target/toolkit.

For low contiguous power-of-two masks, `_waveReduceScalar/Multiple` runs simultaneous XOR stages
from population/2 down to 1. Other multi-lane masks scan original lane values in ascending order,
starting from the caller. `_wavePrefixScalar/Multiple` uses shuffle-up offsets 1, 2, 4 and so on
for the same low-mask class. The transmitted state starts from the caller independently of the
returned exclusive seed; both update from the same previous-stage predecessor. Other masks scan
original values and combine only predecessors strictly earlier than the caller. Singleton masks
perform no comparison. ElementTypeTrait applies the same scalar behavior independently to all leaves.

The oracle is independently derived from binary16 fields. It classifies NaNs when the magnitude
exceeds `0x7c00`, treats signed zeros as equal, compares other encodings with a sign-aware integer
ordering key, and selects an unchanged raw operand. It performs no host floating arithmetic and
does not use source/GPU output as expected data. Hand assertions cover signed subnormals, signed
zeros, unordered operand selection, both finite-seed infinity examples, and the low4 exclusive
NaN case: inputs `0x7d01`, `0xfe02`, 0, 0 produce lane2 `0x7d01`, since the last read is lane0.
A simple ascending scan instead selects `0xfe02` there. A global ascending-scan countermodel differs
in 197 launches and 102,116 output words; an infinity-seed countermodel differs in all 1,478 family
launches and 22,792 words. These rejected models are preserved separately from expected results.

The source kernel writes 50 components per caller: four prefixes over scalar/vector2/vector4,
and two reductions over scalar/vector2/vector4/matrix2x2. The controls write 26 components: four
each of raw identity, varying valid-member lane read, ordered MIN selection, ordered MAX selection,
and lane-conditioned SELECT, plus six floating constants (positive/negative finite extrema,
positive/negative zero, one, minimum normal). The controls use independently derived raw expectations.
Every launch verifies the unchanged 192-word header/input region and all inactive `deadbeef` words.
No existing-operation correctness failure was found.

| Fresh check                               |      Requested | Result                               |
| ----------------------------------------- | -------------: | ------------------------------------ |
| FP16 family, NVRTC O3                     | 1,478 launches | All exact                            |
| Existing half controls, NVRTC O3          | 1,478 launches | All exact                            |
| Existing half controls, NVVM O0           | 1,478 launches | All exact                            |
| Existing half controls, NVVM O3           | 1,478 launches | All exact                            |
| Minimal direct family probes              |    40 compiles | E52017, no PTX                       |
| Matrix prefix capability, all three modes |    12 compiles | E36100/E36107, no PTX                |
| Catalog descriptors                       |              6 | 4 admitted, 2 intentionally excluded |
| Research PTX assembly                     |    4 artifacts | All pass                             |
| GPU smoke                                 |        4 tests | All pass                             |

The exact runtime inventory is 5,912 unique launches with no missing, duplicate or extra cases.
They compare 6,053,888 output words: 3,395,456 active raw16-in-u32 results and 2,658,432 inactive
sentinels. Each source/control mode runs 69 structured patterns under all 14 masks, plus 512
full-mask batches. The patterns contain 25 constant boundaries, 25 permutations, 10 repeated-value
patterns, distinct NaNs, signed-zero alternation and seven isolated-NaN placements. Masks are full,
low2/4/8/16, low15, high16/high17, even/odd, sparse extremes and singletons0/7/31. The 512 batches
place every one of the 65,536 encodings once among their 32 lanes by four components. This is complete
input-encoding coverage for those full-mask batches, not exhaustive tuples, scalar caller placements,
comparisons or mask combinations. The other masks use the 69 structured patterns only.

The 40 minimal direct probes independently cover each scalar/vector shape and existing matrix
reduction at O0/O3. All retain return code 255, exact canonical GenericAsm diagnostics and no PTX.
The exclusive-MIN IR dump is a distinct log from the diagnostic-only invocation. There are no
failed shader/source fixtures. An initial auxiliary script had a substring extraction SyntaxError
before any compile or launch; its script/log are retained separately and the corrected run records
all 12 matrix capability probes. Smoke was run immediately after the already-started research suite,
rather than before it as the workflow requests; this ordering deviation is recorded, with no
competing GPU suites or claim of pre-suite smoke. No successful research was repeated to conceal it.

The catalog admits half WAVE_READ_LANE_AT, LESS_THAN, GREATER_THAN and SELECT. It deliberately does
not admit half numeric MIN/MAX descriptors. Provider `_emitWaveReadLaneAt` bitcasts half to i16,
zero-extends to i32 for the indexed shuffle, then truncates and bitcasts back. FloatCompare uses
LLVM ordered OLT/OGT and SELECT preserves original operands. `_getFloatingPointConstant` accepts
16-bit patterns using APFloat IEEEhalf semantics. Source and direct O0/O3 controls establish these
transport, selection and finite constant paths on the actual unchanged library. Numeric provider
MIN/MAX for other floating widths goes through libdevice; that is a separate contract and is not
needed by this proposed ordered recipe.

The responsible admission boundaries are `_getNVVMMaskedWaveScalarIdentity` and
`_initializeNVVMMaskedWaveScalarOperation` in `slang-emit-nvvm.cpp`. The former currently excludes
half and must supply the finite seed only for the bounded MIN/MAX family. The latter's
`usesSourceMinMax` policy currently admits FP32 reductions and FP64 reductions/prefixes. A follow-up
should explicitly add FP16 reductions/prefixes to that existing policy, retaining FP32 prefix and
all unrelated boundaries. `_emitNVVMMaskedWaveScalarValue` already materializes floating constants
with semantic bit width and consumes the source-order flags. Its scalar recipe is shared by
canonical aggregate leaves. No alternative type/value representation, producer repair or numeric
half MIN/MAX provider descriptor is warranted by this evidence.

The self-review inventory has no new production helper, fallback or special case. The ignored
research generator, binary16 classifier, algorithm model and CUDA launcher serve only reproducible
evidence. The proposed finite-half seed is a valid source contract with a named producer and
consumer, not a workaround; infinity and singleton tests demonstrate exactly why that layer owns it.
A later implementation must reproduce the same saved inputs and expectations at direct O0/O3,
register a focused dynamic raw16 fixture, retain before-rejection proof and neighboring boundaries,
and perform the normal bounded compiler acceptance. No implementation begins in this research slice.

All 29 accepted source hashes, 12 artifact hashes, 554 registered input hashes and submodule pins
match before/after. Both source_commit and source_revision are the actual research base
`94209dfd6a3dc1a7ac729c0c0a928a598d570d58`. Compiler library remains
`948d300ec9f21f9000d242fcd83ee109a97ec63e1a768511553bc64c35b21c08`; provider remains
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI36. Native Ubuntu24.04,
L4 SM89, target SM80, driver580.126.09, CUDA12.9.2/NVRTC12.9.86 and LLVM14 are unchanged.

All 1,674 registered cells inherit slice231 (1,623 correct, 51 known failures and six resolved
histories), including its 639 fresh cells and 1,035 inherited full229 cells. No registered runtime
cell is fresh here. Units479 plus one existing Windows-only skip, toolkit18, contracts6 and all six
material compile/assembly cells are inherited. Frozen452/discovery106, latest implementation231,
latest full229 and cadence one remain unchanged. The four FP16 prefix failures remain unresolved.
Material runtime still lacks application bindings, textures/LUT/input and output oracle; no runtime
or performance claim follows. FP16 correctness research remains the next useful bounded support
candidate while that contract is absent. No next independent blocker was investigated or changed.

Parent acceptance independently reconstructed binary16 comparisons using exact integer multiples
of the smallest subnormal, then applied the source-order recipe. Every saved input, expectation
and actual output matched: 5,912 launches and 17,736 raw binaries. All 130 unique compact references,
source/artifact/input hashes and exact rejection logs also verified.
