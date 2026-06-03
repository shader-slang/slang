//TEST(compute, vulkan):SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -capability spvDescriptorHeapEXT -capability vk_mem_model
//TEST(compute):SIMPLE(filecheck=HLSL): -target hlsl -profile cs_6_6 -entry computeMain

// SPIRV: OpLoad {{.*}} MakePointerVisible|NonPrivatePointer
// SPIRV: OpStore {{.*}} MakePointerAvailable|NonPrivatePointer

// HLSL-COUNT-2: globallycoherent RWStructuredBuffer<uint

uniform DescriptorHandle<coherent RWStructuredBuffer<uint>> coherentBuffer;
uniform DescriptorHandle<globallycoherent RWStructuredBuffer<uint>> globallyCoherentBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    (*coherentBuffer)[index] = (*globallyCoherentBuffer)[index];
}
