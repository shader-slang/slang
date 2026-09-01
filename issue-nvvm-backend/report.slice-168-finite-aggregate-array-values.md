# Slice 168: Finite aggregate-array values

## Motivation

Two healthy discovery workloads reached adjacent forms of the same broad problem: finite values
that already had a structural LLVM representation but were not admitted at every canonical use.
Consider this module constant:

```slang
static const float3x2 values[2] = {
    float3x2(1, 2, 3, 4, 5, 6),
    float3x2(7, 8, 9, 10, 11, 12),
};
```

`legalizeMatrixTypes` retains this as module-owned `makeVector` and `makeArray` instructions. Local
arrays and matrices already used those same constructors, but direct NVVM treated only scalar
module literals as values available to a function. A second workload placed `Texture2D[2]` in the
synthesized global parameter aggregate, loaded the complete array, selected one texture in a
helper, and joined the selected handle through a merge block. Direct NVVM supported each texture
handle and resource struct, yet rejected the fixed resource array and then the texture-valued phi.

## Proposed solution

Recognize a resource array only when it is the exact nonempty, two-operand fixed-array type with
natural stride and every recursive leaf already has an executable resource-value representation.
Lower it with the existing generic array type, aggregate construction, extraction, load, and phi
operations.

Recognize a module constant only when its complete, cycle-free tree consists of selected scalar
literals, `makeVector`, and value `makeArray`/aggregate constructors. Rematerialize the constructor
tree inside every using function. Do not cache function-owned aggregate instructions in the value
map shared across function emission. Provider ABI revision 32 remains unchanged.

## Change summary

- Resource-value classification and lowering now include exact naturally laid-out fixed arrays.
- Sequential extraction, aggregate construction, reachable-type traversal, and value lowering use
  that shared classification.
- Validation admits finite module-owned literal/construction trees, and emission recreates them in
  the current function with existing generic builder operations.
- Non-entry block parameters use the established executable-value algebra, allowing canonical
  resource-handle merge parameters as well as copyable scalar values.
- The two discovery fixtures gain permanent direct O0/O3 differential lanes. Separate frozen and
  discovery artifacts, a two-gate measurement manifest, the design, ledger, plan, and this report
  retain the evidence.

## Concepts and vocabulary

**Natural resource array** means a fixed array with no explicit stride operand whose leaves already
have selected resource-value representations, such as an i64 CUDA texture handle. It is a value
representation, not proof that an arbitrary source resource aggregate has a compatible launch ABI.

**Module constant tree** means immutable module-owned SSA built solely from supported literals and
finite value constructors. It is not global storage and does not imply support for mutable globals.

**Executable-value algebra** is the existing recursive classification of values that direct NVVM
can represent in SSA. It includes selected scalars, vectors, aggregates, pointers, and resource
handles; a basic-block parameter is LLVM phi transport for one of those values.

## Process report

The first input-shape audit started with `static-const-matrix-array.slang`. Its failing instruction
was module-scope `makeArray`, produced by `legalizeMatrixTypes` for the outer array and matrix rows.
The same fixture already proved local array construction, storage, helper parameters, and element
selection. The shape is therefore canonical and intentionally retained, not malformed upstream IR.
`_isNVVMSupportedModuleConstantValue` proves the complete module-owned graph rather than admitting
one opcode in isolation: selected integer, Boolean, floating, and null-pointer literals are leaves;
exact vector and value-aggregate constructors are recursive nodes; cycles and every other module
operation are rejected. `_validateAvailableValue` and module validation share that predicate.

Emission initially appeared able to cache the lowered aggregate like a scalar constant. The
input-shape/self-review audit rejected that design: the emitter's value map spans functions, while
LLVM vector and aggregate construction instructions belong to the function where they are
inserted. `_getLoweredNVVMValue` therefore rematerializes each supported constructor tree at its
use. It may still reuse provider scalar constants, but it never records the function-owned
aggregate result in the shared map. This preserves LLVM ownership without inventing module storage
or reconstructing source syntax.

The second audit traced `func-resource-result-complex.slang` from
`collectEntryPointUniforms` to a synthesized `GlobalParams` field of type `Texture2D[2]`, followed
by an ordinary `IRLoad`. `_getNVVMResourceValueAlignment` already defined recursive structs and
opaque-resource leaves. It now accepts only `ArrayType(element, count)` with two operands, a
positive bounded literal count, no explicit stride, and a recursively supported leaf. The new
`asNVVMSupportedResourceArrayType` is deliberately distinct from helper arrays: accepting a value
array does not silently widen helper signatures. `NVVMTypeLoweringContext`, aggregate construction,
element extraction, and reachable-type traversal all consume that one classification and reuse
the provider's generic array operations.

Once the array load passed preflight, the helper's conditional texture choice exposed a
`Texture2D` merge-block parameter. The canonical producer is ordinary control-flow lowering: both
predecessors pass an already-supported texture handle to one block parameter, which LLVM represents
as a phi. The old copyable-value gate was narrower than the emitter's actual generic phi operation.
`_validateNVVMFunction` now asks `_getNVVMExecutableValueAlignment`, the established classification
used for executable SSA values. This is a principled transport invariant, not a resource-specific
branch.

Three apparent neighboring failures were deliberately not widened. `buffer-type-splitting` has an
`S[2]` whose `S` contains two raw buffer views; the provider's pointer/count aggregate size does not
match the CUDA launch layout, so `_hasNVVMCompatibleAggregateStorageLayout` continues to reject it.
Frozen `cbuffer-float3-offsets-unaligned` combines a resource field with packed constant data, and
`type-legalize-bug-1` combines a resource field with a parameter block. Their first field-pointer
diagnostics are symptoms of broader global-block layout contracts, not evidence that an arbitrary
resource pointer is valid. All three exact diagnostics remain unchanged.

No synthetic fake-provider test was retained. The attempted constant fixture was optimized to
scalar constants before reaching the intended forms, so its counters would not prove ownership of
the new paths. The two real fixtures preserve the exact linked IR, exercise LLVM verification,
libNVVM, PTX, and runtime comparison, and each establishes a distinct semantic combination without
expanding the already-large unit harness.

Frozen corpus v1 retains exactly 452 workloads and 427 healthy references at 402/402/402
O0/O3/both, with no changed row or old-correct regression. Discovery retains exactly 82 workloads
and 72 healthy references and advances from 66/66/66 to 68/68/68, with precisely the two selected
workloads as gains. In each direct mode its classification totals are 68 correct, six preflight,
seven infrastructure, and one runtime mismatch. The selected NVVM prefix passes 433/433, the full
permanent NVVM category passes 50/50, and the promoted lanes pass 4/4.

The exploratory measurement generated ten accepted PTX/cubin rows. The module-constant workload
measured 256.9 ms and 8,094-byte PTX at direct O0 SM70, and 264.8 ms and 1,163 bytes at direct O3
SM70, versus 369.7 ms and 9,519 bytes through NVRTC O3. The resource-array workload measured
246.3 ms and 8,204-byte PTX at direct O0 SM70, and 251.3 ms and 884 bytes at direct O3 SM70, versus
354.8 ms and 8,823 bytes through NVRTC O3. Direct O3 PTX for both assembled with CUDA 12.9 for
SM70, SM80, and SM90. Three repetitions and end-to-end census timings remain exploratory rather
than controlled benchmark claims.

The final helper/special-case inventory retains three bounded changes. The resource-array
classifier survives because one exact producer and natural-layout proof feed every consumer. The
module-constant predicate survives because it proves a complete immutable constructor tree and
prevents arbitrary module SSA admission. Executable-value block parameters survive because generic
phi emission already owns every selected SSA representation. No fixture-name check, compatibility
fallback, syntax reconstruction, arbitrary graph search, downstream layout patch, serialized-text
rewrite, or provider callback remains.
