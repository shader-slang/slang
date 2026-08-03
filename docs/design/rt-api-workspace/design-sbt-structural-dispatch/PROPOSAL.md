# Slang Pipeline Ray Tracing API Proposal: Structural Dispatch

Status: draft proposal

Scope: pipeline ray tracing only. Inline ray tracing and ray queries are intentionally out of
scope for this first design.

This proposal sketches a new Slang ray tracing API that treats Metal as a first-class target
while preserving the D3D and Vulkan pipeline model. The central idea is to make the shader source
declare a conceptual shader binding table, or SBT, as structured Slang types. D3D and Vulkan can
continue to use the native host-created SBT. Metal can use the same structure to synthesize the
post-trace closest-hit and miss dispatch logic that Metal programmers normally write by hand.

Catalog
-------

- [1. Challenges Extending Current Slang Ray Tracing To Metal](#1-challenges-extending-current-slang-ray-tracing-to-metal)
  - [1.1 Dispatch Model Gap](#11-dispatch-model-gap)
  - [1.2 Metal Function Table And Function Buffer Resource Mismatch](#12-metal-function-table-and-function-buffer-resource-mismatch)
  - [1.3 Metal Tag List And Reachability](#13-metal-tag-list-and-reachability)
  - [1.4 Reserved Challenges](#14-reserved-challenges)
- [2. Proposed API Sketch](#2-proposed-api-sketch)
  - [2.1 Overview](#21-overview)
  - [2.2 Detailed Component Descriptions](#22-detailed-component-descriptions)
    - [2.2.1 Resolving The Dispatch Model Gap With A Conceptual SBT](#221-resolving-the-dispatch-model-gap-with-a-conceptual-sbt)
    - [2.2.2 Resolving The Metal Function Table And Function Buffer Resource Mismatch With TraceProgramDescriptor](#222-resolving-the-metal-function-table-and-function-buffer-resource-mismatch-with-traceprogramdescriptor)
      - [2.2.2.1 Lowering To `intersection_function_table`](#2221-lowering-to-intersection_function_table)
      - [2.2.2.2 Lowering To `intersection_function_buffer_arguments`](#2222-lowering-to-intersection_function_buffer_arguments)
    - [2.2.3 Resolving The Metal Tag-List Issue With Context Types](#223-resolving-the-metal-tag-list-issue-with-context-types)
  - [2.3 Writing Stages As Interface-Conforming Types](#23-writing-stages-as-interface-conforming-types)
- [3. Migration Examples](#3-migration-examples)
  - [3.1 Migrating Existing Metal Code To The New API](#31-migrating-existing-metal-code-to-the-new-api)
  - [3.2 Migrating Existing Slang D3D/Vulkan Ray Tracing Code](#32-migrating-existing-slang-d3dvulkan-ray-tracing-code)
  - [3.3 Host Reflection Patterns](#33-host-reflection-patterns)
- [4. Open Design Questions](#4-open-design-questions)

## 1. Challenges Extending Current Slang Ray Tracing To Metal

### 1.1 Dispatch Model Gap

D3D and Vulkan expose ray tracing as a pipeline-stage model. A trace call enters traversal, and
the driver or hardware uses host-created SBT records to select miss, any-hit, intersection, and
closest-hit shaders. The closest-hit function is not directly called from ray-generation source.

Metal exposes a different model. `intersector.intersect(...)` returns an `intersection_result`.
AnyHit and custom Intersection behavior can still be dispatched during traversal through Metal's
function table or function buffer, but Miss and ClosestHit are ordinary post-trace shader logic
written by the user.

Figure 1 shows the key mismatch: D3D/Vulkan assign stage dispatch to the host SBT and the
driver/hardware, while Metal assigns Miss and ClosestHit dispatch to shader code after
`intersect(...)` returns. A portable Slang API needs enough structure to synthesize that Metal
post-trace dispatch without changing the native D3D/Vulkan model.

<a id="fig-dispatch-model-gap"></a>
![D3D and Vulkan SBT dispatch compared with Metal user-specified post-trace miss and closest-hit dispatch](figures/dispatch-model-gap.svg)

*Figure 1. Dispatch model gap: D3D and Vulkan select pipeline stages through host-created SBT records, while Metal requires user-written post-trace dispatch for Miss and ClosestHit.*

### 1.2 Metal Function Table And Function Buffer Resource Mismatch

Metal introduces `intersection_function_table` resource objects and
`intersection_function_buffer_arguments` values that are visible to shader code and must be bound
from host code when traversal needs custom intersection behavior. This is different from the
D3D/Vulkan SBT model. The SBT is built by host code, but it is not a shader-visible resource and
shader code does not declare a binding point for it.

This creates an asymmetric programming model. Metal shader code may need a parameter that
represents a function table or function buffer. D3D/Vulkan shader code has no corresponding
parameter, even though host code still needs to build SBT records. A portable Slang API therefore
needs a way to describe this logical binding without forcing D3D/Vulkan targets to expose a fake
shader resource.

### 1.3 Metal Tag List And Reachability

This proposal uses the term **reachability** to describe the shader binding table entries that a
single trace call can access. The trace call supplies dispatch parameters, traversal contributes
geometry and instance information, and the target selects one of the SBT records. Those selectable
records are the entries reachable from that trace call.

In existing D3D/Vulkan-style ray tracing models, this reachability is determined by host-created
binding data. The shader source contains the trace call, but the SBT records and the binding edges
from those records to AnyHit, Intersection, ClosestHit, and Miss shaders are provided by host code.
Therefore, as shown in Figure 2, the complete reachability set is not known from shader source at
ordinary compile time.

<a id="fig-reachability-definition"></a>
![Reachability is the set of SBT entries that one trace call can select](figures/reachability-definition.svg)

*Figure 2. Reachability definition: the reachable entries are the SBT records one trace call can select at runtime, but in the existing model that set is determined by host-created binding data.*

Metal adds a second constraint: each custom intersection function reachable from an intersector
must have a compatible `[[intersection(...)]]` tag list. Native Metal can validate a mismatch at
pipeline build time because the user writes both the intersector tags and the function tags in
source. Figure 3 shows why this works: pipeline build sees the intersector tags, function tags,
and host bindings together.

<a id="fig-native-metal-tag-validation"></a>
![Native Metal can validate explicit intersector and custom intersection function tags at pipeline build time](figures/native-metal-tag-validation.svg)

*Figure 3. Native Metal tag validation: the user-authored tag lists give pipeline build enough information to reject incompatible host bindings.*

Slang does not currently expose that Metal tag system. When lowering AnyHit or Intersection entry
points to Metal, Slang must synthesize `[[intersection(...)]]` tags for the generated Metal
functions. The compiler can see trace sites and stage entry points, but in the existing model it
cannot see the host binding edges that determine which stage entries are reachable from each trace
site.

Figure 4 shows the information-flow problem. If two trace sites lower to different Metal tag
sets, and several AnyHit or Intersection entries may be bound by the host, the compiler cannot
know whether a generated function needs tag set A, tag set B, or another tag set. Emitting no tag,
or emitting a tag inferred from the wrong trace site, can make the generated Metal pipeline fail
to build.

<a id="fig-slang-tag-synthesis-gap"></a>
![Slang cannot synthesize Metal intersection tags when host binding data owns reachability](figures/slang-tag-synthesis-gap.svg)

*Figure 4. Slang tag synthesis gap: Slang must emit Metal `[[intersection(...)]]` tags before host binding data reveals which AnyHit or Intersection entries are reachable from each trace site.*

### 1.4 Reserved Challenges

Other details remain important, but they are not the main shape of this proposal:

- Metal has limitations on custom attributes returned from custom intersection functions.
- Metal has multiple `intersect(...)` overload families, including no-dispatch, function-table,
  and function-buffer forms.
- Device user data should be modeled in a way that maps to non-pointer targets.

Those issues can be handled after the dispatch and tag-reachability model is settled.

## 2. Proposed API Sketch

### 2.1 Overview

The proposed API asks shader authors to describe a trace program in source:

1. Define payload and context types.
2. Write miss, closest-hit, any-hit, and intersection logic as structs that implement Slang
   interfaces.
3. Group those stage structs into ordered hit, miss, and callable group lists.
4. Use `rt::RayTracer<ProgramLayout>`, `rt::RayTraversalDesc`, and
   `rt::TraceProgramDescriptor<ProgramLayout>` from ray-generation code.

The group declarations are the source-level conceptual SBT. They are visible to the compiler and
to reflection. Host code can query `ITraceProgramLayout` reflection to build D3D/Vulkan SBT
records or Metal function tables/function buffers from the same grouping contract, so the grouping
logic does not need to be duplicated outside shader source. Figure 5 gives a high-level view of
this API shape.

<a id="fig-api-overview"></a>
![API overview](figures/api-overview.svg)

*Figure 5. Proposed API overview: users define contexts, stage structs, and grouped dispatch metadata in an `ITraceProgramLayout`; host code uses reflection of that same program layout to build D3D/Vulkan SBT records and Metal function tables/function buffers.*

The running example uses the prototype files in this directory. Older alternative
sketches outside this directory are design history and are not part of this PR.

### 2.2 Detailed Component Descriptions

#### 2.2.1 Resolving The Dispatch Model Gap With A Conceptual SBT

The dispatch solution is to declare the SBT structure in shader source. At this layer, the key
concepts are ordered groups, group lists, a trace program layout, a descriptor, and traversal
parameters. A slot is the zero-based position of a group in its corresponding list.

Simplified API shape:

```slang
namespace rt
{
    public struct RayTraversalDesc
    {
        public RayDesc ray;
        public uint instanceMask;
        public uint sbtOffset;
        public uint sbtStride;
        public uint missIndex;
    }

    public interface IHitGroup
    {
        associatedtype Context : IHitGroupContext;
        associatedtype ClosestHit : IClosestHitShader<Context>;
        associatedtype AnyHit : IAnyHitShader<Context>;
        associatedtype IntersectionAttributes;
        associatedtype Intersection : IIntersectionShader<Context, IntersectionAttributes>;
    }

    public interface IMissGroup
    {
        associatedtype Context : IMissGroupContext;
        associatedtype Miss : IMissShader<Context>;
    }

    public interface ICallableGroup { ... }

    public struct HitGroupList<TraceContext, each HitGroup> { ... }
    public struct MissGroupList<TraceContext, each MissGroup> { ... }
    public struct CallableGroupList<TraceContext, each CallableGroup> { ... }

    public interface ITraceProgramLayout
    {
        associatedtype TraceContext : ITraceContext;
        associatedtype MissGroups : IMissGroupList<TraceContext>;
        associatedtype HitGroups : IHitGroupList<TraceContext>;
        associatedtype CallableGroups : ICallableGroupList<TraceContext>;
    }

    public struct TraceProgramDescriptor<ProgramLayout>
        where ProgramLayout : ITraceProgramLayout
    {}

    public struct RayTracer<ProgramLayout>
        where ProgramLayout : ITraceProgramLayout
    {
        public void trace(
            RayTraversalDesc desc,
            AccelerationStructure<
                ProgramLayout.TraceContext.ASKind,
                ProgramLayout.TraceContext.Motion> scene,
            TraceProgramDescriptor<ProgramLayout> descriptor,
            inout ProgramLayout.TraceContext.Payload payload);
    }
}
```

This simplified shape omits some details, but keeps the current naming and ownership model:
`ITraceProgramLayout` is the schema, `TraceProgramDescriptor<ProgramLayout>` is the value-level
descriptor, and `RayTraversalDesc` carries per-trace SBT-style indices.

The compiler-synthesized closest-hit slot calculation still follows the D3D/Vulkan SBT
calculation. This is not a user-facing API; it is shown only as the conceptual formula Slang will
generate for Metal:

```slang
slot = instanceContribution + geometryContribution * desc.sbtStride + desc.sbtOffset;
```

For D3D and Vulkan, these fields are mostly descriptive:

- `RayTraversalDesc.sbtOffset` maps to D3D `RayContributionToHitGroupIndex` and Vulkan
  `sbtRecordOffset`.
- `RayTraversalDesc.sbtStride` maps to D3D `MultiplierForGeometryContributionToHitGroupIndex` and
  Vulkan `sbtRecordStride`.
- `RayTraversalDesc.missIndex` maps to the native miss shader index.
- The native driver or hardware still performs the SBT lookup.

For Metal, this API becomes executable structure:

- Slang lowers `RayTracer<ProgramLayout>.trace(...)` to `intersector.intersect(...)`.
- Slang uses `RayTraversalDesc.missIndex` to index generated Miss visible-function dispatch.
- Slang synthesizes the internal hit-group slot calculation to index generated ClosestHit
  visible-function dispatch.
- Slang uses the same hit-group list slots to validate and emit Metal function-table or
  function-buffer entries for any-hit and custom intersection functions.

Conceptual Metal lowering:

```slang
// Internal lowering pseudocode, not user-visible rt API.
let result = metalIntersector.intersect(desc.ray, scene, descriptor, payload);

if (result.isNone)
{
    let missFn = descriptor.__generatedMissVisibleFunctions[desc.missIndex];
    missFn(makeMissInput(...));
}
else
{
    uint geometryContribution = __rtGetGeometryContribution<ProgramLayout>(result);
    uint instanceContribution = __rtGetInstanceContribution<ProgramLayout>(result);
    uint slot = instanceContribution + geometryContribution * desc.sbtStride + desc.sbtOffset;

    let closestHitFn = descriptor.__generatedClosestHitVisibleFunctions[slot];
    closestHitFn(makeClosestHitInput(...));
}
```

The important property is that Metal closest-hit dispatch is not invented independently. It is
generated from the same conceptual SBT slots that the host uses for D3D and Vulkan.

The generated Metal dispatch should not normally be emitted as a literal `switch` over every
reachable Miss or ClosestHit body. A switch keeps all selected stage bodies in the same Metal
shader compilation unit, so an expensive ClosestHit path that is unreachable for a given
instance can still increase register pressure for a lightweight ClosestHit path compiled in the
same function. The preferred Metal lowering is therefore to use generated visible-function tables
for Miss and ClosestHit dispatch. This keeps the source model close to SBT dispatch while giving
Metal a separate callable target for each reflected stage body. A switch remains useful as
semantic pseudocode or as a restricted fallback for very small programs or targets where
visible-function dispatch is not available.

Alternative considered: require users to write a structural Metal-like dispatch function.

```slang
struct PrimaryClosestHitDispatcher
{
    void invoke(rt::ClosestHitInput<PrimaryTraceContext> input)
    {
        switch (input.geometryIndex)
        {
        case 0: shadeOpaqueTriangle(input); break;
        case 1: shadeCurve(input); break;
        case 2: shadeSphere(input); break;
        }
    }
}
```

Slang could analyze this user-written dispatch logic and reflect enough metadata for D3D/Vulkan
host code to reconstruct the SBT. This is closer to the normal Metal mental model, but it is a
harder compiler problem:

- The compiler must recognize and preserve the user's dispatch structure.
- Reflection depends on successful control-flow analysis.
- Dynamic dispatch tables, buffer lookups, and helper functions complicate extraction.
- Small shader-code changes could affect host reflection in surprising ways.
- If lowered as a literal switch, heavy and light stage bodies can be forced into the same Metal
  shader compilation unit and interfere through register allocation.

The proposed design chooses the reverse direction: users declare the SBT structure directly, then
Slang synthesizes Metal dispatch. This is simpler to validate and easier to reflect.

#### 2.2.2 Resolving The Metal Function Table And Function Buffer Resource Mismatch With TraceProgramDescriptor

Metal introduces `intersection_function_table` resource objects and
`intersection_function_buffer_arguments` values that can be visible to shader code and bound from
host code. D3D and Vulkan instead use an SBT. The SBT is also built by host code, but it is not a
shader-visible resource, so shader code does not declare an SBT binding point.

D3D and Vulkan do not expose an equivalent shader-visible function table or function buffer.
Their comparable structure is the host-created SBT. It is useful to view the SBT as one
host-side database with separate hit-group, miss, and callable sections. A hit-group record can
name ClosestHit, AnyHit, and Intersection shaders together, while the miss and callable sections
are one-dimensional lists.

Figure 6 shows the SBT baseline that the portable layout is trying to preserve. The native
D3D/Vulkan SBT is one host-side object with hit-group, miss, and callable sections.

<a id="fig-d3d-vulkan-sbt-layout"></a>
![D3D and Vulkan shader binding table layout](figures/d3d-vulkan-sbt-layout.svg)

*Figure 6. D3D/Vulkan SBT layout: one host-side object contains hit-group, miss, and callable sections, and shader code has no SBT binding point.*

The previous subsection already makes the two sides share the same conceptual layout: the shader
source declares a `ProgramLayout`, and host code can reflect that layout to build the target-side
records. The remaining mismatch is purely about binding. Metal needs a shader-visible resource
for the function table or function buffer path, while D3D/Vulkan need no corresponding shader
parameter.

The proposed answer is to introduce an opaque resource type:

```slang
struct TraceProgramDescriptor<ProgramLayout>
    where ProgramLayout : ITraceProgramLayout
{
}
```

`TraceProgramDescriptor<ProgramLayout>` is described by `ProgramLayout`, but it does not itself
declare the trace context. Its job is to represent the target-specific descriptor object derived from
the reflected program layout.

On D3D and Vulkan, `TraceProgramDescriptor<ProgramLayout>` does not need to lower to a shader-visible
resource. The native SBT remains a host-side object, as shown in Figure 6.
Host code still uses the same `ProgramLayout` reflection to build hit-group, miss, and callable
SBT records, but shader code does not receive a Metal-style function-table or function-buffer
object.

On Metal, `TraceProgramDescriptor<ProgramLayout>` lowers to the physical resources needed by the
selected Metal traversal path. This proposal does not yet expose an API for users to choose the
Metal lowering form. For now, assume the Slang compiler and runtime can lower the same opaque
descriptor to either an ordinary `intersection_function_table` shape or an
`intersection_function_buffer_arguments` shape. The two shapes have the same source-level contract
but different target-side binding structure.

##### 2.2.2.1 Lowering To `intersection_function_table`

An ordinary Metal `intersection_function_table` is a shader-visible resource-object table. Figure
7 shows both the resource layout and the dispatch relationship that Slang needs to synthesize.

<a id="fig-intersection-function-table-layout"></a>
![Metal intersection function table resource layout with closest-hit dispatch zoom](figures/intersection-function-table-layout.svg)

*Figure 7. Ordinary intersection function table lowering: the left side shows the whole IFT resource, including function entries, buffer bindings, and three visible-function tables; the right side zooms into the paired dispatch between the ClosestHit visible-function table and IFT entries. A 1:1 mapping connects native IFT entries to logical hit slots. Miss and Callable visible-function tables use independent indices.*

**Native Layout**

The ordinary function-table layout has two conceptually separate parts:

```text
intersection_function_table<generatedTags>
    function entries:
        entry metalIFTIndex -> generated candidate-hit function
                               // AnyHit / custom Intersection behavior only
                               // mapped 1:1 to a logicalHitSlot

    resource bindings:
        buffer slot 0 -> generated descriptor data
                         // records, slot maps, bindless resource handles

        visible-function table slot 0 -> generated Miss functions
        visible-function table slot 1 -> generated ClosestHit functions
                                         // indexed by logicalHitSlot
        visible-function table slot 2 -> generated Callable functions
```

There are at most three generated visible-function tables associated with the function table:
`visible_function_table_0` is the Miss table, `visible_function_table_1` is the ClosestHit table,
and `visible_function_table_2` is the Callable table. These visible-function tables are resource
bindings of the ordinary Metal function table; they are not the native IFT entries themselves.

**Lowering Strategy**

When the compiler lowers `TraceProgramDescriptor<ProgramLayout>` to this ordinary function-table
path, it can be thought of as:

```text
TraceProgramDescriptor<ProgramLayout>
    intersection_function_table<generatedTags>:
        function entries -> generated candidate-hit functions
        buffer slot 0 -> generated descriptor data
        visible-function table slot 0 -> generated Miss functions
        visible-function table slot 1 -> generated ClosestHit functions
        visible-function table slot 2 -> generated Callable functions
```

The generated Metal-side use can be thought of as:

```slang
// Internal Metal-shaped pseudocode.
let table = descriptor.__intersectionFunctionTable;
let descriptorData = table.get_buffer<__RTDescriptorData*>(0);
let missFns = table.get_visible_function_table<__RTMissFn>(0);
let closestHitFns = table.get_visible_function_table<__RTClosestHitFn>(1);

let result = metalIntersector.intersect(desc.ray, scene, table, payload);

if (result.isNone)
{
    missFns[desc.missIndex](payload, descriptorData, desc.missIndex);
}
else
{
    uint logicalHitSlot =
        instanceOffset + geometryId * desc.sbtStride + desc.sbtOffset;

    closestHitFns[logicalHitSlot](payload, descriptorData, logicalHitSlot, result);
}
```

**Gaps, Fixes, And Constraints**

- **Gap 1: Native function-table entries do not dispatch every ray-tracing stage.** The native
  entries dispatch only candidate-hit behavior: AnyHit filtering and custom Intersection logic.
  They do not dispatch Miss or ClosestHit. Slang fixes this by lowering Miss and ClosestHit to
  generated visible-function tables stored as function-table resource bindings.

  **Constraint:** the host or Slang runtime must populate those generated visible-function tables
  as part of the `TraceProgramDescriptor` lowering. The Miss, ClosestHit, and Callable entries can
  be queried from the `ProgramLayout` reflection data described in the previous section. Miss uses
  `desc.missIndex`, ClosestHit uses `logicalHitSlot`, and Callable uses its own callable index.

- **Gap 2: Native function-table indexing is not the portable hit-slot formula.** Ordinary
  `intersection_function_table` traversal does not use `RayTraversalDesc.sbtOffset` or
  `RayTraversalDesc.sbtStride` when selecting candidate-hit functions. Metal selects an IFT entry
  from acceleration-structure offsets:

  ```text
  metalIFTIndex =
      geometryIntersectionFunctionTableOffset +
      instanceIntersectionFunctionTableOffset
  ```

  Slang treats `logicalHitSlot` as the portable identity of the hit group:

  ```text
  logicalHitSlot = instanceOffset + geometryId * desc.sbtStride + desc.sbtOffset
  ```

  **Constraint:** the host must construct acceleration-structure function-table offsets and function
  table contents so every `metalIFTIndex` selected by traversal maps to exactly one
  `logicalHitSlot`. The numbers do not need to be equal. What matters is that the selected
  candidate-hit function and the generated ClosestHit visible function represent the same logical
  hit group:

  ```text
  intersection_function_table[metalIFTIndex]
      -> generated AnyHit / custom Intersection candidate function
      -> maps to logicalHitSlot

  visible_function_table_1[logicalHitSlot]
      -> generated ClosestHit function
  ```

  Separate `TraceProgramDescriptor` values per ray type are also a natural way to keep this
  mapping simple.

**Concrete Example**

Suppose one trace call uses `desc.sbtStride = 2`, `desc.sbtOffset = 1`, and logical
`instanceOffset = 0`. The portable logical slots are:

```text
logicalHitSlot(geometry 0) = 0 + geometryId 0 * 2 + 1 = 1
logicalHitSlot(geometry 1) = 0 + geometryId 1 * 2 + 1 = 3
```

The Metal IFT indices do not need to be `1` and `3`. They only need a 1:1 mapping back to logical
slots `1` and `3`:

```text
instance:
    instanceIntersectionFunctionTableOffset = 8

geometry 0:
    geometryIntersectionFunctionTableOffset = 0
    metalIFTIndex = 0 + 8 = 8
    maps to logicalHitSlot 1

geometry 1:
    geometryIntersectionFunctionTableOffset = 4
    metalIFTIndex = 4 + 8 = 12
    maps to logicalHitSlot 3

intersection_function_table[8]  -> candidate function for logicalHitSlot 1
intersection_function_table[12] -> candidate function for logicalHitSlot 3

visible_function_table_1[1] -> ClosestHit for logicalHitSlot 1
visible_function_table_1[3] -> ClosestHit for logicalHitSlot 3
```

##### 2.2.2.2 Lowering To `intersection_function_buffer_arguments`

The Metal 4 function-buffer path represents candidate-hit dispatch with an
`intersection_function_buffer_arguments` value rather than an ordinary resource-object table.
Figure 8 shows both the function-buffer layout and the descriptor-side data needed for generated
Miss, ClosestHit, and Callable dispatch.

<a id="fig-intersection-function-buffer-layout"></a>
![Metal intersection function buffer arguments layout](figures/intersection-function-buffer-layout.svg)

*Figure 8. Metal function-buffer lowering: `intersection_function_buffer_arguments` carries the candidate-hit table, while descriptor-side data carries records and visible-function dispatch resources for Miss, ClosestHit, and Callable dispatch.*

**Native Layout**

The native function-buffer argument describes the traversal-time candidate-hit table:

```text
intersection_function_buffer_arguments:
    intersection_function_buffer      -> raw table of generated candidate-hit functions
    intersection_function_buffer_size -> byte size of that table
    intersection_function_stride      -> byte stride between table entries
```

The function-buffer table still only contains candidate-hit behavior: AnyHit filtering and custom
Intersection logic. Descriptor-side data carries the rest of the portable trace program state:
records, slot maps, bindless resources, and generated visible-function tables for Miss,
ClosestHit, and Callable dispatch.

**Lowering Strategy**

When the compiler lowers `TraceProgramDescriptor<ProgramLayout>` to this function-buffer path, it
can be thought of as:

```text
TraceProgramDescriptor<ProgramLayout>
    intersection_function_buffer_arguments:
        intersection_function_buffer      -> table of generated candidate-hit functions
        intersection_function_buffer_size -> byte size of the table
        intersection_function_stride      -> byte stride between entries

    generated descriptor-side data:
        records
        slot maps
        bindless resource handles
        visible-function table for Miss
        visible-function table for ClosestHit
        visible-function table for Callable
```

The generated Metal-side use can be thought of as:

```slang
// Internal Metal-shaped pseudocode.
let ifbArgs = descriptor.__intersectionFunctionBufferArguments;
let descriptorData = descriptor.__generatedDescriptorData;
let missFns = descriptorData.missVisibleFunctions;
let closestHitFns = descriptorData.closestHitVisibleFunctions;

metalIntersector.set_base_id(desc.sbtOffset);
metalIntersector.set_geometry_multiplier(desc.sbtStride);

let result = metalIntersector.intersect(desc.ray, scene, ifbArgs, descriptorData, payload);

if (result.isNone)
{
    missFns[desc.missIndex](payload, descriptorData, desc.missIndex);
}
else
{
    uint logicalHitSlot =
        instanceOffset + geometryId * desc.sbtStride + desc.sbtOffset;

    closestHitFns[logicalHitSlot](payload, descriptorData, logicalHitSlot, result);
}
```

**Gaps, Fixes, And Constraints**

- **Gap 1: The function buffer still does not dispatch every ray-tracing stage.** Like ordinary
  `intersection_function_table`, the function-buffer table dispatches candidate-hit behavior, not
  Miss or ClosestHit. Slang fixes this by keeping candidate-hit functions in the function buffer
  and lowering Miss, ClosestHit, and Callable to generated visible-function tables in
  descriptor-side data.

  **Constraint:** the host or Slang runtime must populate the generated visible-function tables
  from the `ProgramLayout` reflection data described in the previous section. Miss uses
  `desc.missIndex`, ClosestHit uses `logicalHitSlot`, and Callable uses its own callable index.

- **Gap 2: The host must still build a target-side candidate-hit table.** The function-buffer form
  is closer to the D3D/Vulkan SBT model than ordinary `intersection_function_table`, because the
  table representation includes an explicit stride. Conceptually, this lets the backend model the
  portable hit slot directly:

  ```text
  logicalHitSlot = instanceOffset + geometryId * desc.sbtStride + desc.sbtOffset
  ```

  **Constraint:** the host or Slang runtime must populate the function buffer consistently with the
  reflected `ProgramLayout` hit groups and with the exact Metal IFB indexing rules. The same
  logical slot should select both the candidate-hit function and the generated ClosestHit visible
  function:

  ```text
  functionBuffer[logicalHitSlot]
      -> generated AnyHit / custom Intersection candidate function

  visible_function_table_1[logicalHitSlot]
      -> generated ClosestHit function
  ```

**Concrete Example**

Suppose one trace call uses `desc.sbtStride = 2`, `desc.sbtOffset = 1`, and logical
`instanceOffset = 0`. The portable logical slots are the same as in the ordinary function-table
example:

```text
logicalHitSlot(geometry 0) = 0 + geometryId 0 * 2 + 1 = 1
logicalHitSlot(geometry 1) = 0 + geometryId 1 * 2 + 1 = 3
```

For the function-buffer lowering, the host or Slang runtime can lay out the candidate-hit table by
logical hit slot:

```text
functionBuffer[0] -> unused or default candidate function
functionBuffer[1] -> candidate function for logicalHitSlot 1
functionBuffer[2] -> unused or default candidate function
functionBuffer[3] -> candidate function for logicalHitSlot 3

visible_function_table_1[1] -> ClosestHit for logicalHitSlot 1
visible_function_table_1[3] -> ClosestHit for logicalHitSlot 3
```

The physical byte address of each function-buffer entry is derived from
`intersection_function_buffer` plus `logicalHitSlot * intersection_function_stride`, subject to the
exact Metal IFB traversal rules. The useful difference from ordinary function-table lowering is
that the function-buffer table can be organized directly around the portable slot calculation,
instead of requiring a separate `metalIFTIndex` to `logicalHitSlot` mapping.

#### 2.2.3 Resolving The Metal Tag-List Issue With Context Types

With the descriptor abstraction separated, the tag-list issue still needs a way to answer: "which
trace object, hit shader, and intersection function share the same semantic requirements?" The
proposed answer is a context type. A trace context defines the properties of a trace family:

```slang
interface ITraceContext
{
    associatedtype Payload;
    associatedtype ASKind;
    associatedtype Motion;
    static const RayDataTags dataTags;
    static const int maxLevels;
}
```

A hit group context can specialize the trace context with a primitive kind and a record type:

```slang
interface IHitGroupContext
{
    associatedtype TraceContext : ITraceContext;
    associatedtype Primitive : IIntersectionPrimitive;
    associatedtype Record;
}
```

A trace program layout connects the ray tracer and every grouped shader through the same trace
context:

```slang
struct PrimaryTraceContext : rt::ITraceContext
{
    typealias Payload = RadiancePayload;
    typealias ASKind = rt::InstanceAS;
    typealias Motion = rt::NoMotion;
    static const rt::RayDataTags dataTags = rt::RayDataTags::Triangle;
    static const int maxLevels = 0;
}

struct PrimaryTriangleContext : rt::IHitGroupContext
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = PrimaryHitRecord;
}

struct PrimaryMissContext : rt::IMissGroupContext
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Record = PrimaryMissRecord;
}

struct PrimaryMissGroup : rt::IMissGroup
{
    typealias Context = PrimaryMissContext;
    typealias Miss = PrimaryMiss;
}

struct PrimaryTriangleGroup : rt::IHitGroup
{
    typealias Context = PrimaryTriangleContext;
    typealias ClosestHit = PrimaryTriangleClosestHit;
    typealias AnyHit = PrimaryTriangleAnyHit;
    typealias IntersectionAttributes = rt::NoAttributes;
    typealias Intersection = rt::NoIntersection<PrimaryTriangleContext>;
}

struct PrimaryTraceProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = PrimaryTraceContext;

    typealias MissGroups = rt::MissGroupList<
        TraceContext,
        PrimaryMissGroup>;

    typealias HitGroups = rt::HitGroupList<
        TraceContext,
        PrimaryTriangleGroup>;

    typealias CallableGroups = rt::NoCallableGroups<TraceContext>;
}

rt::TraceProgramDescriptor<PrimaryTraceProgramLayout> gPrimaryDescriptor;

[shader("raygeneration")]
void rayGen()
{
    RadiancePayload payload;
    rt::RayTracer<PrimaryTraceProgramLayout> tracer;
    tracer.trace(desc, scene, gPrimaryDescriptor, payload);
}
```

This gives the compiler a source-level relationship:

- `RayTracer<PrimaryTraceProgramLayout>` identifies one `ITraceProgramLayout`.
- `PrimaryTraceProgramLayout.TraceContext` defines the trace-wide Metal tags.
- Every group in `PrimaryTraceProgramLayout.HitGroups` is constrained to that trace context.
- Any-hit and intersection stage structs receive input types derived from the same context.

Figure 9 zooms into the handwritten code lines that carry the contract: the trace call names the
program layout, and each stage `invoke(...)` method names the input context it accepts.

<a id="fig-context-reachability"></a>
![Context connects ray tracer and hit shaders](figures/context-reachability.svg)

*Figure 9. Context reachability contract: the user-written trace call and stage `invoke(...)` signatures give the compiler a source-visible relationship between `RayTracer<ProgramLayout>`, the trace-wide context, hit groups, and stage input types.*

This does not prove that arbitrary host data is correct. If the host builds an SBT or Metal
function table that violates the reflected program layout, the program can still be wrong. The
goal is to make the shader-side contract explicit enough that:

- Slang can generate Metal tags for reachable custom intersection functions.
- Slang can reject shader declarations that are inconsistent inside the program layout.
- Reflection can expose the expected table to host code.
- Validation layers or Slang runtime helpers can compare host records against the reflected
  contract.

### 2.3 Writing Stages As Interface-Conforming Types

In the new model, miss, closest-hit, any-hit, and intersection logic are not written as independent
entry points. They are written as ordinary structs that conform to built-in stage interfaces.

Example:

```slang
struct PrimaryTriangleClosestHit
    : rt::IClosestHitShader<PrimaryTriangleContext>
{
    void invoke(rt::ClosestHitInput<PrimaryTriangleContext> input)
    {
        input.payload.color = float4(input.distance, 0.0, 0.0, 1.0);
    }
}

struct PrimaryTriangleAnyHit
    : rt::IAnyHitShader<PrimaryTriangleContext>
{
    void invoke(rt::AnyHitInput<PrimaryTriangleContext> input)
    {
        if (isTransparent(input.triangle))
            input.ignoreHit();
    }
}

struct PrimarySphereIntersection
    : rt::IIntersectionShader<PrimarySphereContext, SphereHitAttributes>
{
    rt::IntersectionReturn<PrimarySphereContext, SphereHitAttributes>
    invoke(rt::IntersectionInput<PrimarySphereContext> input)
    {
        SphereHitAttributes attr;
        float t = intersectSphere(input, attr);
        return rt::IntersectionReturn<PrimarySphereContext, SphereHitAttributes>::accept(t, attr);
    }
}
```

The compiler is responsible for lowering these structs to the target form:

- D3D and Vulkan: generated native entry points and hit groups, connected to SBT records.
- Metal: generated intersection functions for any-hit/custom-intersection behavior, plus generated
  post-trace Miss and ClosestHit visible-function dispatch.

The user writes one source-level model. The target backend chooses the appropriate pipeline shape.

## 3. Migration Examples

### 3.1 Migrating Existing Metal Code To The New API

Existing Metal users often write post-trace logic directly:

```metal
kernel void rayGen(...)
{
    intersector<instancing, triangle_data> tracer;
    tracer.assume_geometry_type(geometry_type::triangle);

    auto result = tracer.intersect(ray, scene, intersectionFunctionBuffer, payload);

    if (result.type == intersection_type::none)
    {
        miss(payload);
    }
    else
    {
        uint slot = result.geometry_id;

        switch (slot)
        {
        case 0: shadeOpaqueTriangle(payload, result); break;
        case 1: shadeAlphaTriangle(payload, result); break;
        case 2: shadeProceduralSphere(payload, result); break;
        }
    }
}
```

With the proposed API, the user moves the manually dispatched operations into stage structs and
declares the trace program layout structurally. The list order defines the portable logical hit
slots:

```slang
struct PrimaryMissGroup : rt::IMissGroup
{
    typealias Context = PrimaryMissContext;
    typealias Miss = PrimaryMiss;
}

struct PrimaryOpaqueTriangleGroup : rt::IHitGroup
{
    typealias Context = PrimaryTriangleContext;
    typealias ClosestHit = PrimaryOpaqueTriangleClosestHit;
    typealias AnyHit = rt::NoAnyHit<PrimaryTriangleContext>;
    typealias IntersectionAttributes = rt::NoAttributes;
    typealias Intersection = rt::NoIntersection<PrimaryTriangleContext>;
}

struct PrimaryAlphaTriangleGroup : rt::IHitGroup
{
    typealias Context = PrimaryTriangleContext;
    typealias ClosestHit = PrimaryAlphaTriangleClosestHit;
    typealias AnyHit = PrimaryAlphaTriangleAnyHit;
    typealias IntersectionAttributes = rt::NoAttributes;
    typealias Intersection = rt::NoIntersection<PrimaryTriangleContext>;
}

struct PrimarySphereGroup : rt::IHitGroup
{
    typealias Context = PrimarySphereContext;
    typealias ClosestHit = PrimarySphereClosestHit;
    typealias AnyHit = rt::NoAnyHit<PrimarySphereContext>;
    typealias IntersectionAttributes = SphereHitAttributes;
    typealias Intersection = PrimarySphereIntersection;
}

struct PrimaryTraceProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = PrimaryTraceContext;

    typealias MissGroups = rt::MissGroupList<
        TraceContext,
        PrimaryMissGroup>;             // miss[0]

    typealias HitGroups = rt::HitGroupList<
        TraceContext,
        PrimaryOpaqueTriangleGroup,     // hitGroup[0]
        PrimaryAlphaTriangleGroup,      // hitGroup[1]
        PrimarySphereGroup>;            // hitGroup[2]

    typealias CallableGroups = rt::NoCallableGroups<TraceContext>;
}

rt::TraceProgramDescriptor<PrimaryTraceProgramLayout> gPrimaryDescriptor;
```

Ray-generation code becomes:

```slang
[shader("raygeneration")]
void rayGen()
{
    RadiancePayload payload;

    rt::RayTraversalDesc desc;
    desc.ray = makeRay();
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 1;
    desc.missIndex = 0;

    rt::RayTracer<PrimaryTraceProgramLayout> tracer;
    tracer.trace(desc, scene, gPrimaryDescriptor, payload);
}
```

For Metal, Slang generates code that is equivalent to the user's old post-trace dispatch, but the
source of truth is now `PrimaryTraceProgramLayout.HitGroups` and
`PrimaryTraceProgramLayout.MissGroups`. The generated dispatch uses Metal visible functions for
Miss and ClosestHit rather than emitting one large switch containing every stage body.

Metal host migration:

1. Query `PrimaryTraceProgramLayout` through Slang reflection.
2. Populate the generated Miss and ClosestHit visible-function tables in the
   `TraceProgramDescriptor` lowering from the reflected miss and hit-group slots.
3. Populate the candidate-hit dispatch resource from the reflected hit groups:
   AnyHit and custom Intersection stages go into either an
   `intersection_function_buffer_arguments` lowering or an ordinary `intersection_function_table`
   lowering.
4. If the lowering uses `intersection_function_buffer_arguments`, lay out the candidate-hit table
   by the same logical hit slots that `RayTraversalDesc` computes. In this example,
   `geometry_id` values `0`, `1`, and `2` map directly to logical hit slots `0`, `1`, and `2`.
5. If the lowering uses ordinary `intersection_function_table`, choose native Metal IFT indices
   for each reachable logical hit slot and build acceleration-structure function-table offsets so
   traversal selects the corresponding native index. The native IFT index and logical hit slot do
   not need to be numerically equal, but the mapping must be 1:1.

This keeps Metal's AnyHit/custom-Intersection dispatch aligned with Slang's generated visible
function dispatch for Miss and ClosestHit.

### 3.2 Migrating Existing Slang D3D/Vulkan Ray Tracing Code

Existing Slang code usually has independent pipeline entry points:

```slang
[shader("raygeneration")]
void rayGen()
{
    RadiancePayload payload;

    TraceRay(
        scene,
        flags,
        instanceMask,
        rayContributionToHitGroupIndex,
        multiplierForGeometryContributionToHitGroupIndex,
        missShaderIndex,
        ray,
        payload);
}

[shader("miss")]
void miss(inout RadiancePayload payload)
{
    payload.color = backgroundColor;
}

[shader("closesthit")]
void closestHit(inout RadiancePayload payload, BuiltInTriangleIntersectionAttributes attr)
{
    payload.color = shadeTriangle(attr);
}
```

The migrated shader keeps the same conceptual data, but moves stage bodies into typed structs:

```slang
struct PrimaryMissContext : rt::IMissGroupContext
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Record = PrimaryMissRecord;
}

struct PrimaryMiss : rt::IMissShader<PrimaryMissContext>
{
    void invoke(rt::MissInput<PrimaryMissContext> input)
    {
        input.payload.color = backgroundColor;
    }
}

struct PrimaryTriangleClosestHit
    : rt::IClosestHitShader<PrimaryTriangleContext>
{
    void invoke(rt::ClosestHitInput<PrimaryTriangleContext> input)
    {
        input.payload.color = shadeTriangle(input.triangle);
    }
}
```

The old trace parameters map directly to fields in `RayTraversalDesc`:

```slang
rt::TraceProgramDescriptor<PrimaryTraceProgramLayout> gPrimaryDescriptor;

rt::RayTraversalDesc desc;
desc.ray = ray;
desc.instanceMask = instanceMask;
desc.sbtOffset = rayContributionToHitGroupIndex;
desc.sbtStride = multiplierForGeometryContributionToHitGroupIndex;
desc.missIndex = missShaderIndex;

rt::RayTracer<PrimaryTraceProgramLayout> tracer;
tracer.trace(desc, scene, gPrimaryDescriptor, payload);
```

D3D/Vulkan host migration:

1. Query `PrimaryTraceProgramLayout` through Slang reflection.
2. For each reflected miss group, add a miss record at its list position.
3. For each reflected hit group, add a hit group record at its list position.
4. Populate any reflected shader-record or local-root data associated with the miss, hit, and
   callable groups.
5. Use the same application data that previously produced `rayContributionToHitGroupIndex`,
   `multiplierForGeometryContributionToHitGroupIndex`, and `missShaderIndex`.

The native SBT model is not replaced, and `TraceProgramDescriptor<PrimaryTraceProgramLayout>` does
not need to become a shader-visible resource on D3D/Vulkan. The new shader declarations make the
intended SBT layout visible to Slang, which enables Metal lowering and gives host code a single
reflected contract.

### 3.3 Host Reflection Patterns

The reflection API shape is not finalized. The expected information is:

```cpp
struct ReflectedTraceProgramLayout
{
    TypeReflection* traceContextType;
    List<ReflectedMissGroup> missGroups;
    List<ReflectedHitGroup> hitGroups;
    List<ReflectedCallableGroup> callableGroups;
};

struct ReflectedMissGroup
{
    int slot;
    TypeReflection* contextType;
    TypeReflection* recordType;
    EntryPointReflection* generatedMissEntryPoint;
};

struct ReflectedHitGroup
{
    int slot;
    TypeReflection* contextType;
    TypeReflection* recordType;
    TypeReflection* intersectionAttributesType;
    EntryPointReflection* generatedClosestHitEntryPoint;
    EntryPointReflection* generatedAnyHitEntryPoint;
    EntryPointReflection* generatedIntersectionEntryPoint;
};

struct ReflectedCallableGroup
{
    int slot;
    TypeReflection* contextType;
    TypeReflection* recordType;
    EntryPointReflection* generatedCallableEntryPoint;
};
```

The helper names in the following examples are illustrative; the reflection API shape and runtime
ownership model are still open design questions.

Pattern A: D3D/Vulkan native SBT.

```cpp
auto programLayout = reflection->findTraceProgramLayout("PrimaryTraceProgramLayout");

for (auto miss : programLayout.missGroups)
{
    sbt.setMissRecord(
        miss.slot,
        miss.generatedMissEntryPoint,
        buildShaderRecordData(miss.recordType));
}

for (auto hit : programLayout.hitGroups)
{
    sbt.setHitGroup(
        hit.slot,
        hit.generatedClosestHitEntryPoint,
        hit.generatedAnyHitEntryPoint,
        hit.generatedIntersectionEntryPoint,
        buildShaderRecordData(hit.recordType));
}

for (auto callable : programLayout.callableGroups)
{
    sbt.setCallableRecord(
        callable.slot,
        callable.generatedCallableEntryPoint,
        buildShaderRecordData(callable.recordType));
}
```

The application still controls geometry contribution, instance contribution, stride, and offset.
The reflected slots tell the application which shader group belongs at each SBT slot, while the
reflected record types tell it what local-root/shader-record data each record expects.

Pattern B: Metal intersection function buffer.

```cpp
auto programLayout = reflection->findTraceProgramLayout("PrimaryTraceProgramLayout");

for (auto miss : programLayout.missGroups)
{
    descriptor.setGeneratedMissVisibleFunction(
        miss.slot,
        miss.generatedMissEntryPoint);
    descriptor.setMissRecordData(
        miss.slot,
        buildShaderRecordData(miss.recordType));
}

for (auto hit : programLayout.hitGroups)
{
    descriptor.setGeneratedClosestHitVisibleFunction(
        hit.slot,
        hit.generatedClosestHitEntryPoint);

    if (hit.generatedAnyHitEntryPoint || hit.generatedIntersectionEntryPoint)
    {
        functionBuffer.setFunction(
            hit.slot,
            buildGeneratedCandidateHitFunction(hit));
    }

    descriptor.setHitRecordData(
        hit.slot,
        buildShaderRecordData(hit.recordType));
}

for (auto callable : programLayout.callableGroups)
{
    descriptor.setGeneratedCallableVisibleFunction(
        callable.slot,
        callable.generatedCallableEntryPoint);
    descriptor.setCallableRecordData(
        callable.slot,
        buildShaderRecordData(callable.recordType));
}
```

For function-buffer lowering, the candidate-hit table is organized by the same logical slots used
by generated ClosestHit dispatch. The host does not author custom post-trace dispatch logic for
Metal, but the `TraceProgramDescriptor` lowering may expose generated visible-function table and
record resources that the host or Slang runtime populates from the reflected program layout.

Pattern C: Metal intersection function table.

Metal's ordinary function table path uses the same reflected `ProgramLayout`, but candidate-hit
selection is driven by acceleration-structure function-table offsets instead of directly by
`RayTraversalDesc.sbtOffset` and `RayTraversalDesc.sbtStride`. Host setup must therefore align the
ordinary function-table entries with the logical hit-group slots used by generated ClosestHit
visible-function dispatch.

```cpp
auto programLayout = reflection->findTraceProgramLayout("PrimaryTraceProgramLayout");

for (auto miss : programLayout.missGroups)
{
    descriptor.setGeneratedMissVisibleFunction(
        miss.slot,
        miss.generatedMissEntryPoint);
    descriptor.setMissRecordData(
        miss.slot,
        buildShaderRecordData(miss.recordType));
}

for (auto hit : programLayout.hitGroups)
{
    descriptor.setGeneratedClosestHitVisibleFunction(
        hit.slot,
        hit.generatedClosestHitEntryPoint);

    if (hit.generatedAnyHitEntryPoint || hit.generatedIntersectionEntryPoint)
    {
        uint metalIFTIndex = engineLayout.chooseMetalFunctionTableIndex(hit.slot);
        functionTable.setFunction(
            metalIFTIndex,
            buildGeneratedCandidateHitFunction(hit));

        engineLayout.recordMetalFunctionTableMapping(
            hit.slot,
            metalIFTIndex);
    }

    descriptor.setHitRecordData(
        hit.slot,
        buildShaderRecordData(hit.recordType));
}

for (auto callable : programLayout.callableGroups)
{
    descriptor.setGeneratedCallableVisibleFunction(
        callable.slot,
        callable.generatedCallableEntryPoint);
    descriptor.setCallableRecordData(
        callable.slot,
        buildShaderRecordData(callable.recordType));
}
```

This is valid when the engine also builds geometry and instance acceleration-structure metadata
so traversal selects the same `metalIFTIndex` for primitives that post-trace dispatch will map to
`hit.slot`. The mapping from `metalIFTIndex` to `hit.slot` must be 1:1, but the numbers do not
need to be equal. Callable and Miss visible-function tables do not use this hit-slot mapping:
Miss is indexed by `missIndex`, and Callable is indexed by the callable index. The proposal does
not yet define an API for choosing ordinary function-table lowering versus function-buffer
lowering; that is left as a backend/runtime policy to revisit.

Pattern D: Manual host construction without reflection.

A developer can still build the table by reading the shader source if the project chooses a fixed
layout convention:

```slang
typealias HitGroups = rt::HitGroupList<
    TraceContext,
    PrimaryOpaqueTriangleGroup,     // hitGroup[0]
    PrimaryAlphaTriangleGroup,      // hitGroup[1]
    PrimarySphereGroup>;            // hitGroup[2]
```

Reflection is strongly preferred because it removes duplicated source-of-truth in host code and
enables validation, but the layout is intentionally visible and reviewable in shader source.

## 4. Open Design Questions

- Exact reflection API names and ownership model.
- Exact generated entry-point naming rules for D3D and Vulkan.
- Whether Metal ordinary function-table lowering should be a restricted feature or a fully
  supported path.
- How to represent custom intersection attributes on Metal when native return-value limits are
  insufficient.
- How much runtime validation Slang should provide between reflected `ITraceProgramLayout` data and
  host-created SBT or Metal function-buffer state.
