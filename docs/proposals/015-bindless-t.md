SP #015 - `Bindless<T>` type
==============

## Status

Author: Yong He

Status: Design review.

Implementation:

Reviewed by:

## Background

Textures, sampler states and buffers are typically passed to shader as opaque handles whose size and storage address is undefined. These handles are communicated to the GPU via "bind states" that are modified with host-side APIs. Because the handle has unknown size, it is not possible to read, copy or construct such a handle from the shader code, and it is not possible to store the handle in buffer memory. This makes both host code and shader code difficult to write and prevents more flexible encapsulation or clean object-oriented designs.

With the recent advancement in hardware capabilities, a lot of modern graphics systems are adopting a "bindless" parameter passing idiom, where all resource handles are passed to the shader in a single global array, and all remaining references to texture, buffers or sampler states are represented as a single integer index into the array. This allows the shader code to workaround the restrictions around the opaque handle types.

Direct3D Shader Model 6.6 introduces the "Dynamic Resources" capability, which further simplifies the way to write bindless shader code by removing the need to even declare the global array.

We believe that graphics developers will greatly benefit from a system defined programming model for the bindless parameter passing idom that is versatile and cross-platform, which will provide a consistent interface so that different shader libraries using the bindless pattern can interop with each other without barriers.

## Proposed Approach

We introduce a `Bindless<T>` type that is defined as:
```
struct Bindless<T> : IComparable<Bindless<T>>
    where T : IOpaqueHandle
{
    __init(uint64_t value);
}
```
Where `IOpaqueHandle` is an interface that is implemented by all texture, buffer and sampler state types:

```slang
enum ResourceKind
{
    Unknown, Texture, ConstantBuffer, StorageBuffer, Sampler
}
interface IOpaqueHandle
{
    static ResourceKind getResourceKind();
}
```

### Basic Usage

`Bindless<T>` should provide the following features:

- Construction/explicit cast from a 64-bit integer.
- Explicit cast to a 64-bit integer.
- Equality comparison.
- Implicit dereference to `T`.
- Implicit conversion to `T`.

For example:

```slang
uniform Bindless<Texture2D> texture;
uniform Bindless<SamplerState> sampler;

void test()
{
    // Explicit cast from bindless handle to uint64_t value.
    let idx = (uint64_t)texture;

    // Constructing bindless handle from uint64_t value.
    let t = Bindless<Texture2D>(idx);

    // Comparison.
    ASSERT(t == texture);

    // OK, `t` is first implicitly dereferenced to producee `Texture2D`, and
    // then `Texture2D::Sample` is called.
    // The `sampler` argument is implicitly converted from `Bindless<SamplerState>`
    // to `SamplerState`.
    t.Sample(sampler, float2(0,0));
}
```

A `Bindless<T>` type is a concrete type whose size is always 8 bytes and is internally a 64-bit integer.
This means that you can use a `Bindless<T>` type in any context where an ordinary data type, e.g. `int` type
is allowed, such as in buffer elements.

On targets where resource handles are already concrete and sized types, `Bindless<T>` simply translates to just `T`.
If `T` has native size or alignment that is less than 8 bytes, it will be rounded up to 8 bytes. If the native size for
`T` is greater than 8 bytes, it will be treated as an opaque type instead of translating to `T`.

### Obtaining Actual Resource Handle from `Bindless<T>`

Depending on the target platform and the design choices of the user's application, the way to obtain the actual
resource handle from a `Bindless<T>` integer handle can vary. Slang does not dictate how this conversion is done,
and instead, this is left to the user via Slang's link-time specialization ability.

Slang defines the following core module declarations:

```slang
extern T getResourceFromBindlessHandle(uint64_t handle) where T : IOpaqueHandle
{
    // Default Implementation
    // ...
}
```

The `getResourceFromBindlessHandle` is used to convert from a bindless handle to actual opaque resource handle.
If this function is not provided by the user, the default implementation defined in the core module will be used.

By default, the core module implementation of `getResourceFromBindlessHandle` should use the `ResourceDescriptorHeap` and
`SamplerDescriptorHeap` builtin object when generating HLSL code. When generating code on other targets, `getResourceFromBindlessHandle`
will fetch the resource handle from a system defined global array of the corresponding resource type.

If/when SPIRV is extended to expose similar capabilities as D3D's `ResourceDescriptorHeap` feature, we should change the default implementation
to use that instead. Until we know the default implementation of `getResourceFromBindlessHandle` is stable, we should advise users
to provide their own implementation of `getResourceFromBindlessHandle` to prevent breakages.

If the user application requires a different bindless implementation, this default behavior can be overrided by defining
`getResourceFromBindlessHandle` in the user code. Below is a possible user-space implementation of `getResourceFromBindlessHandle`
for Vulkan:

```slang

// All texture and buffer handles are defined in descriptor set 100.
[vk::binding(0, 100)]
__DynamicResource<__DynamicResourceKind.General> resourceHandles[];

// All sampler handles are defined in descriptor set 101.
[vk::binding(0, 101)]
__DynamicResource<__DynamicResourceKind.Sampler> samplerHandles[];

export getResourceFromBindlessHandle<T>(uint64_t handle) where T : IOpaqueHandle
{
    if (T.getResourceKind() == ResourceKind.Sampler)
        return (T)samplerHandles[handle];
    else
        return (T)resourceHandles[handle];
}
```

### Invalid Handle

We reserve `uint64_t.maxValue` as a special handle value of `Bindless<T>` types to mean an invalid/null resource.
This will allow us to optimize `Optional<Bindless<Texture2D>>` to use the reserved value to mean no-value.

The user should also be able to use `Bindless<T>.invalid` to refer to such an invalid value:

```slang
struct Bindless<T> where T:IOpaqueHandle
{
    static const Bindless<T> invalid = Bindless<T>(uint64_t.maxValue);
}
```

## Conclusion

This proposal introduces a standard way to achieve bindless parameter passing idom on current graphics platforms.
Standardizing the way of writing bindless parameter binding code is essential for creating reusable shader code
libraries. The convenience language features around `Bindless<T>` type should also make shader code easier to write
and to maintain. Finally, by using Slang's link time specialization feature,
this proposal allows Slang to not get into the way of dicatating one specific way of passing
the actual resource handles to the shader code, and allows the user to customize how the conversion from integer handle
to resource handle is done in a way that best suites the application's design.