# Slice 161: Parameter-group value transport

## Motivation

Direct NVVM already represented a loaded `ParameterBlock<T>` or `ConstantBuffer<T>` as an
immutable global pointer to the canonical storage for `T`. That representation stopped at the
conventional collected-global path. Consider the nested helper graph in
`nested-parameter-block-3.slang`:

```slang
struct MaterialSystem
{
    ParameterBlock<Material> material;
    uint4 getValue() { return material.getValue(); }
}

struct Scene
{
    ParameterBlock<MaterialSystem> materialSystem;
    uint4 getValue() { return materialSystem.getValue(); }
}
```

Specialization produces a finite first-class `Scene` value whose parameter-group fields are
pointer-valued resource leaves. The helper ABI already transports the surrounding struct by value,
but the resource-struct classifier did not admit those leaves and the emitter did not recognize
the canonical `IRFieldExtract` that produces the nested pointer.

The second gap appears directly at the launch boundary in
`generic-shader-object-cbuffer2.slang`:

```slang
[numthreads(1, 1, 1)]
void computeMain(
    uniform ParameterBlock<TestParams<Elem, FooImpl>> params,
    RWStructuredBuffer<float> outputBuffer)
{
    outputBuffer[0] = params.element.getElementVal() + params.foo.getVal();
}
```

Here the selected entry `IRParam` is already the same global pointer value. Preflight rejected the
parameter-group type before the provider could express it. Once that real ABI gap was removed, the
workload reached an older bring-up gate requiring explicit `IRCudaKernelDecoration`, even though
an ordinary `[numthreads]` compute entry has canonical `IREntryPointDecoration` and the direct
emitter already marks it as a kernel.

## Proposed solution

Make a selected parameter group one exact pointer-valued resource leaf. Admit it only when its
element has a recursively finite parameter-group storage representation that is identical to its
ordinary loaded value representation. Reuse the existing global-pointer type and generic provider
operations; do not flatten the wrapper, copy the element at the boundary, or add a provider
callback.

Recognize the three canonical pointer producers observed in final IR:

- a raw entry `IRParam` with selected parameter-group type;
- an exact `IRFieldExtract` of a parameter-group field from a supported first-class struct;
- the existing conventional `IRLoad` of an exact `IRFieldAddress`.

Remove the explicit CUDA-kernel-decoration preflight restriction. Kernel identity belongs to the
selected ordinary entry-point decoration, while `[CUDAKernel]` is only another source spelling.
Keep kernel emission unchanged: the direct backend still calls the provider's kernel-marking
operation for the selected entry.

## Change summary

- `slang-emit-nvvm-type-lowering.cpp` includes parameter groups in recursive resource-value
  alignment, storage/value-identity checking, entry-parameter legality, and role lowering.
- `slang-emit-nvvm.cpp` resolves the three exact parameter-group pointer producers, validates the
  entry pointee's recursive CUDA/LLVM layout, and removes the obsolete explicit-decoration gate.
- The fake-emitter unit converts the old ordinary-compute rejection into a positive raw-parameter
  and kernel-marking contract. The source diagnostic test becomes a positive ordinary compute
  smoke test.
- `nested-parameter-block-3.slang` and `generic-shader-object-cbuffer2.slang` gain permanent direct
  NVVM O0 and O3 differential lanes.
- Frozen and discovery census/Pareto artifacts remain separate. The representative measurement
  manifest grows from 17 to 19 gates.
- The design document and capability ledger record the durable representation and coverage state.

## Concepts and vocabulary

**Parameter-group value** means the selected `ParameterBlock<T>` or `ConstantBuffer<T>` SSA value.
Its provider representation is a global pointer to the canonical storage representation of `T`;
the wrapper is not a first-class LLVM aggregate.

**Storage/value identity** means that recursively lowering `T` for parameter-group storage gives
the same provider type and layout used when `T` is loaded as an ordinary value. A parameter group
is not admitted when a conversion would be required.

**Canonical pointer producer** means one of the exact final-IR instructions that owns this pointer
representation at a semantic boundary. Recognizing the producer is stricter than accepting any
SSA value whose result type happens to be a parameter group.

**Ordinary compute entry** means a selected function carrying `IREntryPointDecoration` from a
normal `[numthreads]` shader entry. It does not need the separate `IRCudaKernelDecoration` emitted
for an explicit `[CUDAKernel]` source attribute.

## Process report

The first change extends the existing parameter-group storage/value predicate instead of adding a
second resource representation. `_getNVVMResourceValueAlignment` now detects a selected parameter
group, enters it in the same active-type set used by recursive resource structs, and asks
`_hasNVVMParameterGroupStorageValueRepresentation` to prove its element. The predicate previously
accepted a nested parameter group as an unconditional leaf. It now recursively verifies that
group's element with the shared active set. This makes a parameter-group/resource-struct cycle a
deterministic rejection and yields pointer alignment only after the complete pointee contract is
proved.

That exact predicate is reused by `isNVVMSupportedParameterType` and
`NVVMTypeLoweringContext::lowerType` for `EntryPointParameter`. The resulting type is still the
global pointer constructed by `_lowerParameterGroupType`; the raw entry is not an aggregate
`byval` parameter and does not share Slice 160's physical-pointer/semantic-value split.
`_validateNVVMFunction` additionally calls `_hasNVVMCompatibleAggregateStorageLayout` on the
element type. This is the right layer for the check because a CUDA launch passes the pointer to
the element's physical storage, and the existing recursive layout checker is the source of truth
for CUDA/LLVM size and field-offset agreement.

The first probe of `nested-parameter-block-3` then failed at a device-scalar-pointer load produced
by `IRFieldExtract`. The exact shape is canonical: specialization leaves
`Scene.materialSystem` as a parameter-group field in the first-class helper struct, and ordinary
field extraction produces its pointer value. `_getNVVMParameterGroupPointer` therefore reuses
`_getNVVMStructFieldValue` to prove the field and requires the declared field type to equal the
result's selected parameter-group type. Removing this branch reproduces that exact preflight
failure, and the promoted nested-method test proves this layer owns it.

The generic entry workload exposed the corresponding `IRParam` shape. The producer is the
selected compute entry itself, and role lowering has already proved the parameter type and pointee
layout before value validation. The resolver accepts that exact producer. The existing
`IRLoad(IRFieldAddress)` branch remains the conventional-global case and retains its exact keyed
field/type proof. No operand walk, syntax reconstruction, name test, or fallback is involved.

After these changes, `generic-shader-object-cbuffer2` reached `CUDA kernel decoration`. Auditing
the producers showed this was an accidental alternative-spelling requirement. The CUDA source
emitter consumes ordinary `IREntryPointDecoration` as `extern "C" __global__`, while
`IRCudaKernelDecoration` is produced for explicit `[CUDAKernel]`. Direct NVVM selects the same
ordinary entry and unconditionally marks it as a kernel. The preflight scan for executable
instructions, raw parameters, conventional globals, and explicit decoration therefore duplicated
kernel identity and rejected a valid canonical entry. The scan and gate were deleted rather than
adding a compatibility decoration upstream. The fake test now proves one ordinary raw parameter
reaches module creation and exactly one kernel-marking call, while
`nvvm-ordinary-compute-entry.slang` proves the real barrier-bearing path.

The self-review inventory contains two generalized helpers/conditions and one removed special
case. `_hasNVVMParameterGroupStorageValueRepresentation` survives because it is the existing
canonical storage/value identity predicate, now made recursive and cycle-safe.
`_getNVVMParameterGroupPointer` survives because each branch names a canonical producer and checks
the exact type relation; deleting either new branch reproduces one selected workload's first
unsupported shape. The explicit-decoration special case is removed because its input-shape audit
showed ordinary entry-point decoration is already the producer-side source of truth. Immutable
`BorrowInParam<T>`, address-space conversion, pointer-to-pointer helper ABI, and parameter groups
whose element requires storage/value conversion remain rejected for later principled slices.

Validation used the Release build produced outside the sandbox. Both promoted CUDA differential
lanes pass at O0 and O3, the ordinary-entry smoke test passes, and the selected NVVM prefix passes
427/427. Frozen corpus v1 remains exactly 452 workloads/427 healthy references at 390 O0, 394 O3,
and 390 both-mode correct with zero old-correct loss. Across all frozen rows, native CUDA is 449
correct and three infrastructure; direct O0 is 403 correct, 36 preflight, eight runtime mismatch,
and five provider; direct O3 is 408 correct, 36 preflight, and eight runtime mismatch.

Discovery remains exactly 82 workloads/72 healthy references and improves from 61/61/61 to
63/63/63 O0/O3/both-mode correct. The newly unlocked rows are
`bindings/nested-parameter-block-3.slang#discovery-1` and
`language-feature/generics/generic-shader-object-cbuffer2.slang#discovery-1`; there are no
old-correct losses. Each direct mode reports 63 correct, nine preflight, two provider, seven
infrastructure, and one runtime mismatch.

All 19 representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90. The
nested parameter-group method gate measures 242.5 ms and 1093-byte PTX at direct O3 SM70 versus
361.6 ms and 9096 bytes through NVRTC O3; direct O0 measures 237.1 ms and emits 4513-byte PTX. The
generic entry gate measures 237.9 ms and 847-byte PTX versus 357.8 ms and 8897 bytes; direct O0
measures 235.3 ms and emits 4592-byte PTX. These timings remain exploratory rather than a
controlled benchmark. Provider ABI revision 30 and both corpus denominators are unchanged.
