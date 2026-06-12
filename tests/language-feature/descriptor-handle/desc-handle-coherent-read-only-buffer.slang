//TEST(compute, vulkan):SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -capability spvDescriptorHeapEXT -capability vk_mem_model
//TEST(compute):SIMPLE(filecheck=HLSL): -target hlsl -profile cs_6_6 -entry computeMain
//TEST(compute):SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute

// SPIRV: OpLoad {{.*}} MakePointerVisible|NonPrivatePointer
// SPIRV-NOT: MakePointerVisible|NonPrivatePointer

// HLSL-COUNT-1: globallycoherent StructuredBuffer<uint >  _S
// HLSL-NOT: globallycoherent
// GLSL emits one resource-heap SSBO declaration for the coherent read-only handle.
// GLSL-COUNT-1: coherent readonly buffer
// GLSL-NOT: coherent

uniform DescriptorHandle<coherent StructuredBuffer<uint>> coherentReadOnlyBuffer;
uniform DescriptorHandle<StructuredBuffer<uint>> plainReadOnlyBuffer;
uniform DescriptorHandle<RWStructuredBuffer<uint>> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    (*outputBuffer)[index] =
        (*coherentReadOnlyBuffer)[index] + (*plainReadOnlyBuffer)[index + 1];
}
