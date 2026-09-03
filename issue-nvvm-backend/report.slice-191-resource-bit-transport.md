# Slice 191: Generalize canonical CUDA resource bit transport

## Motivation

Consider these operations from two existing frozen-v1 workloads:

```slang
uint2 textureBits = reinterpret<uint2>(foo.texHandle);
RWStructuredBuffer<half2> pairs = reinterpret<RWStructuredBuffer<half2>>(*inputBuffer);
```

AnyValue lowering retained both as canonical `bitCast` instructions. Direct NVVM supported only
`DescriptorHandle<RawBuffer> <-> uint4`, so `anyvalue-layout` stopped on
`uint2 -> DescriptorHandle<Texture2D<float4>>` and `reinterpret-structured-buffer` stopped on
`RWStructuredBuffer<half> -> uint4`. The emitter also resolved its old raw-buffer descriptor case
independently in preflight, operand validation, and emission despite the newer immutable emission
plan.

## Proposed solution

Classify the semantic resource side once during preflight from its already-established physical
CUDA representation. Selected texture, sampler, and surface values are opaque 64-bit handles and
therefore transport through exactly `uint2`; selected raw resources are
`{global element*, uint64 count}` and transport through exactly `uint4`. A supported
`DescriptorHandle<T>` retains T's representation. Store that decision and the complete typed
recipe in `NVVMEmissionPlan`, then make validation and emission consume the plan entry.

The generic typed `BIT_REINTERPRET` family now accepts distinct selected scalar/vector shapes with
the same total bit width. This is the existing LLVM `bitcast` operation, not a new provider
callback. Raw-buffer pointer/count reconstruction continues to use the established Slice 172
recipe.

## Change summary

- `NVVMPlannedResourceBitCast` owns the exact resource kind, semantic source/result relation,
  physical raw-buffer element facts, operands, and recipe steps.
- Preflight recognizes selected opaque-resource `uint2` transport and selected raw-resource
  `uint4` transport for resources and supported descriptor handles; later phases use the plan.
- Typed bit reinterpretation accepts equal-total-width shape changes and has positive bidirectional
  and negative mismatched-width unit coverage.
- `anyvalue-layout` replaces its expected diagnostic with permanent direct O0/O3 runtime lanes.
- Census GenericAsm clustering now uses the emitted assembly diagnostic rather than fixture paths,
  correctly merging the newly exposed Half2 reduction with the existing atomic cluster.
- Frozen-v1, discovery, and measurement artifacts record the resulting coverage. Provider ABI
  revision 34 is unchanged.

## Concepts and vocabulary

**Resource bit transport** is byte-preserving movement between a selected CUDA resource value and
the unsigned word vector used by AnyValue. **Opaque handle** is the established 64-bit CUDA value
for a texture, sampler, or surface. **Raw-buffer view** is the established two-field LLVM aggregate
containing a global data pointer and 64-bit element count. **Emission plan** is the immutable,
source-keyed result of direct-NVVM preflight.

## Process report

Native CUDA for `anyvalue-layout` represents `DescriptorHandle<Texture2D<float4>>` as one
eight-byte `CUtexObject` and `DescriptorHandle<StructuredBuffer<float>>` as the same 16-byte
structured-buffer view as its resource. Its generated AnyValue code uses `uint2` and `uint4`
respectively. Native CUDA for `reinterpret-structured-buffer` first packs the source
`RWStructuredBuffer<half>` into four words and reconstructs the destination
`RWStructuredBuffer<half2>`. These are canonical, producer-owned target representations rather
than accidental alternate IR spellings, so target emission owns the final physical transport.

`_getNVVMResourceBitCastKind` unwraps only a supported descriptor handle and then delegates to the
existing exact resource classifiers. `_resolveNVVMResourceBitCast` requires one selected resource
side and an unsigned i32 vector whose lane count comes from the resource kind. It does not use a
fixture name, infer an arbitrary size, walk syntax, or admit an unsupported resource. Preflight
stores `NVVMPlannedResourceBitCast`; `_validateNVVMFunction` and
`emitNVVMIRFromLinkedIR` find that source record instead of re-running the resolver.

For a raw buffer, the plan records the canonical element type and whether type lowering selected
structured-buffer storage. `_emitNVVMResourceBitCast` then uses the existing pointer/count split
and reconstruction. This is valid for either a resource or its descriptor because
`NVVMTypeLoweringContext::lowerType` already defines `DescriptorHandle<T>` as T's exact CUDA value.
The change does not create another semantic type or rediscover layout downstream.

The construction callback named `emitBitCast` is a pointer/integer transport interface and
correctly rejected the first opaque-resource prototype. The generic value-operation provider
already implements `ValueOperationFamily::BitReinterpret` with LLVM `CreateBitCast`. Its catalog
previously required equal per-lane widths and lane counts. LLVM's actual invariant is equal total
bit width and distinct first-class types, so the catalog now expresses that invariant. The real
`anyvalue-layout` O0/O3 executions prove both UInt2-to-UInt64 and UInt64-to-UInt2 directions;
`nvvmIRBuilderBuildsNumericTypeFamilies` additionally rejects a 96-bit-to-64-bit relation.

After the change, `anyvalue-layout` is correct in both direct modes. The resource reinterpret
workload clears its `RawBuffer -> uint4 -> RawBuffer` operations and deterministically stops at
`StmtLoweringVisitor::visitIntrinsicAsmStmt`'s
`__slang_atomic_reduce_add(RefParam<half2>, half2, int)` GenericAsm. That operation is valid but
independent. It remains unsupported for a later atomic slice. The census summarizer now clusters
GenericAsm from the exact assembly diagnostic; this removes the previous fixture-name dependency
and correctly puts both Half2 workloads in `generic-asm-atomic`.

The self-review inventory contains the resource-kind classifier, resource-bitcast resolver,
planned record/index, equal-total-width catalog rule, and diagnostic-based census classifier. All
survive. Removing the resolver restores both motivating bitcast failures; removing the catalog
widening makes the provider reject opaque scalar/vector transport; removing the plan record would
restore three-phase reclassification. No guard masks malformed IR, no syntax is reconstructed, no
compatibility fallback was added, and the provider ABI remains revision 34.

Frozen corpus v1 remains exactly 452 workloads with 427 healthy MVP references. It advances from
419/419/419 to 420/420/420 for O0/O3/both with one newly correct workload, zero old-correct
regressions, and zero runtime mismatches. Across all 452 rows each direct mode reports 434 correct,
17 preflight failures, and one infrastructure failure. The seven remaining healthy failures are
three helper-ABI type contracts, two identical Half2 atomic GenericAsm workloads, one ordinary
texture GenericAsm, and one `RequirePrelude` marker.

Discovery remains separate at 82 selected workloads and 72 healthy references. It stays
72/72/72 for O0/O3/both with no newly unlocked row. Each direct mode reports 72 correct, two
preflight failures, one runtime mismatch on an unhealthy native reference, and seven infrastructure
failures. The selected NVVM prefix passes 437/437 and the permanent NVVM category expands from
94/94 to 96/96.

The exploratory measurement gate assembles native NVRTC, direct O0 SM70, and direct O3
SM70/SM80/SM90 PTX. Median standalone compilation is 370.6 ms native, 274.3 ms direct O0 SM70,
and 273.7 ms direct O3 SM70. Native PTX is 9,143 bytes; direct O3 is 1,120 bytes at all three
architectures. Direct O0 is 46,395 bytes, a useful productionization signal for later controlled
benchmarking rather than a correctness failure.
