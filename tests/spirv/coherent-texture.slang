//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=VK_MEM_MODEL): -target spirv-asm -entry computeMain -stage compute -capability vk_mem_model

//SPIRV: OpDecorate {{.*}} Coherent

//VK_MEM_MODEL-NOT: OpDecorate {{.*}} Coherent
//VK_MEM_MODEL: OpImageRead %{{.*}} MakeTexelVisible|NonPrivateTexel %
//VK_MEM_MODEL: OpImageWrite %{{.*}} MakeTexelAvailable|NonPrivateTexel %
//VK_MEM_MODEL: OpImageRead %{{.*}} MakeTexelVisible|NonPrivateTexel %
//VK_MEM_MODEL: OpImageWrite %{{.*}} MakeTexelAvailable|NonPrivateTexel %

globallycoherent
RWTexture2D<int4> outputTexture2D;

[numthreads(1,1,1)]
void computeMain()
{
    // This line issues kIROp_ImageLoad/Store
    outputTexture2D[0].xz = outputTexture2D[0].yw;

    // The current implementation uses spirv_asm for subscription,
    // which goes through a different part of implementation in slang-emit-spirv.cpp.
    outputTexture2D[uint2(0,0)] = outputTexture2D[uint2(1,1)];
}
