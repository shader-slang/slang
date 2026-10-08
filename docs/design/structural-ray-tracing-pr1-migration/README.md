# PR1 structural ray-tracing migration report

This report describes the changes the full implementation and demos need to adopt from
[PR1](https://github.com/kaizhangNV/slang/pull/24).

Reference: the current PR1 source and the shared-table design documented in this report and
the accompanying source comments. Updated on 2026-10-08. The full implementation comparison point
is `f96ab27c0` from [PR #12691](https://github.com/shader-slang/slang/pull/12691). The shared Metal
layout is agreed design; its backend implementation is deferred.

## 1. Remove open shader-list sections

Removed `OpenHitGroups`, `OpenMissShaders`, and `OpenCallableShaders`. Use explicit variadic lists
or the corresponding empty-list aliases:

```slang
typealias HitGroups = rt::HitGroupList<OpaqueHitGroup, ShadowHitGroup>;
typealias MissShaders = rt::MissShaderList<SkyMiss, ShadowMiss>;
typealias CallableShaders = rt::EmptyCallableShaderList;
```

Remove compiler/linker discovery that automatically includes implementations of a tag interface.
Update demos to list their shaders explicitly. The list interfaces remain sealed.

## 2. Unify attribute access on both closest-hit and any-hit inputs

Both input types now expose:

```slang
public property Context.Primitive.Attributes attributes { get; }
```

Demo migrations:

```text
input.triangle.barycentricCoord -> input.attributes.barycentricCoord
input.triangle.frontFacing      -> input.attributes.frontFacing
input.curve.parameter           -> input.attributes.parameter
```

Procedural `input.attributes` remains unchanged. Backend lowering must distinguish built-in
`TriangleData`/`CurveData` views from application-defined attribute values. Access to a field of
application-defined attributes must remain ordinary field access, even if its name happens to
match a built-in property.

## 3. Make TraceProgramDescriptor an opaque resource

`TraceProgramDescriptor<Schema>` no longer exposes a source-level `.resources` wrapper or
placeholder buffers, and has no public constructor. Demos should receive it through host binding:

```slang
rt::TraceProgramDescriptor<Schema> program;
```

It may also be a field in a host-bound parameter block. An already-bound descriptor can still be
copied or passed to a function; opaque does not mean it cannot flow as a value.

The compiler now represents it as `TraceProgramDescriptorType(schema, schemaWitness)`. Replace
the full implementation's older `StructuralRayTracingProgramDescriptorType(storageType, schema)`
wrapper and storage-field assumptions rather than keeping both representations. Target lowering
creates Metal resources or erases the descriptor on native SBT targets.

The standard-module build now uses `-compile-slang-raytracing-module` to authorize the packaged
module's compiler-owned descriptor declaration.

## 4. Share Metal tables across payload types

**Status: comments/specification updated; backend implementation pending.**

The descriptor has five resources, independent of payload count:

| Metal ID | Resource                           |
| -------- | ---------------------------------- |
| 0        | Intersection function table        |
| 1        | Miss visible-function table        |
| 2        | Closest-hit visible-function table |
| 3        | Callable visible-function table    |
| 4        | Record buffer                      |

Required implementation changes:

- Assign function indices across each entire schema section; do not restart numbering per payload.
- Generate uniform visible-function adapters using `thread void*` and consistent remaining
  parameters. Source stage payloads remain strongly typed.
- Share primitive intersection dispatchers and infer one schema-wide intersection-tag set.
  Candidate payload transport still uses Metal's `ray_data` address space.
- Update descriptor reflection, host construction, and tests together.

Preserve one logical geometry-by-ray-type hit-record table. For instance contribution zero:

| Geometry | Radiance: sbtOffset = 0 | Shadow: sbtOffset = 1 |
| -------- | ----------------------- | --------------------- |
| 0        | Hit record 0            | Hit record 1          |
| 1        | Hit record 2            | Hit record 3          |

Both ray types use `sbtStride = 2`. The Metal record index is:

```text
recordIndex = instanceContribution + geometryIndex * sbtStride + sbtOffset
```

Each record stores its selected hit group's reflected `functionIndex` and application data.
`sbtStride` counts records, not bytes. Function indices identify programs; record indices identify
host-selected positions. Multiple records may refer to the same function index.

Metal's native IFT index selects a primitive dispatcher, which then calculates this logical SBT
index. It is not itself the complete SBT index. Closest-hit and miss use explicit visible-table
dispatch after traversal. Native D3D/Vulkan/OptiX SBT representations remain native; the Metal
layout change does not add an integer function-index field to those native records.

A host abstraction can hide buffer layout and upload, but callers must still identify the chosen
hit group and record data for each geometry/ray-type combination. The record index is calculated
from those coordinates; the function index is looked up from the chosen group using reflection.
That host abstraction was discussed; it has not been implemented.

## 5. Update older demo uses of RayDesc

Compared with the full implementation snapshot, PR1 uses the existing core field names. This
change was already present before the latest review revisions:

```text
ray.origin    -> ray.Origin
ray.direction -> ray.Direction
ray.tMin      -> ray.TMin
ray.tMax      -> ray.TMax
```

Apply this to `desc.ray` and other values of `RayDesc`, including object-space rays.

## 6. Replace the empty-list structs with aliases

The separate `NoHitGroups`, `NoMissShaders`, and `NoCallableShaders` types are removed. Update
their uses to the following aliases:

```slang
public typealias EmptyHitGroupList = HitGroupList<>;
public typealias EmptyMissShaderList = MissShaderList<>;
public typealias EmptyCallableShaderList = CallableShaderList<>;
```

Each alias is the same type as its empty variadic list and inherits `count == 0`. The compiler
and backend should handle empty sections through that empty pack, without a separate type identity.
Direct `HitGroupList<>` and equivalent miss/callable spellings remain valid; a bare generic name
such as `HitGroupList` is not valid type syntax.

Carry over the capability-inference fix in `CapabilityDeclReferenceVisitor`: a type-pack conformance
proof contributes the capabilities of its element proofs. An empty proof contributes none. Visiting
its constraint interface directly incorrectly imposed miss/callable requirements on empty lists.

## Additional API and integration updates

These are additive declarations or behavior corrections rather than the main source migrations:

- **List counts:** `IHitGroupList`, `IMissShaderList`, and `ICallableShaderList` now require
  `static const int count`. Variadic lists provide their pack length; the three empty-list aliases
  therefore provide zero. Generic code can use the count through the interface
  constraint, including `Schema.MissShaders.count`.
- **Intersection payload:** `IntersectionInput<Context>.payload` now gives mutable access to
  `Context.Payload` on Metal and CUDA/OptiX, guarded by
  `structural_raytracing_intersection_payload`. D3D/Vulkan reject use of the property; intersection
  shaders that do not access it remain portable. Empty payloads still cannot be accessed explicitly.
  The full implementation must lower this accessor to Metal's `ray_data` payload and OptiX payload
  state; adapter implementation remains outside PR1.
- **Transforms:** `objectToWorld` and `worldToObject` are common properties on closest-hit,
  any-hit, and intersection inputs. They represent the complete composed instance path; direct
  primitive-AS traversal uses identity.
- **Multilevel instances:** all three hit inputs now expose runtime `instanceCount`,
  `getInstanceIndex(uint level)`, and `getInstanceID(uint level)` for multilevel contexts. They
  also expose static `maxInstanceCount = Context.TraceContext.AccelerationStructure.levelCount - 1`.
  `MultiLevelAccelerationStructure<N>.levelCount` equals `N`, including the primitive-AS leaf.
  Levels run outermost to innermost; require `level < instanceCount`, not merely the static maximum.
  Existing scalar `instanceIndex` and `instanceID` remain specific to the portable two-level type.
  The internal `IMultiLevelAccelerationStructure` now inherits `IAccelerationStructure`, so the
  concrete multilevel handle declares only the derived conformance. The portable handle retains
  an extension on the existing core `RaytracingAccelerationStructure` to preserve its resource type.
- **Unimplemented paths:** Metal getters and operations now diagnose missing synthesis instead
  of returning fabricated values. The full implementation must supply real lowering. CUDA
  `geometryIndex` and `time` also have explicit pending paths. Front-end-only diagnostic tests
  need to be reconciled with implemented backend paths during integration.
- **Capabilities:** preserve ordinary capability inference for callable-stage checks rather than
  restoring the bespoke source call graph. `CurveData` requires Metal 3.1; triangle properties use
  the structural any-hit/closest-hit capability. `IAccelerationStructure` is sealed.

## Decisions that remain unchanged

Keep the existing primitive hierarchy, `IIntersectionStage`, associated-type `Context`, and
generic `NoClosestHit<C>`, `NoAnyHit<C>`, and `NoIntersection<C>`. Their proposed redesigns were
not adopted. Callable-data homogeneity also remains an existing API restriction; sharing a Metal
table does not itself require that restriction.

## References

- [Program schemas and explicit lists](../../../source/standard-modules/raytracing/program-schema.slang)
- [Opaque descriptor and logical SBT layout](../../../source/standard-modules/raytracing/descriptor.slang)
- [Stage inputs](../../../source/standard-modules/raytracing/stage-inputs.slang)
- [Ray, primitive, and acceleration-structure types](../../../source/standard-modules/raytracing/ray-types.slang)
- [PR1 contract tests](../../../tests/ray-tracing-2/frontend/contracts)

The full proposal is in the PR #12691 checkout at
`docs/design/rt-api-workspace/design-sbt-structural-dispatch/PROPOSAL.md`; section 2.2.2 describes
the revised Metal layout. That proposal is not included in this split PR's checkout. Use current
PR1 source for API signatures: older proposal/tutorial examples still need the same API migrations.
