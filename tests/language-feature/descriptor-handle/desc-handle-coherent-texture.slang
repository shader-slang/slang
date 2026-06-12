//TEST(compute, vulkan):SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -capability spvDescriptorHeapEXT -capability vk_mem_model
//TEST(compute):SIMPLE(filecheck=HLSL): -target hlsl -profile cs_6_6 -entry computeMain
//TEST(compute):SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute

// SPIRV: OpImageRead {{.*}} MakeTexelVisible|NonPrivateTexel
// SPIRV-NEXT: OpImageWrite {{.*}} MakeTexelAvailable|NonPrivateTexel
// SPIRV: OpImageRead {{.*}} MakeTexelVisible|NonPrivateTexel
// SPIRV-NEXT: OpImageWrite {{.*}} MakeTexelAvailable|NonPrivateTexel
// SPIRV-NOT: MakeTexelVisible|NonPrivateTexel
// SPIRV-NOT: MakeTexelAvailable|NonPrivateTexel

// HLSL-COUNT-4: globallycoherent RWTexture2D<float4 >  _S
// HLSL-NOT: globallycoherent
// GLSL emits one resource-heap image declaration shared by both coherent handles.
// GLSL-COUNT-1: coherent
// GLSL-NOT: coherent

uniform DescriptorHandle<coherent RWTexture2D<float4>> coherentTexture;
uniform DescriptorHandle<globallycoherent RWTexture2D<float4>> globallyCoherentTexture;
uniform DescriptorHandle<RWTexture2D<float4>> plainTexture;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 coord = dispatchThreadID.xy;
    (*coherentTexture)[coord] = (*globallyCoherentTexture)[coord];
    (*globallyCoherentTexture)[coord + uint2(1, 0)] = (*coherentTexture)[coord + uint2(1, 0)];
    (*plainTexture)[coord + uint2(2, 0)] = (*plainTexture)[coord + uint2(3, 0)];
}
