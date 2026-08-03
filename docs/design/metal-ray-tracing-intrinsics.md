# Metal Ray Tracing Intrinsics

This document summarizes the Metal Shading Language ray tracing intrinsics as a
developer-facing specification for Slang API design work. It is based on Apple's
Metal Shading Language Specification PDF dated 2025-10-23 and checked against the
local Metal SDK header `<metal_raytracing>` as installed with Metal compiler
`32023.883`.

The focus is shader-visible API. Host-side Metal objects are discussed only where
they affect shader correctness.

Reading map:

1. Sections 1-4 orient the reader: workflow, includes, availability, and tag
   notation.
2. Section 5 defines the core data types.
3. Sections 6-10 cover the traversal APIs: `intersector`, intersection
   functions, function tables, IFB, and `intersection_query`.
4. Sections 11-12 collect utilities and exact tag validity rules.
5. Sections 13-14 show examples and practical tips.
6. Sections 15-16 capture Slang API design notes and sources.

## 1. Development Workflow

1. On the host, require `MTLDevice.supportsRaytracing` before using ray tracing.
   Require `supportsRaytracingFromRender` if ray tracing calls are made from
   vertex or fragment shaders. Require motion/curve/Metal 4 capabilities before
   using the matching shader features.
2. Build one or more `MTLAccelerationStructure` resources:
   - primitive acceleration structures contain triangles, bounding boxes, and,
     in Metal 3.1 and later, curves;
   - instance acceleration structures reference primitive acceleration structures
     and provide instance masks, transforms, and optional user instance IDs.
3. In MSL, include `<metal_raytracing>` and use `metal::raytracing`.
4. Pick one traversal model:
   - `intersector<...>`: Metal traverses and calls intersection functions
     from an intersection function table or Metal 4 intersection function buffer.
   - `intersection_query<...>`: shader code explicitly advances traversal
     with `next()` and commits candidate intersections.
5. When a ray tracing type has a `<...>` capability list, keep that list
   consistent across the related shader objects. Tag Notation introduces this
   syntax, and Tag Reference gives the exact rules.
6. Construct a valid `ray`, configure traversal state, trace, then consume either
   an `intersection_result<...>` or the candidate/committed query state.

## 2. Headers and Namespaces

Use:

```metal
#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;
using namespace metal::raytracing;
```

Curve helper functions are in `<metal_curves>`.

Ray payload parameters in intersection functions use the `ray_data` address
space. `ray_data` objects are only accessible inside intersection functions.

## 3. Availability Summary

| Feature | Availability |
| --- | --- |
| Ray tracing types and `intersector` | Metal 2.3 and later |
| Compute shader ray tracing | Metal 2.3 and later |
| Vertex, fragment, and tile shader ray tracing | Metal 2.4 and later, subject to device support |
| Motion tags `primitive_motion`, `instance_motion` | Metal 2.4 and later |
| `intersection_query` | Metal 2.4 and later |
| `primitive_data` result/query data | Metal 3 and later |
| Curves, `curve_data`, `max_levels<Count>` | Metal 3.1 and later |
| `intersection_result_ref` callback form | Metal 3.2 and later on supported Apple silicon |
| `intersection_function_buffer`, `user_data` | Metal 4 and later |

## 4. Tag Notation

<a id="tag-notation"></a>

Many Metal ray tracing types are written as templates, for example
`intersector<instancing, triangle_data>`. The template arguments are called
tags. For a first reading, think of tags like enum-like compile-time flags: they
select the acceleration-structure shape, enable extra result data, and make some
methods available. They are not runtime values.

This document introduces the core types and functions first. The full tag list
and valid combinations are collected later in [Tag Reference](#tag-reference).
See [Practical Tips](#practical-tips) for common gotchas such as object-space
intersection attributes versus `world_space_data`.

## 5. Core Types

<a id="core-types"></a>

### 5.1 `ray`

```metal
struct ray {
    ray(float3 origin = 0.0f,
        float3 direction = 0.0f,
        float min_distance = 0.0f,
        float max_distance = INFINITY);
    float3 origin;
    float3 direction;
    float min_distance;
    float max_distance;
};
```

For a top-level trace call, `origin` and `direction` are world-space values: the
same coordinate system used to place instances in the scene. If traversal enters
an instance, Metal applies that instance's world-to-object transform before
calling an intersection function. Therefore `[[origin]]` and `[[direction]]` in
an intersection function are object-space values, in the local coordinate system
of the primitive acceleration structure.

For example, suppose a sphere is stored in a primitive acceleration structure at
local center `(0, 0, 0)`, but the host instances that primitive with a transform
that places it at world position `(10, 0, 0)`. The caller traces a world-space
ray toward `(10, 0, 0)`. The bounding-box intersection function still tests
against a sphere centered at `(0, 0, 0)`, because Metal passes the transformed
object-space ray:

```metal
ray worldRay(float3(0.0f, 0.0f, 0.0f),
             normalize(float3(10.0f, 0.0f, 0.0f)),
             0.0f,
             100.0f);

intersector<instancing> i;
intersection_result<instancing> hit = i.intersect(worldRay, scene, 0xffu);

struct SphereIntersectionResult
{
    bool accept [[accept_intersection]];
    bool continueSearch [[continue_search]];
    float distance [[distance]];
};

[[intersection(bounding_box, instancing)]]
SphereIntersectionResult sphereIntersection(
    float3 objectOrigin [[origin]],
    float3 objectDirection [[direction]],
    float minDistance [[min_distance]],
    float maxDistance [[max_distance]])
{
    // Test in primitive-local coordinates. The sphere is centered at local zero,
    // even if an instance transform places it elsewhere in world space.
    float3 center = float3(0.0f);
    float3 oc = objectOrigin - center;
    float a = dot(objectDirection, objectDirection);
    float halfB = dot(oc, objectDirection);
    float c = dot(oc, oc) - 1.0f;
    float discriminant = halfB * halfB - a * c;

    float t = INFINITY;
    if (discriminant >= 0.0f && a != 0.0f)
    {
        float root = sqrt(discriminant);
        float t0 = (-halfB - root) / a;
        float t1 = (-halfB + root) / a;
        t = t0 >= minDistance && t0 <= maxDistance ? t0 : t1;
    }

    bool accept = t >= minDistance && t <= maxDistance;
    return SphereIntersectionResult{accept, true, t};
}
```

See [Practical Tips](#practical-tips) for how `world_space_data` adds
world-space attributes without changing the object-space meaning of `[[origin]]`
and `[[direction]]`.

Invalid rays return `intersection_type::none`. A ray is invalid if:

- `origin` or `direction` contains NaN or infinity;
- `min_distance` or `max_distance` is NaN;
- `min_distance` is infinity;
- `direction` has zero length;
- `min_distance > max_distance`;
- either distance is negative.

The direction may be unnormalized, but must be nonzero.

### 5.2 Acceleration Structures

```metal
template <typename... tags>
acceleration_structure<tags...>

primitive_acceleration_structure          // alias for acceleration_structure<>
instance_acceleration_structure           // alias for acceleration_structure<instancing>
```

The aliases are only convenience spellings. `instance_acceleration_structure` is
the same shader type as `acceleration_structure<instancing>`. Other instance
acceleration-structure variants are spelled by adding tags to the template, for
example `acceleration_structure<instancing, primitive_motion>`.

Tags do not convert one acceleration structure kind into another at runtime.
`acceleration_structure<>` is a primitive acceleration structure type, while
`acceleration_structure<instancing>` is an instance acceleration structure type.
The shader parameter type must be compatible with the `MTLAccelerationStructure`
resource bound by the host.

Null checks:

```metal
bool is_null_primitive_acceleration_structure(primitive_acceleration_structure);
bool is_null_instance_acceleration_structure(instance_acceleration_structure);
bool is_null_acceleration_structure(acceleration_structure<tags...>);
```

Metal 3.1 acceleration structure types that contain the `instancing` tag,
including `instance_acceleration_structure`, also support these member
functions:

```metal
uint get_instance_count() const;
template <typename... result_tags>
acceleration_structure<result_tags...> get_acceleration_structure(uint instance_id);
```

The requested return type must match the referenced instance's actual
acceleration structure type. For instance structures, a no-motion structure may
be returned through a motion-capable shader type, at a traversal cost.

### 5.3 `intersection_type`

```metal
enum class intersection_type {
    none,
    triangle,
    bounding_box,
    curve // Metal 3.1+
};
```

## 6. `intersector<tags...>`

`intersector<tags...>` is Metal's main shader-side traversal object. A shader
configures traversal state on the intersector, calls `intersect(...)` with a
`ray` and acceleration structure, and receives an `intersection_result<tags...>`.
When custom primitive intersection functions are needed, the same `intersect`
call can also take an `intersection_function_table` or Metal 4 intersection
function buffer.

The smallest useful triangle trace looks like this:

```metal
kernel void traceOneRay(
    instance_acceleration_structure scene [[buffer(0)]],
    constant ray& r [[buffer(1)]],
    device float* outDistance [[buffer(2)]])
{
    intersector<instancing, triangle_data> tracer;
    tracer.assume_geometry_type(geometry_type::triangle);
    tracer.set_triangle_cull_mode(triangle_cull_mode::back);

    intersection_result<instancing, triangle_data> hit =
        tracer.intersect(r, scene, 0xffu);

    *outDistance =
        hit.type == intersection_type::none ? INFINITY : hit.distance;
}
```

This example introduces the main pieces:

- `intersector<instancing, triangle_data>` says traversal starts from an
  instance acceleration structure and returns triangle-specific data.
- **[Traversal state](#intersector-traversal-state)**:
  `assume_geometry_type(...)` and `set_triangle_cull_mode(...)` configure how
  the intersector walks the acceleration structure before `intersect(...)` runs.
- **[Parameters](#intersector-parameters)**:
  `intersect(r, scene, 0xffu)` passes the ray, the acceleration structure, and
  an instance mask.
- **[Return type](#intersector-return-type)**:
  `intersection_result<instancing, triangle_data>` is the return value. The same
  tags control which result fields are available.

The rest of this section explains those pieces in the same order: traversal
state, parameters, return type, overload families, callbacks, and invalid uses.

<a id="intersector-traversal-state"></a>

### 6.1 Traversal State

An intersector owns traversal state. State setters restrict or guide traversal
before `intersect(...)` runs. The separate `intersection_params` type exposes the
same controls for `intersection_query`.

```metal
enum class winding { clockwise, counterclockwise };
enum class triangle_cull_mode { none, front, back };
enum class geometry_cull_mode { none, triangle, bounding_box, curve };
enum class opacity_cull_mode { none, opaque, non_opaque };
enum class forced_opacity { none, opaque, non_opaque };
enum class geometry_type { none, triangle, bounding_box, curve, all };
enum class curve_basis { bspline, catmull_rom, linear, bezier, all };
enum class curve_type { round, flat, all };
```

`geometry_type` is a bitmask. `curve_basis` is not a bitmask.

The defaults are the initial values of the intersector's state fields. There is
no single overloaded state setter; each method below sets one specific field.

| State field | Default | Setter |
| --- | --- | --- |
| Triangle winding | `winding::clockwise` | `set_triangle_front_facing_winding(winding)` |
| Triangle cull mode | `triangle_cull_mode::none` | `set_triangle_cull_mode(triangle_cull_mode)` |
| Geometry cull mode | `geometry_cull_mode::none` | `set_geometry_cull_mode(geometry_cull_mode)` |
| Opacity cull mode | `opacity_cull_mode::none` | `set_opacity_cull_mode(opacity_cull_mode)` |
| Forced opacity | `forced_opacity::none` | `force_opacity(forced_opacity)` |
| Assumed geometry type | `geometry_type::triangle \| geometry_type::bounding_box` | `assume_geometry_type(geometry_type)` |
| Curve basis | `curve_basis::all` | `assume_curve_basis(curve_basis)` in Metal 3.1+ |
| Curve type | `curve_type::all` | `assume_curve_type(curve_type)` in Metal 3.1+ |
| Curve control point count | `0` | `assume_curve_control_point_count(uint)` in Metal 3.1+ |
| Assume identity transforms | `false` | `assume_identity_transforms(bool)` |
| Accept any intersection | `false` | `accept_any_intersection(bool)` |

Getter methods are available on `intersection_params`, and
`intersection_query::get_intersection_params()` returns the active parameters.

Invalid traversal state:

- Do not combine `force_opacity(opaque/non_opaque)` with
  `set_opacity_cull_mode(opaque/non_opaque)`. If either one is non-`none`, the
  other must be `none`; otherwise behavior is undefined.
- Do not omit `geometry_type::curve` from `assume_geometry_type()` when tracing
  curves. Curves are not assumed by default.
- Do not pass a curve control point count other than `0`, `2`, `3`, or `4`.
  Counts `2`, `3`, and `4` must be compatible with the assumed curve basis.
- Only use `assume_identity_transforms(true)` when the relevant instance
  transforms are actually identity transforms.

<a id="intersector-parameters"></a>

### 6.2 `intersect(...)` Overloads and Parameters

All `intersect(...)` overloads take a `ray` and an acceleration structure. The
other parameters are added by specific overload families: instancing adds
`mask`, motion adds `time`, custom intersection functions add a function table
or function buffer, payload overloads add a thread payload, and direct-access
overloads add a callable object.

In the schematic signatures below, `AS` means "the acceleration-structure
parameter selected by the primitive/instance/motion rules," and `callable` means
a lambda, function object, or named function. They are documentation shorthand,
not Metal types.

The overloads are grouped by the main thing the call adds:

1. **Without callback and without user intersection functions**: ordinary
   built-in traversal that returns `intersection_result<tags...>`.
2. **With callback**: traversal returns `void` and reports the result through a
   callable. This group also includes callback forms that use a function table
   or IFB, because the callback calling convention is the main difference at the
   call site.
3. **With user intersection functions**: non-callback traversal can invoke user
   `[[intersection(...)]]` functions through an ordinary
   `intersection_function_table` or Metal 4 intersection function buffer and
   returns `intersection_result<tags...>`.

#### 6.2.1 Without Callback and Without User Intersection Functions

Use this group when the call should return `intersection_result<tags...>` and
should not invoke a user `[[intersection(...)]]` function.

##### 6.2.1.1 Built-In Primitive Traversal

```metal
intersection_result<tags...> intersect(
    ray r,
    primitive_acceleration_structure as) const;
```

Use this overload when the intersector does not have `instancing`, does not use
motion, and does not need custom intersection functions. Because this overload
does not take an `intersection_function_table` or
`intersection_function_buffer_arguments`, it has no user `[[intersection(...)]]`
function to call.

| Parameter | Meaning |
| --- | --- |
| `ray r` | World-space ray to trace. Must be valid. |
| `primitive_acceleration_structure as` | Primitive acceleration structure containing triangles, bounding boxes, or curves. |

There is no `mask` parameter because instance masks only exist when traversing
an instance acceleration structure. There is no `time` parameter unless the
intersector has a motion tag.

If no `intersection_function_table` or Metal 4 intersection function buffer is
passed, Metal does not call user `[[intersection(...)]]` functions. It uses
built-in tests for built-in primitive geometry:

- Triangle primitives use Metal's default triangle intersection test.
- Curve primitives use Metal's default curve intersection test in Metal 3.1+.
- Bounding-box primitives are procedural candidates. To turn a box candidate
  into an actual hit with `intersector`, provide a `[[intersection(bounding_box)]]`
  function through an intersection function table or IFB. Without that custom
  function, there is no user primitive test that can return the accepted hit
  distance.

Triangle and curve intersection functions are optional filters/extensions around
the built-in tests. A bounding-box intersection function is the procedural hit
test itself.

##### 6.2.1.2 Built-In Instance Traversal

```metal
intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as) const;

intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    uint mask) const;
```

Use these overloads when the intersector has `instancing` and no motion tag.
Because these overloads do not take an `intersection_function_table` or
`intersection_function_buffer_arguments`, they have no user
`[[intersection(...)]]` function to call.

| Parameter | Meaning |
| --- | --- |
| `ray r` | World-space ray to trace. Must be valid. |
| `instance_acceleration_structure as` | Instance acceleration structure. Equivalent to `acceleration_structure<instancing>`. |
| `uint mask` | Optional instance mask. The default is `~0U`. During traversal, Metal skips instances whose descriptor mask does not overlap this value. |

##### 6.2.1.3 Motion Traversal

```metal
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<primitive_motion> as,
    float time) const;

intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion> as,
    uint mask,
    float time) const;

intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, instance_motion> as,
    uint mask,
    float time) const;

intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion, instance_motion> as,
    uint mask,
    float time) const;
```

Use motion overloads when the intersector has `primitive_motion`,
`instance_motion`, or both. The acceleration structure type must match the
motion tags used by the intersector. These overloads still do not call user
`[[intersection(...)]]` functions unless a function table or IFB overload is
used.

| Parameter | Meaning |
| --- | --- |
| `ray r` | World-space ray to trace. Must be valid. |
| `acceleration_structure<...> as` | Motion-capable acceleration structure matching the intersector's acceleration-structure tags. |
| `uint mask` | Instance mask. Valid only when `instancing` is present. Defaults to `~0U` on overloads that omit it. |
| `float time` | Ray time used for primitive and/or instance motion interpolation. Valid only when a motion tag is present. |

<a id="intersector-callback-overloads"></a>

#### 6.2.2 With Callback

Use this group when the platform supports Metal 3.2 direct result access and
the shader wants traversal to report the result through a callable. Callback
overloads return `void` and call the supplied lambda, function object, or named
function with an `intersection_result_ref<tags...>`.

##### 6.2.2.1 Direct-Access Callback Without Function Tables

```metal
// Built-in primitive/instance traversal callback.
void intersect(
    ray r,
    primitive_acceleration_structure as,
    callable) const;

void intersect(
    ray r,
    instance_acceleration_structure as,
    callable) const;

void intersect(
    ray r,
    instance_acceleration_structure as,
    uint mask,
    callable) const;

// Motion traversal callback.
void intersect(
    ray r,
    acceleration_structure<primitive_motion> as,
    float time,
    callable) const;

void intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion> as,
    uint mask,
    float time,
    callable) const;

void intersect(
    ray r,
    acceleration_structure<instancing, instance_motion> as,
    uint mask,
    float time,
    callable) const;

void intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion, instance_motion> as,
    uint mask,
    float time,
    callable) const;
```

Use these overloads when the platform supports Metal 3.2 direct result access
and the shader wants to inspect a built-in traversal result in-place. These
overloads return `void`; Metal performs traversal and calls the supplied
callable with an `intersection_result_ref<tags...>`. Because these overloads do
not take an `intersection_function_table`, they do not call user
`[[intersection(...)]]` functions.

| Parameter | Meaning |
| --- | --- |
| `ray r` | World-space ray to trace. Must be valid. |
| `primitive_acceleration_structure as` | Primitive acceleration structure. Valid when the intersector does not have `instancing` or motion tags. |
| `instance_acceleration_structure as` | Instance acceleration structure. Valid when the intersector has `instancing` and no motion tag. |
| `acceleration_structure<...> as` | Motion-capable acceleration structure matching the intersector's acceleration-structure tags. |
| `uint mask` | Instance mask, when `instancing` is present. |
| `float time` | Ray time, when motion tags are present. |
| callable object | Lambda, function object, or named function invoked after traversal. It receives `intersection_result_ref<tags...>`. |

The callable can be a lambda, function object, or named function, but it must be
callable with exactly the argument list required by the overload. It is not a
Metal intrinsic type named `Callable` and not an arbitrary runtime function
pointer.

```metal
float closestDistance = INFINITY;

tracer.intersect(r, scene, 0xffu,
    [&](intersection_result_ref<instancing, triangle_data> hit)
    {
        if (hit.get_type() != intersection_type::none)
            closestDistance = hit.get_distance();
    });
```

```metal
static void recordHit(intersection_result_ref<instancing, triangle_data> hit)
{
    if (hit.get_type() != intersection_type::none)
    {
        // Read result fields here. Do not store `hit` for later use.
    }
}

tracer.intersect(r, scene, 0xffu, recordHit);
```

Use the next subsection when the callback traversal also needs custom
intersection functions from an `intersection_function_table`.

##### 6.2.2.2 Function-Table Traversal with Direct-Access Result Callback

An `intersection_function_table<tags...>` is the shader-visible handle for a
host-created `MTLIntersectionFunctionTable`. The host fills table entries with
compiled `[[intersection(...)]]` functions and binds any resources those
functions use. The shader does not call a table entry directly; it passes the
whole table to `intersect(...)`, and Metal selects the entry from the
acceleration structure's geometry offset plus the instance offset when
instancing is used.

This section combines two independent mechanisms:

- **Intersection functions from the table** run during traversal. Metal may call
  them zero or more times while testing candidate primitives.
- **The direct-access result callback** runs at the `intersect(...)` call site.
  It receives the final `intersection_result_ref<tags...>` instead of having
  `intersect(...)` return an `intersection_result<tags...>` value.

So, if the host bound `sphereIntersection` in the table and the shader calls a
callback overload, both can run:

```text
shader calls intersect(ray, as, ft, resultCallback)
    Metal traverses the acceleration structure
        Metal selects entries from ft
        Metal calls selected [[intersection(...)]] functions as needed
    Metal decides the final hit or miss
    Metal calls resultCallback(intersection_result_ref<...>)
intersect(...) returns void
```

The table function answers "does this candidate primitive count as a hit, and at
what distance?" The callback answers "what should my shader do with the final
hit result?" They are not competing function slots.

The purpose of the table-bound `[[intersection(...)]]` function is primitive
testing or primitive filtering:

- For a bounding-box procedural primitive, traversal only knows the ray entered
  a box. The intersection function performs the real primitive test, such as
  "does this ray hit the sphere inside this box?", and returns the accepted hit
  distance.
- For triangle and curve primitives, Metal already has a built-in intersection
  candidate. The intersection function can filter or annotate that candidate,
  for example rejecting an alpha-tested triangle or updating a ray payload.
- The function is not a closest-hit or material shader. It may run for
  candidates that do not become the final closest hit, so final shading usually
  belongs in the caller after `intersect(...)` or in the result callback.

If you need custom primitive testing but do not need a result callback, use the
non-callback function-table overloads in
[With User Intersection Functions](#intersector-user-intersection-overloads).

Use these overloads when callback traversal should also call custom
`[[intersection(...)]]` functions through an ordinary intersection function
table. They combine the direct-access callback return path with the function
selection rules described in
[`intersection_function_table<tags...>`](#intersection-function-table).

###### 6.2.2.2.1 Function-Table Callback Without Explicit Mask or Time

```metal
void intersect(
    ray r,
    AS as,
    intersection_function_table<tags...> ft,
    callable) const;

template<typename T>
void intersect(
    ray r,
    AS as,
    intersection_function_table<tags...> ft,
    const thread T& payload,
    callable) const;
```

Use these forms when the matching non-callback function-table overload would not
need an explicit instance `mask` or motion `time` parameter.

###### 6.2.2.2.2 Function-Table Callback with Mask and/or Time

```metal
void intersect(
    ray r,
    AS as,
    uint mask,
    intersection_function_table<tags...> ft,
    callable) const;

template<typename T>
void intersect(
    ray r,
    AS as,
    uint mask,
    intersection_function_table<tags...> ft,
    const thread T& payload,
    callable) const;

void intersect(
    ray r,
    AS as,
    float time,
    intersection_function_table<tags...> ft,
    callable) const;

template<typename T>
void intersect(
    ray r,
    AS as,
    float time,
    intersection_function_table<tags...> ft,
    const thread T& payload,
    callable) const;

void intersect(
    ray r,
    AS as,
    uint mask,
    float time,
    intersection_function_table<tags...> ft,
    callable) const;

template<typename T>
void intersect(
    ray r,
    AS as,
    uint mask,
    float time,
    intersection_function_table<tags...> ft,
    const thread T& payload,
    callable) const;
```

Use these forms when the matching non-callback function-table overload needs an
instance `mask`, a motion `time`, or both.

| Parameter | Meaning |
| --- | --- |
| `ray r` | World-space ray to trace. Must be valid. |
| `AS as` | Acceleration structure selected by the same primitive, instance, and motion rules as the non-callback overloads. |
| `uint mask` | Instance mask, when `instancing` is present. It appears in the same position as in the matching non-callback overload. |
| `float time` | Ray time, when motion tags are present. It appears in the same position as in the matching non-callback overload. |
| `intersection_function_table<tags...> ft` | Function table for custom intersection functions. Its tag list must match the intersector and intersection functions. |
| `const thread T& payload` | Optional initial payload. Metal passes the final payload to the callback as `const ray_data T&`. |
| callable object | Lambda, function object, or named function invoked after traversal. It receives `intersection_result_ref<tags...>`, and also `const ray_data T&` when a payload is supplied. |

When a payload is supplied, the callback must also accept the final payload
argument:

```metal
struct Payload { uint materialID; };

// Assume `table` is an intersection_function_table<instancing, triangle_data>.
Payload payload = {};

tracer.intersect(r, scene, table, payload,
    [&](intersection_result_ref<instancing, triangle_data> hit,
        const ray_data Payload& payload)
    {
        (void)hit;
        (void)payload;
    });
```

<a id="intersector-ifb-callback-overloads"></a>

##### 6.2.2.3 IFB Direct-Access Callback

IFB callback forms use the same IFB parameters as the non-callback IFB overloads
in [With User Intersection Functions](#intersector-user-intersection-overloads),
but return `void` and append a callable object after the final non-callback
parameter.

```metal
void intersect(
    ray r,
    primitive_acceleration_structure as,
    intersection_function_buffer_arguments ifba,
    callable) const;

template<typename T>
void intersect(
    ray r,
    instance_acceleration_structure as,
    uint mask,
    intersection_function_buffer_arguments ifba,
    const device void* user_data,
    const thread T& payload,
    callable) const;
```

##### 6.2.2.4 Callback Result Reference: `intersection_result_ref<tags...>`

`intersection_result_ref<tags...>` always provides these methods:

- `get_type()`
- `get_distance()`
- `get_primitive_id()`
- `get_geometry_id()`
- `get_primitive_data()`
- `get_ray_origin()`
- `get_ray_direction()`
- `get_ray_min_distance()`

Conditional methods match the conditional fields of `intersection_result`:
instance ID methods, triangle methods, curve parameter, and transform methods.
Storing the reference or `ray_data` payload pointer after the callback returns is
invalid. Recursive ray tracing inside the callback is invalid.

<a id="intersector-user-intersection-overloads"></a>

#### 6.2.3 With User Intersection Functions

Use this group when traversal should invoke user `[[intersection(...)]]`
functions. These non-callback overloads return `intersection_result<tags...>`.
In this grouping, "user intersection function" is the Metal term for the
custom `[[intersection(...)]]` shader function that acts like an intersection
kernel.
The callback variants that also use an `intersection_function_table` or IFB are
listed under [With Callback](#intersector-callback-overloads), because their
call-site shape is the callback form.

##### 6.2.3.1 Ordinary Intersection Function Table

```metal
// Primitive acceleration structure, no instancing, no motion.
intersection_result<tags...> intersect(
    ray r,
    primitive_acceleration_structure as,
    intersection_function_table<tags...> ft) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    primitive_acceleration_structure as,
    intersection_function_table<tags...> ft,
    thread T& payload) const;

// Instance acceleration structure, no motion.
intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    intersection_function_table<tags...> ft) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    intersection_function_table<tags...> ft,
    thread T& payload) const;

intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    uint mask,
    intersection_function_table<tags...> ft) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    uint mask,
    intersection_function_table<tags...> ft,
    thread T& payload) const;

// Motion-capable acceleration structure without instancing.
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<primitive_motion> as,
    float time,
    intersection_function_table<tags...> ft) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<primitive_motion> as,
    float time,
    intersection_function_table<tags...> ft,
    thread T& payload) const;

// Motion-capable acceleration structure with instancing.
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion> as,
    uint mask,
    float time,
    intersection_function_table<tags...> ft) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion> as,
    uint mask,
    float time,
    intersection_function_table<tags...> ft,
    thread T& payload) const;

intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, instance_motion> as,
    uint mask,
    float time,
    intersection_function_table<tags...> ft) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, instance_motion> as,
    uint mask,
    float time,
    intersection_function_table<tags...> ft,
    thread T& payload) const;

intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion, instance_motion> as,
    uint mask,
    float time,
    intersection_function_table<tags...> ft) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion, instance_motion> as,
    uint mask,
    float time,
    intersection_function_table<tags...> ft,
    thread T& payload) const;
```

Use these overloads when traversal should call custom `[[intersection(...)]]`
functions through an ordinary intersection function table. Section 8,
[`intersection_function_table<tags...>`](#intersection-function-table), gives a
detailed explanation of how the table is created, populated, bound, and selected
during traversal.

| Parameter | Meaning |
| --- | --- |
| `ray r` | World-space ray to trace. Must be valid. |
| `primitive_acceleration_structure as` | Primitive acceleration structure. Valid when the intersector does not have `instancing` or motion tags. |
| `instance_acceleration_structure as` | Instance acceleration structure. Valid when the intersector has `instancing` and no motion tag. |
| `acceleration_structure<...> as` | Motion-capable acceleration structure matching the intersector's acceleration-structure tags. |
| `uint mask` | Instance mask, when `instancing` is present. |
| `float time` | Ray time, when motion tags are present. |
| `intersection_function_table<tags...> ft` | Function table whose tag list exactly matches the intersector and the intersection functions. |
| `thread T& payload` | Optional caller payload. Metal copies it into `ray_data`, passes it to intersection functions, then copies the final value back. |

##### 6.2.3.2 Metal 4 Intersection Function Buffer

An intersection function buffer is the Metal 4 form of custom intersection
dispatch. Use it when the set of intersection functions is better represented as
a GPU buffer of function handles than as an `MTLIntersectionFunctionTable`.
Typical reasons are:

- you are porting a shader-binding-table style renderer, such as a DXR path;
- the scene has many materials, geometries, or ray types and you want a compact
  buffer layout for function selection;
- the shader needs to choose a ray type with `set_base_id(...)` while sharing
  the same acceleration structure;
- intersection functions need a `[[user_data_buffer]]` pointer, enabled by the
  additional `user_data` tag.

The basic workflow is:

1. Add `intersection_function_buffer` to the `intersector` tag list and to the
   matching `[[intersection(...)]]` functions.
2. On the host, build a buffer whose records contain intersection-function
   handles, then pass its pointer, byte size, and stride through
   `intersection_function_buffer_arguments`.
3. In the shader, configure `set_geometry_multiplier(...)` and
   `set_base_id(...)` if the buffer stores multiple ray types per geometry.
4. Call `intersect(...)` with the ray, acceleration structure, and IFB
   arguments. Metal computes the function-buffer entry from the geometry offset,
   optional instance offset, geometry multiplier, and base ID.

For small fixed sets of custom intersection functions, an ordinary
`intersection_function_table` is usually simpler and supports older Metal
versions. Section 9,
[Metal 4 Intersection Function Buffers](#metal-4-intersection-function-buffers),
gives a detailed explanation of how the function buffer is represented, how its
record index is computed, and how `set_base_id`, `set_geometry_multiplier`, and
`user_data` participate in traversal.

```metal
// Primitive acceleration structure, no instancing, no motion.
intersection_result<tags...> intersect(
    ray r,
    primitive_acceleration_structure as,
    intersection_function_buffer_arguments ifba) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    primitive_acceleration_structure as,
    intersection_function_buffer_arguments ifba,
    thread T& payload) const;

// Instance acceleration structure, no motion.
intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    intersection_function_buffer_arguments ifba) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    intersection_function_buffer_arguments ifba,
    thread T& payload) const;

intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    uint mask,
    intersection_function_buffer_arguments ifba) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    uint mask,
    intersection_function_buffer_arguments ifba,
    thread T& payload) const;

// Motion-capable acceleration structure without instancing.
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<primitive_motion> as,
    float time,
    intersection_function_buffer_arguments ifba) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<primitive_motion> as,
    float time,
    intersection_function_buffer_arguments ifba,
    thread T& payload) const;

// Motion-capable acceleration structure with instancing.
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba,
    thread T& payload) const;

intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, instance_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, instance_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba,
    thread T& payload) const;

intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion, instance_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion, instance_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba,
    thread T& payload) const;
```

Use these overloads when the intersector tag list contains
`intersection_function_buffer` and does not contain `user_data`. The overload
families mirror the ordinary function-table families: instance forms may insert
`mask`, motion forms insert `time`, and payload forms add a final
`thread T& payload`.

When the tag list also contains `user_data`, use these matching user-data forms.
The `const device void* user_data` parameter appears after `ifba` and before the
optional payload.

```metal
// Primitive acceleration structure, no instancing, no motion.
intersection_result<tags...> intersect(
    ray r,
    primitive_acceleration_structure as,
    intersection_function_buffer_arguments ifba,
    const device void* user_data) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    primitive_acceleration_structure as,
    intersection_function_buffer_arguments ifba,
    const device void* user_data,
    thread T& payload) const;

// Instance acceleration structure, no motion.
intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    intersection_function_buffer_arguments ifba,
    const device void* user_data) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    intersection_function_buffer_arguments ifba,
    const device void* user_data,
    thread T& payload) const;

intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    uint mask,
    intersection_function_buffer_arguments ifba,
    const device void* user_data) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    instance_acceleration_structure as,
    uint mask,
    intersection_function_buffer_arguments ifba,
    const device void* user_data,
    thread T& payload) const;

// Motion-capable acceleration structure without instancing.
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<primitive_motion> as,
    float time,
    intersection_function_buffer_arguments ifba,
    const device void* user_data) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<primitive_motion> as,
    float time,
    intersection_function_buffer_arguments ifba,
    const device void* user_data,
    thread T& payload) const;

// Motion-capable acceleration structure with instancing.
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba,
    const device void* user_data) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba,
    const device void* user_data,
    thread T& payload) const;

intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, instance_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba,
    const device void* user_data) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, instance_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba,
    const device void* user_data,
    thread T& payload) const;

intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion, instance_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba,
    const device void* user_data) const;

template<typename T>
intersection_result<tags...> intersect(
    ray r,
    acceleration_structure<instancing, primitive_motion, instance_motion> as,
    uint mask,
    float time,
    intersection_function_buffer_arguments ifba,
    const device void* user_data,
    thread T& payload) const;
```

The parameter order stays the same across the family: `ray`, acceleration
structure, optional `mask`, optional `time`, `ifba`, optional `user_data`,
optional `payload`.

IFB callback forms use the same IFB parameters, but are listed in
[IFB Direct-Access Callback](#intersector-ifb-callback-overloads) because their
call-site shape belongs to the callback group.

| Parameter | Meaning |
| --- | --- |
| `ray r` | World-space ray to trace. Must be valid. |
| `primitive_acceleration_structure as` | Primitive acceleration structure. Valid only when the intersector does not have `instancing`. |
| `instance_acceleration_structure as` | Instance acceleration structure. Valid only when the intersector has `instancing` and no motion tag. |
| `acceleration_structure<...> as` | Motion-capable acceleration structure matching the intersector's `instancing`, `primitive_motion`, and `instance_motion` tags. |
| `uint mask` | Instance mask, when `instancing` is present. |
| `float time` | Ray time, when `primitive_motion`, `instance_motion`, or both are present. |
| `intersection_function_buffer_arguments ifba` | Metal 4 function-buffer descriptor. Its function-buffer pointer must be uniform within the SIMD-group. |
| `const device void* user_data` | Optional user data buffer passed to `[[user_data_buffer]]`; valid only with both `intersection_function_buffer` and `user_data` tags. |
| `thread T& payload` | Optional caller payload copied through `ray_data`, as with ordinary function-table overloads. |

<a id="intersector-return-type"></a>

### 6.3 Return Type: `intersection_result<tags...>`

The normal `intersect(...)` return type is `intersection_result<tags...>`. Its
fields depend on the same tag list used by the intersector.

Always available fields:

- `intersection_type type`
- `float distance`
- `uint primitive_id`
- `uint geometry_id`
- `const device void* primitive_data` in Metal 3+

Conditional fields:

| Required tag | Fields |
| --- | --- |
| `instancing` without `max_levels` | `uint instance_id`, `uint user_instance_id` |
| `instancing` with `max_levels<Count>` | `uint instance_count`, `uint instance_id[Count - 1]`, `uint user_instance_id[Count - 1]` |
| `triangle_data` | `float2 triangle_barycentric_coord`, `bool triangle_front_facing` |
| `instancing, world_space_data` | `float4x3 world_to_object_transform`, `float4x3 object_to_world_transform` |
| `curve_data` | `float curve_parameter` |

The reported `distance` is in world space. Triangle interpolation uses
`v1 * bary.x + v2 * bary.y + v0 * (1 - bary.x - bary.y)`.

### 6.4 Invalid Intersector Use

- Do not use an instance acceleration structure without `instancing`.
- Do not use a primitive acceleration structure with `instancing`.
- Do not pass a function table whose tags differ from the intersector tags.
- Do not pass `mask` without `instancing`.
- Do not pass `time` unless the intersector has a motion tag.
- Do not instantiate a no-instancing intersector with `instance_motion` or
  `world_space_data`. No-instancing motion intersectors can only use
  `primitive_motion`.
- Do not pass `user_data` without both `intersection_function_buffer` and `ifba`.
- Do not perform recursive ray tracing inside a result-ref callback.

## 7. Intersection Functions

Declare custom intersection functions with:

```metal
[[intersection(primitive_type, tags...)]]
```

Valid primitive types:

| Primitive type | Meaning |
| --- | --- |
| `triangle` | Extends the built-in triangle test. |
| `bounding_box` | Runs when traversal reaches a bounding box primitive. |
| `curve` | Extends the built-in curve test in Metal 3.1+. |

Intersection functions:

- may take `device` and `constant` buffer arguments;
- may receive textures only through argument buffers;
- cannot use threadgroup memory;
- cannot use `threadgroup_barrier` or `simdgroup_barrier`;
- cannot start new rays;
- may be invoked in a different SIMD-group than the caller;
- must match the primitive type represented in the acceleration structure.
  A triangle function for a bounding box primitive, or the reverse, is undefined.

### 7.1 Input Attributes

| Attribute | Type | Valid when | Meaning |
| --- | --- | --- | --- |
| `[[origin]]` | `float3` | Always | Object-space ray origin. |
| `[[direction]]` | `float3` | Always | Object-space ray direction. |
| `[[min_distance]]` | `float` | Always | Current ray minimum distance. |
| `[[max_distance]]` | `float` | Always | Current closest-search maximum distance. |
| `[[payload]]` | `ray_data T&` | Caller passed payload | User payload copied from the caller and back to the caller. |
| `[[geometry_id]]` | `ushort` or `uint` | Always | Geometry identifier. |
| `[[primitive_id]]` | `ushort` or `uint` | Always | Primitive identifier; for curves, the curve segment index. |
| `[[instance_id]]` | `ushort`, `uint`, or `array_ref<uint>` | `instancing`; use `array_ref<uint>` with `max_levels<Count>` | Instance IDs. |
| `[[user_instance_id]]` | `ushort`, `uint`, or `array_ref<uint>` | `instancing`; use `array_ref<uint>` with `max_levels<Count>` | User-defined instance IDs. |
| `[[world_space_origin]]` | `float3` | `world_space_data` | World-space ray origin. |
| `[[world_space_direction]]` | `float3` | `world_space_data` | World-space ray direction. |
| `[[barycentric_coord]]` | `float2` | `primitive_type == triangle` and `triangle_data` | Triangle barycentric coordinates. |
| `[[front_facing]]` | `bool` | `triangle_data` | Whether the triangle front face is visible from the ray origin. |
| `[[distance]]` | `float` | `primitive_type == triangle` | Distance along the ray at the triangle hit. |
| `[[opaque]]` | `bool` | `primitive_type == bounding_box` | Resolved opacity state. |
| `[[instance_intersection_function_table_offset]]` | `ushort` or `uint` | Intersection table workflow | Instance offset used to select the intersection function. |
| `[[geometry_intersection_function_table_offset]]` | `ushort` or `uint` | Intersection table workflow | Geometry offset used to select the intersection function. |
| `[[time]]` | `float` | `primitive_motion` | Ray intersection time. |
| `[[motion_start_time]]` | `float` | `primitive_motion` | Geometry motion start time. |
| `[[motion_end_time]]` | `float` | `primitive_motion` | Geometry motion end time. |
| `[[key_frame_count]]` | `ushort` or `uint` | `primitive_motion` | Number of motion key frames. |
| `[[object_to_world_transform]]` | `float4x3` | `instancing, world_space_data` | Object-to-world transform; interpolated if `instance_motion` is present. |
| `[[world_to_object_transform]]` | `float4x3` | `instancing, world_space_data` | World-to-object transform; interpolated if `instance_motion` is present. |
| `[[primitive_data]]` | `const device T*` or `const device T&` | Metal 3+ with per-primitive data | Read-only primitive data. |
| `[[curve_parameter]]` | `float` | `curve_data` | Curve parameter for reconstructing curve position. |
| `[[function_id]]` | `ushort` or `uint` | `intersection_function_buffer` | Function index invoked by the GPU. |
| `[[user_data_buffer]]` | `const device T*` or `const device T&` | `intersection_function_buffer, user_data` | User data pointer passed through IFB intersector call. |

Payload type `T` may contain device/constant pointers or references, integers,
enums, floating-point types, vectors, arrays of those types, structs, and unions.
It must not contain `atomic<T>` or `imageblock<T>`.

### 7.2 Return Attributes

| Attribute | Type | Valid when | Meaning |
| --- | --- | --- | --- |
| `[[accept_intersection]]` | `bool` | All intersection functions | If true, the primitive is accepted as a candidate committed hit. |
| `[[continue_search]]` | `bool` | All intersection functions | If accepted, controls whether traversal continues looking for a closer hit. Defaults to true. |
| `[[distance]]` | `float` | `primitive_type == bounding_box` | Hit distance found within the bounding box. Ignored if the hit is rejected. |

For triangle intersection functions, a plain `bool` return value is treated as
`[[accept_intersection]]`.

The returned bounding box `[[distance]]` must be within the current
`[[min_distance]]` to `[[max_distance]]` interval and inside the primitive's
bounding box. If the distance equals the current max distance and the hit is
accepted, this hit wins over the previous same-distance hit. Invalid distances
produce undefined behavior.

Payload writes and device memory writes take effect even when a primitive is
rejected. Intersection functions may be invoked even when the ray does not
actually intersect that primitive's box because an implementation may group
multiple primitives into a leaf.

## 8. `intersection_function_table<tags...>`

<a id="intersection-function-table"></a>

An ordinary intersection function table is Metal's traversal-time dispatch
table for user `[[intersection(...)]]` functions. It is host-created and then
passed into shader code as `intersection_function_table<tags...>`.

The shader does not index the table like `ft[i]`, and it cannot directly call a
table entry. The shader passes the whole table to `intersector::intersect(...)`.
While walking the acceleration structure, Metal computes the table index for
each candidate primitive, fetches the corresponding table entry, and invokes
that entry's `[[intersection(...)]]` function.

This table is not a D3D/Vulkan SBT. It only controls custom intersection
dispatch during traversal. It does not dispatch Metal miss or closest-hit logic;
that logic still runs after `intersect(...)` returns.

### 8.1 What The Table Contains

Think of an ordinary intersection function table as two related collections:

- **Function entries.** Each function entry is a slot containing either a
  custom `[[intersection(...)]]` function or a built-in opaque intersection
  function supported by Metal.
- **Resource bindings.** The table also stores buffers and visible function
  tables that are visible to the selected intersection functions.

The function entries answer "which intersection function should traversal
call?" The resource bindings answer "what resources can that function access
through its table-provided parameters?"

For example, a small table may be arranged like this:

| Table slot | Selected during traversal for | Function entry |
| --- | --- | --- |
| `0` | opaque triangle geometry | built-in opaque triangle intersection |
| `1` | alpha-tested triangle geometry | `alphaTriangleIntersection` |
| `2` | sphere bounding-box geometry | `sphereIntersection` |

The shader-side `intersect(...)` call does not mention slot `0`, `1`, or `2`.
Those slots are selected by acceleration-structure metadata.

### 8.2 How Metal Selects A Function Entry

The ordinary table index is computed from acceleration-structure offsets:

```text
primitive AS:
    effectiveFunctionIndex = geometryIntersectionFunctionTableOffset

instance AS:
    effectiveFunctionIndex =
        geometryIntersectionFunctionTableOffset +
        instanceIntersectionFunctionTableOffset
```

The geometry offset comes from the acceleration-structure geometry descriptor.
The instance offset comes from the instance descriptor when traversal starts at
an `instance_acceleration_structure`. With no instancing, there is no instance
offset contribution.

This means the host must set up the table and the acceleration structure
consistently:

```text
table[0] = built-in opaque triangle function
table[1] = alphaTriangleIntersection
table[2] = sphereIntersection

opaqueTriangleGeometry.intersectionFunctionTableOffset = 0
alphaTriangleGeometry.intersectionFunctionTableOffset  = 1
sphereBoxGeometry.intersectionFunctionTableOffset      = 2
```

For an instance acceleration structure, the instance can add another offset:

```text
table[16 + 2] = sphereIntersectionForMaterialSet16

sphereBoxGeometry.intersectionFunctionTableOffset = 2
instance.intersectionFunctionTableOffset          = 16

effectiveFunctionIndex = 2 + 16
```

The exact host API spelling differs between Swift, Objective-C, and metal-cpp,
but the responsibilities are the same:

```cpp
// Host-side pseudocode.
table.setFunction(opaqueTriangleFunction, 0);
table.setFunction(alphaTriangleIntersection, 1);
table.setFunction(sphereIntersection, 2);

table.setBuffer(materialBuffer, 0, 0); // visible as [[buffer(0)]]
table.setBuffer(sphereBuffer, 0, 1);   // visible as [[buffer(1)]]

opaqueTriangleGeometry.intersectionFunctionTableOffset = 0;
alphaTriangleGeometry.intersectionFunctionTableOffset = 1;
sphereBoxGeometry.intersectionFunctionTableOffset = 2;
```

The important point is that the offset selects the table entry. It is not an
argument to the shader's `intersect(...)` call.

### 8.3 Shader-Side Shape

The intersection function's signature describes the data Metal will provide
when that function is selected. The body below intentionally omits the sphere
math; the signature is the important part:

```metal
struct BoxIntersectionReturn
{
    bool accept [[accept_intersection]];
    float distance [[distance]];
};

[[intersection(bounding_box)]]
BoxIntersectionReturn sphereIntersection(
    float3 origin [[origin]],
    float3 direction [[direction]],
    float minDistance [[min_distance]],
    float maxDistance [[max_distance]],
    uint primitiveID [[primitive_id]],
    const device Sphere* spheres [[buffer(1)]])
{
    // `origin`, `direction`, and distance limits describe the candidate ray.
    // `primitiveID` selects which bounding-box primitive is being tested.
    // `spheres` comes from table.setBuffer(..., index = 1) on the host.
    float t = testSphere(origin, direction, minDistance, maxDistance, spheres[primitiveID]);
    return BoxIntersectionReturn{t != INFINITY, t};
}
```

The tracing kernel only receives and forwards the table:

```metal
kernel void traceSpheres(
    primitive_acceleration_structure scene [[buffer(0)]],
    intersection_function_table<> ft [[buffer(1)]],
    constant RayDesc& rayDesc [[buffer(2)]],
    device float* distance [[buffer(3)]])
{
    intersector<> tracer;
    tracer.assume_geometry_type(geometry_type::bounding_box);

    ray r(rayDesc.origin, rayDesc.direction, rayDesc.minDistance, rayDesc.maxDistance);
    intersection_result<> hit = tracer.intersect(r, scene, ft);
    *distance = hit.type == intersection_type::none ? INFINITY : hit.distance;
}
```

In this example, `sphereIntersection` is not named at the call site. The host
placed it in table entry `2`, and `sphereBoxGeometry` selects entry `2` through
its `intersectionFunctionTableOffset`.

### 8.4 Operation Reference And Simple Use Cases

Valid shader-side operations:

```metal
bool is_null_intersection_function_table(intersection_function_table<tags...>);
bool empty() const;
uint size() const;
template<typename T> T get_buffer(uint index) const;                 // Metal 3+
template<typename T> visible_function_table<T> get_visible_function_table(uint index) const; // Metal 3+
void set_buffer(const device void* buf, uint index);                 // Metal 3.1+
void set_buffer(constant void* buf, uint index);                     // Metal 3.1+
template<typename T> void set_visible_function_table(visible_function_table<T>, uint index); // Metal 3.1+
```

`T` for `get_buffer` must be a pointer or reference in `device` or `constant`
address space. `T` for `get_visible_function_table` is the function signature
stored in that visible function table.

Most code only passes the table to `intersect(...)`. The operations are useful
when the shader wants to validate the table handle, inspect table-provided
resources, or update table resource bindings before traversal.

#### Null And Empty Checks

Use `is_null_intersection_function_table(ft)` to test whether the table handle
itself is null. Use `ft.empty()` and `ft.size()` to reason about the table
contents.

```metal
if (is_null_intersection_function_table(ft) || ft.empty())
{
    *distance = INFINITY;
    return;
}

uint entryCount = ft.size();
```

A null table and an empty table are different states, so test them separately
when that distinction matters.

#### Reading A Table Resource Binding

`get_buffer<T>(index)` reads the resource binding stored in the table at that
resource index. This is useful when the tracing shader wants to use the same
resource that intersection functions will see.

```metal
const device Sphere* spheres = ft.get_buffer<const device Sphere*>(1);
Sphere first = spheres[0];
```

This does not read a function entry. It reads the table's buffer binding whose
index corresponds to an intersection function's `[[buffer(1)]]` parameter.

#### Updating A Table Resource Binding

`set_buffer(...)` changes a table resource binding visible to intersection
functions. It does not install or replace an intersection function entry; those
function entries are created by the host, and acceleration-structure offsets
still select the same table entries as before.

This operation should be read as rebinding a resource slot of the table, not as
changing the shader signature of any intersection function. The selected
intersection function still has a fixed typed parameter, for example
`const device Sphere* spheres [[buffer(1)]]`. The buffer currently bound at
index `1` must be valid for that type and layout. If host code originally binds
`buffer1` at index `1` and shader code later calls `ft.set_buffer(buffer2, 1)`,
then subsequent intersection functions that read `[[buffer(1)]]` see `buffer2`.
That is only valid if `buffer2` contains data compatible with the fixed
`[[buffer(1)]]` declarations of the reachable intersection functions.

```metal
// Rebind the resource that intersection functions receive as [[buffer(1)]]
// before tracing with this table.
ft.set_buffer(frameSphereBuffer, 1);

intersection_result<> hit = tracer.intersect(r, scene, ft);
```

This pattern is useful when the function table layout and intersection function
signatures are stable, but the resource selected for a particular trace changes.
It is not a type-safe way to mix unrelated buffer layouts behind the same
`[[buffer(n)]]` parameter. If different intersection functions need unrelated
resource types, prefer distinct buffer indices, a common explicit record layout,
separate tables, or an IFB/user-data design.

#### Visible Function Table Bindings

`get_visible_function_table<T>(index)` and `set_visible_function_table(...)`
perform the same kind of operation for visible function table bindings. Use
them when intersection functions call helper functions indirectly through a
visible function table, such as material-specific alpha tests.

```metal
visible_function_table<AlphaTestFn> alphaTests =
    ft.get_visible_function_table<AlphaTestFn>(0);

ft.set_visible_function_table(alphaTests, 0);
```

The ordinary intersection function table still decides which intersection
function to call. A visible function table is a secondary resource that the
selected intersection function can use. Rebinding it does not change the
ordinary intersection function table's function-entry layout, and it must remain
compatible with the fixed `visible_function_table<T>` type expected by the
selected intersection functions.

### 8.5 Validity Rules

- Do not use `intersection_function_buffer` or `user_data` tags on an ordinary
  `intersection_function_table`.
- The table's tag list must match the `intersector` and the selected
  `[[intersection(...)]]` functions.
- Do not call through a table containing functions whose primitive type or tags
  do not match traversal. Behavior is undefined if it is not diagnosed.
- Do not assume a null table and an empty table are the same state; test each
  with the appropriate API.

## 9. Metal 4 Intersection Function Buffers

<a id="metal-4-intersection-function-buffers"></a>

Metal 4 intersection function buffers are an alternative to ordinary
`intersection_function_table<tags...>` values. They are still traversal-time
dispatch for user `[[intersection(...)]]` functions, but the dispatch object is
a GPU buffer of function records instead of a fixed table object.

Like an ordinary function table, an IFB is not a D3D/Vulkan SBT. It does not
dispatch Metal miss or closest-hit logic. It only decides which custom
intersection function traversal calls for a candidate primitive.

The shader does not index the function buffer directly and does not call a
record directly. The shader passes an `intersection_function_buffer_arguments`
descriptor to `intersector::intersect(...)`. During traversal, Metal computes a
record index, fetches the record from the buffer, and invokes the selected
`[[intersection(...)]]` function.

### 9.1 What The IFB Descriptor Contains

```metal
struct intersection_function_buffer_arguments {
    const device void* intersection_function_buffer;
    size_t intersection_function_buffer_size;
    size_t intersection_function_stride;
};
```

The descriptor describes where the function records live:

- `intersection_function_buffer` is the device pointer to the first record.
- `intersection_function_buffer_size` is the buffer size in bytes.
- `intersection_function_stride` is the byte distance between records.

The stride must be in 8-byte increments and in the range 0 through 4096 bytes.
The function buffer pointer must be uniform within the SIMD-group for an IFB
intersect call. `MTLIntersectionFunctionBufferArguments` is convertible to this
shader type.

The function records themselves are populated by the host. Conceptually, a
small IFB with two ray types may look like this:

| IFB record | Selected during traversal for | Function entry |
| --- | --- | --- |
| `0` | triangle geometry, primary ray | `trianglePrimaryIntersection` |
| `1` | triangle geometry, shadow ray | `triangleShadowIntersection` |
| `2` | sphere geometry, primary ray | `spherePrimaryIntersection` |
| `3` | sphere geometry, shadow ray | `sphereShadowIntersection` |

The shader-side `intersect(...)` call does not mention record `0`, `1`, `2`, or
`3`. Those records are selected from acceleration-structure offsets plus the
intersector's IFB ray-type state.

### 9.2 How Metal Selects An IFB Record

The selected IFB record is based on acceleration-structure offsets plus two
intersector values:

```metal
void set_base_id(uint index);
void set_geometry_multiplier(uint multiplier);
```

Both use only the lower four bits in the base-index / geometry-multiplier
calculation. Defaults are base ID `0` and multiplier `1`.

The record index is:

```text
primitive AS:
    effectiveFunctionIndex =
        geometryIntersectionFunctionTableOffset * geometryMultiplier + baseID

instance AS:
    effectiveFunctionIndex =
        instanceIntersectionFunctionTableOffset +
        geometryIntersectionFunctionTableOffset * geometryMultiplier +
        baseID
```

Use `set_geometry_multiplier(rayTypeCount)` when each geometry has multiple
entries, such as primary-ray and shadow-ray functions. Use `set_base_id(rayType)`
to choose which entry within that per-geometry group the current trace uses.

For example:

```text
RayTypePrimary = 0
RayTypeShadow  = 1
RayTypeCount   = 2

triangleGeometry.intersectionFunctionTableOffset = 0
sphereGeometry.intersectionFunctionTableOffset   = 1

intersector.set_geometry_multiplier(RayTypeCount)
intersector.set_base_id(RayTypeShadow)

triangle shadow record = 0 * 2 + 1 = 1
sphere shadow record   = 1 * 2 + 1 = 3
```

The host must populate records with the same layout:

```text
IFB record[0] = trianglePrimaryIntersection
IFB record[1] = triangleShadowIntersection
IFB record[2] = spherePrimaryIntersection
IFB record[3] = sphereShadowIntersection
```

For an instance acceleration structure, the instance offset is added after the
geometry contribution. This lets the host reserve ranges of IFB records per
instance or material set:

```text
IFB record[16 + 3] = sphereShadowIntersectionForMaterialSet16

sphereGeometry.intersectionFunctionTableOffset = 1
instance.intersectionFunctionTableOffset       = 16
geometryMultiplier = 2
baseID = 1

effectiveFunctionIndex = 16 + 1 * 2 + 1
```

### 9.3 Host-Side Responsibilities

The exact host API spelling depends on the Metal binding layer, but the
responsibilities are:

1. Build a function-buffer object containing the compiled
   `[[intersection(...)]]` function records.
2. Choose a record layout, such as `geometryOffset * rayTypeCount + rayType`.
3. Store acceleration-structure geometry offsets, and optionally instance
   offsets, that select the intended record range.
4. Produce `MTLIntersectionFunctionBufferArguments` and pass it to shader code
   as `intersection_function_buffer_arguments`.
5. If the intersection functions use `[[user_data_buffer]]`, bind the user-data
   buffer separately as the `user_data` argument to the `intersect(...)` call.

Host-side pseudocode:

```cpp
// Host-side pseudocode. API spelling differs by binding layer.
ifb.setFunction(trianglePrimaryIntersection, 0);
ifb.setFunction(triangleShadowIntersection, 1);
ifb.setFunction(spherePrimaryIntersection, 2);
ifb.setFunction(sphereShadowIntersection, 3);

triangleGeometry.intersectionFunctionTableOffset = 0;
sphereGeometry.intersectionFunctionTableOffset = 1;

intersection_function_buffer_arguments args = makeIFBArguments(ifb);
```

The acceleration-structure offset selects a geometry group. `set_base_id`
selects the ray-type lane inside that group.

### 9.4 Shader-Side Shape

The intersector and intersection functions must include the
`intersection_function_buffer` tag. Add `user_data` only when the trace call
passes a user-data pointer and the selected intersection functions use
`[[user_data_buffer]]`.

```metal
struct BoxIntersectionReturn
{
    bool accept [[accept_intersection]];
    bool continueSearch [[continue_search]];
    float distance [[distance]];
};

struct SphereUserData
{
    device Sphere* spheres;
};

[[intersection(bounding_box, intersection_function_buffer, user_data)]]
BoxIntersectionReturn sphereShadowIntersection(
    float3 origin [[origin]],
    float3 direction [[direction]],
    float minDistance [[min_distance]],
    float maxDistance [[max_distance]],
    uint primitiveID [[primitive_id]],
    uint functionID [[function_id]],
    const device SphereUserData* userData [[user_data_buffer]])
{
    // `functionID` is the effective IFB record index selected by traversal.
    // `userData` is the pointer passed as the IFB `user_data` argument.
    float t = testSphere(origin, direction, minDistance, maxDistance,
        userData->spheres[primitiveID]);
    return BoxIntersectionReturn{t != INFINITY, false, t};
}
```

The tracing kernel configures the IFB selection state, then forwards the IFB
descriptor and optional user-data pointer:

```metal
kernel void traceWithIFB(
    primitive_acceleration_structure scene [[buffer(0)]],
    constant RayDesc* rays [[buffer(1)]],
    constant intersection_function_buffer_arguments& functions [[buffer(2)]],
    const device SphereUserData& userData [[buffer(3)]],
    constant uint& rayType [[buffer(4)]],
    device HitRecord* hits [[buffer(5)]],
    uint tid [[thread_position_in_grid]])
{
    intersector<intersection_function_buffer, user_data> tracer;
    tracer.assume_geometry_type(geometry_type::bounding_box);
    tracer.set_geometry_multiplier(RayTypeCount);
    tracer.set_base_id(rayType);

    intersection_result<intersection_function_buffer, user_data> hit =
        tracer.intersect(makeRay(rays[tid]), scene, functions, &userData);

    hits[tid] = makeHitRecord(hit);
}
```

In this example, `sphereShadowIntersection` is not named at the call site. The
host placed it in an IFB record, and Metal selects that record from geometry
offsets, `RayTypeCount`, and `rayType`.

### 9.5 User Data

IFB user data is separate from the IFB descriptor. It is enabled by adding the
`user_data` tag and by using an `intersect(...)` overload that includes
`const device void* user_data`.

The selected intersection function receives that pointer through a
`[[user_data_buffer]]` parameter:

```metal
[[intersection(bounding_box, intersection_function_buffer, user_data)]]
BoxIntersectionReturn sphereIntersection(
    uint primitiveID [[primitive_id]],
    const device SphereUserData* userData [[user_data_buffer]])
{
    return testSphere(userData->spheres[primitiveID]);
}
```

Do not pass a user-data pointer unless both the intersector and the intersection
function use the `intersection_function_buffer, user_data` tag combination.

### 9.6 Validity Rules

- `intersection_function_buffer` is a Metal 4 feature.
- `user_data` is valid only with `intersection_function_buffer`.
- The IFB function record pointer must be uniform within the SIMD-group for the
  `intersect(...)` call.
- The stride must be in 8-byte increments and in the range 0 through 4096
  bytes.
- The intersector, result type, and selected `[[intersection(...)]]` functions
  must use matching tag lists.
- `baseID` and `geometryMultiplier` use only their lower four bits in the
  effective-index calculation.

## 10. `intersection_query<tags...>`

`intersection_query` is a non-copyable, thread-local traversal object. It cannot
be a struct or union member, cannot be returned from a function, and cannot be
assigned.

Construct or reset it before calling `next()`. Calling `next()` before a
nondefault constructor or `reset(...)` is undefined.

Constructors and reset overloads without `instancing`:

```metal
intersection_query(ray r, primitive_acceleration_structure as);
intersection_query(ray r, primitive_acceleration_structure as, intersection_params params);
void reset(ray r, primitive_acceleration_structure as);
void reset(ray r, primitive_acceleration_structure as, intersection_params params);
```

Constructors and reset overloads with `instancing`:

```metal
intersection_query(ray r, instance_acceleration_structure as);
intersection_query(ray r, instance_acceleration_structure as, intersection_params params);
intersection_query(ray r, instance_acceleration_structure as, uint mask);
intersection_query(ray r, instance_acceleration_structure as, uint mask, intersection_params params);
void reset(ray r, instance_acceleration_structure as);
void reset(ray r, instance_acceleration_structure as, intersection_params params);
void reset(ray r, instance_acceleration_structure as, uint mask);
void reset(ray r, instance_acceleration_structure as, uint mask, intersection_params params);
```

Query control:

- `bool next()`
- `void abort()`
- `void commit_triangle_intersection()`
- `void commit_bounding_box_intersection(float distance)`
- `void commit_curve_intersection()` in Metal 3.1+

Common query values:

- `get_candidate_intersection_type()`
- `get_committed_intersection_type()`
- `get_world_space_ray_origin()`
- `get_world_space_ray_direction()`
- `get_ray_min_distance()`
- `get_intersection_params()`

Candidate values:

| Method | Valid for |
| --- | --- |
| `get_candidate_triangle_distance()` | Triangle candidates |
| `get_candidate_curve_distance()` | Curve candidates, SDK-observed in local Metal compiler 32023.883 |
| `get_candidate_geometry_id()` | Triangle, bounding box, curve candidates |
| `get_candidate_primitive_id()` | Triangle, bounding box, curve candidates |
| `get_candidate_primitive_data()` | Metal 3+ |
| `get_candidate_ray_origin()` | Object-space ray origin for current candidate |
| `get_candidate_ray_direction()` | Object-space ray direction for current candidate |
| `is_candidate_non_opaque_bounding_box()` | Bounding box candidates |
| `get_candidate_triangle_barycentric_coord()` | Requires `triangle_data` and triangle candidate |
| `is_candidate_triangle_front_facing()` | Requires `triangle_data` and triangle candidate |
| `get_candidate_curve_parameter()` | Requires `curve_data` and curve candidate |
| `get_candidate_instance_id()` | Requires `instancing` without `max_levels` |
| `get_candidate_user_instance_id()` | Requires `instancing` without `max_levels` |
| `get_candidate_instance_count()` | Requires `instancing, max_levels<Count>` |
| `get_candidate_instance_id(uint depth)` | Requires `instancing, max_levels<Count>` |
| `get_candidate_user_instance_id(uint depth)` | Requires `instancing, max_levels<Count>` |
| `get_candidate_object_to_world_transform()` | Requires `instancing` |
| `get_candidate_world_to_object_transform()` | Requires `instancing` |

Committed values:

| Method | Valid for |
| --- | --- |
| `get_committed_distance()` | Any committed hit |
| `get_committed_geometry_id()` | Any committed hit |
| `get_committed_primitive_id()` | Any committed hit |
| `get_committed_primitive_data()` | Metal 3+ |
| `get_committed_ray_origin()` | Object-space ray origin for committed hit |
| `get_committed_ray_direction()` | Object-space ray direction for committed hit |
| `get_committed_triangle_barycentric_coord()` | Requires `triangle_data` and triangle committed hit |
| `is_committed_triangle_front_facing()` | Requires `triangle_data` and triangle committed hit |
| `get_committed_curve_parameter()` | Requires `curve_data` and curve committed hit |
| `get_committed_instance_id()` | Requires `instancing` without `max_levels` |
| `get_committed_user_instance_id()` | Requires `instancing` without `max_levels` |
| `get_committed_instance_count()` | Requires `instancing, max_levels<Count>` |
| `get_committed_instance_id(uint depth)` | Requires `instancing, max_levels<Count>` |
| `get_committed_user_instance_id(uint depth)` | Requires `instancing, max_levels<Count>` |
| `get_committed_object_to_world_transform()` | Requires `instancing` |
| `get_committed_world_to_object_transform()` | Requires `instancing` |

Query workflow:

```metal
intersection_params params;
params.set_triangle_cull_mode(triangle_cull_mode::back);

intersection_query<instancing, triangle_data> q(r, scene, 0xffu, params);
while (q.next()) {
    switch (q.get_candidate_intersection_type()) {
    case intersection_type::triangle:
        // Inspect barycentrics/front-facing data if needed.
        q.commit_triangle_intersection();
        break;
    case intersection_type::bounding_box:
        // User computes a hit distance for the procedural primitive.
        float distance = computeDistanceForCandidate(q);
        q.commit_bounding_box_intersection(distance);
        break;
    case intersection_type::curve:
        q.commit_curve_intersection();
        break;
    default:
        break;
    }
}

if (q.get_committed_intersection_type() != intersection_type::none) {
    float t = q.get_committed_distance();
}
```

Invalid query use:

- Do not call candidate methods unless `next()` returned true and the candidate
  type matches the method.
- Do not call triangle-data methods unless the query includes `triangle_data`.
- Do not call curve-parameter methods unless the query includes `curve_data`.
- Do not call instance methods unless the query includes `instancing`.
- Do not call `depth` overloads unless the query includes `max_levels<Count>`.
- `depth` must be less than the returned instance count and less than `Count`.
- Do not commit a candidate with the wrong commit function.
- `commit_bounding_box_intersection(distance)` must use a valid hit distance in
  the active ray interval for the candidate primitive.

## 11. Curve Utility Functions

Metal 3.1+ `<metal_curves>` provides curve basis helpers. `Ps` is `float` or
`half`; `P` is a scalar or vector of `Ps`.

Families:

- `bezier`, `bezier_derivative`, `bezier_second_derivative`
- `bspline`, `bspline_derivative`, `bspline_second_derivative`
- `hermite`, `hermite_derivative`, `hermite_second_derivative`
- `catmull_rom`, `catmull_rom_derivative`, `catmull_rom_second_derivative`

Quadratic functions take `t, p0, p1, p2`; cubic functions take
`t, p0, p1, p2, p3`, except Hermite, which takes `t, p0, p1, m0, m1`.

## 12. Tag Reference

<a id="tag-reference"></a>

[Back to Tag Notation](#tag-notation) | [Back to Core Types](#core-types)

Tags are empty struct types in `metal::raytracing`. They specialize which
acceleration structure kind is traversed, which data is available, and which
methods exist. In practice, a tag list is a compile-time contract shared by the
related objects in one traversal path:

```metal
intersector<instancing, triangle_data> i;
intersection_result<instancing, triangle_data> hit = i.intersect(r, scene, 0xffu);
```

Tag order is part of the type. If an intersection function, intersector,
function table, result type, or query uses a different tag set or order, the
compiler may reject the program; otherwise behavior is undefined.

### 12.1 Available Tags

| Tag | Meaning | Valid on |
| --- | --- | --- |
| `instancing` | Traverse an instance acceleration structure and expose instance IDs. | Intersection functions, results, intersectors, acceleration structures, queries, function tables. |
| `triangle_data` | Expose triangle barycentrics and front-facing state. | Intersection functions, results, intersectors, queries, function tables. Invalid on acceleration structures. |
| `world_space_data` | Expose world-space ray and transform data. | Intersection functions, results, intersectors, function tables. Invalid on acceleration structures and queries. Requires `instancing` for transform result fields. |
| `primitive_motion` | Enable primitive-level motion. | Intersectors, function tables, acceleration structures, intersection functions. Invalid on queries. |
| `instance_motion` | Enable instance-level motion. | Intersectors, function tables, acceleration structures, intersection functions. Invalid on queries. Requires `instancing` for acceleration structures. |
| `extended_limits` | Match acceleration structures built with extended primitive/geometry/instance/mask limits. | Intersectors, function tables, results, intersection functions. Invalid on acceleration structures and queries. |
| `curve_data` | Expose curve parameter data. | Intersection functions, results, intersectors, queries, function tables. Metal 3.1+. |
| `max_levels<Count>` | Enable multi-level instancing. | Intersectors, queries, function tables, results, intersection functions. Invalid on acceleration structures. Requires `instancing`. |
| `intersection_function_buffer` | Use Metal 4 intersection function buffers instead of an intersection function table. | Intersectors, intersection functions, and corresponding result types. Not a valid `intersection_function_table` tag. |
| `user_data` | Make a user data buffer available to an intersection function buffer function. | Only valid together with `intersection_function_buffer`. |

`max_levels<Count>` ranges:

| Type | Valid `Count` |
| --- | --- |
| `intersection_query` | 2 through 16 |
| `intersector` | 2 through 32 |
| `intersection_function_table` | 2 through 32, and must match its query or intersector |

### 12.2 Invalid Tag Rules

- Do not use tags that are not listed for the object type.
- Do not use `user_data` without `intersection_function_buffer`.
- Do not use `max_levels<Count>` without `instancing`.
- Do not repeat `max_levels` with different counts.
- Do not use `world_space_data` in an `intersection_query`.
- Do not use motion tags in an `intersection_query`.
- Do not pass an intersection function with a different tag set/order through a
  table or buffer. If the compiler cannot diagnose the mismatch, behavior is
  undefined.

### 12.3 Valid Tag Combinations

For intersection functions, `intersection_result`, ordinary
`intersection_function_table`, and, subject to the stricter intersector rules
below, `intersector`, Metal 2.3 starts with:

- no tags
- `triangle_data`
- `instancing`
- `instancing, triangle_data`
- `instancing, world_space_data`
- `instancing, triangle_data, world_space_data`

Metal 2.4 adds motion variants:

- `primitive_motion`
- `triangle_data, primitive_motion`
- `instancing, primitive_motion`
- `instancing, triangle_data, primitive_motion`
- `instancing, world_space_data, primitive_motion`
- `instancing, triangle_data, world_space_data, primitive_motion`
- `instance_motion`
- `instancing, instance_motion`
- `instancing, triangle_data, instance_motion`
- `instancing, world_space_data, instance_motion`
- `instancing, triangle_data, world_space_data, instance_motion`
- `instancing, primitive_motion, instance_motion`
- `instancing, triangle_data, primitive_motion, instance_motion`
- `instancing, world_space_data, primitive_motion, instance_motion`
- `instancing, triangle_data, world_space_data, primitive_motion, instance_motion`

Then:

- `extended_limits` may be added to the combinations above.
- Metal 3.1 `curve_data` may be added to the combinations above.
- Metal 3.1 `max_levels<Count>` may be added to combinations containing
  `instancing`.
- Metal 4 `intersection_function_buffer` may be added for intersector and
  intersection function buffer workflows.
- Metal 4 `user_data` may be added only with `intersection_function_buffer`.

For `acceleration_structure<tags...>`, only these are valid:

| Structure kind | Valid shader type |
| --- | --- |
| Primitive | `acceleration_structure<>` |
| Primitive with primitive motion | `acceleration_structure<primitive_motion>` |
| Instance | `acceleration_structure<instancing>` |
| Instance with primitive motion | `acceleration_structure<instancing, primitive_motion>` |
| Instance with instance motion | `acceleration_structure<instancing, instance_motion>` |
| Instance with both motion kinds | `acceleration_structure<instancing, primitive_motion, instance_motion>` |

For `intersection_query<tags...>`, only these are valid:

- no tags
- `triangle_data`
- `instancing`
- `instancing, triangle_data`
- `instancing, max_levels<Count>`
- `instancing, triangle_data, max_levels<Count>`
- Metal 3.1 `curve_data` may be added to those combinations.

## 13. Usage Examples

<a id="usage-examples"></a>

The examples below are shader-side MSL only. They assume the host has already
built and bound the acceleration structures, intersection function tables, and
Metal 4 intersection function buffers where required. The snippets are written
so they can be read as one file; common helper structs are defined first and
reused by later examples. The representative code shapes were checked with the
local Metal compiler `32023.883` using `-std=metal4.0`.

### 13.1 Shared Setup

```metal
#include <metal_stdlib>
#include <metal_raytracing>
#include <metal_curves>
using namespace metal;
using namespace metal::raytracing;

struct RayDesc
{
    float3 origin;
    float3 direction;
    float minDistance;
    float maxDistance;
};

struct HitRecord
{
    uint type;
    float distance;
    uint primitiveID;
    uint geometryID;
    uint instanceID;
    uint userInstanceID;
    float2 barycentrics;
    bool frontFacing;
};

static ray makeRay(RayDesc desc)
{
    return ray(desc.origin, desc.direction, desc.minDistance, desc.maxDistance);
}
```

Use `ray` as the handoff type between your own launch data and Metal traversal.
The top-level ray is in world space. If traversal reaches an intersection
function or an inline query candidate, candidate `origin` and `direction` values
are object-space.

The examples start with `intersector` and intersection-function workflows,
because those are the closest Metal has to a pipeline-style ray tracing model.
Metal does not expose DXR-style ray-generation, closest-hit, any-hit, and miss
shader stages; the caller shader owns those decisions and uses `intersector` or
intersection functions as traversal building blocks. Inline `intersection_query`
examples are intentionally placed at the end.

### 13.2 Built-In Triangle Intersector

Use `intersector` when Metal should perform traversal and return the best hit
directly. This is the shortest built-in triangle path.

```metal
kernel void traceTriangleScene(
    instance_acceleration_structure scene [[buffer(0)]],
    constant RayDesc* rays [[buffer(1)]],
    device HitRecord* hits [[buffer(2)]],
    uint tid [[thread_position_in_grid]])
{
    intersector<instancing, triangle_data> i;
    i.assume_geometry_type(geometry_type::triangle);
    i.set_triangle_cull_mode(triangle_cull_mode::back);

    intersection_result<instancing, triangle_data> result =
        i.intersect(makeRay(rays[tid]), scene, 0xffu);

    HitRecord h = {};
    h.type = uint(result.type);
    if (result.type != intersection_type::none)
    {
        h.distance = result.distance;
        h.primitiveID = result.primitive_id;
        h.geometryID = result.geometry_id;
        h.instanceID = result.instance_id;
        h.userInstanceID = result.user_instance_id;
        h.barycentrics = result.triangle_barycentric_coord;
        h.frontFacing = result.triangle_front_facing;
    }
    hits[tid] = h;
}
```

Here the tag list must match between `intersector` and `intersection_result`.
The `mask` argument is accepted because `instancing` is present.

### 13.3 Procedural Primitive Helpers

The next examples use procedural sphere primitives backed by bounding boxes in
the acceleration structure. The primitive ID selects a sphere from a shader
buffer.

```metal
struct Sphere
{
    float3 center;
    float radius;
};

static float intersectSphere(
    float3 origin,
    float3 direction,
    Sphere sphere,
    float minDistance,
    float maxDistance)
{
    float3 oc = origin - sphere.center;
    float a = dot(direction, direction);
    if (a == 0.0f)
        return INFINITY;

    float halfB = dot(oc, direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = halfB * halfB - a * c;
    if (discriminant < 0.0f)
        return INFINITY;

    float root = sqrt(discriminant);
    float t0 = (-halfB - root) / a;
    float t1 = (-halfB + root) / a;
    if (t0 >= minDistance && t0 <= maxDistance)
        return t0;
    if (t1 >= minDistance && t1 <= maxDistance)
        return t1;
    return INFINITY;
}
```

### 13.4 Custom Bounding-Box Intersection Function

An intersection function lets Metal traversal call custom primitive code through
an `intersection_function_table`. The function receives object-space ray data
and optional `ray_data` payload.

```metal
struct SpherePayload
{
    uint testedCount;
    uint acceptedPrimitive;
};

struct BoxIntersectionReturn
{
    bool accept [[accept_intersection]];
    bool continueSearch [[continue_search]];
    float distance [[distance]];
};

[[intersection(bounding_box)]]
BoxIntersectionReturn sphereIntersection(
    float3 origin [[origin]],
    float3 direction [[direction]],
    float minDistance [[min_distance]],
    float maxDistance [[max_distance]],
    uint primitiveID [[primitive_id]],
    const device Sphere* spheres [[buffer(0)]],
    ray_data SpherePayload& payload [[payload]])
{
    payload.testedCount += 1;

    float t = intersectSphere(
        origin, direction, spheres[primitiveID], minDistance, maxDistance);
    bool accept = t != INFINITY;

    if (accept)
        payload.acceptedPrimitive = primitiveID;

    return BoxIntersectionReturn{accept, true, t};
}

kernel void traceProceduralSceneWithTable(
    primitive_acceleration_structure scene [[buffer(0)]],
    intersection_function_table<> functions [[buffer(1)]],
    constant RayDesc* rays [[buffer(2)]],
    device HitRecord* hits [[buffer(3)]],
    device SpherePayload* payloadOut [[buffer(4)]],
    uint tid [[thread_position_in_grid]])
{
    intersector<> i;
    i.assume_geometry_type(geometry_type::bounding_box);

    SpherePayload payload = {};
    intersection_result<> result =
        i.intersect(makeRay(rays[tid]), scene, functions, payload);

    HitRecord h = {};
    h.type = uint(result.type);
    if (result.type != intersection_type::none)
    {
        h.distance = result.distance;
        h.primitiveID = result.primitive_id;
        h.geometryID = result.geometry_id;
    }

    hits[tid] = h;
    payloadOut[tid] = payload;
}
```

Important validity points shown here:

- The function table type is `intersection_function_table<>`, matching both
  `intersector<>` and `[[intersection(bounding_box)]]`.
- The host must put `sphereIntersection` in the table slot selected by the
  primitive's geometry/function-table offsets.
- The payload is a thread value at the call site and a `ray_data` reference in
  the intersection function.

### 13.5 Direct-Access Callback

Metal 3.2 adds intersector callback overloads. The callback receives an
`intersection_result_ref<tags...>` whose lifetime ends when the callback
returns.

```metal
kernel void traceTriangleSceneCallback(
    instance_acceleration_structure scene [[buffer(0)]],
    constant RayDesc* rays [[buffer(1)]],
    device HitRecord* hits [[buffer(2)]],
    uint tid [[thread_position_in_grid]])
{
    intersector<instancing, triangle_data> i;
    i.assume_geometry_type(geometry_type::triangle);

    HitRecord h = {};
    i.intersect(makeRay(rays[tid]), scene, 0xffu,
        [&](intersection_result_ref<instancing, triangle_data> result)
        {
            h.type = uint(result.get_type());
            if (result.get_type() != intersection_type::none)
            {
                h.distance = result.get_distance();
                h.primitiveID = result.get_primitive_id();
                h.geometryID = result.get_geometry_id();
                h.instanceID = result.get_instance_id();
                h.userInstanceID = result.get_user_instance_id();
                h.barycentrics = result.get_triangle_barycentric_coord();
                h.frontFacing = result.is_triangle_front_facing();
            }
        });

    hits[tid] = h;
}
```

Do not store `result`, any pointer derived from it, or a callback payload
reference after the lambda returns.

<a id="ifb-usage-example"></a>

### 13.6 Metal 4 IFB Usage Example

Metal 4 can use an intersection function buffer instead of an ordinary function
table. The IFB arguments are passed as an `intersection_function_buffer_arguments`
object, usually populated from `MTLIntersectionFunctionBufferArguments` on the
host. The `intersection_function_buffer` pointer inside that object must be
uniform within the SIMD-group for the intersect call.

This example shows the reason to use IFB: one acceleration structure can share a
buffer layout that stores multiple intersection functions per geometry. The
shader chooses the ray type with `set_base_id(...)`, while
`intersection_function_buffer_arguments` tells Metal where the function records
live, how large the buffer is, and how far apart records are.

Host-side layout:

```text
RayTypePrimary = 0
RayTypeShadow  = 1
RayTypeCount   = 2

effectiveFunctionIndex =
    geometryIntersectionFunctionTableOffset * RayTypeCount + rayType

IFB record[geometryOffset * 2 + 0] = spherePrimaryIntersection
IFB record[geometryOffset * 2 + 1] = sphereShadowIntersection
```

With an ordinary `intersection_function_table`, the shader receives a fixed
table object. With IFB, the shader receives a function-buffer descriptor and can
select the ray-type lane inside the buffer without rebinding a different table.

```metal
enum RayType : uint
{
    RayTypePrimary = 0,
    RayTypeShadow = 1,
    RayTypeCount = 2,
};

struct SphereUserData
{
    device Sphere* spheres;
};

[[intersection(bounding_box, intersection_function_buffer, user_data)]]
BoxIntersectionReturn spherePrimaryIntersection(
    float3 origin [[origin]],
    float3 direction [[direction]],
    float minDistance [[min_distance]],
    float maxDistance [[max_distance]],
    uint primitiveID [[primitive_id]],
    uint functionID [[function_id]],
    const device SphereUserData* userData [[user_data_buffer]])
{
    (void)functionID;

    float t = intersectSphere(
        origin, direction, userData->spheres[primitiveID], minDistance, maxDistance);
    bool accept = t != INFINITY;
    return BoxIntersectionReturn{accept, true, t};
}

[[intersection(bounding_box, intersection_function_buffer, user_data)]]
BoxIntersectionReturn sphereShadowIntersection(
    float3 origin [[origin]],
    float3 direction [[direction]],
    float minDistance [[min_distance]],
    float maxDistance [[max_distance]],
    uint primitiveID [[primitive_id]],
    uint functionID [[function_id]],
    const device SphereUserData* userData [[user_data_buffer]])
{
    (void)functionID;

    float t = intersectSphere(
        origin, direction, userData->spheres[primitiveID], minDistance, maxDistance);
    bool accept = t != INFINITY;
    return BoxIntersectionReturn{accept, false, t};
}

kernel void traceProceduralSceneWithIFB(
    primitive_acceleration_structure scene [[buffer(0)]],
    constant RayDesc* rays [[buffer(1)]],
    device HitRecord* hits [[buffer(2)]],
    constant intersection_function_buffer_arguments& functions [[buffer(3)]],
    const device SphereUserData& userData [[buffer(4)]],
    constant uint& rayType [[buffer(5)]],
    uint tid [[thread_position_in_grid]])
{
    intersector<intersection_function_buffer, user_data> i;
    i.assume_geometry_type(geometry_type::bounding_box);
    i.set_geometry_multiplier(RayTypeCount);
    i.set_base_id(rayType);
    if (rayType == RayTypeShadow)
        i.accept_any_intersection(true);

    intersection_result<intersection_function_buffer, user_data> result =
        i.intersect(makeRay(rays[tid]), scene, functions, &userData);

    HitRecord h = {};
    h.type = uint(result.type);
    if (result.type != intersection_type::none)
    {
        h.distance = result.distance;
        h.primitiveID = result.primitive_id;
        h.geometryID = result.geometry_id;
    }
    hits[tid] = h;
}
```

Important validity points shown here:

- `user_data` is valid only because `intersection_function_buffer` is also in
  the tag list.
- `intersection_function_buffer_arguments` supplies the function-buffer pointer,
  size, and stride that let Metal fetch the selected IFB record.
- `set_geometry_multiplier(RayTypeCount)` says each geometry owns two IFB
  records. `set_base_id(rayType)` chooses the primary or shadow record.
- The intersection function and intersector must use the same IFB/user-data tag
  list.

### 13.7 Secondary-Ray Techniques

Reflection, shadows, and refraction are not separate Metal shader stages. They
are ordinary shader code that launches additional rays with `intersector`. The
following helpers extract a triangle surface, offset secondary rays to avoid
self-intersection, and compute refraction with total-internal-reflection fallback.

```metal
struct TriangleInfo
{
    uint3 vertexIndex;
    uint materialID;
};

struct SurfaceData
{
    bool hit;
    float distance;
    float3 position;
    float3 normal;
    uint materialID;
    bool frontFacing;
};

static float3 offsetRayOrigin(float3 p, float3 n)
{
    return p + n * 1.0e-3f;
}

static SurfaceData makeSurfaceData(
    intersection_result<instancing, triangle_data> result,
    ray r,
    const device TriangleInfo* triangles,
    const device float3* positions)
{
    SurfaceData s = {};
    s.hit = result.type != intersection_type::none;
    if (!s.hit)
        return s;

    TriangleInfo tri = triangles[result.primitive_id];
    float3 p0 = positions[tri.vertexIndex.x];
    float3 p1 = positions[tri.vertexIndex.y];
    float3 p2 = positions[tri.vertexIndex.z];

    s.distance = result.distance;
    s.position = r.origin + r.direction * result.distance;
    s.normal = normalize(cross(p1 - p0, p2 - p0));
    s.frontFacing = result.triangle_front_facing;
    if (!s.frontFacing)
        s.normal = -s.normal;
    s.materialID = tri.materialID;
    return s;
}

static bool refractDirection(
    float3 incident,
    float3 normal,
    float eta,
    thread float3& outDirection)
{
    float cosI = clamp(dot(-incident, normal), -1.0f, 1.0f);
    float sinT2 = eta * eta * (1.0f - cosI * cosI);
    if (sinT2 > 1.0f)
        return false;

    outDirection = normalize(eta * incident + (eta * cosI - sqrt(1.0f - sinT2)) * normal);
    return true;
}
```

Shadow rays usually only need to know whether anything was hit. Use
`accept_any_intersection(true)` so traversal may stop at the first valid blocker.

```metal
static bool isOccluded(
    instance_acceleration_structure scene,
    float3 origin,
    float3 direction,
    float maxDistance,
    uint mask)
{
    intersector<instancing> shadow;
    shadow.assume_geometry_type(geometry_type::triangle);
    shadow.accept_any_intersection(true);

    ray shadowRay(origin, direction, 1.0e-3f, maxDistance);
    intersection_result<instancing> blocker = shadow.intersect(shadowRay, scene, mask);
    return blocker.type != intersection_type::none;
}

kernel void shadeShadowVisibility(
    instance_acceleration_structure scene [[buffer(0)]],
    constant RayDesc* shadowRays [[buffer(1)]],
    device uchar* visibility [[buffer(2)]],
    uint tid [[thread_position_in_grid]])
{
    RayDesc desc = shadowRays[tid];
    bool blocked = isOccluded(scene, desc.origin, desc.direction, desc.maxDistance, 0xffu);
    visibility[tid] = blocked ? 0 : 1;
}
```

Mirror reflection is a primary `intersect`, surface reconstruction, then a
secondary `intersect` along `reflect(incident, normal)`.

```metal
kernel void shadeMirrorReflection(
    instance_acceleration_structure scene [[buffer(0)]],
    constant RayDesc* primaryRays [[buffer(1)]],
    const device TriangleInfo* triangles [[buffer(2)]],
    const device float3* positions [[buffer(3)]],
    device float3* colors [[buffer(4)]],
    uint tid [[thread_position_in_grid]])
{
    intersector<instancing, triangle_data> primary;
    primary.assume_geometry_type(geometry_type::triangle);

    ray r = makeRay(primaryRays[tid]);
    intersection_result<instancing, triangle_data> hit =
        primary.intersect(r, scene, 0xffu);
    SurfaceData surface = makeSurfaceData(hit, r, triangles, positions);
    if (!surface.hit)
    {
        colors[tid] = float3(0.02f, 0.04f, 0.08f);
        return;
    }

    float3 reflectionDir = normalize(reflect(normalize(r.direction), surface.normal));
    ray reflectionRay(
        offsetRayOrigin(surface.position, surface.normal),
        reflectionDir,
        0.0f,
        1000.0f);
    intersection_result<instancing, triangle_data> reflected =
        primary.intersect(reflectionRay, scene, 0xffu);

    colors[tid] =
        reflected.type == intersection_type::none
            ? float3(0.02f, 0.04f, 0.08f)
            : float3(0.9f, 0.9f, 0.9f);
}
```

Refraction chooses an entering or exiting index-of-refraction ratio, then falls
back to reflection when total internal reflection occurs.

```metal
kernel void shadeGlassRefraction(
    instance_acceleration_structure scene [[buffer(0)]],
    constant RayDesc* primaryRays [[buffer(1)]],
    const device TriangleInfo* triangles [[buffer(2)]],
    const device float3* positions [[buffer(3)]],
    device float3* colors [[buffer(4)]],
    uint tid [[thread_position_in_grid]])
{
    constexpr float airIOR = 1.0f;
    constexpr float glassIOR = 1.5f;

    intersector<instancing, triangle_data> primary;
    primary.assume_geometry_type(geometry_type::triangle);

    ray r = makeRay(primaryRays[tid]);
    intersection_result<instancing, triangle_data> hit =
        primary.intersect(r, scene, 0xffu);
    SurfaceData surface = makeSurfaceData(hit, r, triangles, positions);
    if (!surface.hit)
    {
        colors[tid] = float3(0.02f, 0.04f, 0.08f);
        return;
    }

    float3 incident = normalize(r.direction);
    float eta = surface.frontFacing ? airIOR / glassIOR : glassIOR / airIOR;
    float3 shadingNormal = surface.normal;
    float3 secondaryDir;
    bool transmitted = refractDirection(incident, shadingNormal, eta, secondaryDir);

    if (!transmitted)
        secondaryDir = normalize(reflect(incident, shadingNormal));

    ray secondaryRay(surface.position + secondaryDir * 1.0e-3f, secondaryDir, 0.0f, 1000.0f);
    intersection_result<instancing, triangle_data> secondary =
        primary.intersect(secondaryRay, scene, 0xffu);

    colors[tid] =
        secondary.type == intersection_type::none
            ? float3(0.6f, 0.8f, 1.0f)
            : float3(0.8f, 0.95f, 1.0f);
}
```

### 13.8 Inline Triangle Query

Use `intersection_query` when shader code wants to decide which candidates are
accepted. This shape is closest to DXR/Vulkan inline ray query.

```metal
kernel void queryTriangleScene(
    instance_acceleration_structure scene [[buffer(0)]],
    constant RayDesc* rays [[buffer(1)]],
    device HitRecord* hits [[buffer(2)]],
    uint tid [[thread_position_in_grid]])
{
    intersection_params params;
    params.set_triangle_cull_mode(triangle_cull_mode::back);
    params.assume_geometry_type(geometry_type::triangle);

    ray r = makeRay(rays[tid]);
    intersection_query<instancing, triangle_data> q(r, scene, 0xffu, params);

    while (q.next())
    {
        if (q.get_candidate_intersection_type() == intersection_type::triangle)
        {
            float2 bc = q.get_candidate_triangle_barycentric_coord();
            bool frontFacing = q.is_candidate_triangle_front_facing();

            (void)bc;
            (void)frontFacing;
            q.commit_triangle_intersection();
        }
    }

    HitRecord h = {};
    h.type = uint(q.get_committed_intersection_type());
    if (q.get_committed_intersection_type() != intersection_type::none)
    {
        h.distance = q.get_committed_distance();
        h.primitiveID = q.get_committed_primitive_id();
        h.geometryID = q.get_committed_geometry_id();
        h.instanceID = q.get_committed_instance_id();
        h.userInstanceID = q.get_committed_user_instance_id();
        h.barycentrics = q.get_committed_triangle_barycentric_coord();
        h.frontFacing = q.is_committed_triangle_front_facing();
    }
    hits[tid] = h;
}
```

Important validity points shown here:

- `instancing` is required because `scene` is an `instance_acceleration_structure`
  and because the code reads instance IDs.
- `triangle_data` is required before calling barycentric/front-facing methods.
- `mask` is valid only because the query has `instancing`.

### 13.9 Inline Procedural Query

For procedural geometry, the acceleration structure contains bounding boxes.
The query reports bounding-box candidates; shader code computes the actual hit
and commits a distance.

```metal
kernel void queryProceduralScene(
    primitive_acceleration_structure scene [[buffer(0)]],
    constant RayDesc* rays [[buffer(1)]],
    device Sphere* spheres [[buffer(2)]],
    device float* distances [[buffer(3)]],
    uint tid [[thread_position_in_grid]])
{
    intersection_params params;
    params.assume_geometry_type(geometry_type::bounding_box);

    ray r = makeRay(rays[tid]);
    intersection_query<> q(r, scene, params);

    while (q.next())
    {
        if (q.get_candidate_intersection_type() != intersection_type::bounding_box)
            continue;

        float maxDistance =
            q.get_committed_intersection_type() == intersection_type::none
                ? r.max_distance
                : q.get_committed_distance();

        uint sphereIndex = q.get_candidate_primitive_id();
        float t = intersectSphere(
            q.get_candidate_ray_origin(),
            q.get_candidate_ray_direction(),
            spheres[sphereIndex],
            q.get_ray_min_distance(),
            maxDistance);

        if (t != INFINITY)
            q.commit_bounding_box_intersection(t);
    }

    distances[tid] =
        q.get_committed_intersection_type() == intersection_type::none
            ? INFINITY
            : q.get_committed_distance();
}
```

Important validity points shown here:

- `intersection_query<>` takes a `primitive_acceleration_structure`, not an
  instance structure.
- `assume_geometry_type(geometry_type::bounding_box)` tells traversal to visit
  procedural geometry.
- `commit_bounding_box_intersection(t)` must receive a finite distance inside
  the active ray interval.

### 13.10 Curve Query

Curves require `curve_data` to read curve parameters, and traversal must be told
that curve geometry is present.

```metal
struct CurveHitRecord
{
    uint type;
    float distance;
    float curveParameter;
    uint primitiveID;
};

kernel void queryCurveScene(
    primitive_acceleration_structure scene [[buffer(0)]],
    constant RayDesc* rays [[buffer(1)]],
    device CurveHitRecord* hits [[buffer(2)]],
    uint tid [[thread_position_in_grid]])
{
    intersection_params params;
    params.assume_geometry_type(geometry_type::curve);
    params.assume_curve_basis(curve_basis::bezier);
    params.assume_curve_type(curve_type::round);
    params.assume_curve_control_point_count(4);

    intersection_query<curve_data> q(makeRay(rays[tid]), scene, params);
    while (q.next())
    {
        if (q.get_candidate_intersection_type() == intersection_type::curve)
            q.commit_curve_intersection();
    }

    CurveHitRecord h = {};
    h.type = uint(q.get_committed_intersection_type());
    if (q.get_committed_intersection_type() == intersection_type::curve)
    {
        h.distance = q.get_committed_distance();
        h.curveParameter = q.get_committed_curve_parameter();
        h.primitiveID = q.get_committed_primitive_id();
    }
    hits[tid] = h;
}
```

If the curve basis, curve type, or control-point count are not uniform across
the traced geometry, leave the corresponding hint at its default broad value.

## 14. Practical Tips

<a id="practical-tips"></a>

[Back to Tag Notation](#tag-notation) | [Back to Core Types](#core-types) |
[Back to Usage Examples](#usage-examples)

`[[origin]]` and `[[direction]]` are always object-space inside an intersection
function. Adding the `world_space_data` tag does not change those attributes.
Instead, it enables additional attributes such as `[[world_space_origin]]`,
`[[world_space_direction]]`, `[[object_to_world_transform]]`, and
`[[world_to_object_transform]]` for functions that also need world-space data.

```metal
struct WorldSpaceBoxResult
{
    bool accept [[accept_intersection]];
    bool continueSearch [[continue_search]];
    float distance [[distance]];
};

[[intersection(bounding_box, instancing, world_space_data)]]
WorldSpaceBoxResult sphereIntersectionWithWorldData(
    float3 objectOrigin [[origin]],
    float3 objectDirection [[direction]],
    float minDistance [[min_distance]],
    float maxDistance [[max_distance]],
    float3 worldOrigin [[world_space_origin]],
    float3 worldDirection [[world_space_direction]],
    float4x3 objectToWorld [[object_to_world_transform]],
    float4x3 worldToObject [[world_to_object_transform]])
{
    // objectOrigin/objectDirection: use for primitive-local intersection math.
    // worldOrigin/worldDirection/objectToWorld/worldToObject: use for
    // world-space shading, debugging, or fetching world-space-dependent data.
    (void)worldOrigin;
    (void)worldDirection;
    (void)objectToWorld;
    (void)worldToObject;

    float3 oc = objectOrigin;
    float a = dot(objectDirection, objectDirection);
    float halfB = dot(oc, objectDirection);
    float c = dot(oc, oc) - 1.0f;
    float discriminant = halfB * halfB - a * c;

    float t = INFINITY;
    if (discriminant >= 0.0f && a != 0.0f)
    {
        float root = sqrt(discriminant);
        float t0 = (-halfB - root) / a;
        float t1 = (-halfB + root) / a;
        t = t0 >= minDistance && t0 <= maxDistance ? t0 : t1;
    }

    bool accept = t >= minDistance && t <= maxDistance;
    return WorldSpaceBoxResult{accept, true, t};
}
```

## 15. Design Notes for a Slang API

- Model Metal's tag list as a compile-time capability contract. It is not just
  decoration: it changes available types, overloads, fields, and attributes.
- Separate acceleration-structure shape from result/query data. For example,
  `triangle_data` is valid for results and queries but invalid for
  `acceleration_structure`.
- Treat `intersection_query` and `intersector` as two different backends for the
  same conceptual operation. Queries are closest to Vulkan/DXR inline ray query;
  intersectors plus intersection functions are closer to Metal's native custom
  intersection workflow.
- Do not design an API that assumes DXR-style programmable closest-hit/miss
  stages are present in MSL. Metal's shader-visible primitives expose traversal
  and custom intersection evaluation, while shading decisions are usually in the
  calling shader.
- Preserve validity at the type level when possible: mask only with instancing,
  time only with motion, `user_data` only with IFB, and world-space transform
  result access only for tag sets that can provide it.

## 16. Sources

- Apple, Metal Shading Language Specification, PDF revision dated 2025-10-23:
  <https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf>
- Apple Developer Documentation, Ray tracing with acceleration structures:
  <https://developer.apple.com/documentation/metal/ray-tracing-with-acceleration-structures>
- Apple Developer Documentation, `MTLAccelerationStructure`:
  <https://developer.apple.com/documentation/metal/mtlaccelerationstructure>
- Apple Developer Documentation, `MTLIntersectionFunctionTable`:
  <https://developer.apple.com/documentation/metal/mtlintersectionfunctiontable>
- Apple Developer Documentation, acceleration-structure geometry and instance
  `intersectionFunctionTableOffset`:
  <https://developer.apple.com/documentation/metal/mtlaccelerationstructuregeometrydescriptor/intersectionfunctiontableoffset>
  and
  <https://developer.apple.com/documentation/metal/mtlaccelerationstructureinstancedescriptor/intersectionfunctiontableoffset>
- Apple Developer, Go further with Metal 4 games, WWDC25:
  <https://developer.apple.com/videos/play/wwdc2025/211/>
- Apple Developer Documentation, `MTLDevice.supportsRaytracing` and
  `supportsRaytracingFromRender`:
  <https://developer.apple.com/documentation/metal/mtldevice/supportsraytracing>
  and
  <https://developer.apple.com/documentation/metal/mtldevice/supportsraytracingfromrender>
- Local SDK check: Metal SDK header `<metal_raytracing>`, Metal
  compiler `32023.883`.
