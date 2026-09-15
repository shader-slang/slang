//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability spvBindlessTextureNV -stage compute -entry main

// An explicit `sizeof`/`alignof` layout query on a `DescriptorHandle` must report the layout of the
// type's SPIR-V representation under the *requested* layout rule. Under `spvBindlessTextureNV` a
// buffer/acceleration-structure handle is a `uint2` (std430/std140 alignment 8, natural alignment 4)
// and a texture/sampler handle is a `uint64` (8 under every rule), so each query below must agree
// with the same query on the handle's underlying type.

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    static_assert(
        alignof(DescriptorHandle<RWStructuredBuffer<float> >, Std430DataLayout) ==
            alignof(uint2, Std430DataLayout),
        "A uint2 descriptor handle must use uint2's std430 alignment.");
    static_assert(
        alignof(DescriptorHandle<RaytracingAccelerationStructure>, Std140DataLayout) ==
            alignof(uint2, Std140DataLayout),
        "A uint2 descriptor handle must use uint2's std140 alignment.");
    static_assert(
        alignof(DescriptorHandle<ConstantBuffer<float> >) == alignof(uint2),
        "A uint2 descriptor handle must use uint2's natural alignment.");

    static_assert(
        alignof(DescriptorHandle<Texture2D<float4> >, Std430DataLayout) ==
            alignof(uint64_t, Std430DataLayout),
        "A uint64 descriptor handle must use uint64's std430 alignment.");

    static_assert(
        sizeof(DescriptorHandle<RWStructuredBuffer<float> >, Std430DataLayout) ==
            sizeof(uint2, Std430DataLayout),
        "A uint2 descriptor handle must use uint2's std430 size.");
}

//CHECK: OpEntryPoint
