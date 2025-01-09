SP #015 - `DescriptorHandle<T>` type
==============

## Status

Author: Yong He

Status: In Experiment.

Implementation: [PR 6028](https://github.com/shader-slang/slang/pull/6028)

Reviewed by: Theresa Foley, Jay Kwak

## Background

Textures, sampler states and buffers are typically passed to shader as opaque handles whose size and storage address is undefined. These handles are communicated to the GPU via "bind states" that are modified with host-side APIs. Because the handle has unknown size, it is not possible to read, copy or construct such a handle from the shader code, and it is not possible to store the handle in buffer memory. This makes both host code and shader code difficult to write and prevents more flexible encapsulation or clean object-oriented designs.

With the recent advancement in hardware capabilities, a lot of modern graphics systems are adopting a "bindless" parameter passing idiom, where all resource handles are passed to the shader in a single global array, and all remaining references to texture, buffers or sampler states are represented as a single integer index into the array. This allows the shader code to workaround the restrictions around the opaque handle types.

Direct3D Shader Model 6.6 introduces the "Dynamic Resources" capability, which further simplifies the way to write bindless shader code by removing the need to even declare the global array.

We believe that graphics developers will greatly benefit from a system defined programming model for the bindless parameter passing idom that is versatile and cross-platform, which will provide a consistent interface so that different shader libraries using the bindless pattern can interop with each other without barriers.

## Proposed Approach

We introduce a `DescriptorHandle<T>` type that is defined as:
```
struct DescriptorHandle<T> : IComparable
    where T : IOpaqueDescriptor
{
    [require(hlsl_glsl_spirv)]
    __init(uint2 value); // For HLSL, GLSL and SPIRV targets only.
}
```
Where `IOpaqueDescriptor` is an interface that is implemented by all texture, buffer and sampler state types:

```slang
enum DescriptorKind
{
    Unknown,
    Texture,
    CombinedTextureSampler,
    Buffer,
    Sampler,
    AccelerationStructure,
}
interface IOpaqueDescriptor
{
    static const DescriptorKind kind;
}
```

All builtin types that implements `IOpaqueDescriptor` interface provide a convenience typealias for `DescriptorHandle<T>`. For example, `Texture2D.Handle` is an alias for `DescriptorHandle<Texture2D>`.

### Basic Usage

`DescriptorHandle<T>` should provide the following features:

- `operator *` to deference the pointer and obatin the actual descriptor handle `T`.
- Implicit conversion to `T` when used in a location that expects `T`.
- When targeting HLSL, GLSL and SPIRV, `DescriptorHandle<T>` can be casted to and from a `uint2` value.
- Equality comparison.

For example:

```slang
uniform DescriptorHandle<Texture2D> texture;

// `SamplerState.Handle` is equivalent to `DescriptorHandle<SamplerState>`.
uniform SamplerState.Handle sampler;

void test()
{
    // Explicit cast from bindless handle to an uint2 value.
    // (Available on HLSL, GLSL and SPIRV targets only)
    let idx = (uint2)texture;

    // Constructing bindless handle from uint2 value.
    // (Available on HLSL, GLSL and SPIRV targets only)
    let t = DescriptorHandle<Texture2D>(idx);

    // Comparison.
    ASSERT(t == texture);

    // OK, `t` is first implicitly dereferenced to producee `Texture2D`, and
    // then `Texture2D::Sample` is called.
    // The `sampler` argument is implicitly converted from `DescriptorHandle<SamplerState>`
    // to `SamplerState`.
    t.Sample(sampler, float2(0,0));

    // Alternatively, the following syntax is also allowed, to
    // make `DescriptorHandle` appear more like a pointer:
    t->Sample(*sampler, float2(0, 0));
}
```

A `DescriptorHandle<T>` type has target-dependent size, but it is always a concrete/physical data type and valid in all memory locations. For HLSL and SPIRV targets, it is represented by a two-component vector of 32-bit unsigned integer (`uint2`), and laid out as such. On these targets, builtin conversion functions are provided to construct
a `DescriptorHandle<T>` from a `uint2` value.

On targets where descriptor handles are already concrete and sized types, `DescriptorHandle<T>` simply translates to `T`, and has size and alignment that matches the corresponding native type, which is queryable with Slang's reflection API.

This means that on all targets where `DescriptorHandle<T>` is supported, you can use a `DescriptorHandle<T>` type in any context where an ordinary data type, e.g. `int` type is allowed, such as in buffer elements.

### Obtaining Descriptor from `DescriptorHandle<T>`

Depending on the target platform and the design choices of the user's application, the way to obtain the actual
descriptor handle from a `DescriptorHandle<T>` integer handle can vary. Slang does not dictate how this conversion is done,
and instead, this is left to the user via Slang's link-time specialization ability.

Slang defines the following core module declarations:

```slang
extern T getDescriptorFromHandle(DescriptorHandle<T> handle) where T : IOpaqueDescriptor
{
    // Default Implementation
    return defaultGetDescriptorFromHandle(handle);
}
```

The `getDescriptorFromHandle` is used to convert from a bindless handle to actual opaque resource handle.
If this function is not provided by the user, the default implementation defined in the core module will be used.

By default, the core module implementation of `getDescriptorFromHandle` should use the `ResourceDescriptorHeap` and
`SamplerDescriptorHeap` builtin object when generating HLSL code. When generating code on other targets, `getDescriptorFromHandle`
will fetch the descriptor handle from a system defined global array of the corresponding descriptor type.

If/when SPIRV is extended to expose similar capabilities as D3D's `ResourceDescriptorHeap` feature, we should change the default implementation
to use that instead. Until we know the default implementation of `getDescriptorFromHandle` is stable, we should advise users
to provide their own implementation of `getDescriptorFromHandle` to prevent breakages.

If the user application requires a different bindless implementation, this default behavior can be overrided by defining
`getDescriptorFromHandle` in the user code. Below is a possible user-space implementation of `getDescriptorFromHandle`
for Vulkan:

```slang

// All texture and buffer handles are defined in descriptor set 100.
[vk::binding(0, 100)]
__DynamicResource<__DynamicResourceKind.General> resourceHandles[];

// All sampler handles are defined in descriptor set 101.
[vk::binding(0, 101)]
__DynamicResource<__DynamicResourceKind.Sampler> samplerHandles[];

export T getDescriptorFromHandle<T>(DescriptorHandle<T> handle) where T : IOpaqueDescriptor
{
    if (T.kind == ResourceKind.Sampler)
        return samplerHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
    else
        return resourceHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
}
```

The user can call `defaultGetDescriptorFromHandle` function from their implementation of `getDescriptorFromBindlessHandle` to dispatch to the default behavior.

### Uniformity

By default, the value of a `DescriptorHandle<T>` object is assumed to be dynamically uniform across all
execution threads. If this is not the case, the user is required to mark the `DescriptorHandle` as `nonuniform`
*immediately* before dereferencing it:
```slang
void test(DescriptorHandle<Texture2D> t)
{
    nonuniform(t)->Sample(...);
}
```

If the descriptor handle value is not uniform and `nonuniform` is not called, the result may be
undefined.

### Combind Texture Samplers

On platforms without native support for combined texture samplers, we will use both components of the
underlying `uint2` value: the `x` component stores the bindless handle for the texture, and the `y` component stores the bindless handle for the sampler.

For example, given:

```slang
uniform DescriptorHandle<Sampler2D> s;
void main()
{
    float2 uv = ...;
    s.SampleLevel(uv, 0.0);
}
```

The Slang compiler should emit HLSL as follows:

```hlsl
uniform uint2 s;
void main()
{
    float2 uv = ...;
    Texture2D(ResourceDescriptorHeap[s.x]).SampleLevel(
        SamplerState(SamplerDescriptorHeap[s.y]),
        uv,
        0.0);
}
```

## Alternatives Considered

We initially considered to support a more general `DescriptorHandle<T>` where `T` can be any composite type, for example, allowing the following:

```slang
struct Foo
{
    Texture2D t;
    SamplerState s;
    float ordinaryData;
}

uniform DescriptorHandle<Foo> foo;
```

which is equivalent to:

```slang
struct Bindless_Foo
{
    DescriptorHandle<Texture2D> t;
    DescriptorHandle<SamplerState> s;
    float s;
}
uniform Bindless_Foo foo;
```

While relaxing `T` this way adds an extra layer of convenience, it introduces complicated
semantic rules to the type system, and there is increased chance of exposing tricky corner
cases that are hard to get right.

An argument for allowing `T` to be general composite types is that it enables sharing the same
code for both bindless systems and bindful systems. But this argument can also be countered by
allowing the compiler to treat `DescriptorHandle<T>` as `T` in a special mode if this feature is found to be useful.

For now we think that restricting `T` to be an `IOpaqueDescriptor` type will result in a much simpler implementation, and is likely sufficient for current needs. Given that the trend of modern GPU architecture is moving towards bindless idioms and the whole idea of opaque handles may disappear in the future, we should be cautious at inventing too many heavy weight mechanisms around opaque handles. Nevertheless, this proposal still allows us to relax this requirement in the future if it becomes clear that such feature is valuable to our users.

In the initial version of this propsoal, `DescriptorHandle<T>` is named `Bindless<T>`. During discussion, we determined that this naming can be confusing to users who are coming from general GPU compute community and haven't heard of the term "bindless resources". We believe `DescriptorHandle<T>` is a better name because it reflects the essense of the type more accurately, and is consistent with D3D12 terminology in that `DescriptorHandle<T>` is the shader side representation of the `D3D12_GPU_DESCRIPTOR_HANDLE` structure.

The initial version of the proposal defines `DescriptorHandle<T>` to be backed by an 8-byte integer value independent of the target. This is changed
so that Slang only guarantees `DescriptorHandle<T>` to be a phyiscal data type, and will have target-dependent size. Slang guarantees that `DescriptorHandle<T>`
will be lowered to a `uint2` value when targeting HLSL, GLSL and SPIRV, but not on other targets. This is because on targets where `T` is already a
phyisical type, their size can vary and may not fit in an 8-byte structure. For example, `StructuredBuffer<T>` maps to a `{T*, size_t}` structure when
targeting CUDA, which takes 16 bytes. In the meanwhile, forcing `DescriptorHandle<T>` to be `uint64_t` makes the feature unusable for lower-tier hardware
where 64-bit integers are not supported. Representing the handle with `uint2` allows the feature to be used without requiring this additional
capability.

The initial proposal also reserves a value for invalid/null handle. This is removed because we cannot find
a safe value that won't be used across all targets we support. In particular, this is not possible on CUDA
and Metal because it is not possible to interpret these handles as plain integers. 

## Conclusion

This proposal introduces a standard way to achieve bindless parameter passing idom on current graphics platforms.
Standardizing the way of writing bindless parameter binding code is essential for creating reusable shader code
libraries. The convenience language features around `DescriptorHandle<T>` type should also make shader code easier to write
and to maintain. Finally, by using Slang's link time specialization feature,
this proposal allows Slang to not get into the way of dicatating one specific way of passing
the actual descriptor handles to the shader code, and allows the user to customize how the conversion from integer handle
to descriptor handle is done in a way that best suites the application's design.