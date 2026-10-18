// An unused `export`ed generic stays in the linked module after every use has its own specialized
// copy. When its body contains `MakeCombinedTextureSamplerFromHandle`, SPIR-V legalization must not
// see the unspecialized body: there the op's result type is the generic parameter `T`, not a
// combined texture-sampler type. See shader-slang/slang#13744.
//
// - `-DDIRECT`: a `getDescriptorFromHandle` override calls the intrinsic; the output shows the
//   override is honored.
// - `-DDIRECT -g2`: debug information must not keep the unused generic alive.
// - `-DHELPER`: the override forwards to a non-exported generic helper that only the unused
//   override references, so both must be removed.
// - `-DNAME`: an exported generic with any other name crashes the same way; the entry point uses
//   the default `getDescriptorFromHandle`.
// - `-DHEAP`: under `spvDescriptorHeapEXT` the override forwards to `defaultGetDescriptorFromHandle`,
//   whose `T.kind` combined-sampler branch cannot fold in the unspecialized body. The entry point
//   loads a `Texture2D`, so the op comes only from the generic body.

//TEST:SIMPLE(filecheck=COMBINED): -target spirv-asm -stage fragment -entry main -DDIRECT
//TEST:SIMPLE(filecheck=COMBINED): -target spirv-asm -stage fragment -entry main -DDIRECT -g2
//TEST:SIMPLE(filecheck=COMBINED): -target spirv-asm -stage fragment -entry main -DHELPER
//TEST:SIMPLE(filecheck=NAME): -target spirv-asm -stage fragment -entry main -DNAME
//TEST:SIMPLE(filecheck=HEAP): -target spirv-asm -stage fragment -entry main -capability spvDescriptorHeapEXT -DHEAP

#if defined(HELPER)
T makeCombined<T : IOpaqueDescriptor>(DescriptorHandle<T> handle)
{
    return __makeCombinedTextureSamplerFromHandle<T>((uint2)handle);
}
#endif

#if defined(NAME)
export T makeDescriptor<T : IOpaqueDescriptor>(DescriptorHandle<T> handle)
{
    return __makeCombinedTextureSamplerFromHandle<T>((uint2)handle);
}
#else
export T getDescriptorFromHandle<T : IOpaqueDescriptor>(DescriptorHandle<T> handle)
{
#if defined(DIRECT)
    return __makeCombinedTextureSamplerFromHandle<T>((uint2)handle);
#elif defined(HELPER)
    return makeCombined(handle);
#else
    return defaultGetDescriptorFromHandle(handle);
#endif
}
#endif

#if defined(HEAP)
[[vk::push_constant]] Texture2D<float4>.Handle tex;
#else
[[vk::push_constant]] Sampler2D<float4>.Handle tex;
#endif

[shader("fragment")]
float4 main(float2 uv : TEXCOORD0) : SV_Target
{
#if defined(HEAP)
    return tex.Load(int3(0, 0, 0));
#else
    return tex.Sample(uv);
#endif
}

// The override is honored: the texture comes from the resource heap at `handle.x`, the sampler from
// the sampler heap at `handle.y`, and the two are combined with OpSampledImage.
// COMBINED: OpEntryPoint Fragment %main
// COMBINED-DAG: OpDecorate %slang_resourceHeap BuiltIn ResourceHeapEXT
// COMBINED-DAG: OpDecorate %slang_samplerHeap BuiltIn SamplerHeapEXT
// COMBINED: %[[TEX_INDEX:[A-Za-z0-9_]+]] = OpCompositeExtract %uint %[[HANDLE:[A-Za-z0-9_]+]] 0
// COMBINED: %[[TEX_PTR:[A-Za-z0-9_]+]] = OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap %[[TEX_INDEX]]
// COMBINED: %[[TEX:[A-Za-z0-9_]+]] = OpLoad {{.*}} %[[TEX_PTR]]
// COMBINED: %[[SAMP_INDEX:[A-Za-z0-9_]+]] = OpCompositeExtract %uint %[[HANDLE]] 1
// COMBINED: %[[SAMP_PTR:[A-Za-z0-9_]+]] = OpUntypedAccessChainKHR {{.*}} %slang_samplerHeap %[[SAMP_INDEX]]
// COMBINED: %[[SAMP:[A-Za-z0-9_]+]] = OpLoad {{.*}} %[[SAMP_PTR]]
// COMBINED: %[[SAMPLED:[A-Za-z0-9_]+]] = OpSampledImage {{.*}} %[[TEX]] %[[SAMP]]
// COMBINED: OpImageSampleImplicitLod {{.*}} %[[SAMPLED]]

// NAME: OpEntryPoint Fragment %main
// NAME: OpImageSampleImplicitLod
// NAME-NOT: makeDescriptor

// HEAP: OpEntryPoint Fragment %main
// HEAP: OpDecorate %slang_resourceHeap BuiltIn ResourceHeapEXT
// HEAP: OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap
// HEAP: OpImageFetch
