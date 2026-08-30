# Slice 140: Lower masked scalar wave algebra through generic loops

## Motivation

The Slice 139 census left 36 workloads in the wave/reconvergence `GenericAsm` cluster. Nineteen
were healthy MVP references, and twelve workloads shared one bounded producer family. Consider:

```slang
int reduction = WaveMultiSum(value, partition);
int prefix = WaveMultiPrefixSum(value, partition);
```

CUDA specialization in `source/slang/hlsl.meta.slang` turns these calls into final one-block
helpers whose bodies contain exact target assembly such as `_waveSum($1.x, $0)` or
`_wavePrefixSum($1.x, $0) `. `StmtLoweringVisitor::visitIntrinsicAsmStmt` represents that final
body as `IRGenericAsm`; linking preserves the complete `int(int, vector<uint,4>)` contract. The
direct path stopped in `_validateNVVMFunction` with diagnostics such as:

```text
direct NVVM lowering does not support Slang IR instruction or shape
'GenericAsm assembly=_waveProduct($1.x, $0), signature=int(int, vector<uint,4>)'
```

Adding one fixture or one wave operation at a time would repeat the same classification and
lowering. The useful invariant is the finite scalar masked-wave algebra: an exact final spelling,
its complete specialized signature, a reduction/prefix mode, a typed combine operation, and an
identity value.

## Proposed solution

The compiler now recognizes 21 exact final assembly spellings covering scalar masked sum,
product, minimum, maximum, bitwise-and, bitwise-or, and bitwise-xor reductions plus their measured
exclusive/inclusive prefix forms. It accepts only canonical final value helpers with the complete
`T(T, uint4)` signature. `T` is one selected 32-bit scalar, the result equals the value parameter,
and bitwise operations require integer values.

One compiler-owned descriptor records the semantic operation and the complete generic operation
closure. Emission extracts `partition.x` and visits exactly its set bits in a compact CFG loop:

```text
laneBit      = remaining & -remaining
sourceLane   = firstbitlow(laneBit)
sourceValue  = waveReadLaneAt(partition.x, value, sourceLane)
accumulated  = combine(accumulated, sourceValue)
remaining    = remaining & ~laneBit
```

Prefix recipes condition the combine on `currentLane > sourceLane` or
`currentLane >= sourceLane`. Typed constants provide zero, one, all-ones, signed extrema, or
Float32 infinities as required by the operation. Capability preflight consumes the same descriptor
before provider discovery or module mutation.

Every primitive is expressible through revision 29's generic type, constant, CFG, phi, structural,
and typed value operations. The LLVM provider ABI and callback set therefore remain unchanged.

## Change summary

- `source/slang/slang-emit-nvvm.cpp` adds the finite scalar masked-wave descriptor, exact
  spelling/signature resolver, identity selection, capability collection, and compact loop
  emission.
- `tools/slang-unit-test/unit-test-nvvm-support.h` adds a real scalar reduction/prefix source and
  an adjacent malformed-signature source.
- `tools/slang-unit-test/unit-test-nvvm-emitter.cpp` verifies the generic operation/CFG graph and
  deterministic malformed-signature preflight.
- `tools/slang-unit-test/unit-test-nvvm-integration.cpp` verifies differential PTX/runtime behavior
  and `ptxas` acceptance with the real provider.
- Twelve existing fixtures gain explicit direct O0 and O3 runtime lanes.
- The fixed census/Pareto artifacts, this report, the slice plan, and durable architecture and
  capability documents record the measured result.

## Concepts and vocabulary

- **Scalar masked-wave algebra**: the bounded reduction/prefix family selected by an exact final
  helper spelling and `T(T, uint4)` signature.
- **Partition mask**: the canonical `uint4` wave mask; CUDA's 32-lane mask is carried in lane zero.
- **Recipe closure**: every typed provider operation needed to emit one recognized helper,
  collected before module construction.
- **Healthy MVP reference**: one of the 427 MVP workloads whose native CUDA/NVRTC O3 lane is
  correct.
- **Selected prefix**: focused direct-NVVM unit tests. It is a regression score, not the corpus
  coverage denominator.

## Process report

### The canonical producer owns a finite semantic family

Consider `WaveActiveProduct` and `WaveMultiPrefixSum`. The source intrinsics are specialized by
the CUDA prelude into final target helpers. `StmtLoweringVisitor::visitIntrinsicAsmStmt` creates
the `IRGenericAsm`, and linking supplies a concrete result/parameter signature. This is a valid,
intentional target-specific representation, not malformed upstream IR. The target emitter owns
translation because it is the boundary that turns finalized CUDA semantics into provider-neutral
typed construction.

`_resolveNVVMMaskedWaveScalarOperation` requires
`_isCanonicalNVVMGenericAsmValueHelper`, compares the whole assembly text against a finite table,
and validates the complete linked signature. It does not parse placeholders, inspect a source
function name, or look at a fixture path. A malformed `_waveSum` taking scalar `uint` rather than
`uint4` remains the ordinary deterministic `GenericAsm` preflight failure. This negative proves
that the spelling alone cannot widen admission.

The exact rows are not compatibility aliases. Each is emitted by the current CUDA specialization
and differs semantically by combine operation or exclusive/inclusive mode. Removing the shared
resolver restores the first unsupported shapes recorded in the Slice 139 census for all twelve
new successes; no fallback accepts them.

### Identities and typed operation closure are explicit

`_getNVVMMaskedWaveScalarIdentity` maps the already validated scalar type and combine operation to
the exact identity bit pattern. Add/or/xor start from zero, multiplication from one, integer and
from all ones, and min/max from the appropriate signed/unsigned extremum or Float32 infinity.
Bitwise operations reject floating-point signatures. These values are semantic properties of the
reduction, so recording them in the descriptor avoids host conversion and avoids reconstructing
source syntax.

`_setNVVMSupportedValueRecipeStep` constructs one typed operation step and applies the existing
`NVVMSemantics` legality rules. `_requireNVVMMaskedWaveScalarOperations` records the common loop
closure plus prefix-only lane index, comparison, and select steps. The focused fake-provider test
proves that two helpers emit two shuffles, two lane-index queries, two `firstbitlow` operations,
two negations, one prefix select, four scalar phis, and eight incoming edges. It also proves that
no libdevice module is requested.

### A compact CFG replaced the rejected unrolled prototype

The first implementation expanded all 32 lanes into straight-line select/combine operations. It
was differentially correct at O0 and admitted the same 13 targeted scalar-wave workloads, but
libNVVM O3 produced `wave-active-product` PTX with 134 b32 SSA registers. CUDA 12.9 `ptxas` failed
register allocation. Keeping that code because its runtime comparison passed would have left an
unusable representation.

The final emitter uses the same arbitrary-mask algebra in a three-block loop. Two phis carry the
remaining mask and accumulated value, so live state stays bounded. The loop isolates one set bit,
reads that lane under the original partition mask, conditionally combines it, clears the bit, and
continues. The focused real-provider PTX test sees `shfl.sync.idx.b32`; differential execution and
the `ptxas` test both pass at O0 and O3.

The provider checks phi incoming blocks against the complete function CFG. Adding incoming edges
as soon as a phi was created failed while the other blocks were still unterminated. Emission now
terminates source, loop, body, and exit blocks first, then adds the two entry and two back-edge
incoming values. This ordering satisfies an existing provider invariant; it does not relax the
provider or special-case a malformed graph.

### Promotion and fixed-denominator coverage

The fixed corpus remains 452 workloads from 448 sources: 430 MVP and 22 extension. Native
CUDA/NVRTC O3 is correct for 449 and has three infrastructure failures.

| Mode | Correct | Runtime mismatch | Preflight | Provider | Compiles and launches |
| --- | ---: | ---: | ---: | ---: | ---: |
| Direct O0 | 330 | 8 | 107 | 7 | 338 |
| Direct O3 | 335 | 8 | 107 | 2 | 343 |

Both direct modes gain twelve exact success identities and lose none from Slice 139. Eleven are
MVP workloads and `wave-multi-prefix-scalar-functional` is extension-tier evidence. Among 427
healthy MVP references, O0 correctness is 328/427 (76.8%), O3 correctness is 332/427 (77.8%), and
both-mode correctness is 328/427 (76.8%).

The twelve promoted fixtures cover active product, masked/divergent reductions, scalar masked
prefix, ordinary wave composition, early return, nested and loop divergence, and multi-wave thread
groups. Eleven complete fixture runs pass. `hlsl-intrinsic/wave.slang` has an unrelated WebGPU
bind-group-layout infrastructure failure; its new direct-NVVM O0 and O3 lanes pass independently.

The total wave/reconvergence cluster falls from 36 to 24, while healthy-MVP failures fall from 19
to eight. The remaining healthy rows are aggregate shuffle/reduction helpers or active-mask
materialization; extension rows include aggregate `*Multiple` prefixes/reductions and advanced
rotate operations. They remain measured failures rather than triggering speculative widening.

The leading remaining healthy-MVP clusters are:

| Root-cause cluster | O0 blocked | O3 blocked |
| --- | ---: | ---: |
| Helper ABI/type contract | 16 | 16 |
| Aggregate/pointer/layout transport | 14 | 14 |
| Ordinary numeric/bit operation | 11 | 11 |
| Residual target marker/undefined value | 9 | 9 |
| Wave/reconvergence GenericAsm | 8 | 8 |
| Atomic/wave operation | 8 | 8 |
| Function identity | 6 | 6 |
| Raw-buffer view access | 4 | 4 |

### Representative and productionization gates

All three representative release gates remain differentially correct. Median standalone compile
time and generated PTX size from three samples are:

| Gate | NVRTC O3 | Direct O0 | Direct O3 |
| --- | ---: | ---: | ---: |
| Resource/aggregate/helper | 386.9 ms / 8,889 B | 268.6 ms / 6,102 B | 272.9 ms / 919 B |
| Parameter-block layout | 390.5 ms / 8,839 B | 257.5 ms / 917 B | 259.0 ms / 793 B |
| Shared control/barriers | 371.2 ms / 9,190 B | 254.3 ms / 1,940 B | 258.8 ms / 1,404 B |

Across all census lanes, startup-inclusive compile/load/execute/compare median/p90/mean times are
4741.0/5064/4801.4 ms for NVRTC O3, 4542.5/4868/4571.7 ms for direct O0, and
4504.0/4722/4536.0 ms for direct O3. These are not kernel-only runtime measurements.

CUDA 12.9 `ptxas` accepts every representative direct O3 module for SM70, SM80, and SM90. Runtime
comparison uses the local RTX 5090/SM120. CUDA 13 tooling and physical SM70/SM80/SM90 workers
remain productionization gaps. The isolated LLVM 14 provider remains compiler-matched at ABI
revision 29.

### Validation and self-review

- Release compiler and unit-test builds pass outside the sandbox; the isolated provider is rebuilt
  even though its source and ABI are unchanged.
- Focused fake, malformed-signature, real differential, and real `ptxas` tests pass.
- All twelve promoted direct O0/O3 lanes pass, including the two isolated lanes in the file with
  the unrelated WGPU failure.
- The final three-mode census has 12 gains and zero old-correct regressions.
- The selected direct-NVVM prefix passes 410/410.
- Representative direct O3 PTX assembles for SM70, SM80, and SM90.

The new-helper inventory is the masked-wave descriptor, recipe-step initializer, identity mapper,
exact resolver, requirements collector, and compact emitter. Every one consumes the canonical
linked helper or produces its typed lowering. The finite spelling rows share the same producer and
complete signature invariant. No fixture-name check, syntax reconstruction, compatibility
fallback, arbitrary operand-graph walk, or provider ABI widening is retained.
