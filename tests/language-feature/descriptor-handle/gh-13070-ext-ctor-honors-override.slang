// The untyped-handle→resource constructors bypass the user-overridable `getDescriptorFromHandle` in
// exactly one configuration: a native bindless texture/sampler type when BOTH `spvBindlessTextureNV`
// and `spvDescriptorHeapEXT` are enabled (its explicit index would otherwise be reinterpreted as a
// native `uint64` handle — #13070). Under `spvDescriptorHeapEXT` alone the bypass must NOT fire, even
// for a native type: the explicit index has no native-handle hazard without NV, and the user's
// `getDescriptorFromHandle` override must still apply. See shader-slang/slang#13070.
//
// This test uses a native `Texture2D` (an NV-encodable type) obtained from `ResourceDescriptorHeap[i]`
// under EXT-only, and makes "was the override honored?" observable by forcing the plain
// dynamic-resource-heap path instead of the EXT heap. If the constructor wrongly bypassed the override,
// the descriptor would be fetched by an untyped access chain into the EXT `%slang_resourceHeap`
// builtin, which this test asserts is absent.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -capability spvDescriptorHeapEXT -stage compute -entry computeMain

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

// A valid module was produced, and the override was honored: the descriptor is NOT fetched from the EXT
// descriptor heap (no untyped access chain into the ResourceHeapEXT builtin). The `-NOT` runs from the
// entry point to end-of-module.
//CHECK: OpEntryPoint GLCompute
//CHECK-NOT: OpUntypedAccessChainKHR {{.*}}%slang_resourceHeap
//CHECK-NOT: BuiltIn ResourceHeapEXT
