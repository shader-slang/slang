# Function Table Abstraction Brainstorm

This note explores whether Metal `intersection_function_table`, D3D/Vulkan SBT
records, and callable shader tables can be described by one higher-level
abstraction.

This is not yet a proposal. The purpose is to stress-test the mental model
before changing the main design.

## Starting Point

At a high level, all targets have some kind of table-driven dispatch:

- D3D/Vulkan use shader binding tables. Traversal selects miss and hit shader
  records, and shader code can also call callable shader records.
- Metal uses `intersection_function_table` for traversal-time custom
  intersection functions. Metal does not use this table to dispatch closest-hit
  or miss logic.

The tempting abstraction is:

```text
TraceProgram describes a conceptual ray dispatch table.

D3D/Vulkan:
    TraceProgram -> native SBT regions

Metal:
    TraceProgram -> intersection function table
                 -> generated post-trace closest-hit/miss dispatch
```

This direction is plausible, but a few details matter.

## Correction 1: Metal Function Tables Do Not Store AnyHit + Intersection Pairs

A Slang hit group may contain:

```text
closest-hit | any-hit | intersection
```

A D3D/Vulkan SBT hit-group record naturally maps to this shape:

```text
closest-hit | any-hit | intersection
```

Metal's ordinary `intersection_function_table` does not. Each function entry is
effectively:

```text
one selected Metal [[intersection(...)] function
```

Metal has no native any-hit stage. Therefore, if Slang exposes both `AnyHit`
and `Intersection`, Metal lowering must represent the traversal-time part of a
hit group with one Metal intersection function.

Conceptually:

```text
Slang HitGroup row:
    closestHit | anyHit | intersection

D3D/Vulkan SBT row:
    closestHit | anyHit | intersection

Metal traversal table row:
    generated-or-selected [[intersection(...)]] function
        represents the anyHit/intersection portion only

Metal post-trace dispatch:
    selected closestHit runs after intersector.intersect(...) returns
```

So Metal traversal tables can participate in the same conceptual hit-group
schema, but they do not store the same per-row stage tuple as D3D/Vulkan.

## Correction 2: Metal Visible Function Tables Are Resource Bindings, Not SBT Sections

It is useful to think of a Metal `intersection_function_table` as having two
kinds of contents:

```text
function-entry slots:
    slot 0 -> selected [[intersection(...)]] function
    slot 1 -> selected [[intersection(...)]] function
    ...

resource-binding slots:
    buffer slot 0 -> material data
    visible-function-table slot 0 -> alpha-test/helper function table
    ...
```

However, a visible function table is not a second traversal-dispatched section
of the ordinary intersection function table.

Traversal selects the intersection function entry. The selected intersection
function may then use table-provided resources, including visible function
tables.

This is different from D3D/Vulkan callable shaders:

```text
D3D/Vulkan callable shader:
    ray tracing shader stage selected from callable SBT region

Metal visible function table:
    resource-like typed table of visible functions used by shader code
```

They are related as indirect-call mechanisms, but they should not be treated as
the same semantic object in the portable API.

## Correction 3: Ray Type Is A Layout Convention, Not A First-Class SBT Column

D3D/Vulkan SBT diagrams often look like:

```text
                    ray type 0                  ray type 1
geometry 0      hit group 0                 hit group 1
geometry 1      hit group 2                 hit group 3
```

That is a useful visualization, but the actual portable contract is index
arithmetic:

```text
hitGroupIndex =
    instanceContribution
  + geometryContribution * sbtStride
  + sbtOffset
```

If an engine chooses:

```text
sbtStride = rayTypeCount
sbtOffset = rayType
```

then the table behaves like a geometry-by-ray-type grid.

The API should expose the arithmetic contract, not hard-code the idea of ray
type columns.

## Metal Ray-Type Support

The ordinary table selects entries from acceleration-structure offsets:

```text
primitive AS:
    functionIndex = geometryIntersectionFunctionTableOffset

instance AS:
    functionIndex =
        geometryIntersectionFunctionTableOffset
      + instanceIntersectionFunctionTableOffset
```

There is no shader-side `base_id` or `geometry_multiplier` on this path.

This means ordinary function tables do not naturally support the full
D3D/Vulkan-style dynamic pair:

```text
sbtOffset
sbtStride
```

They can still be useful when the desired slot layout is baked into
acceleration-structure offsets, when a different table is selected, or when the
program uses a restricted dispatch layout.

## Proposed Abstraction Boundary

The portable abstraction should not be named or shaped as a Metal function
table. Instead:

```text
TraceProgram = shader-visible schema for conceptual ray dispatch tables
```

`TraceProgram` should describe logical regions:

```slang
struct PrimaryProgram : rt::TraceProgram
{
    typealias TraceContext = PrimaryTraceContext;

    typealias HitGroups = rt::HitGroupList<
        rt::HitGroup<Slot0, ClosestHit0, AnyHit0, Intersection0>,
        rt::HitGroup<Slot1, ClosestHit1, NoAnyHit, BuiltinTriangle>>;

    typealias MissGroups = rt::MissGroupList<
        rt::MissGroup<MissSlot0, PrimaryMiss>>;

    // Optional future extension:
    // typealias CallableDomains = rt::CallableDomainList<...>;
}
```

Reflection over this schema can then expose:

```text
TraceProgram
    HitGroups[]
        slot
        closest-hit shader
        any-hit shader, or none
        intersection shader, or built-in
        required Metal traversal tags

    MissGroups[]
        slot
        miss shader

    CallableDomains[]       // optional future extension
        domain
        parameter type
        callable slots
```

## Target Mapping

### D3D/Vulkan

```text
HitGroups      -> native SBT hit-group records
MissGroups     -> native SBT miss records
CallableGroups -> native callable SBT region, if added
```

The target already performs hit/miss dispatch. Slang does not synthesize a
post-trace closest-hit/miss switch.

### Metal Ordinary Function Table

```text
HitGroups.anyHit/intersection -> function-table entries
HitGroups.closestHit          -> generated post-trace switch
MissGroups                    -> generated post-trace switch
```

This path only matches the general slot model when the acceleration-structure
offsets, selected table, and shader dispatch values are constrained to a
compatible layout. It should not be treated as the fully general baseline for
`sbtOffset/sbtStride`.

## Host-Side Goal

The host should not have to hand-maintain separate grouping logic for each
target. It should be able to query `TraceProgram` reflection and build the
target-specific objects:

```text
D3D/Vulkan:
    build ray tracing pipeline
    build hit/miss/callable SBT records from reflected groups

Metal ordinary table:
    build function table entries from reflected hit groups
    ensure AS offsets match reflected slot layout
    rely on generated post-trace closest-hit/miss dispatch
```

This keeps the first priority on shader-side portability while still allowing a
backend such as slang-rhi to hide target-specific host object creation.

## Open Questions

1. Can ordinary Metal `intersection_function_table` support the same practical
   dispatch patterns as D3D/Vulkan SBT without introducing extra host-side
   requirements?
2. Should ordinary Metal `intersection_function_table` be exposed as a
   restricted lowering mode if it cannot support the full dynamic
   `RayDispatch.sbtOffset/sbtStride` model?
3. How should the API express that a target supports only a restricted dispatch
   layout?
4. Should callable shader tables be added to `TraceProgram` now, or left as a
   follow-up extension after hit/miss/hit-group dispatch is stable?
5. If callables are added later, should they be grouped by typed callable
   domains rather than one untyped global callable list?

## Provisional Position

The common abstraction should be `TraceProgram` as a dispatch schema, not a
portable "function table" object.

The schema can map to:

```text
D3D/Vulkan SBT
Metal ordinary function table with restrictions
generated Metal closest-hit/miss dispatch
```

This keeps the shader-side model target-neutral while acknowledging that the
underlying target tables are not the same object.
