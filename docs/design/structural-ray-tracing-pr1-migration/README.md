# PR1 structural ray-tracing migration report

This report describes the changes the full implementation and demos need to adopt from
[PR1](https://github.com/kaizhangNV/slang/pull/24).

Reference: the current PR1 source and the shared-table design documented in this report and
the accompanying source comments. Updated on 2026-10-20. The full implementation comparison point
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
    static void invoke(in MissInput<Context> input);
}
```

The checked type is a `MissShaderType`; the declaration remains an ordinary interface with normal
conformance witnesses. All five stage contracts lower to ordinary `IRInterfaceType` instructions
with their associated-type and function requirements. Generic conformances and inheritance witnesses
use ordinary interface lowering and module serialization.
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

The annotation constants belong to the ray-tracing module as `rt::__RayTracingBuiltin`.
The module's `compiler-intrinsics.slang` includes the generated constants. Public trace and
callable methods are ordinary functions without operation-specific source attributes.

Build `raytracing-builtins.meta.slang` into the ordinary `raytracing-builtins.slang` include using
`slang-standard-module-generator`. Its values come directly from `KnownBuiltinDeclName` in the
shared C++ header; do not copy numeric IDs into the module. Ship this generated include with the
other RT source files so source compilation and serialized imports use the same declarations.

Stage checking reads the declaration annotations preserved through source imports and serialized
modules; an unannotated interface named `IMissShader` remains an ordinary interface. Metal trace
calls lower to an explicit IR instruction, as described in section 11. Metal callable dispatch
uses a static assertion until its implementation is available. Builtin loading and ownership are
separate responsibilities, described in section 9.

Application modules implement or refine the RT library's stage interfaces and use
`rt::TraceProgramDescriptor` directly. Tests check these references through source and serialized
application-helper imports instead of declaring replacement compiler types in the fixtures.

The RT library's magic types retain their checked declaration references, including generic
arguments. Entry selection obtains `invoke` from the selected conformance witness and preserves
its checked function reference. Later lowering uses the method's substituted input signature. The RT-specific
restriction on collecting builtin-registration entries has been removed; existing registration
can be used when the RT library has builtin ownership.

Do not keep a stage-implementation registry on `Linkage` or cache stage identity on an `invoke`
method. One generic method can implement both an ordinary type and a shader type. Ordinary calls
can forward an existing stage input to either implementation. Module checking validates stage
capabilities and input parameters through the completed conformance witnesses. For a refined
interface, its checked inheritance facets already identify the stage-interface path. The check
applies ordinary `This` substitution with the completed conformance to that facet witness before
reading `invoke`; it does not perform a second search for a conformance path.
Representation diagnostics use the semantic checker's existing duplicate suppression. Native
entry-point validation checks the selected parameter and result types for logical RT inputs,
using the same type classifier as ordinary input-use checking.

## 8. Centralize special rules and allow ordinary metadata/payload values

All additional semantic rules and declaration-identity queries live in
`source/slang/slang-check-structural-ray-tracing.cpp`, with examples beside the rule implementations.
Conformance, expression, entry-point, and code-generation boundaries call into that file.
The ordinary library constraints remain the source of truth for type relationships. The semantic
rules do not require a separate registry of stage implementations. Library loading and builtin
registration should use the existing builtin mechanisms described in section 9.

Schemas, hit groups, and shader lists can be constructed, stored, and used by ordinary generics.
A value such as `MySchema schema = {};` neither constructs shader instances nor allocates an SBT.
Concrete shader implementation structs are ordinary value types. Stage inputs are different:
they refer to data supplied by a shader invocation and allow `in` parameters and initialized local
copies:

```slang
void helper(in rt::MissInput<SkyContext> input)
{
    let local = input;
    local.payload.value = local.record.value;
}
```

Copying the input view preserves access to the same invocation's payload and SBT record; it does
not clone their storage. The `record` getter still returns application data by value. Payload and
callable-data accessors use `[nonmutating] ref`, so the view can be immutable while the referenced
data remains mutable. Full-backend adapter synthesis must preserve this behavior for copied views.
PR1 checks the source rules; it does not yet implement or test that target execution.

All five executable stage interfaces require `static void invoke(...)`. Add `static` to each
closest-hit, any-hit, intersection, miss and callable implementation in demos and helper modules.
`NoClosestHit<C>` and `NoAnyHit<C>` provide static no-op implementations; `NoIntersection<C>`
still has no method. Ordinary Slang conformance checking rejects an instance method for this
static requirement.

A static method has no `this`. A stage struct may contain fields, constructors and instance methods,
and ordinary code may construct, copy, return or use it as a generic argument. For example:

```slang
struct SkyMiss : rt::IMissShader
{
    uint state;
    typealias Context = SkyContext;
    static void invoke(in rt::MissInput<Context> input) {}
}

void forward(in rt::MissInput<SkyContext> input)
{
    SkyMiss local = {1};
    local.invoke(input);
    SkyMiss::invoke(input);
}
```

Both calls pass only `input`; neither passes `local` or makes its `state` available inside `invoke`.
The method can explicitly create an object and read that object's fields, like any other static
method. Selecting `SkyMiss` as an entry point or listing it in a schema does not create an object
or run a constructor. Generated adapters must call the static implementation with its input. The
function's IR therefore has an input parameter and no hidden `this` parameter.

### Checked entry-point signature for later adapter synthesis

Consider a stage whose context is still a generic parameter:

```slang
struct MyHit<C : rt::IHitContext> : rt::IClosestHitShader
{
    typealias Context = C;
    static void invoke(in rt::ClosestHitInput<Context> input) {}
}
```

The checked `invoke` signature contains `rt::ClosestHitInput<C>`, including the generic argument
and the proof that `C` conforms to `rt::IHitContext`. Its payload, record, primitive, and attribute
types can remain dependent on `C`, just as in an ordinary generic function. Selecting
`MyHit<ExternalContext>` can leave those requirements unresolved when `ExternalContext` is an
`extern` type whose definition comes from another module at link time.

Both legacy and structural entries use the existing `EntryPoint` component. Its `m_funcDeclRef`
references the selected AST function with its generic substitutions. The function's checked
parameter type is the source of structural type information:

| Information                                            | Source                                           |
| ------------------------------------------------------ | ------------------------------------------------ |
| Selected `invoke` implementation and generic arguments | `EntryPoint::m_funcDeclRef`                      |
| Selection through a checked structural conformance     | `EntryPoint::m_isStructuralRayTracingEntryPoint` |
| Context and its associated requirements                | The checked stage-input parameter type           |
| Shader stage and profile                               | `EntryPoint::m_profile`                          |
| Public and generated shader names                      | The entry-point name and generated-name override |

Consumers use ordinary `getParameters` and `getType` with the selected `DeclRef<FuncDecl>` to
read the input type. Conformance checking guarantees exactly one parameter; the callers assert
that invariant and use `unwrapModifiedType` to obtain its checked type. Ordinary type checking
has already selected the conformance paths used by the input type and its accessors, including
requirements provided by extensions or inherited interfaces. Generic substitutions on the
function reference apply through the ordinary parameter lookup.

`initializeStructuralRayTracingEntryPoint` marks the entry as structural, preserves the requested
public name (`SkyMiss` or `SkyMiss.invoke`), and derives the generated shader name. Creating an
`EntryPoint` collects specialization parameters and function parameters through the ordinary path.
`markAsStructuralRayTracingEntryPoint` sets the classification flag and currently clears the
shader parameter list to keep the logical stage input out of native parameter layout. As a
result, entry-point layout reflection has an empty parameter scope; it does not yet describe the
adapter's native parameters. Generic specialization parameters remain available, and function
reflection can still inspect the input in the AST function signature.

The generated name includes both the selected type and shader stage. For a struct `MultiStage`
that implements closest-hit and any-hit, the generated names are `MultiStage_closesthit` and
`MultiStage_anyhit`; selecting `Miss` as a miss shader produces `Miss_miss`. The public entry name
remains the requested source name: selecting `Miss.invoke` also generates `Miss_miss`, while its
public name is `Miss.invoke`. `getStructuralRayTracingSourceTypeName` reuses ordinary
type/declaration-reference formatting (`DeclRefType::toString()`) for a non-generic type's qualified
name. For a generic specialization, it uses the canonical type's existing
mangled name, which includes the selected type arguments.
`getStructuralRayTracingEntryPointName` replaces dots in that type name with underscores, then
appends `_` and the canonical spelling from `getStageName(stage)`. For example, `Stages.Miss`
becomes `Stages_Miss_miss`, and `main` becomes `main_miss`. Entry lookup, stage selection, and
the selected method are unchanged by this generated name. The API regression selects both stages of
`MultiStage`, combines the entries in one program, and verifies equal public names with distinct
generated-name overrides. It also selects `Stages.main` as closest-hit and checks the generated
name `Stages_main_closesthit`. For `M<P>` and `M<R>`, it checks that each generated name is the
selected type's ordinary mangled name followed by `_miss`.

AST-to-IR lowering produces an ordinary function and its input type. A generic implementation
uses the normal generic parameters, witness parameters, and `IRSpecialize` operations. Dependent
type and witness lookups remain valid IR until linking and specialization supply their arguments
and definitions. `lowerFrontEndEntryPointToIR` applies ordinary entry-point attribute handling.
The stage-input declaration's `KnownBuiltin` annotation is preserved by ordinary lowering and
specialization.

During linking, `specializeIRForEntryPoint` uses the selected `EntryPoint`'s structural flag to
skip copying parameter layout onto the logical `invoke` input. Its current entry-point parameter
layout is empty; the native adapter and its layout have not been generated yet.
Future adapter synthesis must consume the checked input contract before transformations discard
information it needs. It can generate dependent operations for ordinary specialization to resolve,
or wait for concrete types where the target ABI requires them. PR1 does not implement that adapter.

With `-experimental-feature` enabled, entry-point lookup accepts both a stage type such as
`SkyMiss` and its conforming method `SkyMiss.invoke` as structural selections. Both use the checked
stage conformance to select the implementation. Other static methods such as `SkyMiss.helper`, or
`Plain.invoke` on a type without a stage conformance, use ordinary function selection. A temporary
diagnostic sink holds only the function-not-found diagnostic during structural lookup, so selecting
`SkyMiss` does not report a missing function. Other lookup diagnostics, including generic-argument
conversion warnings, go directly to the real sink.

For an entry declared in an extension, `ComponentType::findDeclFromString` checks the member name
in a semantic context containing the selected module and its dependencies, including the module
itself. Ordinary extension lookup then resolves that member. The non-RT `Pixel.MyType.Main`
regression selects a static extension method and generates SPIR-V without experimental features.

Both lookup paths return a `DeclRef<FuncDecl>`. `findAndValidateEntryPoint` creates the entry
once and calls `initializeStructuralRayTracingEntryPoint` in the RT checker to mark a structural
entry and set its public and generated shader names when needed, then calls `validateEntryPoint`.
The structural finder also supplies the stage selected by conformance; native entries resolve
their stage through the usual function attributes.

Generic entry selection is incomplete. A concrete name such as `-entry "M<Payload,4>"` works,
but `-entry M -specialize Payload -specialize 4` currently fails during lookup. Supporting separate
arguments through the CLI and compilation API is deferred to the front-end follow-up
[#13800](https://github.com/shader-slang/slang/issues/13800). The fix must specialize the enclosing
struct and retain that substitution on `invoke` and its checked input signature. Recognizing
the bare generic name alone is insufficient; the separate entry-selection path must provide
the same checked function specialization as inline selection.
This is an implementation gap; generic stage structs should follow ordinary entry-point
specialization semantics. Section 5.1 of the review walkthrough remains incomplete for this path.

Ordinary conformance checking already validates the selected `invoke` signature.
`validateEntryPoint(entryPoint, sink)` reads the entry's structural classification flag to skip
native parameter classification because conformance has already checked its compiler-supplied
input. Native entries keep the flag unset. Their signatures are checked for logical RT
inputs before native parameter and capability validation; an invalid signature stops validation.
The structural branch passes the annotated input declaration's source-stage requirements to
`_validateEntryPointTargetCapabilities`, including synthesized stages on Metal; the native branch
passes its ordinary stage profile.
For example, a correctly typed `invoke` that calls a Metal-only helper must still be rejected for
HLSL. Capability diagnostics use the ordinary function location at `invoke`; the public entry
name remains the requested source name.

Target validation uses the requirements that ordinary capability checking infers from the
selected method's uses. The curve-data and curve-attribute tests exercise the restrictions where
curve features are used. Whether an otherwise unused `typealias` should contribute requirements
to its enclosing struct remains an open capability-system question.

An `invoke` method can be called as an ordinary static function with an existing input. For example:

```slang
struct SharedMiss : rt::IMissShader
{
    typealias Context = SkyContext;
    static void invoke(in rt::MissInput<Context> input)
    {
        input.payload.value = input.record.value;
    }
}

struct SkyMiss : rt::IMissShader
{
    typealias Context = SkyContext;
    static void invoke(in rt::MissInput<Context> input)
    {
        SharedMiss::invoke(input);
    }
}
```

The call uses the current invocation's input; it does not dispatch a new shader or select a new
SBT record. Constructing a stage object does not supply the input that `invoke` requires.

The adapter implementation must preserve an ordinary callable body when generating a native
entry wrapper. The source `invoke` remains callable with its checked input parameter, while the
wrapper supplies native shader parameters and constructs that input. Ordinary entry-point
callsite handling provides the existing separation between an entry wrapper and a helper body.
Native adapters remain deferred; the positive tests here check frontend acceptance only.

Fresh construction (`MissInput<C>()` or `{}`), uninitialized locals, global variables, `uniform`
parameters, fields, writable/reference parameters, returned views, and arbitrary input generic
arguments retain their checks. Normal initializer checking and same-type coercion handle copies;
the RT checker uses the checked variable declaration and existing parameter-direction/scope helpers.
The generic restriction prevents a generic body such as
`make<T>() { T local = T(); }` from manufacturing an input after frontend checking. Supporting
broader type-only uses would require validating the eventual specialized body. An associated type
can hide the input from this direct argument check: an ordinary `Carrier` can define
`typealias Item = MissInput<C>`, while a generic function constructs `T.Item()` or a generic struct
stores a `T.Item` field. Those ordinary storage and construction uses need checking once T becomes
Carrier. The direct use checks do not inspect ordinary structs' fields recursively to discover
such specializations.
Native entry signatures have a separate check described below. This broader limitation is
tracked in [#13686](https://github.com/shader-slang/slang/issues/13686).

Stage inputs are opaque compiler-supplied shader IO, like patch and stream inputs. They cannot be
the value type of `Optional`, including when the optional would contain an existing input:

```slang
void inspect<C : rt::IPayloadContext>(in rt::MissInput<C> input)
{
    Optional<rt::MissInput<C>> maybe = input; // Rejected.
    let castResult = input as rt::MissInput<C>; // Rejected: produces Optional.
}
```

An explicit `Optional<Input>` is rejected by the input generic-argument rule. `isOpaqueHandleType`
also recognizes the exact stage-input declarations so ordinary opaque-type validation rejects
optional values produced by casts or conversions, including `none` converted to an optional input.
This classification applies even when the input's context is still generic. Inputs hidden behind
associated-type fields remain part of #13686. Direct local copies and `in` helper parameters
retain the existing input-use rules.

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
annotations. Both public call forms reach the same payload implementation. No source-operation
decoration or operation-kind enum is needed.

Post-link lowering must normalize the internal empty value with the reached stages' application
empty payload types. The frontend neither selects a unique application empty type nor inserts an
argument. That normalization and native adapters remain work for the full implementation.

A native entry point cannot accept or return a logical RT input. This applies to all five input
types and every native shader stage, including a miss function that directly takes
`MissInput<Context>`. A type alias still names that same input. Known input storage is rejected
when its declaration is checked, and `Optional<Input>` is invalid as described above. After
handling structural entries, entry-point validation applies the shared RT type classifier to
the substituted native parameter and result types before native parameter and capability rules.
The check runs only when no earlier errors were reported. A forbidden input produces its own
diagnostic and stops validation, avoiding unrelated uniform warnings or capability diagnostics.
The classifier follows builtin container elements without inspecting ordinary struct fields.
Explicit selection, automatic entry discovery and selection from serialized modules use the same
check. A concrete aggregate can still hide an input behind a generic field such as `T.Item`; that
case remains deferred under #13686 because the native check does not traverse fields. Rechecking
entry types supplied only by later API specialization is also follow-up work.

Native and structural shaders may coexist in one module. Ordinary helpers may forward an existing
input. Imports and calls to `TraceRay`, `TraceMotionRay`, `CallShader` or structural trace wrappers
do not impose an API-family restriction. The independent input-use, stage and capability checks
still apply. Native stage adapters, descriptor layout and Metal dispatch remain backend work for
a later PR.

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
motion call. Ordinary target selection and static-assert checking report this missing
implementation.

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

The library's native `TraceRay` calls need no annotation-based exemption. The serialized name
for `metalTraceRay` has stable ID 912.

The Metal operation remains in the target-selected function body for later backend lowering.
That implementation must generate Metal intersector dispatch, shared tables, adapters and
payload transport. Native stage adapters and the descriptor's target resource layout are also
deferred; this PR checks the frontend contracts and preserves their IR representation.

`RayTracer.callShader` is an ordinary library method. Its native branch calls core `CallShader`.
Its Metal branch uses the existing assertion mechanism while callable dispatch is unfinished:

```slang
case metal:
    static_assert(false, "Metal structural ray-tracing callable dispatch requires compiler synthesis");
    break;
```

Target selection retains this assertion only for Metal, and ordinary static-assert checking
reports reachable uses.
There is no callable-specific AST attribute, operation-kind enum, IR decoration, or post-link
marker scan. A later Metal implementation can replace the assertion with an internal intrinsic
and explicit operation, following the trace-call pattern when its operands and lowering are known.

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
