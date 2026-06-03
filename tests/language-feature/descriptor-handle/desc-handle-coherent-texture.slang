//TEST(compute, vulkan):SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -capability spvDescriptorHeapEXT -capability vk_mem_model
//TEST(compute):SIMPLE(filecheck=HLSL): -target hlsl -profile cs_6_6 -entry computeMain

// SPIRV: OpImageRead %{{.*}} {{.*}} MakeTexelVisible|NonPrivateTexel %
// SPIRV: OpImageWrite {{.*}} {{.*}} {{.*}} MakeTexelAvailable|NonPrivateTexel %

// HLSL-COUNT-2: globallycoherent RWTexture2D<float4

uniform DescriptorHandle<coherent RWTexture2D<float4>> coherentTexture;
uniform DescriptorHandle<globallycoherent RWTexture2D<float4>> globallyCoherentTexture;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 coord = dispatchThreadID.xy;
    (*coherentTexture)[coord] = (*globallyCoherentTexture)[coord];
}
