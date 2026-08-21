# Metal Tag-List Design: Inferred Requirements

## Conclusion

`RayDataTags` should not be part of the source-level trace context. Slang can infer
`triangle_data` and `curve_data` from reachable uses of compiler-known stage properties, and
`world_space_data` from candidate-stage uses of world-space ray properties. The selected
compilation capabilities supply `extended_limits`.

Stage-derived primitive data follows two source-level rules:

1. `IHitContext.Primitive` determines which primitive-specific properties are legal.
2. Using a legal property contributes its required Metal data tag.

For example, `input.triangle` is only available when `Primitive == TrianglePrimitive`. A reachable
use contributes `triangle_data`. Likewise, `input.curve` is only available for `CurvePrimitive` and
contributes `curve_data`.

Declaring a primitive does not by itself request its optional data. A triangle group that never
reads barycentrics or front-facing state does not require `triangle_data`.

## Metal's Two Separate Axes

Metal separates the primitive handled by one intersection function from the shared tag signature:

```metal
[[intersection(triangle, triangle_data, curve_data)]]
bool triangleFunction();
```

The first argument says that this function handles triangle candidates. The remaining arguments
form the shared signature used by the trace program.

Apple Metal compiler version 32023.883 accepts the mixed tag list above under Metal 3.1,
including with the two data tags reversed. It rejects primitive-incompatible input attributes:

```metal
[[intersection(triangle, triangle_data, curve_data)]]
bool invalidTriangleFunction(float parameter [[curve_parameter]]);
```

The diagnostic identifies `curve_parameter` as invalid for a triangle intersection function. The
opposite mismatch, `barycentric_coord` on a curve function, is also rejected.

Therefore:

```text
Primitive
    controls which primitive-specific inputs may be consumed

Shared tag signature
    controls which optional data capabilities are carried by the trace program
```

Both `triangle_data` and `curve_data` may appear in one shared signature, but each generated
intersection function consumes only data valid for its primitive.

## Source-Level Model

The trace context contains trace-wide properties, but no authored primitive-data tags:

```slang
interface ITraceContext
{
    associatedtype Payload;
    associatedtype AccelerationStructure : IAccelerationStructure;
    associatedtype Motion;
}
```

Each hit context fixes one primitive kind:

```slang
interface IHitContext
{
    associatedtype TraceContext : ITraceContext;
    associatedtype Primitive : IIntersectionPrimitive;
    associatedtype Record;
}
```

Primitive-specific properties are supplied through constrained extensions:

```slang
public extension<Context> AnyHitInput<Context>
    where Context : IHitContext
    where Context.Primitive == TrianglePrimitive
{
    public property TriangleHitAttributes triangle
    {
        get { return __rtAnyHitTriangle<Context>(); }
    }
}
```

The property is absent from `AnyHitInput<CurveContext>` and
`AnyHitInput<BoundingBoxContext>`. Invalid cross-primitive access is rejected during Slang type
checking, before Metal code is generated.

The property getter is compiler-known. A reachable `triangle` getter records a `triangle_data`
requirement. A reachable `curve` getter records `curve_data`.

## Concrete Inference Example

This source does not declare any Metal data tags:

```slang
struct PrimaryTraceContext : rt::ITraceContext
{
    typealias Payload = RadiancePayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TriangleContext : rt::IHitContext
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TriangleRecord;
}

struct RejectBackFaces : rt::IAnyHitShader<TriangleContext>
{
    void invoke(rt::AnyHitInput<TriangleContext> input)
    {
        if (!input.triangle.frontFacing)
            input.ignoreHit();
    }
}
```

The use of `input.triangle` contributes `triangle_data`. Slang applies the inferred tag to every
related Metal declaration:

```metal
intersector<instancing, triangle_data> tracer;
intersection_function_table<instancing, triangle_data> table;

[[intersection(triangle, instancing, triangle_data)]]
bool generatedAnyHit(bool frontFacing [[front_facing]]);
```

If the trace program also contains a curve group that uses `input.curve`, Slang adds `curve_data`
to the same shared signature. If the curve group does not access curve-specific data, its presence
alone does not add the tag.

## Inference Sources

The sources describe separate semantic axes. Topology and lowering select one trace-wide mode, the
primitive selector is chosen independently for each generated function, and optional data
requirements are combined by set union. Motion selects one valid trace-wide configuration, which
may contain both motion tags.

### Type-Directed Inference

`TraceContext.AccelerationStructure` supplies the topology tags:

```text
AccelerationStructure
    -> instancing

MultiLevelAccelerationStructure<1>
    -> no instancing and no max_levels

MultiLevelAccelerationStructure<N>, N >= 2
    -> instancing, max_levels<N>
```

`TraceContext.Motion` supplies `primitive_motion`, `instance_motion`, both, or neither. Every group
in a program layout is constrained to the same trace context, so all reachable stages have one
topology and motion configuration.

`IHitContext.Primitive` selects `triangle`, `bounding_box`, or `curve` independently for each
generated `[[intersection(...)]]` function. This primitive selector is not part of the shared tag
set.

### Reachability-Directed Inference

Reachable compiler-known properties contribute target requirements:

```text
ClosestHitInput.triangle or AnyHitInput.triangle     -> triangle_data
ClosestHitInput.curve or AnyHitInput.curve           -> curve_data
AnyHit/Intersection worldSpaceOrigin or Direction    -> world_space_data on Metal
ClosestHit/Miss worldSpaceOrigin or Direction        -> original trace ray on Metal
```

D3D and Vulkan lower all four world-space property forms to native world-ray builtins. Metal's
candidate stages require `[[world_space_origin]]` or `[[world_space_direction]]`, so their uses add
`world_space_data`. Metal's generated post-trace *ClosestHit* and *Miss* dispatch instead forwards
the original `RayTraversalDesc.ray`, so those uses add no tag.

The compiler unions the tag-producing requirements across reachable stages. The constrained input
APIs prevent primitive-incompatible property access. They also reject candidate-stage
`world_space_data` with the primitive-only topology, for which Metal has no valid pipeline
intersector combination. Post-trace world-ray access remains valid for that topology.

### Capability-Directed Inference

The selected compilation capabilities contribute `extended_limits`. The capability represents an
enabled build mode, not merely hardware support. Metal emits the tag and reflects the requirement
to the host. D3D and Vulkan emit no shader tag and validate native acceleration-structure limits on
the host.

### Lowering-Directed Inference

The first version always chooses ordinary `intersection_function_table` lowering and contributes
no lowering tag. A future function-buffer lowering contributes `intersection_function_buffer`;
using its user-data argument additionally contributes `user_data`.

## Complete Metal Tag Coverage

| Metal item | Axis | Inference source | Combination and validation rule |
| --- | --- | --- | --- |
| `triangle`, `bounding_box`, `curve` | Per-function primitive selector | `IHitContext.Primitive` | Emit exactly one per generated function; reject primitive-incompatible properties. |
| `instancing` | Acceleration-structure topology | `TraceContext.AccelerationStructure` | The one acceleration-structure type fixes the program-wide topology. |
| `max_levels<N>` | Acceleration-structure topology | `MultiLevelAccelerationStructure<N>`, `N >= 2` | Require `instancing`; validate one supported level count. |
| `primitive_motion` | Motion configuration | `TraceContext.Motion` | Select as part of one trace-wide configuration; allow coexistence with `instance_motion`; validate target support. |
| `instance_motion` | Motion configuration | `TraceContext.Motion` | Allow coexistence with `primitive_motion`; require `instancing`. |
| `triangle_data` | Shared optional data | Reachable use of `ClosestHitInput.triangle` or `AnyHitInput.triangle` | Union with other data requirements; expose both properties only for `TrianglePrimitive`. |
| `curve_data` | Shared optional data | Reachable use of `ClosestHitInput.curve` or `AnyHitInput.curve` | Union with other data requirements; expose both properties only for `CurvePrimitive`. |
| `world_space_data` | Shared optional data | Reachable use of `AnyHitInput.worldSpaceOrigin`, `AnyHitInput.worldSpaceDirection`, `IntersectionInput.worldSpaceOrigin`, or `IntersectionInput.worldSpaceDirection` | Union with other data requirements; require an instanced acceleration structure. *ClosestHit* and *Miss* uses do not add this tag. |
| `extended_limits` | Build capability | Selected compilation capabilities | Add only when selected, reflect the mode, and reject unsupported targets. |
| `intersection_function_buffer` | Lowering mode | Future IFB lowering | Select one trace-wide IFB path instead of an ordinary IFT; unavailable in the first version. |
| `user_data` | Function-buffer data | Future IFB user-data argument | Union into an IFB signature; require `intersection_function_buffer`. |

This table covers all Metal ray-tracing template tags. The first row additionally covers the
primitive selector that precedes the shared tags in `[[intersection(...)]]`.

## Signature Construction

For a program layout `L`, selected capabilities `C`, and lowering `M`, Slang constructs:

```text
ReachableStageRequirements(L)
    = union of requirements from reachable compiler-known stage properties

SharedMetalTags(L, C, M)
    = validateAndNormalize(
          L.TraceContext.AccelerationStructure.sharedRequirements,
          L.TraceContext.Motion.requirements,
          ReachableStageRequirements(L),
          C.requirements,
          M.requirements)
```

The compiler must then:

1. Check that every group uses `L.TraceContext`.
2. Select one primitive for each generated intersection function.
3. Collect and union requirements from every reachable stage operation.
4. Add the selected capability and lowering requirements.
5. Validate dependencies, parameter values, and target availability.
6. Choose one deterministic tag order.
7. Project only valid topology and motion tags onto the Metal acceleration-structure parameter.
8. Reuse the same ordered shared signature for the intersector, result, and ordinary function
   table, and append it after the primitive selector on every generated intersection function.

For future IFB lowering, step 8 uses the IFB-compatible declarations instead of an ordinary
function table.

## Why Source-Level Tag Conflicts Cannot Survive

- One `TraceContext` fixes topology, level count, and motion for the entire program layout.
- One `IHitContext.Primitive` fixes the primitive selector for each generated function.
- Optional data requirements are monotonic: compatible requirements are unioned rather than chosen
  independently for each stage.
- Parameterized and dependent tags are validated before Metal emission.
- One lowering mode fixes IFT versus IFB for the entire trace-program descriptor.
- One canonical order is reused for every related native declaration.

Consequently, Slang either generates one compatible tag signature or diagnoses the conflict during
compilation. Host code can still violate the reflected contract by binding an incompatible
acceleration structure or function table; reflection and runtime validation address that separate
problem.

## Simplified Conceptual Split

```text
TraceContext types
    AccelerationStructure -> instancing, max_levels<N>
    Motion                -> primitive_motion, instance_motion

IHitContext
    Primitive -> one per-function primitive selector

Reachable stage operations
    ClosestHitInput.triangle or AnyHitInput.triangle -> triangle_data
    ClosestHitInput.curve or AnyHitInput.curve       -> curve_data
    candidate-stage worldSpaceOrigin/Direction -> world_space_data
    post-trace worldSpaceOrigin/Direction       -> original trace ray

Selected compilation capabilities
    extended-limits mode -> extended_limits

Lowering
    first version: OrdinaryIFT
    future: IFB -> intersection_function_buffer, optional user_data
```

## Source

The native rules summarized here come from the
[Metal Shading Language Specification](https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf),
especially sections 2.17.1, 2.17.4, 5.1.6, 5.2.3.7, and 6.19.5.
