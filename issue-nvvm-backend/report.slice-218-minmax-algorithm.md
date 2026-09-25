# Slice 218: preserve the CUDA masked FP32 min/max algorithm

## Motivation

Consider a full warp loading distinct NaNs from a host buffer:

```slang
uint lane = tid.x;
float value = asfloat(inputBits[lane]);
uint4 members = uint4(0xffffffff, 0, 0, 0);
uint minimumBits = asuint(WaveMultiMin(value, members));
uint maximumBits = asuint(WaveMultiMax(value, members));
```

CUDA's helper selects original input words. Accepted research 217 found NVVM returning injected
positive/negative infinity even when every input was NaN. Mixed-NaN and signed-zero cases also
selected different words because the direct recipe changed the source comparison and order.
The singleton fix in 216 remained correct but did not repair this nonsingleton algorithm.

## Proposed solution

For FP32 min/max reductions, preserve the source helper algorithm through the existing typed scalar
recipe. Start with the caller's value. A low-bit contiguous power-of-two mask performs descending
XOR butterfly stages over prior accumulated values. Other masks scan original named-lane values
in ascending order. Each combination uses ordered less/greater comparison and selects the second
operand on ties or unordered inputs. Aggregate leaves reuse this same graph.

The existing two-phi loop serves both cases. One phi holds the accumulated value; the unsigned phi
holds either the butterfly offset or remaining scan mask, selected once from the validated mask.
The source lane, shuffle input and next unsigned state follow that invariant. No new provider operation,
ABI, second loop implementation or source-text interpretation is needed.

## Change summary

- `source/slang/slang-emit-nvvm.cpp` configures ordered comparison for the admitted FP32 reduction
  recipe, shares the existing butterfly mask classifier with FP64 sum seed handling, and reuses the
  typed loop for both source algorithms. The now-redundant FP32 singleton final selection is removed.
- `tools/slang-unit-test/unit-test-nvvm-emitter.cpp` checks original-parameter loop seeds and typed
  comparison/selection while retaining the FP64 singleton and signed-zero seed assertions.
- `tests/cuda/nvvm-fp32-minmax-order.slang` checks dynamic scalar/vector/matrix NaN and signed-zero
  selections using closed-form expected source lanes. One discovery identity registers it.
- The design note, completed plan, acceptance manifest, census tables and STATUS retain architecture,
  exact preservation evidence, inherited outcomes and the next bounded action.

## Concepts and vocabulary

The _source algorithm_ is the concrete CUDA helper selected by canonical GenericAsm, rather than a
new universal promise about wave reduction order. A _butterfly offset_ identifies the XOR partner
for one stage, halving after each iteration. The _scan mask_ identifies original lane inputs still
unread. The _recipe operation closure_ is the complete set of typed provider operations required
before emission; it now includes the source algorithm's integer selections, shifts and XOR.

## Process report

Fresh-context delegation remains unavailable because the app reached its agent thread limit. The
parent performs this slice locally under WORKFLOW's recorded fallback. No independent worker review
is implied; source audit, before/after fixtures and exact evidence checks provide the review record.

The input shape is canonical. `hlsl.meta.slang` produces scalar `_waveMin` / `_waveMax` GenericAsm spellings
and aggregate Multiple helpers with validated value/mask signatures. `_resolveNVVMMaskedWaveScalarOperation`
and `_resolveNVVMAggregateWaveOperation` build `NVVMMaskedWaveScalarOperation`; aggregate recursion
calls `_emitNVVMMaskedWaveScalarValue` per scalar leaf. The source value and mask already contain all
required semantics. There is no lost generic context, alternate representation, or syntax to rebuild.

The defect belonged to that recipe: it supplied infinity and combined through numeric MIN/MAX,
which correctly ignores a NaN when paired with a number. `_initializeNVVMMaskedWaveScalarOperation`
now chooses an ordered LESS_THAN/GREATER_THAN descriptor returning Bool for FP32 min/max reductions.
`_emitNVVMMaskedWaveScalarValue` uses that result to select the original first or second floating word.
The provider's numeric MIN/MAX behavior remains unchanged for ordinary operations and other recipes.
The exact value-operation descriptor determines library requirements, so this path no longer requests
libdevice just to perform min/max; all operation requirements remain in preflight.

`_emitNVVMWaveButterflyMask` extracts the existing contiguous-low-bit and power-of-two-population
predicate from FP64 sum identity handling. For mask `0x0000ffff` it identifies a sixteen-lane butterfly;
for `0xffff0000` it identifies a scan, despite equal populations. `_emitNVVMFloat64WaveSumIdentity`
reuses that predicate with its previous negative-zero/positive-zero selection unchanged.

For the butterfly, the loop starts at population/2, reads `currentLane ^ offset` from the previous
accumulator, compares/selects, and halves the offset. For the scan, it starts at the original mask,
reads the lowest named lane from the original input, compares/selects, and clears that lane bit.
The initial accumulator is the caller value in both cases. For a low singleton the initial offset
is zero; other singletons compare/select the same original word during their one scan iteration.
Neither uses floating arithmetic, and both preserve signaling payloads. The previous FP32-only
final passthrough is therefore redundant and removed; FP64 arithmetic keeps its necessary selection.
The existing pending-phi protocol is unchanged: callers add incoming edges after blocks terminate.

The helper and special-case inventory is bounded:

| Entry                        | Disposition and input-shape audit                                                                                                                                                                               |
| ---------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `_emitNVVMWaveButterflyMask` | Keep: extract one existing canonical-mask classification, used by FP64 seed and FP32 source order. No new graph traversal or representation.                                                                    |
| `usesSourceMinMaxReduction`  | Keep: one classification of already-supported FP32 reduction signatures. It owns caller seed, ordered comparison, butterfly/scan state and operation closure. FP64 admission and prefixes stay outside it.      |
| Shared unsigned loop state   | Keep: the mask-uniform predicate chooses offset versus remaining-mask semantics; all named lanes use the same source algorithm. Dynamic expected-word tests and exact 217 replay fail without this distinction. |
| FP32 singleton final select  | Remove: the corrected source algorithm preserves the original word directly. FP64 arithmetic still needs its independent passthrough.                                                                           |

No new malformed-shape guard, default, fallback or semantic equivalence relation exists. The zero
mask still has no named participant. Existing source min/max and provider contracts remain the sources
of truth. A final all-NaN patch, seed-only numeric combine, or unconditional ascending comparison scan
would each leave part of the observed source algorithm incorrect and was rejected before implementation.

The final formatted fixture passed NVRTC and failed both NVVM modes on unchanged accepted 216
binaries. It loads raw bits dynamically and derives expected selected lanes independently: when all
inputs are unordered or tied, source comparisons choose the second operand. XOR stages select
`lane ^ (population - 1)`; scans select the last named lane. Full, low/high16, low15/high17, even/odd
and singleton partitions cover scalar, float4 and float2x2 quiet/signaling/mixed NaNs and signed zeros.
The unchanged 216 fixture separately preserves every singleton lane and finite neighbors.

The complete 217 matrix is replayed into a new raw directory with byte-identical shader source,
inputs and expected arrays. All 288 executions now match: 65,016 active words and 64,008 inactive
sentinels. The 96 formerly mismatching executions are fixed. Finite/infinity, positional NaNs,
signed zeros, scalar/float2/float2x2 shapes and all eight masks pass at NVRTC O3 and NVVM O0/O3.
This is exact concrete CUDA-helper compatibility; no universal cross-backend payload/order guarantee
or GPU speed claim is inferred.

Final validation completed on 2026-09-25:

| Gate                                | Fresh result                                                  |
| ----------------------------------- | ------------------------------------------------------------- |
| GPU smoke                           | 4/4                                                           |
| Focused runtime/structural/negative | 13/13                                                         |
| Research 217 replay                 | 288/288, unchanged input/expected words; three PTX assemblies |
| Units                               | 478/478, one existing Windows-only skip                       |
| Toolkit                             | 18/18 compile/assemble                                        |
| Selected frozen                     | 321 cells: 313 correct, eight retained preflight stops        |
| Full discovery                      | 300 cells: 270 correct, 30 retained failures                  |
| Material                            | Six compile/assembly cells pass; runtime unvalidated          |

All 618 old fresh cells exactly match accepted 216 across classification, return code, complete
execution counts, diagnostic and canonical shape. Three additions pass separately. Another 1,035
frozen cells explicitly inherit full 214. The cumulative ledger is 1,656 cells, 1,603 correct and
53 retained failures, with all first-known evidence and four resolved histories preserved. Research
217's 96 mismatches are fixed separately; they were never folded into those 53 failure histories.
There are no missing, extra or duplicate requested cells and no old input/oracle changes.

Frozen diagnostic mode returns zero for its known preflight stops; discovery returns two for
retained infrastructure/output failures. The research apparatus also returns zero upon completing
a mismatching run, so acceptance checks all structured expected/actual arrays directly. No runner
exit code substitutes for exact comparison. The initial evidence generator referenced its new output
instead of the prior census; that reporting-only filename was corrected, and no test rerun was needed.

Optimized compiler SHA256 is
`a13354a47acefc8684cbfb5ae52b84bcf0331a83e5310e9675f53a0e030107d7`.
The ABI 36 provider remains
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
All 18 tested source hashes and 12 artifact hashes match after gates. The original 547 runtime input
hashes match 216, and the new fixture adds one. The before fixture hash is unchanged. Raw commands,
logs, PTX and oracle arrays are under `build/nvvm-loop/slice-218-before` and `slice-218-after`.

The bounded impact and exact neighboring results support targeted acceptance; no provider/library/ABI,
general lowering or runner contract changed. Full checkpoint 214 remains latest, and implementation
cadence advances to two. FP64 min/max semantic/admission research can resume. Discovery now contains
100 identities, its declared capacity; future additions require a separate capacity change rather
than bypassing the loader. No material runtime or performance claim, GPU loss, system change or push
occurred. Local review is recorded under the delegation limitation.

Final local acceptance audit, 2026-09-25: verified 114 evidence references, 18 tested source hashes,
12 artifacts and 548 runtime input hashes. Independently compared all 321 selected frozen and
297 old discovery cells across five stable fields; three additions pass. All 53 first-known failure
records and four resolved histories remain intact. Accepted targeted, full 214 retained, cadence two.
