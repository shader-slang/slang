//TEST:COMPARE_HLSL:-profile cs_5_0
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm
//TEST:SIMPLE(filecheck=VK_MEM_MODEL): -target spirv-asm -emit-spirv-directly -capability vk_mem_model

// Check output for `globallycoherent`

//SPIRV: OpDecorate {{.*}} Coherent

//VK_MEM_MODEL-NOT: OpDecorate {{.*}} Coherent
//VK_MEM_MODEL: OpLoad {{.*}} MakePointerVisible|NonPrivatePointer
//VK_MEM_MODEL: OpStore {{.*}} MakePointerAvailable|NonPrivatePointer

#ifndef __SLANG__
#define gBuffer gBuffer_0
#define SV_DispatchThreadID SV_DISPATCHTHREADID
#endif

globallycoherent
RWStructuredBuffer<uint> gBuffer : register(u0);

[numthreads(16,1,1)]
void main(
	uint tid : SV_DispatchThreadID)
{
	uint index = tid;

    gBuffer[index] = gBuffer[index + 1];
}
