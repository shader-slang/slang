# D3D12 `Record` RHI Implementation

## 1. Summary

The structural ray-tracing `Record` gap crosses the compiler/RHI boundary.

The compiler must lower a non-void `Record` as an ordinary constant buffer with a real D3D binding. Slang RHI must then recognize that this binding is local to a shader record, construct the matching D3D12 local root signature, and place the address of each record's data in the SBT.

These are two independent properties:

| Property                                                                  | Owner     | Purpose                                                                                 |
| ------------------------------------------------------------------------- | --------- | --------------------------------------------------------------------------------------- |
| Constant-buffer type, layout, `bN, spaceM`, HLSL register, and reflection | Compiler  | Gives the generated variable an ordinary D3D resource ABI                               |
| Shader-record role and per-export local root association                  | Slang RHI | Makes the binding read from the selected SBT record instead of the global shader object |

The public structural ray-tracing API does not need to change.

The RHI work is tracked by
[slang-rhi #874](https://github.com/shader-slang/slang-rhi/issues/874).

## 2. What D3D12 Requires

Consider this structural stage:

```slang
struct MaterialRecord
{
    float3 baseColor;
};

struct MaterialClosestHit : IClosestHitShader<MaterialHitContext>
{
    void invoke(ClosestHitInput<MaterialHitContext> input)
    {
        consume(input.record.baseColor);
    }
}
```

For D3D, the generated shader needs the equivalent of:

```hlsl
cbuffer record_0 : register(b0, space1)
{
    MaterialRecord record_0;
}
```

That declaration alone still describes an ordinary constant-buffer binding. D3D12 does not know whether its value comes from the globally bound shader object or from an SBT entry.

The pipeline therefore also needs a local root signature containing a root CBV for `b0, space1`, associated with the native shader export. The corresponding SBT record contains a GPU virtual address for that CBV:

```text
native SBT entry
├── 32-byte shader identifier
└── 8-byte GPU address of this entry's MaterialRecord data
```

The application still supplies ordinary `MaterialRecord` bytes. Slang RHI owns the D3D-specific transformation from those bytes to a constant-buffer allocation and an SBT-local GPU address.

## 3. The Original Failure

The initial compiler lowering gave the generated variable only the `ShaderRecord` layout category. It did not pass the variable through ordinary resource-binding allocation.

The resulting HLSL was effectively:

```hlsl
cbuffer record_0
{
    MaterialRecord record_0;
}
```

This caused a chain of failures:

```text
no ordinary binding allocation
    ↓
no stable b-register and space in HLSL or reflection
    ↓
RHI cannot describe the binding in a local root signature
    ↓
SBT data cannot be exposed to the shader as ConstantBuffer<Record>
```

Adding only the missing HLSL register is not sufficient. The RHI must also create and associate the local root signature and must convert the application bytes into the native SBT representation.

## 4. Compiler-to-RHI Contract

For every selected structural stage, reflection provides:

- the `Record` type;
- its target layout;
- the D3D constant-buffer register;
- the D3D register space; and
- the final native export name.

For a void `Record`, the binding indices remain invalid and no record storage is required.

The compiler reserves these bindings in an internal D3D register space so that generated record buffers do not collide with user-declared resources. The binding is still reflected as an ordinary constant buffer; the structural metadata adds the shader-record role without replacing that ordinary binding information.

## 5. RHI Pipeline Construction

Slang RHI reads the reflected contract for each selected stage and records:

```text
export name → { Record byte size, b-register, register space }
```

Miss and callable shaders are selected directly by their export names, so each non-void structural export can be associated with its own one-CBV local root signature.

Hit shaders require additional handling because D3D12 selects a hit-group export rather than an individual closest-hit, any-hit, or intersection export.

### 5.1 One hit group

Suppose the compiler assigned:

```text
ClosestHitA    → b0, space2
AnyHitA        → b1, space2
```

and the native hit group contains both stages. The hit group's local root signature must contain both root CBVs:

```text
HitGroupA local root signature
├── CBV(b0, space2)
└── CBV(b1, space2)
```

Both root parameters receive the same record-data address. The separate CBVs exist because independently reflected stage exports can use different compiler-reserved bindings, not because the hit group has two application records.

### 5.2 Hit groups that share a stage

D3D12 requires a shader export to have one compatible local-root association everywhere it is used. The [DXR functional specification](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#subobject-associations-for-hit-groups) states that a hit-group association applies to its component shaders and that group/component associations must match. Consider:

```text
ClosestHitA    → b0, space2
ClosestHitB    → b1, space2
SharedAnyHit   → b2, space2

HitGroupA = ClosestHitA + SharedAnyHit
HitGroupB = ClosestHitB + SharedAnyHit
```

Constructing each group independently would produce:

```text
HitGroupA → { b0, b2 }
HitGroupB → { b1, b2 }
```

That is invalid because `SharedAnyHit` would participate in two different local root signatures.

The RHI instead treats hit groups as a graph. Two groups are connected when they reuse any closest-hit, any-hit, or intersection export. It computes the union of bindings for each connected component and assigns the same signature to every group in that component:

```text
component { HitGroupA, HitGroupB }
└── shared local root signature { b0, b1, b2 }
```

Each native SBT entry for either group stores the same record-data address three times, once for each root CBV. As specified under [local root signatures versus global root signatures](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#local-root-signatures-vs-global-root-signatures), DXR local root signatures are exempt from the ordinary 64-DWORD root-signature limit. Their local-argument footprint is bounded by the 4,096-byte maximum shader-record stride minus the 32-byte shader identifier, so this representation can contain at most 508 root-CBV addresses.

Selected structural hit-stage exports that are not used by any hit group are associated directly with their own local root signature. Stages used by a group are associated only through the group/component signature; adding a conflicting direct association would be invalid.

### 5.3 Groups that only inherit the component signature

A connected component can include a native hit group whose own stages do not use a structural
`Record`. For example, it can share a legacy, record-free _AnyHit_ export with another hit group
whose structural _ClosestHit_ uses a generated CBV. DXR still requires both hit groups to use the
same component-wide local root signature.

This does not turn the record-free hit group into a structural-record consumer. DXR only requires
an individual local-root argument to be initialized when the executing shader references it. The
RHI therefore distinguishes these cases explicitly:

| Hit-group contract                                    | `ShaderRecordData` interpretation                                      |
| ----------------------------------------------------- | ---------------------------------------------------------------------- |
| At least one stage has a non-void structural `Record` | Reflected application data placed in RHI-owned constant-buffer storage |
| The group only inherits the component signature       | Legacy raw bytes kept inline after the shader identifier               |

The distinction cannot be inferred from byte size because a non-void `Record` can have a zero-byte
target layout. The RHI carries an explicit `hasStructuralRecord` bit from reflected stage metadata.
It also rejects a legacy `ShaderRecordOverwrite` for an actual structural record instead of letting
an absolute overwrite silently corrupt the generated root-CBV address.

## 6. RHI Shader-Table Construction

The structural API exposes one logical application record. The D3D12 backend materializes it as follows:

```text
application ShaderRecordData bytes
    ↓
RHI-owned, zero-padded, 256-byte-aligned constant-buffer storage
    ↓
GPU virtual address of that allocation
    ↓
native SBT local-root arguments
```

For a hit-group component with several compiler-reserved bindings, the same GPU address is repeated for every root CBV. The backing buffer remains alive with the ray-tracing pipeline data and is transitioned to constant-buffer state before dispatch.

This indirection is intentional. Writing the application bytes directly after the shader identifier would describe root constants, not the `ConstantBuffer<Record>` ABI generated by the compiler. A root CBV also permits records larger than the inline DXR shader-record payload limit, subject to D3D12 constant-buffer access limits.

## 7. Separate Entry-Point Compilation

Runtime validation also exposed an independent, pre-existing Slang RHI defect. Under `SeparateEntryPointCompilation`, the RHI composed each entry point with the global scope but did not call `link()` on the composite. Target compilation could consequently report unresolved external symbols before D3D12 pipeline creation.

The correct sequence is:

```text
global scope + one selected entry point
    ↓ createCompositeComponentType
composed component
    ↓ link
linked per-entry component
    ↓ reflect and emit
native shader library
```

This is tracked as [slang-rhi #870](https://github.com/shader-slang/slang-rhi/issues/870). It is not caused by the structural API, but the structural D3D12 test using separately compiled stages depends on the fix.

## 8. Why This Belongs in Slang RHI

The compiler owns language semantics, target layout, resource binding, and reflection. It should not construct D3D12 state-object subobjects or allocate GPU memory.

Slang RHI already owns:

- D3D12 global and local root signatures;
- state-object export associations;
- shader-table layout and population;
- GPU buffer allocation and resource-state transitions; and
- the mapping from portable `ShaderRecordData` to the native SBT.

Therefore the compiler should expose the ordinary binding plus structural metadata, while Slang RHI performs the D3D12 local-root and SBT materialization.

## 9. Validation Required

The implementation needs coverage for:

- a non-void closest-hit `Record` on D3D12;
- different application bytes for repeated records using the same shader;
- hit groups containing more than one structural stage;
- two hit groups sharing a closest-hit, any-hit, or intersection export;
- miss and callable records;
- separately compiled entry points;
- a record larger than the inline DXR SBT payload limit;
- coexistence with user resources at ordinary `bN, spaceM` bindings;
- reflection of the generated constant-buffer register, space, and layout; and
- void `Record`, which must not allocate a local-record binding.
- a record-free hit group that inherits another group's component signature while retaining legacy
  inline record bytes; and
- rejection of a legacy native-record overwrite on an actual structural `Record`.

The expected end state is simple from the user's perspective: `input.record` behaves as a typed per-SBT-entry constant buffer, and the RHI hides all D3D12 local-root bookkeeping.
