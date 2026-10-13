# PR1 structural ray-tracing migration report

This report describes the changes the full implementation and demos need to adopt from
[PR1](https://github.com/kaizhangNV/slang/pull/24).

Reference: the current PR1 source and the shared-table design documented in this report and
the accompanying source comments. Updated on 2026-10-12. The full implementation comparison point
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

The compiler IR represents it as `TraceProgramDescriptor(schema, schemaWitness)`. Replace
the full implementation's older `StructuralRayTracingProgramDescriptorType(storageType, schema)`
wrapper and storage-field assumptions rather than keeping both representations. Target lowering
creates Metal resources or erases the descriptor on native SBT targets.

The module uses ordinary Slang module compilation. Its descriptor declaration now uses
`__magic_type(TraceProgramDescriptorType)` for its dedicated AST type and retains
`__intrinsic_type(TraceProgramDescriptor)` for its existing IR mapping. Opaque-type checks use
`as<TraceProgramDescriptorType>` directly. Remove `isStructuralRayTracingOpaqueHandleType` and
the shared intrinsic-op helper introduced for it; the original helper stays private to
`slang-check-shader.cpp`, with no PR change to `slang-check-type.cpp`.

The descriptor's checked declaration reference remains the source of its generic arguments.
Normal intrinsic-type lowering preserves the schema and its conformance witness in IR; the
dedicated AST class does not introduce a second schema representation or a target resource layout.
`isUniformParameterType` also recognizes the class directly, preserving host-bound entry parameters
without requiring an explicit `uniform` modifier. The new `BuiltinType` no longer uses the previous
ordinary-struct fallback for that behavior.
There is no dedicated descriptor build option. Section 9 records the agreed builtin-module loading
design, which still needs implementation.

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

## 7. Recognize stage contracts through their AST types

The RT library's stage interfaces declare their compiler identity with `__magic_type`, for example:

```slang
// Inside slang.raytracing, in namespace rt:
__magic_type(MissShaderType)
public interface IMissShader
{
    [KnownBuiltin(rt::__RayTracingBuiltin.StageContext)]
    associatedtype Context : IPayloadContext;
    void invoke(in MissInput<Context> input);
}
```

The checked type is a `MissShaderType`; the declaration remains an ordinary interface with normal
conformance witnesses. Ordinary interface lowering emits its stage-specific IR interface opcode.
The five executable stage interfaces and the descriptor handle have dedicated AST classes.
The descriptor uses `TraceProgramDescriptorType`; it is not a stage interface. Context, primitive,
input, list, schema, and tracer declarations remain ordinary Slang types. `KnownBuiltin` annotations mark
only compiler-consumed associated requirements, the five stage-input structs, the shader-list
containers, and the two built-in primitive attribute models. Preserve these declaration annotations through ordinary serialization;
do not introduce a special AST class for each supporting library type.

`isRayTracingBuiltinTypeModifier` permits stage identities on interface declarations and the
descriptor identity on struct declarations. Both use the existing `DeclRefType::create` construction
path with one inherited declaration reference, so ordinary substitution and serialization preserve
their generic arguments. No new AST constructor or IR lowering path is needed for the descriptor.

The annotation constants now belong to the ray-tracing module as `rt::__RayTracingBuiltin`.
The module's `compiler-intrinsics.slang` also declares the internal callable attribute
`__RayTracingCallShader`. Core no longer declares this RT-only name. Its C++ attribute class
and checker stay in the compiler. Remove `__RayTracingTrace` and its C++ attribute class;
public trace methods need no trace-specific annotation.

Build `raytracing-builtins.meta.slang` into the ordinary `raytracing-builtins.slang` include using
`slang-standard-module-generator`. Its values come directly from `KnownBuiltinDeclName` in the
shared C++ header; do not copy numeric IDs into the module. Ship this generated include with the
other RT source files so source compilation and serialized imports use the same declarations.

Remove the old RT-specific declaration discovery and IR opcode replacement. Stage checking reads
the declaration annotations preserved through source imports and serialized modules; an
unannotated interface named `IMissShader` remains an ordinary interface. Metal trace calls lower
to an explicit IR instruction, as described in section 11. Callable dispatch still uses its
source-operation IR decoration. Builtin loading and ownership are separate responsibilities,
described in section 9.

Application modules implement or refine the RT library's stage interfaces and use
`rt::TraceProgramDescriptor` directly. Tests check these references through source and serialized
application-helper imports instead of declaring replacement compiler types in the fixtures.

The RT library's magic types retain their checked declaration references, including generic
arguments. Resolve a stage's associated types and `invoke` implementation from its selected
conformance witness so generic specializations retain their own identities. The RT-specific
restriction on collecting builtin-registration entries has been removed; existing registration
can be used when the RT library has builtin ownership.

Do not keep a stage-implementation registry on `Linkage` or cache stage identity on an `invoke`
method. One generic method can implement both an ordinary type and a shader type. Direct-call
validation uses the actual receiver's conformance and its selected implementation. Module checking
validates stage capabilities and input parameters through the completed conformance witnesses.
Representation diagnostics use the semantic checker's existing duplicate suppression. The first
structural and legacy shader declarations are retained on the module declaration so later
entry-point selection can enforce the same-module mixing restriction, including after serialization.

## 8. Centralize special rules and allow ordinary metadata/payload values

All additional semantic rules and declaration-identity queries live in
`source/slang/slang-check-structural-ray-tracing.cpp`, with examples beside the rule implementations.
Conformance, expression, entry-point, and code-generation boundaries call into that file.
The ordinary library constraints remain the source of truth for type relationships. The semantic
rules do not require a separate registry of stage implementations. Library loading and builtin
registration should use the existing builtin mechanisms described in section 9.

Schemas, hit groups, and shader lists can be constructed, stored, and used by ordinary generics.
A value such as `MySchema schema = {};` neither constructs shader instances nor allocates an SBT.
Stage/input construction, storage, writable parameters, returned values, and arbitrary generic
arguments retain their checks. The generic restriction prevents a deferred generic body such as
`make<T>() { T local = T(); }` from manufacturing an input after frontend checking. Supporting
broader type-only uses would require validating the eventual specialized body.

An empty struct is a valid explicit payload. Allow both `trace(..., emptyPayload)` and
`input.payload` when its type is empty. Remove the old explicit-empty diagnostics and property
annotations from the full implementation. Backend adapters must preserve the native payload
representation needed by the existing target legalization passes.

Both no-payload overloads forward through an ordinary library call:

```slang
internal struct EmptyTracePayload {}

// Inside the appropriate RayTracer<Schema> extension:
public void trace(RayTraversalDesc desc,
    Schema.TraceContext.AccelerationStructure scene,
    TraceProgramDescriptor<Schema> program)
{
    EmptyTracePayload payload = {};
    trace(desc, scene, program, payload);
}
```

Both wrappers and payload-taking implementations are ordinary library methods without trace
annotations. Both public call forms reach the same payload implementation. Remove
`__RayTracingTrace`, `__RayTracingPayload`, the explicit/implicit method classifier, and the separate
implicit-payload IR operation kind from the full implementation. Only `CallShader = 2` remains
in the source-operation decoration enum; values 0 and 1 are unused. Rebuild serialized RT modules
after this change.

Post-link lowering must normalize the internal empty value with the reached stages' application
empty payload types. The frontend neither selects a unique application empty type nor inserts an
argument. That normalization and native adapters remain work for the full implementation.

Only shader declarations choose the module's API family; dispatch calls do not. A structural stage
conformance or a function parameter using a structural stage input records structural use. A
native hit, miss, intersection or callable entry records legacy use, including an entry selected
later through the command line. Declaring both families in one module remains an error.

Remove the call-level API bookkeeping, including name recognition for core `TraceRay`,
`TraceMotionRay` and `CallShader`. A structural shader may call a native tracing function, and
a native shader may call an ordinary structural trace wrapper, without the call itself recording
a second family. Imports and helper layers therefore do not change the family classification.
Keep the independent stage/input rules, capability checks and unfinished-backend guards. The
post-link check still rejects a reachable Metal trace instruction; descriptor and stage-adapter
checks still block incomplete native paths.

## 9. Load RT as an optional builtin module

**Status: agreed design; global-session loading and caching are not yet implemented.**

Compile the RT library separately from core so it does not increase core compilation time. Load
it only on the first `import slang.raytracing`, and reuse the loaded module for subsequent imports
in the same global session. An application that does not import RT should not load it.

The global session should own the RT declarations, as it owns the other builtin declarations.
The existing builtin-registration machinery can then store them with the correct lifetime.
Keep the declarations and extensions out of the application's scope until it imports RT.
Builtin ownership does not imply automatic visibility.

The current cleanup restores the original builtin-registration collection function. The current
module loader still uses ordinary module ownership; moving RT to builtin ownership remains
integration work. Do not introduce a separate RT stage-implementation registry for that work.

## 10. Share trace overloads across handles and enabled motion types

`RayTracer<Schema>` has four `trace` overloads: a payload-taking method and a no-payload wrapper
for each of two extensions. One requires `Schema.TraceContext.Motion == NoMotion`; the other
requires `Schema.TraceContext.Motion : IEnabledRayMotion`. The enabled-motion extension accepts
`PrimitiveMotion`, `InstanceMotion`, and `PrimitiveAndInstanceMotion`, and passes `desc.time` to
the target implementation. Stage-input `time` accessors already use this same interface constraint.

Keep those three concrete motion types. They retain their target requirements and tell Metal
lowering which motion settings to use. Sharing the method body does not erase the exact type
from `Schema.TraceContext.Motion`.

Both native and multilevel acceleration structures use the same parameter:

```slang
Schema.TraceContext.AccelerationStructure accelerationStructure
```

That parameter already requires the supplied scene to have the schema's acceleration-structure
type. Remove the separate multilevel overloads, their `maxLevelCount` method parameter, and the
native-handle equality constraints. Callers use `tracer.trace(desc, scene, program, payload)` for
either handle type; they do not supply a separate depth argument. The depth remains part of a
multilevel schema's `MultiLevelAccelerationStructure<N>` type.

The native branches pass `bit_cast<AccelerationStructure>(accelerationStructure)` to `TraceRay`
or `TraceMotionRay`, whose parameter is concrete. On their supported targets the schema handle
is already `AccelerationStructure`, so the cast preserves the same handle. Metal uses its own
branch, where multilevel traversal will be implemented. This does not add native multilevel
support.

The CUDA motion branch now contains
`static_assert(false, "CUDA structural ray-tracing motion is not implemented")`.
Its pending OptiX implementation can no longer silently do nothing. Core `TraceMotionRay` currently
supports GLSL/SPIR-V, while the CUDA `TraceRay` wrapper fixes time to zero and cannot implement a
motion call. The current descriptor guard can diagnose first, before the compiler checks this
assertion; the explicit failure remains necessary when descriptor lowering is implemented.

`MultiLevelAccelerationStructure<N>` retains its Metal requirement. There is a separate compiler
[capability-checking bug](https://github.com/shader-slang/slang/issues/13579) where a generic type's
requirement can be missed; some invalid target uses can therefore pass frontend checking. This
missing diagnostic does not require duplicate API overloads. The general capability fix is outside
PR1. The multilevel HLSL negative test is disabled with a reference to that issue until the
capability check is fixed; it is no longer listed as an expected failure.

## 11. Lower Metal trace calls through one explicit IR operation

The public API still has the same two trace extensions and four overloads. Their Metal branches
now call internal `__metalTraceRay<Schema, Payload>` or `__metalTraceRayMotion<Schema, Payload>`.
Both declarations use the existing `__intrinsic_op(metalTraceRay)` modifier. Ordinary intrinsic-call
lowering emits `IRMetalTraceRay`; no new parser rule or AST lowering special case is needed.

For example, a no-payload motion call forwards an `EmptyTracePayload` to the ordinary payload
overload. Its Metal branch then passes the ray fields, descriptor, payload and time to the motion
intrinsic. The resulting instruction has these operands, in order:

```text
scene, descriptor, rayFlags, instanceMask,
sbtOffset, sbtStride, missIndex,
rayOrigin, rayTMin, rayDirection, rayTMax, payload[, time]
```

There are 12 operands without motion and 13 with motion. An absent `time` means no motion;
a present `time` of zero still requests motion sampling at zero. Preserve that distinction rather
than substituting a zero time for the no-motion form. The descriptor's existing IR type preserves
the schema and its conformance witness, including the exact motion and acceleration-structure
types. The payload operand is an ordinary typed `inout` pointer. Later Metal lowering can use
those types directly without reconstructing generic arguments from a callee or a second schema
operand.

Remove the trace source-operation decoration and its `Trace` enum value from the full
implementation, together with `__RayTracingTrace`. Shader declarations now choose the API family,
so the library's native `TraceRay` calls need no annotation-based exemption. Keep
`__RayTracingCallShader` for its callable IR decoration and existing generic-argument exception;
it has no API-family role. The serialized name for `metalTraceRay` has stable ID 912.

After `linkIR` selects target-switch branches, the centralized unfinished-operation check walks
functions reachable from entry points and looks for the actual `IRMetalTraceRay` instruction.
Unused functions do not cause a trace diagnostic, and native branches do not acquire a Metal
operation. Descriptor layout and native stage-adapter guards still prevent incomplete native
paths from generating code. A later backend pass must consume this operation and generate the
Metal intersector dispatch, shared tables, adapters and payload transport; that synthesis is
deferred.

## Additional API and integration updates

These are additive declarations or behavior corrections rather than the main source migrations:

- **List counts:** `IHitGroupList`, `IMissShaderList`, and `ICallableShaderList` now require
  `static const int count`. Variadic lists provide their pack length; the three empty-list aliases
  therefore provide zero. Generic code can use the count through the interface
  constraint, including `Schema.MissShaders.count`.
- **Intersection payload:** `IntersectionInput<Context>.payload` now gives mutable access to
  `Context.Payload` on Metal and CUDA/OptiX, guarded by
  `structural_raytracing_intersection_payload`. D3D/Vulkan reject use of the property; intersection
  shaders that do not access it remain portable. Empty payloads may be accessed explicitly.
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
  of returning fabricated values. The full implementation must supply real lowering. CUDA motion
  trace has an explicit pending-implementation assertion; `geometryIndex` and `time` also have
  explicit pending paths. Front-end-only diagnostic tests
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
