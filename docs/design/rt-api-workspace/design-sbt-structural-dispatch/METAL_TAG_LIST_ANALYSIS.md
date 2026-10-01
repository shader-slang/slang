# Metal Tag-List Inference

Status: supporting analysis for [PROPOSAL.md](PROPOSAL.md). The proposal is normative.

## 1. Decision

Shader authors do not declare a `RayDataTags` type. Slang infers the Metal intersection tag list
from facts already expressed by the structural API:

- `ITraceContext.AccelerationStructure` supplies topology;
- `ITraceContext.Motion` supplies motion;
- each generated dispatcher supplies exactly one primitive selector;
- reachable stage-input property uses supply optional data; and
- the selected target capabilities supply target-wide requirements.

The compiler forms one shared requirement set for each `(schema, payload)` partition. Every
primitive dispatcher in that partition uses the same shared set plus its own primitive selector:

```text
SharedTags(schema, payload)
    = topology
    + motion
    + union(reachable property requirements for that payload)
    + selected target-capability requirements

FunctionTags(schema, payload, primitive)
    = primitive selector
    + SharedTags(schema, payload)
```

This decomposition prevents contradictory primitive selectors while preserving Metal's requirement
that functions used with one intersector agree on a shared signature.

## 2. Primitive selector versus optional data

Metal separates the geometry kind handled by an intersection function from optional data carried
by the intersector. For example, `triangle` selects a triangle function, while `triangle_data`
permits triangle-specific inputs such as barycentrics.

A local compiler experiment with Apple Metal compiler 32023.883 confirmed that Metal 3.1 accepts a
function tag list containing both optional data tags:

```metal
[[intersection(triangle, triangle_data, curve_data)]]
bool triangleFunction();
```

It still rejects a primitive-incompatible parameter, such as `[[curve_parameter]]` on that triangle
function. Therefore:

```text
primitive selector
    chooses which geometry kind the function handles

optional data tags
    declare data capabilities shared by the intersector and function tables

stage-input type constraints
    decide which primitive-specific properties source code may read
```

`IHitContext.Primitive` gates the source properties. A triangle context can read `input.triangle`
but not `input.curve`; a curve context has the opposite rule. Merely declaring a triangle or curve
does not request its optional data tag.

## 3. Complete inference table

| Metal tag | Axis | Inference source | Validation |
| --- | --- | --- | --- |
| `triangle` | Per-function primitive selector | Triangle dispatcher | Mutually exclusive with the other primitive selectors |
| `bounding_box` | Per-function primitive selector | Bounding-box dispatcher | Mutually exclusive with the other primitive selectors |
| `curve` | Per-function primitive selector | Curve dispatcher | Requires the Metal curve capability |
| `instancing` | Shared topology | `AccelerationStructure`, or `MultiLevelAccelerationStructure<N>` with `N >= 2` | Omitted for direct primitive-AS traversal |
| `max_levels<N>` | Shared topology | `MultiLevelAccelerationStructure<N>` with `N >= 2` | Requires `instancing` and a supported `N` |
| `primitive_motion` | Shared motion | `PrimitiveMotion` or `PrimitiveAndInstanceMotion` | Capability-gated |
| `instance_motion` | Shared motion | `InstanceMotion` or `PrimitiveAndInstanceMotion` | Requires instancing and target support |
| `triangle_data` | Shared optional data | Reachable triangle barycentric/front-facing property, or triangle `hitKind` | Source property must belong to a triangle context |
| `curve_data` | Shared optional data | Reachable curve-parameter property | Source property must belong to a curve context |
| `world_space_data` | Shared optional data | Reachable candidate world-ray property, _ClosestHit_ object-space ray, or hit-stage transform property | Requires instancing |
| `extended_limits` | Shared target requirement | `metal_raytracing_extended_limits` capability | Supplied by target selection |
| `intersection_function_buffer` | Future lowering | None in version one | Excluded |
| `user_data` | Future IFB data | None in version one | Excluded |

There is no remaining tag axis that shader authors must specify independently.

## 4. Property triggers

The optional-data triggers are deliberately based on reachable use after specialization:

```text
input.triangle.barycentricCoord  ─┐
input.triangle.frontFacing       ├─> triangle_data
triangle input.hitKind           ─┘

input.curve.parameter             ─> curve_data

AnyHitInput.worldSpaceOrigin      ─┐
AnyHitInput.worldSpaceDirection    │
IntersectionInput.worldSpaceOrigin │
IntersectionInput.worldSpaceDirection
ClosestHitInput.objectSpaceRay     ├─> world_space_data
hit-stage input.objectToWorld      │
hit-stage input.worldToObject     ─┘
```

_ClosestHit_ and _Miss_ world-space origin/direction are reconstructed from the original ray and do
not request `world_space_data`. Metal candidate functions need Metal-provided world-space state, so
the analogous _AnyHit_ and _Intersection_ uses do request it.

Transform and object-space reconstruction require an instance transform. A schema using direct
primitive-AS traversal (`MultiLevelAccelerationStructure<1>`) is therefore rejected when reachable
code would require `world_space_data`.

Reachability is transitive. If `invoke` calls a helper and the helper reads one of these properties,
the property still contributes its requirement. Uses eliminated by specialization do not.

## 5. Payload partitioning

Shared tags are computed per payload, not once for the entire schema. Consider:

```text
RadiancePayload groups
    read triangle barycentrics

ShadowPayload groups
    do not read triangle data
```

The radiance IFT signature includes `triangle_data`; the shadow signature does not. Both still use
the same schema and runtime record buffer.

All trace operations using the same schema and payload contribute to one normalized partition
signature. This makes the result independent of link or entry-point processing order. The final
signature is published in target metadata and keyed by exact schema name plus reflected payload
index.

## 6. Why the combinations are valid

Each potentially conflicting choice has one canonical source:

- A generated function has one primitive kind.
- A schema has one acceleration-structure topology.
- A schema has one sealed motion mode.
- A payload partition unions only compatible optional data and target requirements.

The compiler checks capability and cross-axis rules before creating the Metal descriptor. A source
program can therefore either produce one valid normalized signature or receive a compile-time
diagnostic; it cannot silently create two incompatible signatures for one payload table.

The host does not reconstruct this logic. It queries the finalized
`IStructuralRayTracingMetadata` payload record and uses the returned
`MTLIntersectionFunctionSignature` when constructing Metal resources.
