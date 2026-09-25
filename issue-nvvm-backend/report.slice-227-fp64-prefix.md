# Preserve FP64 masked min/max prefix order

## Motivation

The original frozen prefix-min/max workloads advance through context admission in 225, then reject
canonical double prefixes. Research 226 establishes their exact source behavior. Consider:

```slang
uint lane = cudaThreadIdx().x;
uint mask = 0xf;
if ((mask & (1u << lane)) != 0)
{
    double value = bit_cast<double>(inputWords[lane]);
    double result = WaveMultiPrefixExclusiveMin(value, uint4(mask, 0, 0, 0));
    outputWords[lane] = bit_cast<uint64_t>(result);
}
```

With distinct NaNs in lanes 0 and 1, lane 2 first reads lane 1 at offset 1, then lane 0 at offset 2. CUDA
returns lane 0's payload. An ascending original-input scan returns lane 1's payload instead. An
identity-only admission change or numeric minimum cannot preserve the source result.

## Proposed solution

Extend the existing typed source min/max recipe to scalar/vector FP64 prefixes. Reuse ordered
comparison/select and mask classification. Inclusive returned state starts with the caller;
exclusive minimum/maximum starts with binary64 positive/negative infinity. Low contiguous masks
with power-of-two population use ascending shuffle-up offsets, carrying inclusive transmitted
state separately from returned state. Other masks retain ascending original-input scans, combining
only earlier source lanes. Existing reductions, FP32 prefixes and arithmetic contracts remain.

## Change summary

- `source/slang/slang-emit-nvvm.cpp`: admit FP64 prefixes to the shared typed recipe; encode exact
  infinity identities, prefix tree partner/offset operations, separate transmitted phi and its
  deferred edges. Extract typed combination shared by returned and transmitted state.
- `tests/cuda/nvvm-fp64-prefix-minmax-order.slang`: dynamic scalar/double4 raw-bit fixture across
  tree/scan/singleton masks, quiet/signaling NaNs, signed zeros, adjacent values and subnormals.
  Three native directives and one discovery row retain independent closed-form expectations.
- Design document, completed plan, this report, STATUS and 227 result/census manifests record the
  architecture, fresh/inherited preservation and exact research replay. Raw logs stay ignored.

## Concepts and vocabulary

The _transmitted state_ is the inclusive partial value read by another lane at the next offset.
The _returned state_ is the caller-seeded inclusive or infinity-seeded exclusive accumulator.
_Low contiguous power-of-two masks_ contain lanes 0 through N−1 for N=1,2,4,8,16,32. Other nonempty
masks use an ascending scan. _Ordered compare/select_ chooses the second operand for ties or NaNs,
retaining its raw bits rather than applying the provider's numeric min/max semantics.

## Process report

`hlsl.meta.slang` specializes `WaveMultiPrefixInclusive/ExclusiveMin/Max` to canonical CUDA
GenericAsm. `_resolveNVVMMaskedWaveScalarOperation` and aggregate leaf resolution already validate
this signature and its homogeneous value layout. The old `_initializeNVVMMaskedWaveScalarOperation`
accepted source min/max only for reductions and explicitly rejected double prefix identities.
This is intentionally valid checked semantic data; no AST/IR producer representation needs repair.
The recipe/emission boundary owns the missing algorithm.

The helper/special-case inventory has three retained entries. First, FP64 prefix classification
extends the existing source-min/max flag and derives a prefix flag. It selects ordered comparison,
strict source-before-caller predicate, left-shift offsets and subtraction partners from existing
typed provider operations. `_requireNVVMMaskedWaveScalarOperations` records the same complete
operation closure before provider discovery. Removing admission reproduces the unchanged new
fixture's canonical double rejection in both direct modes. The final fixture passes NVRTC on
original source in a retained revert drill; an initial fixture-only E30081 warning was corrected
without changing its inputs or expected words.

Second, `_emitNVVMMaskedWaveScalarValue` reuses `_emitNVVMWaveButterflyMask` and the existing loop.
For a prefix tree, initial offset is 1 when population exceeds 1; each stage doubles it and ends
before population. Subtracting offset from an unsigned caller lane wraps for lanes below offset.
Selecting the caller for those lanes exactly implements CUDA shuffle-up boundary behavior, keeps
every named lane active, and makes the strict prefix predicate skip combination. This is valid
collective behavior, not a guard hiding malformed compiler data. The full mask executes the same
sequence of shuffles; predicates control accumulation rather than participation. Irregular masks
retain original-input reads and ascending source order.

Third, the graph adds a caller-seeded transmitted phi and `_emitNVVMMaskedWaveCombine` factors the
existing typed algebra. Every iteration reads the previous transmitted state, updates returned
state from that read, and separately updates transmitted state with the same ordered algebra.
The exclusive returned identity cannot stand in for inclusive transmitted data. Deferred phi
edges use the same established after-block-termination mechanism as remaining-mask/returned state.
The helper does not redo substitution, lookup, canonicalization or type construction; it emits
one existing scalar recipe, preserving the exact reduction combine behavior. The independent
research 226 oracle and its 8,658-word counterexample to universal scans justify keeping separate
state/order. Replacing it with an assertion or producer-side reconstruction would reject valid
source programs rather than implement their semantics.

Validation and original-workload reassessment are recorded in `runtime-validation.slice-227.json`.
The exact 226 replay preserves all 196 input/expected sets in each of NVRTC O3/NVVM O0/O3. It checks
172,872 active binary64 results and 353,976 inactive sentinels (526,848 binary64 comparisons), input
buffers unchanged, and three successful PTX assemblies. No oracle uses another wave operation.
The registered fixture additionally checks all 32 output lanes in all three modes.

The affected domain is 107 frozen wave/quad/double/helper/vector/matrix identities plus all 104
discovery identities, 633 fresh cells; 1035 remaining frozen cells explicitly inherit full 225.
Recipe/graph changes are confined to masked waves, with no general lowering, provider or ABI change.
All previously supported wave reductions/arithmetic and FP32 prefixes are preservation obligations.
Material checks cover six compile/assembly cells only; application runtime bindings/oracle remain
missing. Matrix prefix capability repair and any next independently unsupported instruction are
out of scope. No performance, material runtime, system change or push claim is made.

The original frozen min/max sources remain correct under NVRTC. Their four direct cells now stop
at the next independent canonical operation: `_wavePrefixExclusiveMin(($1).x, $0)` or
`_wavePrefixExclusiveMax(($1).x, $0)`, both with `int8_t(int8_t, vector<uint,4>)` signatures.
No narrow-integer prefix change is included. Each existing failure record retains its first-known
evidence, reproduction and diagnostic history, including the double-prefix observation from 225.

Final gates pass: focused 3/3, smoke 4/4, units 479/479 plus one existing Windows-only skip,
toolkit 18/18, discovery contracts 6/6, research 588/588 and material compile/assembly 6/6.
The 633 fresh runtime cells comprise 630 old cells and three additions. Only the four recorded
prefix diagnostics change; 594 old correct cells pass freshly and 1020 correct cells inherit 225.
The cumulative ledger is 1668 cells/1617 correct/51 known failures, with all six resolved histories
retained. All 551 old runtime-input hashes and 103 old discovery rows are unchanged. Last full
checkpoint remains 225; implementation cadence is one after parent acceptance.

Parent independently accepted the final diff and evidence after ownership returned. Its audit
verified 216 evidence references, 27 sources, 12 artifacts and 552 runtime input hashes. No additional
compiler change or test rerun was needed after the final validated candidate.
