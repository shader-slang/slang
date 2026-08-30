# Slice 138 canonical scalar intrinsic recipe report

## 1. Motivation

The Slice 137 Pareto report left 21 healthy-MVP workloads at ordinary CUDA intrinsic
`IRGenericAsm`, the second-largest measured cluster. These were not 21 unrelated missing
operations. CUDA specialization had produced a bounded family of final one-block helpers for Half
bit transport, packed-Half conversion, Double word transport, floating classification, `sincos`,
and `frexp`.

Consider `asuint(double, out uint, out uint)`:

```text
IRGenericAsm("$P_asuint($0, $1, $2)")
signature = Void(Double, OutParam<UInt32>, OutParam<UInt32>)
```

`StmtLoweringVisitor::visitIntrinsicAsmStmt` creates the target helper, CUDA specialization fixes
its types, and linking preserves its final signature. Before this slice, `_validateNVVMFunction`
could only reject the assembly even though the existing builder already expressed Double/UInt64
reinterpretation, integer shifts/conversions, pointer stores, and a void return.

The goal was to remove this entire root-cause cluster through one reusable compiler representation,
not to add fixture-specific emit paths.

## 2. Proposed solution

The direct emitter now recognizes a scalar recipe only when the helper is the established
one-block GenericAsm shape and both its complete assembly string and specialized signature match
one of 18 measured contracts. Out-parameter roles, pointee types, result type, parameter order, and
arity are all part of the key. Fixture paths, source function names, and substring parsing never
participate.

Each recipe is a bounded graph of generic typed value operations. The same step descriptors drive
preflight capability queries and emission. Half and Double transport use reinterpretation and
integer operations; classification uses physical IEEE masks; `sincos` composes the existing sine,
cosine, store, and return operations.

`frexp` was the one concrete operation revision 27 could not express exactly. Libdevice returns the
fraction and writes the exponent through an LLVM pointer, while the provider's generic callback has
one semantic result. Forward-only ABI revision 28 adds two value-operation IDs for the fraction and
exponent projections. The provider keeps the temporary i32 pointer local, calls exact
`__nv_frexpf` or `__nv_frexp`, and returns the requested projection through the existing callback.
No intrinsic-specific callback family is added.

## 3. Change summary

- `slang-emit-nvvm.cpp` generalizes the existing compound recipe step, adds exact scalar recipe
  signatures, queries the complete operation closure before provider mutation, and emits each
  recipe using ordinary typed builder operations.
- The shared semantic catalog and builder API add four exact Float32/Float64 `frexp` projection
  descriptors under ABI revision 28. `slang-llvm-nvvm` implements them with provider-local
  temporary storage and exact libdevice calls.
- Focused fake-provider coverage exercises all 18 recipe signatures and an adjacent wrong
  out-parameter signature. Real-provider coverage verifies both projections and the generated
  LLVM/libNVVM declarations.
- Nineteen fixtures receive O0/O3 direct lanes. `bit-cast-16-bit` receives an O3-only lane because
  its unoptimized Half operation remains an explicit provider failure.
- The fixed 452-row census/Pareto artifacts, capability ledger, and design status are updated with
  exact success identities and remaining clusters.

## 4. Concepts and vocabulary

- **Scalar intrinsic recipe**: a compiler-owned, bounded sequence of generic typed operations for
  one exact final GenericAsm helper contract.
- **Complete helper signature**: the specialized result and ordered parameter types, including
  whether each parameter is an `OutParam` and its exact pointee type.
- **Projection operation**: one semantic result of a provider implementation that internally calls
  a multi-result ABI such as libdevice `frexp`.
- **Healthy MVP reference**: one of the 427 MVP workloads whose native CUDA/NVRTC result is correct.

## 5. Process report

### The recipe boundary is the canonical final helper

The exact shape reaching the resolver is produced by
`StmtLoweringVisitor::visitIntrinsicAsmStmt`, CUDA target specialization, and final linking. It is
canonical and intentionally allowed: the one-block helper is how the CUDA prelude represents these
target operations after overload selection. The helper's linked signature is already the semantic
source of truth, so the emitter does not reconstruct syntax or rediscover types from operands.

`_resolveNVVMScalarIntrinsicRecipe` first requires that final helper topology, extracts every
semantic parameter/result type, records exact out-pointer roles, and compares the full contract to
the measured table. A wrong high-word type in
`Void(Double, OutParam<UInt32>, OutParam<Int32>)` fails deterministic preflight and never reaches
provider discovery. Removing the resolver restores the exact GenericAsm first failures in all 21
original rows.

The initial census exposed 12 exact first-blocker pairs. Once those advanced, six adjacent
signatures in the same family appeared: Half-to-bits and Float16/Float32/Float64 finite/infinite
classification. They use the same producer, recognition invariant, and operation graph, so they
were included in this bounded vertical slice. The final table has 18 exact rows rather than a text
parser or open-ended spelling fallback.

### Generic operations own transport and classification

Half bit transport remains bit transport: signed/unsigned i16 is reinterpreted as Half and Half is
reinterpreted as UInt16. Packed conversion narrows or widens the container around exact Half/Float
conversion. Double construction widens and shifts the high UInt32 word, combines it with the low
word, and reinterprets UInt64 as Double. Decomposition performs the inverse and stores both words
through the canonical helper pointers.

Finite, infinite, and Half NaN classification use exact physical exponent/mantissa masks. These are
valid target representations because the recipe contracts explicitly select IEEE Half, Float, or
Double bit transport, and the provider's typed reinterpretation preserves those bits. The focused
test observes each typed descriptor; `classify-float.slang` and `classify-double.slang` prove runtime
ownership at O0 and O3.

No new provider surface is needed for any of these operations. Their integer constants,
reinterpretations, conversions, shifts, Boolean comparisons, stores, and returns were already
generic revision-27 capabilities.

### `sincos` is a compiler composition, while `frexp` proves one provider gap

The final `sincos` helper has one floating input and two exact floating out pointers. The compiler
uses the existing sine and cosine semantic operations, stores both results, and returns void. This
keeps the provider free of a callback that merely duplicates existing behavior.

For `frexp`, the same approach cannot produce the exponent: revision 27 has no generic operation
whose result includes libdevice's pointer write. Reimplementing special-value and subnormal
semantics with compiler arithmetic would duplicate libdevice and risk mismatches. ABI revision 28
therefore adds `FREXP_FRACTION` and `FREXP_EXPONENT` to the existing typed value-operation namespace.

`_emitFrexpProjectionOperation` validates the exact Float32/Float64 operand and selected result,
creates an entry-block i32 temporary, and calls `__nv_frexpf` or `__nv_frexp`. Fraction and exponent
queries currently make independent pure calls because the shared callback has one result. This is
a deliberate bounded cost; it preserves exact libdevice behavior without leaking LLVM pointers or
adding a multi-result callback. Both corpus `frexp` fixtures compare correctly in both modes, and
the real-provider test verifies the declarations, calls, and exponent load.

### Promotion and fixed-denominator coverage

Nineteen workloads become correct at both O0 and O3. `bit-cast-16-bit` additionally becomes correct
at O3; at O0 its valid Half-to-bits recipe reaches libNVVM's existing unoptimized-Half operation
failure, so only the supported O3 lane is promoted. `scalar-half` advances to a later `$P_min`
Half overload and remains the one ordinary-GenericAsm healthy-MVP failure.

All 59 CUDA lanes in the 20 promoted fixture files pass: 20 native references plus 39 explicit
direct lanes. The fixed denominator remains 452 workloads from 448 sources: 430 MVP and 22
extension. Native CUDA/NVRTC O3 is correct for 449 and has three infrastructure failures.

| Mode | Correct | Runtime mismatch | Preflight | Provider | Compiles and launches |
| --- | ---: | ---: | ---: | ---: | ---: |
| Direct O0 | 298 | 8 | 140 | 6 | 306 |
| Direct O3 | 303 | 8 | 140 | 1 | 311 |

Compared with Slice 137, O0 gains 19 exact success identities and O3 gains 20; neither loses an
old-correct identity. Among 427 healthy MVP references, O0 correctness is 297/427 (69.6%), O3 is
301/427 (70.5%), and both-mode correctness is 297/427 (69.6%). The selected 406/406 unit prefix is
a regression score, not the coverage denominator.

The leading healthy-MVP failure clusters are now:

| Root-cause cluster | O0 blocked | O3 blocked |
| --- | ---: | ---: |
| Helper ABI/type contract | 28 | 28 |
| Wave/reconvergence semantics | 19 | 19 |
| Aggregate/pointer/layout transport | 17 | 17 |
| Ordinary numeric/bit operation | 16 | 16 |
| Residual target marker/undefined value | 9 | 9 |
| Atomic/wave operation | 8 | 8 |
| Function identity | 6 | 6 |
| Ordinary intrinsic GenericAsm | 1 | 1 |

The ordinary GenericAsm cluster falls from 21 to one. The four O0 healthy-MVP unoptimized-Half
provider failures remain separately visible rather than being hidden by O3 success.

### Representative and productionization gates

All three release-gate workloads remain differentially correct. Median standalone compile time and
generated PTX size from three samples are:

| Gate | NVRTC O3 | Direct O0 | Direct O3 |
| --- | ---: | ---: | ---: |
| Resource/aggregate/helper | 387.4 ms / 8,889 B | 273.0 ms / 6,102 B | 273.9 ms / 919 B |
| Parameter-block layout | 382.3 ms / 8,839 B | 250.3 ms / 917 B | 250.8 ms / 793 B |
| Shared control/barriers | 386.5 ms / 9,190 B | 251.6 ms / 1,940 B | 259.9 ms / 1,404 B |

Across all census lanes, startup-inclusive compile/load/execute/compare median/p90/mean times are
4423.0/4739/4489.6 ms for NVRTC O3, 4297.5/4564/4325.9 ms for direct O0, and
4314.5/4590/4343.4 ms for direct O3. These are not kernel-only runtime measurements.

CUDA 12.9 `ptxas` accepts every representative direct O3 module for SM70, SM80, and SM90. Runtime
comparison uses the local RTX 5090/SM120. CUDA 13 tooling and physical SM70/SM80/SM90 workers remain
productionization gaps. The isolated LLVM 14 provider remains compiler-matched and moves forward
with Slang at ABI revision 28.

### Validation and self-review

- Release host and isolated-provider builds pass outside the sandbox.
- Focused recipe topology, malformed-signature, and real-provider `frexp` tests pass.
- The selected NVVM unit prefix passes 406/406.
- All 59 native/direct CUDA lanes in the promoted files pass.
- The final three-mode 452-workload census has zero old-correct regression.
- All representative runtime comparisons pass and direct O3 PTX assembles for SM70/80/90.

The new-helper inventory contains the exact recipe resolver, bounded step appenders, recipe
emitter, provider `frexp` projection emitter, out-store helper, and integer-constant helper. Each
survives the audit: the resolver consumes a canonical linked helper and complete semantic
signature; step helpers describe already-queried generic operations; the emitter does not repair
IR; the provider helper owns a concrete libdevice pointer ABI that cannot cross the semantic
callback; and store/constant helpers remove duplication without widening accepted shapes. There is
no fixture-name check, syntax reconstruction, compatibility fallback, custom semantic equivalence,
assembly substring parsing, unqueried operation, or downstream patch for malformed IR.
