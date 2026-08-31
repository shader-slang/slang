# Slice 159: Selected optional-resource default values

## Motivation

Consider these two source shapes:

```slang
Optional<IFoo> value = none;
```

where the only concrete `IFoo` implementation contains a `StructuredBuffer<int>`, and:

```slang
struct DomeLight
{
    DescriptorHandle<Texture2D<float4>> texture;
    DescriptorHandle<SamplerState> sampler;
}

Optional<DomeLight> value = none;
```

Slice 158 transported both resource-bearing aggregates through helpers and explicit construction,
then exposed their common next blocker: lowering `none` retains an exact resource-typed
`IRDefaultConstruct` payload even though the false optional tag makes that payload semantically
irrelevant. Direct NVVM rejected the instruction before provider creation. The frozen corpus had
two healthy rows with this first canonical blocker, making a bounded representation slice more
valuable than another fixture-specific operation.

## Proposed solution

Classify exactly two canonical zero-operand resource-default families. A raw structured buffer
becomes the zero value of the physical raw-view representation already selected by
`_lowerRawBufferType`: a null LLVM global-address-space pointer paired with an i64 zero count. A
selected texture or sampler descriptor handle becomes an i64 zero because Slice 154 established
that `DescriptorHandle<T>` is an exact provider-representation alias of its selected resource
`T`.

The compiler owns this semantic classification and composes provider ABI revision 30's existing
generic operations. The provider gains no default-construction callback. Adjacent defaultable
types remain rejected, and rejection now names the exact result type instead of merging all
defaults under the opcode alone.

## Change summary

- `source/slang/slang-emit-nvvm.cpp` adds one exact default-resource descriptor shared by
  preflight, SSA validation, and emission, plus typed zero/null materialization.
- The two motivating shaders gain stable direct NVVM O0 and O3 comparison lanes.
- Frozen-v1 and discovery census/Pareto snapshots retain their separate exact denominators.
- The measurement manifest grows from 14 to 16 gates so both newly supported semantic
  combinations assemble for SM70, SM80, and SM90.
- The design guide and capability ledger record the representation invariant and metrics.
- An unrelated Slice 158 test-only regression is repaired by restoring the stateful aggregate
  fixture's established optimizer-tolerant minimum-store assertion.

## Concepts and vocabulary

`IRDefaultConstruct` asks lowering to create a value of a type without source operands. In these
optional-none paths it creates an irrelevant payload, but the instruction still needs a valid
typed provider value because the surrounding aggregate is constructed normally.

A raw structured-buffer view is the backend's physical value representation for a Slang raw
buffer: `{global element pointer, i64 count}`. A descriptor handle is a semantic Slang wrapper
whose selected direct-NVVM representation is exactly its underlying resource handle.

## Process report

`OptionalTypeLoweringContext::processMakeOptionalNone` handles the `Optional<DomeLight>` example.
It asks `IRBuilder::emitDefaultConstruct` for the payload and recursively constructs `DomeLight`,
leaving exact `defaultConstruct<DescriptorHandle<Texture2D<float4>>>` and
`defaultConstruct<DescriptorHandle<SamplerState>>` leaves. Those leaves are canonical: the
producer intentionally needs a placeholder for a well-typed false optional and is not losing a
more precise semantic value. Slice 154 already proves each admitted handle is represented by the
underlying selected resource's i64 value, so zero is its correct placeholder representation.

`TypeFlowSpecializationContext::specializeMakeOptionalNone` owns the single-concrete existential
example. It constructs the untagged-union payload through `emitDefaultConstruct`; after later
specialization and aggregate lowering, final linked IR retains
`defaultConstruct<StructuredBuffer<int>>`. `_lowerRawBufferType` is the physical source of truth:
the value is a struct containing an address-space-1 element pointer and an i64 count. The default
recipe creates zero, converts that integer bit pattern to the exact global pointer type, and
constructs the same raw-view type with `{null, 0}`. It does not invent a second buffer ABI.

`_resolveNVVMDefaultResourceValue` is the single classifier used in all three compiler boundaries.
It requires an `IRDefaultConstruct`, zero operands, and either an exact supported structured raw
buffer or an exact supported descriptor handle whose underlying resource is a read-only texture
or sampler. Preflight diagnoses the unsupported result type before provider mutation. SSA
validation marks only that proven instruction available. `_emitNVVMDefaultResourceValue` then
uses typed integer constants, pointer-type creation, integer-to-pointer conversion, and aggregate
construction already present in revision 30. Bare textures and samplers, byte-address buffers,
writable surfaces, arbitrary resource structs, and unrelated default values have no producer
evidence in this slice and are not admitted.

Removing the structured-buffer branch restores the exact first blocker in
`optional-single-concrete-layout.slang`; removing the descriptor-handle branch restores it in
`optional-descriptor-handle.slang`. Both workloads compare correctly with their healthy NVRTC
reference at direct O0 and O3 and gain four permanent lanes. Their full promoted files pass 4/4
with only unsupported platform lanes ignored.

Frozen corpus v1 remains exactly 452 workloads/427 healthy MVP references and improves from
384/388/384 to 386/390/386 O0/O3/both-mode correct (90.4%/91.3%/90.4%). There are zero losses from
either old-correct set. Across all 452 rows, native CUDA is 449 correct/three infrastructure;
direct O0 is 399 correct, 40 preflight, eight runtime mismatch, and five provider; direct O3 is
404 correct, 40 preflight, and eight runtime mismatch.

Discovery remains exactly 82 workloads/72 healthy references and 60/60/60 (83.3%), with zero
old-correct loss and no newly unlocked row. Each direct mode remains 60 correct, 12 preflight, two
provider, seven infrastructure, and one runtime mismatch. The frozen and discovery denominators
are not combined.

The complete selected unit prefix initially exposed an accidental Slice 158 assertion change in
the unrelated stateful aggregate-helper fixture. Slice 158 changed its established
`emitStoreCallCount >= 4` optimizer-tolerant contract to `== 5`, while the current valid lowering
emits four stores. An isolated rerun proved the value and the pre-Slice-158 assertion was restored;
the isolated test and complete prefix then pass 1/1 and 427/427. This changes no compiler behavior
and removes no semantic check.

All sixteen representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90. The
optional structured-buffer gate measures 251.4 ms and 590-byte PTX at direct O3 SM70 versus
361.0 ms and 8570 bytes through NVRTC O3; direct O0 measures 247.9 ms and emits 6968 bytes. The
optional descriptor gate measures 253.1 ms and 645-byte PTX versus 365.2 ms and 8584 bytes; direct
O0 measures 248.0 ms and emits 2198 bytes. These measurements remain exploratory.

The self-review inventory contains one new classifier and one emitter recipe. Both survive: the
two producer traces above prove the input shapes are canonical, the existing physical resource
types remain the source of truth, and the named differential tests prove this layer owns
materialization. There is no custom semantic equivalence, operand-graph walk, syntax
reconstruction, fixture-name check, fallback, emitted-text patch, or provider ABI change.

The repository formatter was attempted but this machine lacks gersemi, clang-format, prettier,
and shfmt. Manual formatting review, `git diff --check`, JSON parsing, and exact TSV row-count
checks pass. The unrelated untracked `external/slang-binaries/` directory remains untouched and
unstaged.
