# Admit FP64 masked min/max reductions

## Motivation

Consider a kernel that loads a double payload from memory and calls:

```slang
uint4 members = uint4(mask, 0, 0, 0);
double value = bit_cast<double>(inputWords);
double minimum = WaveMultiMin(value, members);
double maximum = WaveMultiMax(value, members);
```

The CUDA source backend executes this contract, while direct NVVM previously rejects the canonical
`_waveMin($1.x, $0)` double helper before producing PTX. Research 219 establishes 112 independently
expected cases, including NaN payloads, signed zeros, adjacent doubles, subnormals and scalar/vector/
matrix transport. The accepted FP32 source algorithm already expresses the required control flow.

## Proposed solution

Admit one-lane Float64 MIN/MAX reduction leaves into that typed source recipe. Preserve the original
caller value as the accumulator seed; do not ask for a numeric identity that does not describe this
source operation. Reuse ordered floating comparison, raw-value selection and the existing two-word
shuffle. Prefix admission and ordinary provider numeric min/max remain unchanged.

## Change summary

- `slang-emit-nvvm.cpp` extends source-reduction classification to scalar Float64, bypasses numeric
  identity lookup for caller-seeded source reductions, and retains final singleton passthrough only
  for FP64 arithmetic reductions.
- The existing structural unit now checks both widths' source seeds/comparison-selection and the
  FP64 arithmetic seed/passthrough separately. Negative cases retain unsupported FP64 prefixes and
  reject vector values passed to scalar helper spelling.
- `nvvm-fp64-minmax-order.slang` adds one explicit discovery source with runtime-loaded words,
  closed-form expected results, five partitions and six value families at all three modes.
- Plan, report, result manifest/census, design and STATUS retain acceptance and provenance.

## Concepts and vocabulary

A _source reduction_ implements the CUDA helper's order and operand choice rather than numeric
minimum/maximum's NaN behavior. A _scalar leaf_ is the one-lane descriptor used recursively for each
vector or matrix element. The _butterfly_ exchanges previous accumulated values with XOR partners;
the _scan_ consumes original values in ascending named-lane order. Neither needs a numeric seed.

## Process report

Fresh worker creation again failed at the app's agent-thread limit. The parent follows WORKFLOW's
local fallback; no independent worker review is implied.

The standard module produces canonical scalar/Multiple GenericAsm helper shapes. Scalar and aggregate
resolvers validate their signatures, then `_initializeNVVMMaskedWaveScalarOperation` constructs each
typed scalar leaf. `_emitNVVMMaskedWaveScalarOperation` lowers that descriptor using the existing
loop and deferred phi incoming values. There is no malformed producer representation to repair:
this consumer was deliberately restricting the accepted type while semantics were researched.

The helper/fallback inventory contains no new production helper or fallback. The existing
`usesSourceMinMaxReduction` classification now accepts Float32 or Float64 only with laneCount one,
reduction mode and MIN/MAX operation. That explicit scalar check preserves the invariant formerly
provided by the identity helper. A malformed double2 signature using scalar `_waveMin` remains a
preflight rejection. Canonical aggregate helpers still recurse through scalar leaves.

Source min/max bypasses `_getNVVMMaskedWaveScalarIdentity` because its accumulator begins with the
caller's operand. All other operations keep the old identity validation, including rejection of
Float64 min/max prefixes. This avoids fabricating an infinity identity or widening numeric min/max
semantics. The source loop already preserves singleton words, so it does not use the additional
FP64 arithmetic final select. Sum/product retain their old handling, and sum retains its signed-zero
seed. No graph/loop, provider operation, ABI, library, target fallback or producer rewrite is added.

The focused fixture loads both uint32 halves of each double. Quiet/signaling NaNs carry payloads in
both halves, and zero signs vary. For those unordered/tied families, its oracle selects
`lane^(population-1)` for low contiguous power-of-two masks and the last named lane for scans.
Positive adjacent-double/subnormal words increase monotonically with lane, so explicit partition
endpoints give independent minimum/maximum expectations. All singleton positions are exercised.
Scalar, double4 and double2x2 checks compare full uint64 words. Research replay separately covers
unique payloads for 32 lanes, finite/infinite and mixed-NaN cases using its integer-only algorithm.

The first fixture draft used firstbithigh(mask) to compute expected endpoints. Existing CUDA
`U32_firstbithigh` complements words with bit31 set; diagnostic instrumentation isolated failures to
those finite/subnormal expected bounds. Before registering the fixture, replace that unrelated
helper dependency with explicit partition endpoints. Preserve both draft and diagnostic evidence.
The finalized before run passes NVRTC and rejects both direct modes with the expected double min
preflight diagnostic. No established source contract or expected output is changed. The bit-index
behavior is a separate bounded follow-up.

The first structural run passed all runtime fixtures but failed its sum-seed count: two source-loop
shuffle-input selects were also Float64. The test now identifies the sum seed by constant operands,
checks the two shuffle-input selects' phi/parameter operands, and separately counts ordered compare/
select and arithmetic passthrough. No production change was needed for that apparatus correction.
The initial assertion log remains under `initial-structural-check`.

The unchanged research219 source and integer oracles pass all 336 executions: 112 per mode,
75,852 active double values (151,704 uint32 words) and 74,676 inactive double sentinels match.
Each direct PTX contains seven ordered less-than and seven greater-than FP64 comparisons, 28 indexed
32-bit shuffles and no numeric FP64 min/max. This is operand-selection evidence, not a performance claim.

The targeted frozen subset has 321 cells, with 315 correct and six retained preflight stops. Exactly
the two registered `wave-multi-min-max.slang` direct cells become correct; every other field matches
full220. All 300 old discovery cells also preserve every stable field, and all three new cells pass.
The complete fresh set is 624 cells: 588 correct and 36 retained failures. Another 1,035 frozen cells
explicitly inherit full220. Cumulative coverage is 1,659 cells, 1,608 correct and 51 open failures.
All 53 prior failure histories survive: 51 remain open and the two demonstrated fixes join four
previously resolved records. No baseline is reset.

Fresh final gates pass: focused16/16, runtime smoke4/4, units478/478 plus one existing Windows-only
skip, toolkit18/18, discovery contracts6/6 and material compile/assembly6/6. Full material execution
still requires its application bindings, texture/LUT/input and output oracle. Frozen diagnostic mode
returns zero for retained preflight stops; discovery returns two for known failures. Structured
outcomes, rather than those exit codes alone, establish acceptance.

Local parent acceptance verifies 132 evidence references, 23 tested source hashes, 12 artifact hashes
and 549 runtime input hashes. The final fixture is byte-identical to its successful NVRTC/failed
NVVM before run. All 548 old runtime inputs and old manifest rows are unchanged. The compiler hash is
`55bd12f280ee51def87219c767557e198cbd07b9b99f06d118a20209dfb46598`; the ABI36 provider remains
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.

Accepted locally on 2026-09-25. Latest full checkpoint remains220 and implementation cadence is one.
Next investigate the independently observed unsigned bit-index helper behavior in a bounded research
slice; do not silently fold a prelude change into this admission.
