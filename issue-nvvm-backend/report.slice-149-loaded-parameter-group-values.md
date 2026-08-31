# Slice 149 report: canonical loaded parameter-group values

## 1. Motivation

Consider this ordinary compute-shader shape:

```slang
struct Params
{
    ParameterBlock<Impl> implementation;
}

ConstantBuffer<Params> params;
Impl value = params.implementation;
```

CUDA entry-uniform collection and specialization preserve that operation as three exact pointer
steps in linked IR:

```text
Ptr<ConstantBuffer<Params>> = fieldAddress(globalParams, params)
ConstantBuffer<Params> = load(...)
Ptr<ParameterBlock<Impl>> = fieldAddress(..., implementation)
ParameterBlock<Impl> = load(...)
Impl = load(...)
```

Slice 148 made the nested field-address path canonical, exposing a four-workload discovery cluster
at the final load. The middle `ParameterBlock<Impl>` value is semantically a wrapper but already
lowers to an LLVM global pointer to `Impl` storage. `_validatePointerValue` only recognized explicit
Slang pointer types, so it rejected that valid provider pointer as `producer=load, consumer=load`.

The tempting widening—accepting every loaded parameter group as a pointer—is not correct. CUDA
parameter-group storage represents a compact `float3` as a scalar array while ordinary values use
an LLVM vector. A stored `UserPointer` leaf is global-address-space storage while its ordinary
value is a generic pointer. A whole-element LLVM load can directly become semantic `T` only when
the complete storage and value representations are identical.

## 2. Proposed solution

Admit one canonical pointer-like value and prove its complete representation:

1. The value must be an `IRLoad` of an admitted `ParameterBlock<T>` or `ConstantBuffer<T>`.
2. Its operand must be an exact `IRFieldAddress` already resolved by
   `_getNVVMStructFieldAddress`.
3. The declared field type must exactly equal the loaded wrapper type.
4. A finite recursive classifier must prove that `ParameterGroupStorage` and ordinary `Value`
   lowering produce the same provider type for `T`.
5. The only legal consumer is an immutable exact-type `IRLoad<T>`, with ordinary dominance and
   availability validation.

Emission shares the producer resolver, uses the existing generic typed `emitLoad`, and marks the
element read invariant. No provider callback or ABI change is required.

## 3. Change summary

- `source/slang/slang-emit-nvvm-type-lowering.*`
  - adds a cycle-safe classifier for exact parameter-group storage/value representation identity;
  - accepts existing identity cases and explicitly rejects compact vector3 and `UserPointer`
    differences.
- `source/slang/slang-emit-nvvm.cpp`
  - resolves the exact `fieldAddress -> load<ParameterGroup<T>>` producer;
  - validates only immutable exact-element loads and shares the resolver with emission;
  - marks whole parameter-group element loads invariant.
- `tools/slang-unit-test/unit-test-nvvm-*`
  - covers a successful whole scalar-struct load through the fake provider;
  - proves that a whole compact-`float3` parameter-group load stops before provider discovery.
- Three repository shaders gain direct O0/O3 differential lanes for generic resource graphs,
  generic scalar parameter-group transport, and texture/sampler tuple transport.
- Separate Slice 149 frozen-v1 and discovery TSV/JSON artifacts retain every classification,
  exact shape, producer, diagnostic, and Pareto cluster.

## 4. Concepts and vocabulary

- **Semantic wrapper pointer:** a Slang parameter-group value whose provider representation is an
  LLVM pointer even though its linked-IR type is not `IRPtrTypeBase`.
- **Representation identity:** the stronger condition that storage-role and value-role lowering
  produce the same complete LLVM type; semantic copyability alone does not prove it.
- **Exact producer chain:** the bounded `IRFieldAddress -> IRLoad<ParameterGroup<T>>` sequence,
  rather than an arbitrary walk through loads, calls, phis, or selects.
- **Invariant load:** a provider load from storage whose accepted root and consumer contract prove
  that the shader cannot modify it.

## 5. Process report

### The field declaration remains the source of pointer provenance

`collectEntryPointUniforms` and ordinary IR lowering produce the field-address/load sequence shown
above. `_getNVVMLoadedParameterGroupPointer` does not infer that every `IRLoad` result is a device
pointer. It requires the loaded type to pass `asNVVMSupportedParameterGroupType`, requires the
operand to be an `IRFieldAddress` accepted by `_getNVVMStructFieldAddress`, and compares the field's
declared type with the loaded wrapper using the existing canonical `isTypeEqual` path.

This shape is canonical and intentionally allowed. The field declaration and specialized wrapper
type are already the semantic sources of truth; no syntax is reconstructed and no alternative type
equivalence is introduced. Calls, phis, selects, arbitrary loaded pointers, and non-field roots
remain rejected. Removing the resolver returns all four measured rows to the exact
`producer=load, consumer=load` diagnostic, which proves the pointer validator owns this contract.

### Representation identity is narrower than supported storage

`hasNVVMParameterGroupStorageValueRepresentation` mirrors existing lowering decisions rather than
comparing opaque provider handles after module mutation. Integer and Float32 scalars, noncompact
32-bit vectors, fixed numeric arrays, physical matrix wrappers, admitted resource handles, and
nested parameter groups already delegate to or structurally match ordinary value lowering.
Recursive arrays and structs are admitted only when every child is identical, with an active-type
set rejecting cycles.

Two counterexamples define the boundary. A standalone 32-bit vector3 uses a scalar-array storage
type and an LLVM-vector value type. A device `UserPointer` uses a global pointer in parameter-group
storage and a generic pointer as an ordinary value. The classifier rejects both rather than adding
a bitcast, address-space cast, byte copy, or fallback. The focused compact-vector whole-load test
receives `loaded parameter-group value representation` before builder discovery. Existing
field-by-field compact-vector reconstruction remains supported and unchanged.

### Preflight and emission consume one immutable contract

When `_validatePointerValue` sees the exact loaded wrapper, it requires an `IRLoad<T>` consumer,
prohibits write access, compares the expected pointee with the specialized element, and validates
that the wrapper load dominates the consumer. Other pointer families continue through their
existing typed producer checks.

During emission, the same resolver identifies the pointer operand. The wrapper value already has
the LLVM pointer created by `_lowerParameterGroupType`; `NVVMIRBuilder::emitLoad` therefore performs
the complete operation. The representation classifier proves that its physical pointee can be
recorded directly as semantic `T`, and the exact immutable root selects
`SLANG_NVVM_LOAD_FLAG_INVARIANT`. Provider ABI revision 30 already expresses every operation.

### Permanent tests protect combinations, not fixture identities

Three healthy discovery rows become correct at both O0 and O3 and receive permanent lanes:

- `generic-shader-object-cbuffer.slang` combines a constant buffer, nested parameter block,
  generic implementation, and multiple structured buffers;
- `parameter-block-unify.slang` transports whole specialized integer structs through generic
  helper calls from both parameter-block and constant-buffer wrappers;
- `tuple-parameter.slang` transports texture/sampler tuples from two constant buffers and samples
  across them.

They share the canonical producer but protect distinct aggregate leaves and downstream consumers.
`parameter-block-load.slang` crosses the new invariant and advances to a separate sequential
element-pointer diagnostic. It remains a discovery failure and receives no permanent directive.
No test name participates in production classification.

### Coverage remains split across two denominators

Frozen corpus v1 remains exactly 452 workloads with 427 healthy MVP references:

| Frozen corpus-v1 metric | Slice 149 result |
|---|---:|
| Direct O0 correct | 371/427 (86.9%) |
| Direct O3 correct | 375/427 (87.8%) |
| Correct in both modes | 371/427 (86.9%) |
| Old-correct regressions | 0 |
| Selected NVVM regression prefix | 425/425 |

Its all-tier raw classifications are 449 native correct plus three infrastructure rows; direct O0
has 384 correct, 53 preflight, eight runtime mismatches, and seven provider failures; direct O3 has
389 correct, 53 preflight, eight runtime mismatches, and two provider failures. No frozen row changes
classification relative to Slice 148.

The separate discovery corpus remains 82 selected workloads with 72 healthy native references:

| Discovery metric | Slice 148 | Slice 149 |
|---|---:|---:|
| Direct O0 correct | 47/72 | 50/72 (69.4%) |
| Direct O3 correct | 47/72 | 50/72 (69.4%) |
| Correct in both modes | 47/72 | 50/72 (69.4%) |
| Newly unlocked in both modes | — | 3 |
| Old-correct regressions | — | 0 |

Across all selected discovery rows, classifications are:

| Route | Correct | Runtime mismatch | Slang preflight | Provider/libNVVM | Infrastructure |
|---|---:|---:|---:|---:|---:|
| Native NVRTC O3 | 72 | 2 | 0 | 0 | 8 |
| Direct NVVM O0 | 50 | 1 | 23 | 1 | 7 |
| Direct NVVM O3 | 50 | 1 | 23 | 1 | 7 |

The former four-healthy-row device-pointer-load cluster is eliminated. The leading remaining
healthy discovery clusters are typed aggregate field pointers at three rows and seven separate
two-row clusters: aggregate array elements, aggregate sequential pointers, entry parameters,
function identity, helper aggregate parameters, helper pointer parameters, and helper resource
results. `parameter-block-load.slang` accounts for the sequential-pointer increase from one to two;
that is advancement, not a regression. Corpus v1 and discovery denominators are never combined,
and no corpus v2 is proposed.

### Performance and architecture measurements remain exploratory

Three standalone compilations per configuration for the newly correct kernels produce:

| Workload | NVRTC O3 median / PTX | Direct O0 SM70 median / PTX | Direct O3 SM70 median / PTX |
|---|---:|---:|---:|
| Generic cbuffer/resource graph | 346.8 ms / 8893 B | 231.2 ms / 4829 B | 227.6 ms / 899 B |
| Scalar parameter-group unification | 358.7 ms / 8786 B | 248.2 ms / 4701 B | 256.5 ms / 730 B |
| Tuple texture/sampler parameter | 342.2 ms / 8660 B | 232.9 ms / 5650 B | 239.2 ms / 687 B |

CUDA 12.9 `ptxas` accepts direct O3 output for these three and the five established discovery
representatives at SM70, SM80, and SM90. Census end-to-end times include compilation, loading,
execution, and comparison; they are not kernel-only runtimes. CUDA 13 and physical SM70/SM80/SM90
workers remain open productionization requirements.

### Self-review inventory

- The loaded-parameter-group resolver survives. It is bounded to the canonical field/load producer
  and exact declared wrapper type; it does not traverse arbitrary operand graphs.
- The representation-identity classifier survives. It mirrors existing lowering roles, rejects
  cycles and known physical differences, and is required by both the positive and negative tests.
- The immutable pointer-validator branch survives. It enforces exact type, consumer, dominance,
  and access rather than weakening the general pointer classifier.
- The emission flag survives. The shared resolver proves immutable parameter storage and the
  provider already owns the typed load.
- All three promotions survive. Each protects a distinct real semantic combination and passed
  native/direct differential execution at O0 and O3.

The diff adds no fixture-name check, syntax reconstruction, compatibility fallback, provider ABI
widening, custom AST/IR equivalence, or downstream repair of malformed IR. Every retained special
case names its canonical producer and has a test that fails when the owning invariant is removed.
