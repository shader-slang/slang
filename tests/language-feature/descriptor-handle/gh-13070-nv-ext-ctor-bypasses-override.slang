// Companion to gh-13070-ext-ctor-honors-override.slang, covering the opposite (intentional) case. In the
// one configuration where both `spvBindlessTextureNV` and `spvDescriptorHeapEXT` are enabled, an encodable
// texture/sampler obtained from an explicit `ResourceDescriptorHeap[i]` index is loaded from the EXT heap
// *directly*, bypassing the user-overridable `getDescriptorFromHandle`. This is deliberate (#13070):
// routing the explicit index through the `(type, capability)`-keyed path would let the NV dispatch
// reinterpret it as a native `uint64` handle. So — unlike the EXT-only case, where the override is honored
// — here the override must NOT run for a native type.
//
// The override below forces the plain dynamic-resource-heap path. If it were (wrongly) honored, the
// descriptor would be fetched from `__getDynamicResourceHeap` and there would be no EXT heap access. This
// test asserts the opposite: the descriptor IS fetched via an untyped access chain into the EXT
// `%slang_resourceHeap` builtin, proving the bypass ran and the override was skipped.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -capability spvBindlessTextureNV+spvDescriptorHeapEXT -stage compute -entry computeMain

export T getDescriptorFromHandle<T : IOpaqueDescriptor>(DescriptorHandle<T> handleValue)
{
    return getDescriptorFromDynamicResourceHeap<T>(handleValue, BindlessDescriptorOptions.VkMutable);
}

[[vk::binding(0)]] RWStructuredBuffer<float> outputBuffer;
uniform uint gIndex;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Texture2D<float> t = ResourceDescriptorHeap[gIndex];
    outputBuffer[0] = t.Load(int3(0, 0, 0));
}

// The bypass ran: the descriptor is loaded from the EXT resource heap (an untyped access chain into the
// `ResourceHeapEXT`-decorated builtin), not from the override's dynamic-resource-heap path.
//CHECK-DAG: OpDecorate %slang_resourceHeap BuiltIn ResourceHeapEXT
//CHECK-DAG: OpUntypedAccessChainKHR {{.*}}%slang_resourceHeap
