# Slice 216: preserve FP32 singleton masked min/max operands

## Motivation

Consider this kernel fragment, with the host supplying `0x7fc12345` or `0x7f812345` as a raw word:

```slang
uint lane = tid.x;
float value = asfloat(inputBits[lane]);
uint4 members = uint4(1u << lane, 0, 0, 0);
uint minimumBits = asuint(WaveMultiMin(value, members));
uint maximumBits = asuint(WaveMultiMax(value, members));
```

Each lane names only itself. CUDA's source helper performs no arithmetic for this reduction and
preserves the input. Slice 215 measured NVVM returning positive infinity for minimum and negative
infinity for maximum instead. This is a supported FP32 correctness defect, separate from the
53 registered frozen/discovery failures. It takes priority over deferred FP64 min/max admission.

## Proposed solution

Reuse the existing typed singleton predicate and original-operand selection. Separate that
behavior from the FP64 sum seed calculation, then activate singleton preservation for FP32
minimum/maximum reductions. The shared aggregate recipe applies the same behavior to every leaf.
The nonsingleton scan, numeric min/max provider, source helpers, canonical IR and ABI remain as before.

## Change summary

- `source/slang/slang-emit-nvvm.cpp` separates common singleton detection from FP64 sum seed
  selection and includes FP32 min/max in the singleton operation closure and final typed select.
- `tests/cuda/nvvm-fp32-singleton-minmax.slang` loads twelve raw patterns dynamically, checks
  scalar/float4/float2x2 minimum and maximum bitwise at every lane, and checks finite nonsingleton
  full, low/high16, 15/17 and even/odd partitions. One discovery identity registers the fixture.
- `tools/slang-unit-test/unit-test-nvvm-emitter.cpp` checks original parameter versus accumulated
  phi selection at both widths, with the separate signed-zero seed restricted to FP64.
- `docs/design/nvvm-backend.md` records the shared singleton invariant and separate FP64 seed.
- The completed plan, result manifest, census tables and STATUS retain validation and inheritance.

## Concepts and vocabulary

A _masked scalar recipe_ describes the typed operations and loop implementing one supported CUDA
wave helper. An _aggregate leaf_ is one scalar component reached by existing vector/array recursion;
matrices reach that path as arrays of vectors. The _singleton predicate_ tests whether the caller's
mask has only its one participating lane. The _seed_ is the value used before scanning participating
lanes; a numeric infinity seed is not an identity for the singleton NaN passthrough contract.

## Process report

The producer is valid. `hlsl.meta.slang` specializes the example into canonical scalar
`_waveMin($1.x, $0)` / `_waveMax($1.x, $0)` GenericAsm helpers; vector/matrix overloads use the
corresponding `Multiple` helpers. Exact spelling/signature resolution in
`_resolveNVVMMaskedWaveScalarOperation` / `_resolveNVVMAggregateWaveOperation` initializes one
`NVVMMaskedWaveScalarOperation`. Existing aggregate recursion invokes its scalar emission for
every leaf. There is no alternate spelling to normalize, lost semantic source of truth, or syntax
to reconstruct.

`_getNVVMMaskedWaveScalarIdentity` supplies positive/negative infinity. The provider's numeric
min/max correctly prefers a numeric operand when the other is NaN. The old recipe scanned that
injected seed even for one lane, thereby selecting infinity. The recipe owns the defect: changing
the provider would alter independent ordinary numeric operations, while a front-end change would
misrepresent valid source. `_emitNVVMMaskedWaveScalarValue` now selects the untouched caller value
at exit for supported FP32 min/max singletons, just as it already did for admitted FP64 reductions.

The helper/special-case inventory is deliberately small:

| Entry                               | Disposition and input-shape audit                                                                                                                                                                                                                                                           |
| ----------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `_emitNVVMWaveReductionIsSingleton` | Keep. Extract the existing `mask == (mask & -mask)` typed predicate from the FP64 helper. The canonical uint mask comes from the validated helper parameter, and all named participants use it. The final raw-bit fixture fails without FP32 activation. No fallback or new representation. |
| `_emitNVVMFloat64WaveSumIdentity`   | Keep. Rename/split the existing seed portion; assert the sum-only contract. Its negative-zero versus positive-zero calculation is unchanged and is never called for FP32. Existing FP64 edge coverage proves signed-zero and payload preservation.                                          |
| `preservesSingletonReduction`       | Keep. Rename the old FP64 flag and extend only to FP32 MIN/MAX reductions. Prefixes and other FP32 arithmetic retain previous recipes. Capability preflight requires the same typed equality/select that emission uses.                                                                     |

No new equivalence relation, semantic graph walk, syntax reconstruction, default, or malformed-shape
guard exists. The zero mask has no named participant and receives no new semantics. FP64 min/max
remains rejected by the unchanged identity admission check and strict negative coverage.

The final formatted GPU fixture ran before production edits on exact accepted214 binaries:
NVRTC passed; NVVM O0 and O3 executed but failed the raw-bit oracle. Its SHA256 is
`d838cc70fe2e4d88bb2b127be48304fce74219bfd6319d25122a8e84815fe381` and remains unchanged.
Twelve host words cover positive/negative zero, positive/negative finite one, both infinities,
positive/negative quiet and signaling NaNs with distinct payloads. Every lane tests every pattern;
scalar2 + vector8 + matrix8 gives 6,912 raw-bit checks per mode. The expected singleton value is
the original integer word, independent of floating-point arithmetic and NVRTC output. Finite
neighbors use independently computed endpoint extrema and check every scalar/vector/matrix leaf.
No nonsingleton NaN/order conclusion is claimed.

The structural fixture initially used a double kernel entry parameter outside current admission;
we replaced that test apparatus with the established lane-derived values and integer destination.
The initial test macro also needed braces. Those failed apparatus logs are retained but excluded
from acceptance. No production admission change followed. The final unit checks two FP32 and two
FP64 original-parameter selections and exactly one separate FP64 seed selection, with guarded
predicate indexing. Existing integer-loop, aggregate, strict unsupported, and FP64 edge tests
remain unchanged.

Final validation on the recorded source state:

| Gate                                | Fresh result                                           |
| ----------------------------------- | ------------------------------------------------------ |
| GPU smoke                           | 4/4 correct                                            |
| Focused GPU and structural/negative | 10/10 correct                                          |
| NVVM/routing/reporter/literal units | 478/478 plus one existing Windows-only skip            |
| Toolkit                             | 18/18 compile/assemble                                 |
| Frozen selected107 identities       | 321cells:313correct,8retained preflight                |
| Full discovery99 identities         | 297cells:267correct,30retained failures                |
| Material                            | All6 cells compile/assemble; no runtime claim          |
| Original215 exact-source replay     | 12/12 correct,24/24 raw words equal,3/3 PTX assemblies |

All615 old fresh runtime cells exactly retain classification, return code, complete execution counts,
diagnostic and canonical shape. Three new cells pass separately. No requested cell is missing,
extra or duplicated. The remaining1035 frozen cells inherit full214 explicitly. The cumulative ledger
is1653 cells,1600 correct,53 open failures plus four resolved histories. The original546 runtime
sources are unchanged. Research215's four mismatching executions are fixed separately and were
never part of those53 failures. Frozen returns0 for its diagnostic-only subset; discovery returns2
for retained infrastructure/output failures. Neither return code substitutes for the structured audit.

This bounded recipe change does not trigger a full checkpoint: provider/library/ABI, general
lowering and runner contracts are unchanged, and neighboring outcomes are exact. Full214 remains
the latest full checkpoint; accepting216 advances implementation cadence from0 to1. Material
bindings, textures/LUTs, inputs and expected outputs remain absent, so no material runtime or speed
claim follows from assembly. Nonsingleton FP32 min/max, the deferred FP64/aggregate/order semantic
matrix, prefixes and other independent blockers remain separate work. Resume research215's
semantic gate next; singleton correctness alone does not justify FP64 min/max admission.

Tested base is `230c3e0eae73be3b2ff01e26e3d346e12fe9b86c` plus the final source changes.
Compiler SHA256 `fa55d1fdc41988e27e672d4a2ad92b93060293d101f4a7a076be3e68dd08298f`;
provider remains `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI36.
The native Ubuntu/L4 SM89 environment, driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86 and
LLVM14 are unchanged. All17 tested-source and12 artifact hashes match after validation.
Raw commands/logs/PTX are in `build/nvvm-loop/slice-216-before` and `slice-216-after`, including
`research-replay`; the checked-in manifest contains exact outcomes, references and failure history.
No GPU loss, system change, worker commit or push occurred. Parent owns acceptance and local commit.

Parent acceptance, 2026-09-24: independently reviewed the final production, structural-test and
runtime-oracle changes. All 615 old fresh outcomes match accepted 214 across five stable fields,
and three additions pass. Verified 101 evidence references, 17 tested sources, 12 artifacts and
547 runtime input hashes. All first-known failure records and four resolved histories survive.
Accepted as targeted implementation; full checkpoint 214 remains current and cadence is one.
