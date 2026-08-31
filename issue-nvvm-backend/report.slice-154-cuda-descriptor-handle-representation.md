# Slice 154: Preserve CUDA descriptor-handle resource representations

## 1. Motivation

Consider this compute-visible data shape:

```slang
struct Data
{
    float4 prefix;
    Texture2D<float4>.Handle texture;
    uint2 result;
}
StructuredBuffer<Data> inputData;
```

CUDA buffer-element lowering deliberately preserves `texture` as
`DescriptorHandle<Texture2D<float4>>`. CUDA's layout rule gives that handle the size, alignment,
and physical representation of its resource type. Before this slice, direct NVVM had executable
representations for the surrounding structured buffer and texture but rejected the descriptor
handle leaf. That one missing classification made the complete `Data` element, raw view, and
synthesized conventional-global block unavailable.

The same gap appeared in helper results, arrays, local aggregates, and exact
`CastDescriptorHandleToResource` instructions. Fixing each fixture or each aggregate boundary
would duplicate one target invariant and make adjacent handle kinds appear supported accidentally.

## 2. Proposed solution

Direct NVVM now selects an exact `DescriptorHandle<T>` only when `T` is already a supported raw
buffer, read-only texture, or sampler value. All accepted type roles lower the handle to
the existing provider type for `T`. Resource alignment, helper values, aggregate storage,
structured-buffer storage, parameter-group storage, and conventional-global classification reuse
the same selected-handle predicate.

The two canonical handle/resource conversion opcodes are accepted only when the opposite semantic
type is exactly `T`. They map to the operand's provider value because CUDA says the representations
are identical. This is a typed identity, not a generic bitcast, integer descriptor encoding, or
fallback.

The change stays compiler-side. Revision-30's existing type, load/store, aggregate, and generic
value operations express the complete representation, so the provider ABI and isolated LLVM 14
implementation do not change.

## 3. Change summary

- `source/slang/slang-emit-nvvm-type-lowering.h/.cpp`
  - define the selected descriptor-handle classifier;
  - thread the handle leaf through recursive storage/helper/resource layout proofs; and
  - alias every legal handle role to its exact underlying resource provider type.
- `source/slang/slang-emit-nvvm.cpp`
  - include selected handles in CUDA/LLVM copyable-layout checks and conventional-global field
    addressing; and
  - validate and emit exact handle/resource conversions as value identities.
- `tests/bugs/gh-6657-bindless-uniform.slang`
  - add permanent direct-NVVM O0 and O3 differential lanes for a handle nested in structured-buffer
    storage.
- Slice 154 census, discovery, measurement-manifest, plan, design, ledger, and this report
  - preserve the separate denominators, record the new Pareto state, and explain every cascade.

## 4. Concepts and vocabulary

- **Descriptor handle:** the canonical `IRDescriptorHandleType<T>` produced for a source
  `T.Handle` value.
- **Selected resource representation:** the exact provider type already established for one
  supported raw buffer, read-only texture, or sampler.
- **Representation identity:** two semantic IR types intentionally use the same provider type and
  bits, so their conversion emits no LLVM instruction.
- **Healthy denominator:** a workload with a stable native CUDA/NVRTC reference that belongs to the
  corpus's correctness denominator.

## 5. Process report

### The canonical CUDA layout rule owns the representation

`slang-ir-lower-buffer-element-type.cpp` preserves resource fields as
`IRDescriptorHandleType(resourceType)` and emits the two descriptor conversion opcodes around uses.
`slang-ir-layout.cpp` then handles `kIROp_DescriptorHandleType` on bindless targets by recursively
asking `resourceType` for its layout. For the motivating discovery shader, that yields the same
64-bit type already selected for `Texture2D`. For a structured-buffer handle, it yields the raw
view aggregate already produced by `_lowerRawBufferType`.

That is a canonical and intentionally allowed shape. Rewriting it to syntax, converting every
handle to `uint64`, or changing buffer-element lowering would discard resource-specific layout
information. The retained classifier instead validates the exact underlying resource with the
existing resource classifiers and returns that semantic type to type lowering.

### One classifier feeds every consumer boundary

Before the change, each recursive algebra stopped at the unknown handle leaf. The new
`asNVVMSupportedDescriptorHandleType` is the single selection boundary. Aggregate and structured
storage treat it as a finite leaf; helper values accept it and derive alignment through the
underlying resource; parameter-group and conventional-global classifiers admit the same leaf; and
`NVVMTypeLoweringContext::lowerType` delegates to `lowerType(T, NVVMTypeUse::Value)`.

The copied-layout predicate compares CUDA and LLVM size/alignment for the handle just as it does for
numeric leaves. This proves that external structured-buffer pointer arithmetic observes the same
stride. No padding type or downstream text repair is introduced.

The discovery test fails without this classifier at the `StructuredBuffer<Data>` field-address
boundary and passes with it. Its direct O0 and O3 lanes prove that the emitter owns this
resource-in-aggregate representation. Adjacent untyped handles and unsupported resource kinds are
still rejected.

### Exact conversions remain semantic identities

Final linked IR for `reinterpret-structured-buffer` contains a loaded
`DescriptorHandle<RWStructuredBuffer<half>>` followed by
`CastDescriptorHandleToResource`. `_getNVVMDescriptorHandleConversion` accepts only one-operand
instances of the two canonical opcodes. Handle-to-resource requires the result type to be the
handle's exact resource type; resource-to-handle requires the operand type to be that exact type.

Preflight still validates availability and dominance of the operand. Emission retrieves the
existing lowered value and maps the conversion result to it. Removing the resolver restores the
unsupported-op diagnostic. A provider callback was rejected because there is no LLVM operation to
perform and no new provider type to construct.

### Cascades remain separate root causes

The six motivating rows were all rerun against native NVRTC and direct NVVM O0/O3:

- `gh-6657-bindless-uniform` becomes correct in both direct modes and is promoted.
- `reinterpret-structured-buffer` advances through the handle field, load, helper ABI, and exact
  conversion. Its next first blocker is canonical
  `bitCast(RWStructuredBuffer<half>, vector<uint,4>)`, produced by raw-view reinterpret transport.
- `optional-descriptor-handle` advances from helper-result rejection to `defaultConstruct` of its
  optional aggregate. Zero/default value production is independent of handle representation.
- The three `layout-descriptor-handle-*` rows still abort before NVVM preflight in
  `slang-ir-extract-value-from-type.cpp`. `AnyValue` packing treats the 16-byte structured-buffer
  descriptor as one leaf, while its bit-extraction helper accepts only 1-, 2-, 4-, or 8-byte
  leaves. This is a producer-side marshalling cluster.

No emitter guard or fixture check was added for those cascades. Each now has an exact producer,
shape, and diagnostic for subsequent Pareto selection.

### Coverage and regression evidence

Frozen corpus v1 remains exactly 452 workloads with 427 healthy MVP references:

| Frozen corpus-v1 metric | Slice 153 | Slice 154 |
|---|---:|---:|
| Direct O0 correct | 380/427 | 380/427 (89.0%) |
| Direct O3 correct | 384/427 | 384/427 (89.9%) |
| Correct in both modes | 380/427 | 380/427 (89.0%) |
| Newly correct in both modes | - | 0 |
| Old-correct regressions | - | 0 |
| Selected NVVM unit prefix | 427/427 | 427/427 |

Across all 452 rows, native NVRTC remains 449 correct and three infrastructure results. Direct O0
is 393 correct, 46 preflight, eight runtime mismatch, and five provider; direct O3 is 398 correct,
46 preflight, and eight runtime mismatch.

Discovery remains exactly 82 workloads with 72 healthy native references:

| Discovery metric | Slice 153 | Slice 154 |
|---|---:|---:|
| Direct O0 correct | 54/72 | 55/72 (76.4%) |
| Direct O3 correct | 54/72 | 55/72 (76.4%) |
| Correct in both modes | 54/72 | 55/72 (76.4%) |
| Newly correct in both modes | - | 1 |
| Old-correct regressions | - | 0 |

Each discovery direct mode contains 55 correct, 18 preflight, one provider, seven infrastructure,
and one runtime-mismatch result. Exact ID/source comparisons prove that neither corpus changed
identity or denominator.

### Exploratory architecture and output evidence

The promoted shader passes all six of its reflection, CPU, and direct regression lanes. The
selected unit prefix passes 427/427. All twelve established measurement gates compile and assemble
through CUDA 12.9 at direct O3 for SM70, SM80, and SM90. The existential-specialization gate
measures 278.6 ms and 1007-byte PTX at direct O3 SM70, versus 376.0 ms and 8946-byte PTX through
NVRTC O3. Timings remain uncontrolled exploratory measurements rather than benchmark claims.

### Self-review inventory

- New classifier: survives. It names one canonical CUDA representation rule, reuses existing exact
  resource classifiers, and is proven by the promoted aggregate-storage workload.
- New conversion resolver: survives. It accepts only the two producer-owned opcodes with exact
  canonical type identity; removing it reproduces the focused preflight failure.
- Storage/helper/legal-role widenings: survive. Each is a consumer of the central classifier and
  no independent handle family is admitted.
- Rejected widening: read-write surface handles. Existing surface values are representable, but no
  motivating test proves descriptor conversion preserves the field-owned storage format required
  by surface operations, so the family remains unsupported.
- Regression directives: survive. The two lanes protect a new semantic combination and are stable
  in both modes.

No compatibility fallback, custom structural equivalence, operand-graph walk, syntax
reconstruction, fixture-name check, malformed-IR patch, provider callback, or ABI revision was
added. The unavailable formatter dependencies are recorded in the plan; the C++ changes were
manually reviewed and `git diff --check` passes.
