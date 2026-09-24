# Slice 208: FP64 masked-wave arithmetic

## Motivation

Consider this program fragment in a 32-lane kernel:

```slang
uint lane = WaveGetLaneIndex();
uint4 partition = uint4(0xffffffff, 0, 0, 0);
double value = 16777216.25l + double(lane) * 0.25l;
double sum = WaveMultiSum(value, partition);
double prefix = WaveMultiPrefixExclusiveSum(value, partition);
```

The CUDA specialization was valid, but direct NVVM rejected `double(double, uint4)` helpers
because the masked-wave recipe admitted only 32-bit leaves. The same boundary blocked vector
sum/product and inclusive/exclusive prefixes in two unchanged frozen workloads. Slice 207 already
provided exact FP64 scalar shuffle transport. No new provider operation is necessary.

## Proposed solution

Admit Float64 add/multiply recipes and homogeneous aggregate leaves through existing validated
operations. Keep FP64 min/max rejected: its compare/select CUDA contract requires a separate
edge-semantics audit. Preserve the complete specialized helper signature and finite exact assembly
spelling, existing scalar emission, component recursion and typed operation closure.

The arithmetic identity also preserves source behavior. A sum of all negative zeros needs a
negative-zero seed for CUDA's caller-seeded butterfly masks, and positive zero for other masks.
Singleton sum/product returns the original bits, so a signaling NaN is not quieted. The emitter
expresses this with existing unsigned arithmetic, bit operations, comparison and selection;
no ABI, catalog, library or provider contract changes.

## Change summary

- `source/slang/slang-emit-nvvm.cpp`: Float64 arithmetic admission, exact double-one identity,
  source-faithful sum identity/singleton selection, and homogeneous Float64 aggregate leaves.
- Three `tests/cuda/nvvm-fp64-*` runtime fixtures: closed-form scalar/vector arithmetic,
  matrix reductions/indexed transport, and raw-bit/exceptional floating-point boundaries.
- Existing unsupported-IR unit test: malformed masks, floating bitwise operations and deferred
  FP64 min/max still reject before provider discovery.
- Discovery manifest: three new source contracts; old discovery and frozen oracles unchanged.
- Plan, selection, result manifest/outcomes, STATUS and design note: bounded acceptance evidence.

## Concepts and vocabulary

A masked-wave recipe is a finite typed operation graph chosen by an exact specialized CUDA
helper spelling and signature. A homogeneous aggregate leaf is the scalar element reached through
vectors/fixed arrays; lowered matrices use arrays of vectors. A singleton partition names exactly
one participating lane. CUDA's butterfly mask is a contiguous run starting at bit zero with a
power-of-two population; its reduction begins with caller values rather than an extra identity.

## Process report

`hlsl.meta.slang` specializes scalar `WaveMultiSum/Product` and prefix overloads to the existing
`GenericAsm` spelling table. These are intentional canonical helper shapes. Vector/matrix overloads
produce `Multiple` helpers; lowered matrices return through `OutParam<Array<vector<double,N>,M>>`.
`_resolveNVVMMaskedWaveScalarOperation` / `_resolveNVVMAggregateWaveOperation` validate complete
signatures, and `_initializeNVVMMaskedWaveScalarOperation` requests the scalar operation closure.
`_emitNVVMMaskedWaveScalarValue` performs masked iteration; aggregate emission applies that graph
to each leaf. The producer is correct; rejection belonged to the typed backend boundary.

The initial admission change passed finite arithmetic but a raw-bit probe found a real mismatch:
CUDA returned negative zero for full-wave and singleton sums, whereas NVVM returned positive zero.
`_waveReduceScalar/Multiple` in the CUDA prelude calls `_waveCalcPow2Offset`; matching masks use
butterfly accumulation from each caller. Sparse masks initialize sum to positive zero. The new
`_emitNVVMFloat64WaveReductionIdentity` computes the same mask classification without replicating
the optimization topology: contiguous low bits plus power-of-two population. Negative zero is a
neutral addition identity that preserves all-negative-zero sums for the butterfly case. The
existing positive-zero identity remains for sparse masks and prefixes. Product's one identity
already preserves signs. A final singleton select returns the original caller bits for both
operations, including signaling NaNs; arithmetic on an unselected temporary cannot replace that
semantic source of truth. General 32-bit recipes remain unchanged.

The input-shape audit found no alternate AST/IR/Val representation, source reconstruction,
structural equivalence, arbitrary graph walk or fallback. The new helper survives because the
source CUDA reduction distinguishes mask shapes with observable IEEE-754 behavior. The existing
operation recipe emitter is reused; no separate operation IDs or provider entry points exist.
Preflight includes every extra operation before provider discovery. Exact final fixtures are
required to reject on the old emitter and execute with the final one.

Alternatives: FP64 min/max would add a separate comparison/NaN/signed-zero question; arbitrary
numeric width admission would request unsupported semantics. Reconvergence, helper KernelContext
pointers and FP8/BF16 are independent features. None is needed to demonstrate this slice. The six
complex material cells are compile/assembly probes only: application bindings and output oracle
remain absent.

Validation and exact identity/count tables are recorded in `runtime-validation.slice-208.json`.
Parent accepted the targeted domain after independent implementation, identity/hash and exact
preservation review. All 57 previous failure histories remain accounted for; 53 remain open.
The last full checkpoint is 207 and the implementation cadence is now 1.

### Boundaries and adjacent findings

The aggregate leaf recognizer also reaches indexed matrix transport, so the matrix fixture checks
all four components with explicit interleaved and partial low16 masks. Ordinary vector-by-value
shuffle still reaches `_isSelectedNVVMWaveVector` and rejects; it was not widened to absorb a
separate compound-operation feature.

The provisional implicit matrix test passed, but `_emitNVVMActiveMaskValue` lowers its active mask
to `WAVE_MASK_BALLOT(0xffffffff,true)`. Actual O0 PTX contains `vote.sync.ballot.b32` with mask -1.
The [PTX ISA 8.8 vote.sync contract](https://docs.nvidia.com/cuda/pdf/ptx_isa_8.8.pdf) requires all
non-exited named lanes to participate. Upper lanes bypassing a low16 branch do not satisfy that
requirement. This provisional pass is retained only as investigation evidence, never as a correctness
claim. Newly admitted FP64 implicit aggregate shuffle therefore rejects at the responsible operation
resolver; this is a supported-domain boundary for valid input, not a malformed-IR repair. A new
negative case checks that boundary. No existing source contract was converted into an expected-error
test. The pre-existing 32-bit active-mask implementation remains a separate correctness candidate.

The helper/special-case inventory is complete: the Float64 identity emitter and its typed recipe
closure survive with the source trace above; Float64 singleton selection survives raw-bit payload
coverage; the implicit Float64 aggregate rejection survives the active-mask contract audit. No
fallback or silent default was introduced. The exact final sources reject on the reverted accepted
emitter and pass after restoration. Parent review independently checked the final identity graph.

### Final validation

| Gate                        | Result                                                          |
| --------------------------- | --------------------------------------------------------------- |
| Focused                     | 10/10; nine GPU mode cells and strict preflight unit            |
| Runtime smoke               | 4/4                                                             |
| NVVM/routing/reporter units | 474/474; one Windows-only skip                                  |
| Toolkit                     | 18/18 compile/assembly cells                                    |
| Targeted frozen             | 321 fresh cells; 313 correct, eight retained preflight failures |
| Full discovery              | 282 fresh cells; 252 correct, 30 retained failures              |
| Complex materials           | 6/6 compile/assembly cells; no runtime claim                    |

The four expected fixes are the unchanged `wave-multi-sum-product` and
`wave-multi-prefix-sum-product` workloads at NVVM O0/O3. No other old classification, return code,
full execution count, diagnostic or canonical-shape field changes. Nine discovery additions pass.
Of the 1572 previous correct cells, 552 were freshly preserved and 1020 explicitly inherited. The
cumulative ledger contains 1585 correct and 53 unresolved cells over 1638; 603 cells are fresh and 1035 are
inherited. All 57 old failure histories remain linked: 53 open and 4 resolved. Latest full checkpoint
remains 207; accepting this targeted slice advances cadence 0 to 1.

The revert build produced compiler SHA256
`a4ffa6b02d200436875e148cf87ee5e2553c4bad02e02951d322562e8314efd9`, exactly accepted 207. It passed
all three final NVRTC fixtures and rejected all six final direct cells. Restoring the formatted
patch and rebuilding produced compiler SHA256
`242c23acfafa2e08d5ae9d23fdaa2457656550a7ba7a0a687d978d18f05319c8`.
Provider SHA256 remains
`6da3abff11e6b67a1dcd5e3b12f341bc7cbfa356fd29c9d7b5dcad8e20efd676` (ABI 35).
All final fixture, source and artifact hashes match after gates. Source base is
`cdb5a654732183df67b5c6db834eee652723e690` plus the recorded diff. No worker commit/push occurred.
