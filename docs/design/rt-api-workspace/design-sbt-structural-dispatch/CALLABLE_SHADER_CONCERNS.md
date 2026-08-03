# Callable Shader Concerns

This note records concerns about adding D3D/Vulkan callable shaders to the
current `TraceProgram` design. The goal is not to reject callable shaders. The
goal is to keep the risks visible while the core hit/miss/any-hit/intersection
model is still being shaped.

## Current Intuition

Callable shaders probably fit the broad `TraceProgram` reflection model:

```slang
struct PrimaryProgram : rt::TraceProgram
{
    typealias TraceContext = PrimaryTraceContext;
    typealias MissGroups = rt::MissGroupList<...>;
    typealias HitGroups = rt::HitGroupList<...>;
    typealias CallableGroups = rt::CallableGroupList<...>;
}
```

The pipeline build step would compile the possible callable shader entry points
and the host would build a callable shader table. At runtime, shader code would
only choose an index into that table.

That broad shape is plausible, but there are several design concerns.

## Concern 1: Callables Are Not Traversal Dispatch

Hit groups and miss groups are selected by ray traversal:

```text
trace ray -> traversal -> hit/miss SBT selection
```

Callable shaders are different:

```text
shader code -> explicit callable-table index -> callable invocation
```

This means callable shaders can be described by the same reflection container,
but they should not be modeled as another kind of hit group. They are an
independent dynamic-call table used from ray tracing shader code.

Design implication:

```slang
typealias HitGroups = ...;
typealias MissGroups = ...;
typealias CallableGroups = ...;
```

is safer than trying to fold callables into `HitGroupList`.

## Concern 2: Typed Dynamic Indexing Is The Real Safety Problem

A normal device function call is statically typed:

```slang
evalLambert(hit, material);
```

A callable shader call is table-indexed:

```slang
CallShader(material.callableIndex, data);
```

If `material.callableIndex` is dynamic, every callable reachable through that
index domain must agree on the call-data type. Otherwise the call site cannot be
type safe.

Possible API direction:

```slang
struct MaterialCallableDomain : rt::ICallableDomain
{
    typealias Parameter = MaterialCallData;
}

typealias CallableGroups = rt::CallableGroupList<
    rt::CallableSlot<MaterialCallableDomain, 0, LambertCallable>,
    rt::CallableSlot<MaterialCallableDomain, 1, GlassCallable>>;
```

Then shader code calls a typed domain:

```slang
rt::CallableTable<MaterialCallableDomain> materialCallables;
materialCallables.call(material.callableIndex, data);
```

Open question:

Should callable slots be grouped by a typed callable domain, or should the call
data type be attached directly to each `CallableGroupList`?

## Concern 3: Reflection Cannot Prove Runtime Indices

Reflection can expose this:

```text
MaterialCallableDomain
    slot 0 -> LambertCallable
    slot 1 -> GlassCallable
```

Reflection cannot prove this:

```slang
material.callableIndex < MaterialCallableDomain.slotCount
```

or this:

```text
material.callableIndex points at the material shader the asset author intended
```

Those remain host/data validation problems. The type system can restrict the
table domain and call-data type, but it cannot fully validate runtime index
contents.

Design implication:

The proposal should be explicit that `TraceProgram` reflection gives the host a
finite set of callable slots to build, but runtime index correctness is still an
engine responsibility.

## Concern 4: Metal Visible Function Tables Are Similar But Not Equivalent

D3D/Vulkan callable shaders are ray tracing shader stages selected from a
callable shader table.

Metal visible function tables are resource-like typed tables of visible
functions. They are useful for indirect calls, but they are not the same
abstraction as a D3D/Vulkan callable shader stage.

Design implication:

We should not define the portable callable API around Metal visible function
tables. A visible function table may be one Metal lowering strategy for a
restricted callable pattern, but it should not be treated as semantically
identical to D3D/Vulkan callable shaders.

## Concern 5: Callable Shaders Have Stack And Scheduling Costs

Callable shaders are not just inlined device functions with different syntax.
They are separate shader invocations in the ray tracing pipeline model and can
affect stack size, recursion, scheduling, and optimization.

Design implication:

Callable shaders should not be encouraged as a replacement for ordinary helper
functions. The API should make the intended use clear:

```text
Use device functions when the callee is statically known.
Use callable shaders when the callee must be selected from a shader table.
```

## Concern 6: This May Distract From The Core Metal Portability Problem

The first-order portability problem is still:

```text
D3D/Vulkan: native SBT dispatches miss and closest-hit.
Metal: shader code must perform post-trace miss and closest-hit dispatch.
```

Callable shaders do not solve that problem. They may fit the same reflection
framework later, but adding them to the first proposal could make the design
look broader and less settled than the core hit/miss/hit-group model.

Recommended staging:

1. Finish `TraceProgram` for hit groups and miss groups.
2. Define Metal lowering for post-trace closest-hit/miss dispatch.
3. Define host reflection for hit/miss groups.
4. Add callable shader groups as a follow-up extension if the same structure
   still feels natural.

## Provisional Position

Callable shaders are compatible with the current direction, but they should be
modeled as an optional typed callable-table extension to `TraceProgram`, not as
part of the core dispatch model.

The core proposal should stay focused on the dispatch mismatch that blocks
Metal support today.
