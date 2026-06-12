//TEST(compute, vulkan):SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -capability spvDescriptorHeapEXT -capability vk_mem_model
//TEST(compute):SIMPLE(filecheck=HLSL): -target hlsl -profile cs_6_6 -entry computeMain
//TEST(compute):SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute

// SPIRV: OpLoad {{.*}} MakePointerVisible|NonPrivatePointer
// SPIRV-NEXT: OpStore {{.*}} MakePointerAvailable|NonPrivatePointer
// SPIRV: OpLoad {{.*}} MakePointerVisible|NonPrivatePointer
// SPIRV-NEXT: OpStore {{.*}} MakePointerAvailable|NonPrivatePointer
// SPIRV-NOT: MakePointerVisible|NonPrivatePointer
// SPIRV-NOT: MakePointerAvailable|NonPrivatePointer

// HLSL-COUNT-4: globallycoherent RWStructuredBuffer<uint >  _S
// HLSL-NOT: globallycoherent
// GLSL emits one resource-heap SSBO declaration shared by both coherent handles.
// GLSL-COUNT-1: coherent buffer
// GLSL-NOT: coherent

uniform DescriptorHandle<coherent RWStructuredBuffer<uint>> coherentBuffer;
uniform DescriptorHandle<globallycoherent RWStructuredBuffer<uint>> globallyCoherentBuffer;
uniform DescriptorHandle<RWStructuredBuffer<uint>> plainBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    (*coherentBuffer)[index] = (*globallyCoherentBuffer)[index];
    (*globallyCoherentBuffer)[index + 1] = (*coherentBuffer)[index + 1];
    (*plainBuffer)[index + 2] = (*plainBuffer)[index + 3];
}
